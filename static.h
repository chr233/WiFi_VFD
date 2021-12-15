/* 
 * @Author       : Chr_
 * @Date         : 2021-04-28 12:46:43
 * @LastEditors  : Chr_
 * @LastEditTime : 2021-12-15 14:53:27
 * @Description  : 静态常量
 */

#ifndef _STATIC_h
#define _STATIC_H

const char WEEKNAME[7][4] = {"SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"};

#define VERSION "2.3.5" // 版本号

#define NTPSERV "ntp2.aliyun.com" // NTP服务器

#define WAIT_TIME 300 // WiFi连接等待时间

#define DEF_WIFI_SSID "MIX 2S"
#define DEF_WIFI_PASSWD "FFFFFUCK"
#define DEF_CONN_PASSWD "CHR233"
#define DEF_AP_SSID "WIFI_VFD_"
#define DEF_AP_PASSWD "CHRXWCHRXW"

#define AUTH_PATH "/auth"
#define WIFI_PATH "/wifi"
#define DISP_PATH "/disp"
#define LOGO_PATH "/logo"
#define PROC_PATH "/proc"
#define AP_PATH "/ap"
#define CONFIG_PATH "/config"
#define BRIGHT_PATH "/bright"

#endif