# WiFi_VFD

无线WiFi时钟+灯牌

## 系统组成

* 单片机: Esp8266 NodeMCU
* 显示屏：12位 VFD显示屏

## 引脚配置

```txt
DIN 5 // D1
CLK 4 // D2
CS 0  // D3
EN 2  // D4
```

## 功能简介

时钟功能基于NTP, 因此需要连接WiFi后才能正常工作, 灯牌功能不需要使用网络.

开机后会自动尝试连接上一次配置的WiFi, 如果一段时间还是连接失败会进入AP模式, AP名称固定为`WiFi_VFD_AP`, AP连接密码固定为`CHRXWCHRXW`, 设备IP固定为`192.168.4.1`, 用电脑或者手机连接后, 可以通过Http API修改WIFi设置.

Http API [在线文档][1]

可以通过浏览器访问API进行设置, 也可以使用专用的配置工具 [WiFI_VFD_Tools][2] 进行设置.

固件内置了3种模式, 时钟模式, 灯牌模式, 进度条模式, 具体用法参考[在线文档][1]

[1]:https://blog.chrxw.com/archives/2021/05/07/1569.html
[2]:https://github.com/chr233/WiFi_VFD_Tools