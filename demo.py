'''
# @Author       : Chr_
# @Date         : 2021-04-30 21:08:50
# @LastEditors  : Chr_
# @LastEditTime : 2021-05-16 17:53:15
# @Description  : 
'''

import requests
from time import sleep

for x in range(0,260,12):
    d={'k':'123456'}
    for i,j in enumerate( range (x,x+12),1):
        d[i] = j

    url = "http://192.168.2.96/set/text/spare"

    resp = requests.post(url=url,params=d)
    print(d)