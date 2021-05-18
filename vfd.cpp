/* 
 * @Author       : Chr_
 * @Date         : 2021-04-25 22:57:37
 * @LastEditors  : Chr_
 * @LastEditTime : 2021-05-06 01:15:08
 * @Description  : VFD显示屏驱动
 */

#include <Arduino.h>

#include "vfd.h"

void delay1()
{
    for (ulong j = 0; j < 1000; j++)
        do
        {
            __asm__ __volatile__("nop");
        } while (0);
}

void write_8bit(u_char w_data)
{
    u_char i;
    for (i = 0; i < 8; i++)
    {
        digitalWrite(V_CLK, LOW);
        if ((w_data & 0x01) == 0x01)
        {
            digitalWrite(V_DIN, HIGH);
        }
        else
        {
            digitalWrite(V_DIN, LOW);
        }
        w_data >>= 1;
        delay1();
        digitalWrite(V_CLK, HIGH);
    }
}

void VFD_cmd(u_char command)
{
    digitalWrite(V_CS, LOW);
    write_8bit(command);
    digitalWrite(V_CS, HIGH);
}

void VFD_show()
{
    VFD_cmd(0xe8); //地址寄存器起始位置
    delay1();
}

void VFD_init()
{
    //端口初始化
    pinMode(V_CLK, OUTPUT);
    pinMode(V_DIN, OUTPUT);
    pinMode(V_CS, OUTPUT);
    pinMode(V_EN, OUTPUT);
    digitalWrite(V_CS, HIGH);
    digitalWrite(V_EN, HIGH);

    //SET HOW MANY digtal numbers
    VFD_cmd(0xe0);
    delay1();
    VFD_cmd(0x0B); //12 digtal
    delay1();

    //set bright
    VFD_set_brightness(0xff);
}

void VFD_all_on()
{
    digitalWrite(V_CS, LOW);
    write_8bit(0xe9);
    delay1();
}

void VFD_all_off()
{
    VFD_display_string(0, "            ");
    u_char i;
    for (i = 0; i < 12; i++)
    {
        VFD_display_logo(i, CTRL_LOGO_OFF);
    }
    delay1();
}

void VFD_display_char(u_char x, u_char chr)
{
    digitalWrite(V_CS, LOW); //开始传输
    write_8bit(0x20 + x);    //地址寄存器起始位置
    write_8bit(chr + 0x30);
    digitalWrite(V_CS, HIGH); //停止传输
}

void VFD_display_raw(u_char x, u_char chr)
{
    digitalWrite(V_CS, LOW); //开始传输
    write_8bit(0x20 + x);    //地址寄存器起始位置
    write_8bit(chr);
    digitalWrite(V_CS, HIGH); //停止传输
}

void VFD_display_string(u_char x, char *str)
{
    digitalWrite(V_CS, LOW); //开始传输
    write_8bit(0x20 + x);    //地址寄存器起始位置
    while (*str)
    {
        //Serial.println(*str);
        write_8bit(*str); //ascii与对应字符表转换
        str++;
    }
    digitalWrite(V_CS, HIGH); //停止传输
                              // VFD_show();
}
void VFD_display_string(u_char x, String str)
{
    const char *p = str.c_str();

    digitalWrite(V_CS, LOW); //开始传输
    write_8bit(0x20 + x);    //地址寄存器起始位置
    while (*p)
    {
        //Serial.println(*str);
        write_8bit(*p); //ascii与对应字符表转换
        p++;
    }
    digitalWrite(V_CS, HIGH); //停止传输
                              // VFD_show();
}

void VFD_display_logo(u_char ad_dat, u_char ctrl_flag)
{
    digitalWrite(V_CS, LOW);   //开始传输
    write_8bit(0x60 + ad_dat); //ADRAM
    write_8bit(ctrl_flag);
    digitalWrite(V_CS, HIGH); //停止传输
                              //VFD_show();
}

void VFD_set_brightness(u_char light)
{
    digitalWrite(V_CS, LOW);
    write_8bit(0xe4);
    delay1();
    write_8bit(light); //leve 255 max
    digitalWrite(V_CS, HIGH);
    delay1();
}
