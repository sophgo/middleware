# coding=utf-8

import http.client
import hashlib
import urllib
import random
import json
import string

appid = '20201119000620285'  # 填写你的appid
secretKey = 'JkusMV4F6MRkou5oy1Og'  # 填写你的密钥
httpClient = None
url = '/api/trans/vip/translate'

class Translate:
    def __init__(self):
        self.fromLang = 'auto'  # 原文语种
        self.toLang = 'en'  # 译文语种
        self.httpClient = http.client.HTTPConnection('api.fanyi.baidu.com')
    def getTranslateRes(self, q):
        if(type(q) == type(1)):
            return ''
        q = q.strip().replace('\n','')
        salt = random.randint(32768, 65536)
        sign = appid + q + str(salt) + secretKey
        sign = hashlib.md5(sign.encode()).hexdigest()
        myurl = url + '?appid=' + appid + '&q=' + urllib.parse.quote(
            q) + '&from=' + self.fromLang + '&to=' + self.toLang + '&salt=' + str(
            salt) + '&sign=' + sign
        while 1:
            self.httpClient.request('GET', myurl)
            response = self.httpClient.getresponse()
            result_all = response.read().decode("utf-8")
            result = json.loads(result_all)
            if(('error_msg' in result) or ('error_code' in result)):
                continue
            else:
                if('trans_result' in result):
                    print(q, "--", result['trans_result'][0]['dst'])
                    return result['trans_result'][0]['dst']


