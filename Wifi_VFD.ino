/* 
 * @Author       : Chr_
 * @Date         : 2021-04-25 22:57:45
 * @LastEditors  : Chr_
 * @LastEditTime : 2021-12-15 14:59:46
 * @Description  : WiFi VFD
 */

#include <Arduino.h>
#include <Ticker.h>
#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <ESP8266WebServer.h>
#include <FS.h>

#include "web_serv.h"
#include "vfd.h"
#include "static.h"
#include "mode.h"
#include "wifi.h"

Ticker ticker;                                   // 伪多线程库
WiFiUDP ntpUDP;                                  // udp包发送器
NTPClient timeClient(ntpUDP, NTPSERV, 8 * 3600); // ntp客户端
ESP8266WebServer web_serv(80);                   // web服务端

u_char G_mode = 0x01;           // 当前模式
u_char G_screen_setting = 0x80; // 文本滚动模式*2,LOGO当做进度条,不显示数值,不显示标签,12小时,显示上下午,不显示星期
u_int G_screen_icon = 0x0000;   // 点亮logo
u_char G_screen_bright = 255;   //屏幕亮度
u_char G_screen_percent = 0;    //百分比

String G_screen_disp = "DEVELOPED BY CHR_ @ 2021 CHRXW.COM"; //屏显内容

String G_conn_passwd = DEF_CONN_PASSWD; //连接密码

String G_wifi_ssid = DEF_WIFI_SSID;     //wifi SSID
String G_wifi_passwd = DEF_WIFI_PASSWD; //wifi 密码

String G_ap_ssid = DEF_AP_SSID;     //AP SSID
String G_ap_passwd = DEF_AP_PASSWD; //AP 密码

// 初始化
void setup()
{
    init_serial();
    init_load_cfg();
    init_vfd();
    init_webserver();
    init_network();
    init_ntp();

    switch_mode(G_mode);
}

// 循环
void loop()
{
    web_serv.handleClient();
}

// ==============================================================
// 初始化串口
void init_serial()
{
    delay(1000);
    Serial.begin(115200);
    Serial.println("\n串口启动");
    Serial.printf("编译时间 %s %s\n", __DATE__, __TIME__);
}

// 初始化VFD
void init_vfd()
{
    Serial.println("VFD初始化");
    VFD_init();
    VFD_set_brightness(G_screen_bright);
    VFD_all_on();
    delay(1000);
    VFD_all_off();
    String str = " Ver ";
    str += VERSION;
    VFD_display_string(0, str);
    VFD_show();
    delay(1000);
}

// 读取设置
void init_load_cfg()
{
    String tmp = "";
    int s = 0;

    // SPIFFS.format();

    if (!SPIFFS.begin())
    {
        Serial.println("文件系统初始化失败");
        return;
    }

    Serial.println("文件系统初始化完成");
    File dataFile;

    if (SPIFFS.exists(AUTH_PATH)) // 读取conn_passwd
    {
        dataFile = SPIFFS.open(AUTH_PATH, "r");
        G_conn_passwd = dataFile.readString();
        dataFile.close();
        Serial.printf("读取%s 【%s】\n", AUTH_PATH, G_conn_passwd.c_str());
    }
    else
    {
        Serial.printf("%s不存在,写入默认值 【%s】\n", AUTH_PATH, G_conn_passwd.c_str());
        init_save_cfg(AUTH_PATH);
    }

    if (SPIFFS.exists(WIFI_PATH)) // 读取wifi_ssid wifi_passwd
    {
        dataFile = SPIFFS.open(WIFI_PATH, "r");
        tmp = dataFile.readString();
        dataFile.close();
        s = tmp.indexOf("\n");

        if (s != -1)
        {
            G_wifi_ssid = tmp.substring(0, s);
            G_wifi_passwd = tmp.substring(s + 1);
            Serial.printf("读取%s 【%s | %s】\n", WIFI_PATH,
                          G_wifi_ssid.c_str(), G_wifi_passwd.c_str());
        }
        else
        {
            SPIFFS.remove(WIFI_PATH);
            Serial.printf("读取%s失败，删除配置文件 【%s | %s】\n", WIFI_PATH,
                          G_wifi_ssid.c_str(), G_wifi_passwd.c_str());
        }
    }
    else
    {
        Serial.printf("%s不存在,写入默认值 【%s | %s】\n", WIFI_PATH,
                      G_wifi_ssid.c_str(), G_wifi_passwd.c_str());
        init_save_cfg(WIFI_PATH);
    }

    // if (SPIFFS.exists(AP_PATH)) // 读取ap_ssid ap_passwd
    // {
    //     dataFile = SPIFFS.open(AP_PATH, "r");
    //     tmp = dataFile.readString();
    //     dataFile.close();
    //     s = tmp.indexOf("\n");

    //     if (s != -1)
    //     {
    //         G_ap_ssid = tmp.substring(0, s);
    //         G_ap_passwd = tmp.substring(s + 1);
    //         Serial.printf("读取%s 【%s | %s】\n", AP_PATH,
    //                       G_ap_ssid.c_str(), G_ap_passwd.c_str());
    //     }
    //     else
    //     {
    //         SPIFFS.remove(AP_PATH);
    //         Serial.printf("读取%s失败，删除配置文件 【%s | %s】\n", AP_PATH,
    //                       G_ap_ssid.c_str(), G_ap_passwd.c_str());
    //     }
    // }
    // else
    // {
    //     Serial.printf("文件%s不存在,写入默认值 【%s | %s】\n", AP_PATH,
    //                   G_ap_ssid.c_str(), G_ap_passwd.c_str());
    //     init_save_cfg(AP_PATH);
    // }

    if (SPIFFS.exists(CONFIG_PATH)) // 读取mode screen_setting
    {
        dataFile = SPIFFS.open(CONFIG_PATH, "r");
        tmp = dataFile.readString();
        dataFile.close();
        s = tmp.indexOf("\n");

        if (s != -1)
        {
            G_mode = (u_char)tmp.substring(0, s).toInt();
            G_screen_setting = (u_char)tmp.substring(s + 1).toInt();
            Serial.printf("读取%s 【%d | %d】\n", CONFIG_PATH,
                          G_mode, G_screen_setting);
        }
        else
        {
            SPIFFS.remove(CONFIG_PATH);
            Serial.printf("读取%s失败，删除配置文件 【%d | %d】\n", CONFIG_PATH,
                          G_mode, G_screen_setting);
        }
    }
    else
    {
        Serial.printf("文件%s不存在,写入默认值 【%d | %d】\n", CONFIG_PATH,
                      G_mode, G_screen_setting);
        init_save_cfg(CONFIG_PATH);
    }

    if (SPIFFS.exists(LOGO_PATH)) // 读取screen_logo
    {
        dataFile = SPIFFS.open(LOGO_PATH, "r");
        G_screen_icon = dataFile.readString().toInt();
        dataFile.close();

        Serial.printf("读取%s 【%d】\n", LOGO_PATH, G_screen_icon);
    }
    else
    {
        Serial.printf("文件%s不存在,写入默认值 【%d】\n", LOGO_PATH, G_screen_icon);
        init_save_cfg(LOGO_PATH);
    }

    if (SPIFFS.exists(BRIGHT_PATH)) // 读取bright
    {
        dataFile = SPIFFS.open(BRIGHT_PATH, "r");
        int bright = dataFile.readString().toInt();
        dataFile.close();

        if (bright > 0 && bright < 256)
        {
            G_screen_bright = (u_char)bright;
            Serial.printf("读取%s 【%d】\n", BRIGHT_PATH, G_screen_bright);
        }
        else
        {
            SPIFFS.remove(BRIGHT_PATH);
            Serial.printf("读取%s失败，删除配置文件 【%d】\n", BRIGHT_PATH, G_screen_bright);
        }
    }
    else
    {
        Serial.printf("文件%s不存在,写入默认值 【%d】\n", BRIGHT_PATH, G_screen_bright);
        init_save_cfg(BRIGHT_PATH);
    }

    if (SPIFFS.exists(DISP_PATH)) // 读取screen_disp
    {
        dataFile = SPIFFS.open(DISP_PATH, "r");
        G_screen_disp = dataFile.readString();
        dataFile.close();
        Serial.printf("读取%s 【%s】\n", DISP_PATH, G_screen_disp.c_str());
    }
    else
    {
        Serial.printf("%s不存在,写入默认值 【%s】\n", DISP_PATH, G_screen_disp.c_str());
        init_save_cfg(DISP_PATH);
    }

    if (SPIFFS.exists(PROC_PATH)) // 读取screen_disp
    {
        dataFile = SPIFFS.open(PROC_PATH, "r");
        G_screen_percent = (u_char)dataFile.readString().toInt();
        dataFile.close();
        Serial.printf("读取%s 【%d】\n", PROC_PATH, G_screen_percent);
    }
    else
    {
        Serial.printf("%s不存在,写入默认值 【%d】\n", PROC_PATH, G_screen_percent);
        init_save_cfg(PROC_PATH);
    }
}

// 保存设置
void init_save_cfg(char *file_name)
{
    File dataFile;
    if (file_name == AUTH_PATH) // 保存conn_passwd
    {
        Serial.printf("写入%s 【%s】\n", AUTH_PATH, G_conn_passwd.c_str());
        dataFile = SPIFFS.open(AUTH_PATH, "w");
        dataFile.printf("%s", G_conn_passwd.c_str());
        dataFile.close();
    }
    else if (file_name == WIFI_PATH) // 保存wifi_ssid wifi_passwd
    {
        Serial.printf("写入%s 【%s | %s】\n", WIFI_PATH,
                      G_wifi_ssid.c_str(), G_wifi_passwd.c_str());
        dataFile = SPIFFS.open(WIFI_PATH, "w");
        dataFile.printf("%s\n%s", G_wifi_ssid.c_str(), G_wifi_passwd.c_str());
        dataFile.close();
    }
    // else if (file_name == AP_PATH) // 保存ap_ssid ap_passwd
    // {
    //     Serial.printf("写入%s 【%s | %s】\n", AP_PATH,
    //                   G_ap_ssid.c_str(), G_ap_passwd.c_str());
    //     dataFile = SPIFFS.open(AP_PATH, "w");
    //     dataFile.printf("%s\n%s", G_ap_ssid.c_str(), G_ap_passwd.c_str());
    //     dataFile.close();
    // }
    else if (file_name == CONFIG_PATH) // 保存mode screen_setting
    {
        Serial.printf("写入%s 【%d | %d】\n", CONFIG_PATH,
                      G_mode, G_screen_setting);
        dataFile = SPIFFS.open(CONFIG_PATH, "w");
        dataFile.printf("%d\n%d", G_mode, G_screen_setting);
        dataFile.close();
    }
    else if (file_name == BRIGHT_PATH) // 保存bright
    {
        Serial.printf("写入%s 【%d】\n", BRIGHT_PATH, G_screen_bright);
        dataFile = SPIFFS.open(BRIGHT_PATH, "w");
        dataFile.printf("%d", G_screen_bright);
        dataFile.close();
    }
    else if (file_name == DISP_PATH) // 保存screen_disp
    {
        Serial.printf("写入%s 【%s】\n", DISP_PATH, G_screen_disp.c_str());
        dataFile = SPIFFS.open(DISP_PATH, "w");
        dataFile.printf("%s", G_screen_disp.c_str());
        dataFile.close();
    }
    else if (file_name == LOGO_PATH) // 保存screen_logo
    {
        Serial.printf("写入%s 【%d】\n", LOGO_PATH, G_screen_icon);
        dataFile = SPIFFS.open(LOGO_PATH, "w");
        dataFile.printf("%d", G_screen_icon);
        dataFile.close();
    }
    else if (file_name == PROC_PATH) // 保存screen_logo
    {
        Serial.printf("写入%s 【%d】\n", PROC_PATH, G_screen_percent);
        dataFile = SPIFFS.open(PROC_PATH, "w");
        dataFile.printf("%d", G_screen_percent);
        dataFile.close();
    }
}

// 初始化网络
void init_network()
{
    String ipaddr = "";
    G_ap_ssid += ESP.getChipId();

    WiFi.mode(WIFI_STA);
    WiFi.hostname(G_ap_ssid.c_str());

    if (connect_wifi(G_wifi_ssid, G_wifi_passwd))
    {
        VFD_display_string(0, " WiFi Ready ");
        VFD_show();
        Serial.printf("SSID: %s\n", G_wifi_ssid.c_str());
        ipaddr = WiFi.localIP().toString();
    }
    else
    {
        WiFi.mode(WIFI_AP);
        WiFi.softAP(G_ap_ssid, G_ap_passwd);

        VFD_display_string(0, " WiFi Field ");
        VFD_show();
        delay(1000);

        VFD_display_string(0, " AP Mode On ");
        VFD_show();
        delay(1000);

        VFD_display_string(0, G_ap_ssid + "    ");
        VFD_show();
        delay(2000);

        VFD_display_string(0, G_ap_passwd + "    ");
        VFD_show();
        delay(3000);

        Serial.printf("\n进入AP模式: %s\n", G_ap_ssid.c_str());
        ipaddr = WiFi.softAPIP().toString();
    }
    delay(1000);

    Serial.printf("IP地址: http://%s\n", ipaddr.c_str());
    if (ipaddr.length() > 12)
    {
        ipaddr = ipaddr.substring(ipaddr.length() - 12);
    }
    else if (ipaddr.length() < 10)
    {
        ipaddr += "     ";
    }
    VFD_display_string(0, ipaddr);
    VFD_show();
}

// 初始化NTP客户端
void init_ntp()
{
    timeClient.begin();
    Serial.println("NTP客户端启动");
    timeClient.update();

    u_char i;
    for (i = 0; i < 12; i++)
    {
        VFD_display_logo(i, CTRL_LOGO_ON);
        delay(200);
    }
    for (i = 0; i < 12; i++)
    {
        VFD_display_logo(i, CTRL_LOGO_OFF);
    }
    Serial.println("当前时间：" + timeClient.getFormattedTime());
}

// 初始化WEB服务端
void init_webserver()
{
    web_serv.begin();
    web_serv.on("/", handle_root);
    web_serv.on("/test", handle_test);
    web_serv.on("/version", handle_versoin);

    web_serv.on("/set/mode", handle_set_mode);
    web_serv.on("/set/wifi", handle_set_wifi);
    web_serv.on("/set/password", handle_set_connpasswd);
    web_serv.on("/set/bright", handle_set_bright);
    web_serv.on("/set/reboot", handle_set_reboot);
    web_serv.on("/set/reset", handle_set_reset);

    web_serv.on("/set/time/config", handle_set_time_config);

    web_serv.on("/set/text/icon", handle_screen_icon);
    web_serv.on("/set/text/spare", handle_screen_spare);
    web_serv.on("/set/text/group", handle_screen_group);
    web_serv.on("/set/text/config", handle_screen_config);

    web_serv.on("/set/proc/config", handle_process_config);
    web_serv.on("/set/proc/value", handle_process_value);
    web_serv.on("/set/proc/string", handle_process_string);

    web_serv.onNotFound(handle_exception);
    Serial.println("WEB服务器启动");
    Serial.printf("连接密码【%s】\n", G_conn_passwd.c_str());
}

// ==========================================================
// 切换模式
void switch_mode(u_char mode)
{
    if (mode < MODE_MIN || mode > MODE_MAX)
    {
        Serial.printf("mode【%d】不合法\n", mode);
    }
    else
    {
        ticker.detach();
        Serial.printf("切换为【%d】模式\n", mode);
        G_mode = mode;
        for (u_char i = 0; i < 12; i++)
        {
            VFD_display_logo(i, ((1 << i) & G_screen_icon) ? CTRL_LOGO_ON : CTRL_LOGO_OFF);
        }
    }
    switch (mode)
    {
    case MODE_TIME:
        VFD_display_logo(LOGO_COLON_LEFT1, (G_screen_icon & (1 << LOGO_COLON_LEFT1))
                                               ? CTRL_LOGO_COLON_ON
                                               : CTRL_COLON_ON);
        VFD_display_logo(LOGO_COLON_LEFT2, (G_screen_icon & (1 << LOGO_COLON_LEFT2))
                                               ? CTRL_LOGO_COLON_ON
                                               : CTRL_COLON_ON);
        mode_time_display(1);
        ticker.attach_ms(500, mode_time_display, 0);
        break;
    case MODE_TEXT:
        VFD_display_string(0, "            ");
        mode_text_display(1);
        ticker.attach_ms(500, mode_text_display, 0);
        break;
    case MODE_PROC:
        mode_proc_display();
        break;
    default:
        switch_mode(MODE_TIME);
        break;
    }
}

// 验证访问密码
bool web_verify()
{
    if (G_conn_passwd != "")
    {
        String passwd = web_serv.arg("k");
        if (passwd.compareTo(G_conn_passwd))
        {
            web_serv.send(403, PLAIN, WEBS_403);
            return false;
        }
    }
    return true;
}

//根目录
void handle_root()
{
    web_serv.send(200, PLAIN, WEBS_ROOT);
}

//测试连接
void handle_test()
{
    web_serv.send(200, PLAIN, WEBS_SUCC);
}

//检查版本
void handle_versoin()
{
    String str = "Firmware version: ";
    str += VERSION;
    str += "  Build date: ";
    str += __DATE__;
    str += " ";
    str += __TIME__;
    web_serv.send(200, PLAIN, str);
}

//设置模式
void handle_set_mode()
{
    if (!web_verify()) // 验证密码
        return;

    int mode = web_serv.arg("m").toInt();

    if (mode < MODE_MIN || mode > MODE_MAX)
    {
        web_serv.send(503, PLAIN, WEBS_FAIL);
    }
    else
    {
        switch_mode(mode);
        if (web_serv.arg("f") != "")
        {
            init_save_cfg(CONFIG_PATH);
        }
        web_serv.send(200, PLAIN, WEBS_SUCC);
    }
}

//设置wifi信息
void handle_set_wifi()
{
    ticker.detach();
    if (!web_verify()) // 验证密码
        return;

    String wifi_ssid = web_serv.arg("s");
    String wifi_passwd = web_serv.arg("p");

    Serial.printf("%s %s\n", wifi_ssid.c_str(), wifi_passwd.c_str());

    if (wifi_ssid != "" && wifi_passwd != "")
    {
        web_serv.send(200, PLAIN, WEBS_SUCC);

        if (G_wifi_ssid == wifi_ssid && WiFi.isConnected())
        {
            Serial.println("WiFi设置相同,无需更改设定");
            return;
        }

        delay(1000);
        if (connect_wifi(wifi_ssid, wifi_passwd))
        {
            G_wifi_ssid = wifi_ssid;
            G_wifi_passwd = wifi_passwd;
            Serial.println("新WiFi连接成功,保存设定");
            init_save_cfg(WIFI_PATH);
            ticker.detach();

            VFD_display_string(0, " Test Ready ");
            VFD_show();
            delay(1000);
            VFD_display_string(0, " Rebooting  ");
            VFD_show();
            Serial.println("即将重新启动");
            delay(1000);
            ESP.restart();
        }
        else
        {
            Serial.println("新WiFi连接失败,恢复原设置");
            VFD_display_string(0, " Test Field ");
            VFD_show();
            delay(1000);
            VFD_display_string(0, " WiFi Reset ");
            VFD_show();
            delay(1000);
            init_network();
            switch_mode(G_mode);
        }
    }
    else
    {
        web_serv.send(503, PLAIN, WEBS_FAIL);
    }
}

//设置访问密码
void handle_set_connpasswd()
{
    if (!web_verify()) // 验证密码
        return;

    String conn_passwd = web_serv.arg("kn");

    Serial.printf("%s %s\n", G_conn_passwd.c_str(), conn_passwd.c_str());

    if (G_conn_passwd != conn_passwd)
    {
        G_conn_passwd = conn_passwd;
        Serial.printf("修改连接密码为【%s】\n", conn_passwd.c_str());
        init_save_cfg(AUTH_PATH);
    }
    else
    {
        Serial.printf("连接密码为【%s】无需修改\n", conn_passwd.c_str());
    }
    web_serv.send(200, PLAIN, WEBS_SUCC);
}

//设置亮度
void handle_set_bright()
{
    if (!web_verify()) // 验证密码
        return;

    int bright = web_serv.arg("b").toInt();
    if (bright < 0 || bright > 255)
    {
        Serial.printf("亮度参数非法 %d\n", bright);
        web_serv.send(503, PLAIN, WEBS_FAIL);
        return;
    }

    Serial.printf("设置亮度为 %d\n", bright);
    VFD_set_brightness(bright);
    if (bright != 0 && web_serv.arg("f") != "")
    {
        G_screen_bright = bright;
        init_save_cfg(BRIGHT_PATH);
    }
    web_serv.send(200, PLAIN, WEBS_SUCC);
}

// 远程重启
void handle_set_reboot()
{
    ticker.detach();
    if (!web_verify())
        return;

    Serial.println("即将重启");
    VFD_all_off();
    VFD_display_string(0, " Rebooting  ");
    VFD_show();
    web_serv.send(200, PLAIN, WEBS_SUCC);
    delay(1000);
    ESP.restart();
}

//恢复默认设置
void handle_set_reset()
{
    if (!web_verify()) // 验证密码
        return;

    web_serv.send(200, PLAIN, VERSION);

    Serial.println("恢复默认设置,格式化内部储存器");
    ticker.detach();
    VFD_display_string(0, " Format FS  ");
    VFD_show();
    delay(1000);
    VFD_display_string(0, " Rebooting  ");
    VFD_show();
    SPIFFS.format();
    ESP.restart();
}

//设置时间显示模式
void handle_set_time_config()
{
    if (!web_verify()) // 验证密码
        return;

    bool h24 = web_serv.arg("h").toInt();
    bool apm = web_serv.arg("a").toInt();
    bool week = web_serv.arg("w").toInt();

    Serial.printf("设置时钟格式为 %d %d %d\n", h24, apm, week);

    u_char tmp = 0;
    if (h24)
    {
        tmp += 4;
    }
    if (apm)
    {
        tmp += 2;
    }
    if (!week)
    {
        tmp += 1;
    }
    G_screen_setting &= 0xf8;
    G_screen_setting |= tmp;

    switch_mode(MODE_TIME);
    if (web_serv.arg("f") != "")
    {
        init_save_cfg(CONFIG_PATH);
    }
    web_serv.send(200, PLAIN, WEBS_SUCC);
}

//设置logo
void handle_screen_icon()
{
    if (!web_verify()) // 验证密码
        return;

    G_screen_icon = web_serv.arg("i").toInt();
    switch_mode(G_mode);
    if (web_serv.arg("f") != "")
    {
        init_save_cfg(LOGO_PATH);
    }
    web_serv.send(200, PLAIN, WEBS_SUCC);
}

//设置单个字符
void handle_screen_spare()
{
    if (!web_verify()) // 验证密码
        return;

    G_screen_setting &= 0x3f; // 设置为静态显示

    if (G_mode != MODE_TEXT)
    {
        switch_mode(MODE_TEXT);
    }
    ticker.detach();

    Serial.println("设置静态屏显");

    for (u_char i = 0; i < web_serv.args(); i++)
    {
        int seg = web_serv.argName(i).toInt();
        int value = web_serv.arg(i).toInt();
        if ((seg < 1 || seg > 12) ||
            (value < 0 || value > 255))
        {
            continue;
        }
        VFD_display_raw((u_char)(--seg), (u_char)value);
    }
    VFD_show();
    web_serv.send(200, PLAIN, WEBS_SUCC);
}

//设置整串字符
void handle_screen_group()
{
    if (!web_verify())
        return;

    G_screen_disp = web_serv.arg("w").substring(0, 300);

    switch_mode(MODE_TEXT);

    if (web_serv.arg("f") != "")
    {
        init_save_cfg(DISP_PATH);
    }

    web_serv.send(200, PLAIN, WEBS_SUCC);
}

//设置屏显模式
void handle_screen_config()
{
    ticker.detach();
    if (!web_verify())
        return;

    int sm_mode = web_serv.arg("m").toInt();

    if (sm_mode < 0 || sm_mode > 3)
    {
        web_serv.send(503, PLAIN, WEBS_FAIL);
        return;
    }

    G_screen_setting &= 0x3f;
    G_screen_setting |= (sm_mode << 6);
    switch_mode(MODE_TEXT);

    if (web_serv.arg("f") != "")
    {
        init_save_cfg(CONFIG_PATH);
    }

    web_serv.send(200, PLAIN, WEBS_SUCC);
}

//设置进度条
void handle_process_config()
{
    ticker.detach();
    if (!web_verify())
        return;

    bool logobar = web_serv.arg("i").toInt();
    bool percent = web_serv.arg("n").toInt();
    bool label = web_serv.arg("l").toInt();

    u_char tmp = 0;
    if (logobar)
    {
        tmp += 0x20;
    }
    if (!percent)
    {
        tmp += 0x10;
    }
    if (!label)
    {
        tmp += 0x08;
    }

    G_screen_setting &= 0xc7;
    G_screen_setting |= tmp;
    switch_mode(MODE_PROC);

    if (web_serv.arg("f") != "")
    {
        init_save_cfg(CONFIG_PATH);
    }

    Serial.printf("设置进度条【%d %d %d】\n", label, percent, logobar);
    web_serv.send(200, PLAIN, WEBS_SUCC);
}

//设置进度条进度
void handle_process_value()
{
    ticker.detach();
    if (!web_verify())
        return;

    int value = web_serv.arg("v").toInt();

    if (value < 0 || value > 100)
    {
        web_serv.send(503, PLAIN, WEBS_FAIL);
        return;
    }

    G_screen_percent = (u_char)value;
    switch_mode(MODE_PROC);

    if (web_serv.arg("f") != "")
    {
        init_save_cfg(PROC_PATH);
    }

    web_serv.send(200, PLAIN, WEBS_SUCC);
}

//设置进度条文本
void handle_process_string()
{
    if (!web_verify())
        return;

    G_screen_disp = web_serv.arg("w").substring(0, 300);

    switch_mode(MODE_PROC);

    if (web_serv.arg("f") != "")
    {
        init_save_cfg(DISP_PATH);
    }

    web_serv.send(200, PLAIN, WEBS_SUCC);
}

//错误处理
void handle_exception()
{
    Serial.println(web_serv.uri());
    web_serv.send(404, PLAIN, WEBS_404);
}

// 显示当前时间
void mode_time_display(int reset)
{
    static bool dot = true;
    static bool h24 = false, apm = false, week = false;

    if (reset)
    {
        h24 = G_screen_setting & 0x04;
        apm = G_screen_setting & 0x02;
        week = G_screen_setting & 0x01;

        Serial.printf("重设【%d %d %d】\n", h24, apm, week);
    }

    String str = "";

    u_char h, m, s;
    h = (u_char)timeClient.getHours();
    m = (u_char)timeClient.getMinutes();
    s = (u_char)timeClient.getSeconds();

    if (!week) // 显示星期,显示上下午
    {
        str = String(WEEKNAME[timeClient.getDay()]);
        if (apm)
        {
            str += h >= 12 ? " PM" : " AM";
        }
        else
        {
            str += "   ";
        }
    }
    else if (week && apm) // 不显示星期,显示上下午
    {
        str = h >= 12 ? "PM    " : "AM    ";
    }
    else
    {
        str = "      ";
    }

    if (!h24 && h > 12)
    {
        str += (h-12) / 10;
        str += (h-12) % 10;
    }
    else
    {
        str += h / 10;
        str += h % 10;
    }
    str += m / 10;
    str += m % 10;
    str += s / 10;
    str += s % 10;

    if (h < 6)
    {
        VFD_all_off();
    }
    else
    {
        VFD_display_string(0, str);
        if (dot)
        {
            VFD_display_logo(LOGO_COLON_RIGHT1, (G_screen_icon & (1 << LOGO_COLON_RIGHT1))
                                                    ? CTRL_LOGO_COLON_ON
                                                    : CTRL_COLON_ON);
            VFD_display_logo(LOGO_COLON_RIGHT2, (G_screen_icon & (1 << LOGO_COLON_RIGHT2))
                                                    ? CTRL_LOGO_COLON_ON
                                                    : CTRL_COLON_ON);
        }
        else
        {
            VFD_display_logo(LOGO_COLON_RIGHT1, (G_screen_icon & (1 << LOGO_COLON_RIGHT1))
                                                    ? CTRL_LOGO_ON
                                                    : CTRL_LOGO_OFF);
            VFD_display_logo(LOGO_COLON_RIGHT2, (G_screen_icon & (1 << LOGO_COLON_RIGHT2))
                                                    ? CTRL_LOGO_ON
                                                    : CTRL_LOGO_OFF);
        }
        dot = !dot;
    }
}

// 滚动文本
void mode_text_display(int reset)
{
    const u_char sm_mode = (G_screen_setting & 0xc0) >> 6;

    if (sm_mode == SM_STATIC)
    {
        if (G_screen_disp.length() > 12)
        {
            VFD_display_string(0, G_screen_disp.substring(0, 12));
        }
        else
        {
            VFD_display_string(0, G_screen_disp);
        }
        VFD_show();
        ticker.detach();
        return;
    }

    static u_int x = 0, y = 0;
    static u_int len = 0;
    static bool z = true;

    static String tmp = "";

    if (reset)
    {
        Serial.printf("滚动模式 %d\n", sm_mode);
        len = G_screen_disp.length();

        tmp = "";

        if (sm_mode == SM_LEFT)
        {
            do
            {
                tmp += G_screen_disp;
                tmp += "  ";
            } while (tmp.length() < len + 13);
        }
        else if (sm_mode == SM_RIGHT)
        {
            do
            {
                tmp += "  ";
                tmp += G_screen_disp;
            } while (tmp.length() < len + 13);
        }
        else if (sm_mode == SM_LR)
        {
            do
            {
                tmp += G_screen_disp;
                tmp += "  ";
            } while (tmp.length() < len + 15);
            tmp = tmp.substring(0, tmp.length() - 2);
        }

        x = 0;
        y = tmp.length();
        // z = true;
        Serial.printf("重设【%s】\n", tmp.c_str());
    }

    String str = "";

    if (sm_mode == SM_LEFT)
    {
        str = tmp.substring(x, x + 12);
        if (x++ > len)
        {
            x = 0;
        }
    }
    else if (sm_mode == SM_RIGHT)
    {
        str = tmp.substring(y - 12, y);
        if (y-- < tmp.length() - len)
        {
            y = tmp.length();
        }
    }
    else if (sm_mode == SM_LR)
    {
        if (z)
        {
            str = tmp.substring(x, x + 12);
            if (x++ > len + 1)
            {
                x = 0;
                z = false;
            }
        }
        else
        {
            str = tmp.substring(y - 12, y);
            if (y-- < tmp.length() - len - 1)
            {
                y = tmp.length();
                z = true;
            }
        }
    }
    VFD_display_string(0, str);
    VFD_show();
    // Serial.print(z);
    // Serial.printf(" %d %d【%s】\n",x, y, str.c_str());
}

// 进度条
void mode_proc_display()
{
    static bool logobar = false, percent = false, label = false;

    static String tmp = "";

    logobar = G_screen_setting & 0x20;
    percent = !(G_screen_setting & 0x10);
    label = !(G_screen_setting & 0x08);

    String str;

    if (label)
    {
        if (G_screen_disp.length() < 12)
        {
            str = G_screen_disp + "            ";
            str = str.substring(0, 12);
        }
        else
        {
            str = G_screen_disp.substring(0, 12);
        }
    }
    else
    {
        str = "         ";
    }

    float value; // 百分比数值

    if (logobar)
    { // 把logo区域当做进度条
        value = G_screen_percent * 12 / 100.0;

        // 计算需要点亮的图标
        u_int bar = 0;

        for (u_char i = 0; i < value; i++)
        {
            bar ^= 1 << i;
        }

        for (u_char i = 0; i < 12; i++)
        {
            VFD_display_logo(i, ((1 << i) & bar) ? CTRL_LOGO_ON : CTRL_LOGO_OFF);
        }

        if (percent)
        {
            str = str.substring(0, 9);
            if (G_screen_percent == 100)
            {
                str += "100";
            }
            else
            {
                str += G_screen_percent / 10;
                str += G_screen_percent % 10;
                str += "%";
            }
        }
        else
        {
            str += "   ";
        }
        VFD_display_string(0, str);
    }
    else
    { // 在显示区显示进度条
        if (label)
        {
            if (percent)
            {
                value = G_screen_percent * 5 / 100.0;
                str = str.substring(0, 4);
                str += "     ";

                if (G_screen_percent == 100)
                {
                    str += "100";
                }
                else
                {
                    str += G_screen_percent / 10;
                    str += G_screen_percent % 10;
                    str += "%";
                }
            }
            else
            {
                value = G_screen_percent * 8 / 100.0;
                str = str.substring(0, 4);
                str += "        ";
            }
            VFD_display_string(0, str);

            for (u_char i = 0; i < value; i++)
            {
                VFD_display_raw(i + 4, VFD_FULL);
            }
        }
        else
        {
            if (percent)
            {
                value = G_screen_percent * 9 / 100.0;
                str = str.substring(0, 4);
                str += "     ";

                if (G_screen_percent == 100)
                {
                    str += "100";
                }
                else
                {
                    str += G_screen_percent / 10;
                    str += G_screen_percent % 10;
                    str += "%";
                }
            }
            else
            {
                value = G_screen_percent * 12 / 100.0;
                str += "   ";
            }
            VFD_display_string(0, str);

            for (u_char i = 0; i < value; i++)
            {
                VFD_display_raw(i, VFD_FULL);
            }
        }
    }
    Serial.printf("百分比【%d%%】【%d %d %d】\n", G_screen_percent, label, percent, logobar);
    VFD_show();
}
