/* 
 * @Author       : Chr_
 * @Date         : 2021-04-25 22:57:45
 * @LastEditors  : Chr_
 * @LastEditTime : 2021-05-06 23:09:18
 * @Description  : VFD显示屏驱动
 */

#ifndef _VFD_H
#define _VFD_H

#define V_DIN 5 //vfd din
#define V_CLK 4 //vfd clk
#define V_CS 0  //spi cs
#define V_EN 2  //enable power of VFD module

// #define V_DIN 13 //vfd din
// #define V_CLK 12 //vfd clk
// #define V_CS 14  //spi cs
// #define V_EN 16  //enable power of VFD module

#define LOGO_USB 0x00
#define LOGO_HD 0x01
#define LOGO_HDD 0x02
#define LOGO_DOLBY 0x03
#define LOGO_MP3 0x04
#define LOGO_MUTE 0x05
#define LOGO_PREV 0x06
#define LOGO_PLAY 0x07
#define LOGO_PAUSE 0x08
#define LOGO_NEXT 0x09
#define LOGO_REC 0x0A
#define LOGO_TIME 0x0B
#define LOGO_COLON_LEFT1 0x07
#define LOGO_COLON_LEFT2 0x08
#define LOGO_COLON_RIGHT1 0x09
#define LOGO_COLON_RIGHT2 0x0A

#define CTRL_LOGO_OFF 0x00
#define CTRL_LOGO_ON 0x02
#define CTRL_COLON_ON 0x01
#define CTRL_LOGO_COLON_ON 0x03

#define VFD_FULL 159 //满字符
#define VFD_NONE 0   //空字符

void delay1();
void write_8bit(u_char w_data);
void VFD_init();
void VFD_all_on();
void VFD_all_off();
void VFD_show();
void VFD_cmd(u_char command);
void VFD_display_char(u_char x, u_char chr);
void VFD_display_raw(u_char x, u_char chr);
void VFD_display_string(u_char x, char *str);
void VFD_display_string(u_char x, String str);
void VFD_display_logo(u_char ad_dat, u_char on_off_flag);
void VFD_set_brightness(u_char light);
#endif