/* 
 * @Author       : Chr_
 * @Date         : 2021-05-05 00:58:22
 * @LastEditors  : Chr_
 * @LastEditTime : 2021-05-05 00:59:28
 * @Description  : 
 */

#include <ESP8266WiFi.h>

#include "static.h"
#include "vfd.h"

// 测试wifi连接
bool connect_wifi(String &ssid, String &passwd)
{
    WiFi.begin(ssid, passwd);
    Serial.println("等待连接WiFi");

    VFD_all_off();
    VFD_display_string(0, " Connecting ");
    VFD_show();

    delay(1000);

    int i = 0;
    while (WiFi.status() != WL_CONNECTED)
    {
        VFD_display_logo(i % 12, CTRL_LOGO_ON);
        delay(80);
        VFD_display_logo(i % 12, CTRL_LOGO_OFF);
        if (i++ % 10 == 0)
        {
            Serial.print(".");
        }
        if (i % 300 == 0)
        {
            Serial.printf(" | %d\n", i);
        }
        if (i > WAIT_TIME)
        {
            Serial.println("\n连接WiFi失败");
            return false;
        }
    }
    Serial.println("\n连接WiFi成功");
    return true;
}