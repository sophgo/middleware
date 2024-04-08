#!/usr/bin/env python3
# -*- coding: utf-8 -*-
__author__ = ''

import json
from docx import Document
import sys
from docx.shared import Inches
from cvi_isp_docx import *



if __name__ == '__main__':
    cvi_docx = cvi_isp_docx_create()
    #cvi_isp_docx_add_chapter(cvi_docx, 1, "CNR", "")

    with open("./resource/new.json","r", encoding='utf-8') as _f:
        #json.load(f.read())
        #print(f.read())
        json_f = json.load(_f)
        #print(json_f)

        #for item in json_f.items():
        #    print(item[1][0])
        #    print("** next one **")

        #for item in json_f["1.1 CNR"]:
        #    print(item)
        for item in json_f.items():
            print(item[0])
            _item = item[0]
            # print(str(json_f[_item]["chapter_num_main"]))
            # print(json_f[_item]["chapter_num_sub"])
            # print(json_f[_item]["module"])
            _item = item[0]
            # print(str(json_f[_item]["chapter_num_main"]))
            # print(json_f[_item]["chapter_num_sub"])
            # print(json_f[_item]["module"])
            _m_chapter = str(json_f[_item]["chapter_num_main"]) + "." + str(json_f[_item]["chapter_num_sub"])
            #_m_string = _m_chapter + " " + json_f[_item]["module"]
            _m_string = json_f[_item]["module"]
            cvi_doc_addheading(cvi_docx, 3, _m_string)
            print(_m_string)

            # add chapter title 1
            print(json_f[_item]["ch_tl3_1"]["tl_num"])
            _m_string = _m_chapter + "." + str(json_f[_item]["ch_tl3_1"]["tl_num"]) + " " + json_f[_item]["ch_tl3_1"][
                "title"]
            print(_m_string)
            _m_string = json_f[_item]["ch_tl3_1"]["title"]
            cvi_doc_addheading(cvi_docx, 4, _m_string)
            cvi_doc_addparagraph(cvi_docx, Cm(2), json_f[_item]["ch_tl3_1"]["content"])
            # add chapter title 2
            _m_string = _m_chapter + "." + str(json_f[_item]["ch_tl3_2"]["tl_num"]) + " " + json_f[_item]["ch_tl3_2"][
                "title"]
            print(_m_string)
            _m_string = json_f[_item]["ch_tl3_2"]["title"]
            cvi_doc_addheading(cvi_docx, 4, _m_string)
            # print(len(json_f[_item]["ch_tl3_2"]["section_start"]))
            if len(json_f[_item]["ch_tl3_2"]["section_start"]) != 0:
                cvi_doc_addparagraph(cvi_docx, Cm(2), json_f[_item]["ch_tl3_2"]["section_start"])
            for _api in json_f[_item]["ch_tl3_2"]["section_api_list"]:
                print(_api)
                _m_content = _api["content"][0] + ": " + _api["content"][1]
                # print(_m_content)
                if _api["style"] == "List Bullet":
                    cvi_doc_addparagraph(cvi_docx, Cm(2.7), _m_content, 'List Bullet')
                else:
                    cvi_doc_addparagraph(cvi_docx, Cm(2), _m_content)
            # add apis description
            for _s_api in json_f[_item]["ch_tl3_2"]["section_apis"]:
                # print(_s_api)
                #cvi_doc_addheading(cvi_docx, 3, _s_api["title"])
                cvi_doc_addparagraph(cvi_docx, Cm(0), _s_api["title"])
                for _item4 in _s_api["detail"]:
                    #print("wl mark: ", _item4)
                    cvi_doc_addparagraph(cvi_docx, Cm(2), "【" + _item4 + "】")
                    #print("type:", type(_s_api["detail"][_item4]))
                    if type(_s_api["detail"][_item4]) is list :
                        for _item5 in _s_api["detail"][_item4]:
                            #print(_item5)
                            if _item5["style"] == "List Bullet":
                                cvi_doc_addparagraph(cvi_docx, Cm(2.7), _item5["content"], 'List Bullet')
                            else:
                                cvi_doc_addparagraph(cvi_docx, Cm(2), _item5["content"])
                    elif type(_s_api["detail"][_item4]) is dict:
                        if _s_api["detail"][_item4]["style"] == "normal":
                            print(_item4)
                            print(_s_api["detail"][_item4]["content"])
                            cvi_doc_addparagraph(cvi_docx, Cm(2), _s_api["detail"][_item4]["content"])
                        elif _s_api["detail"][_item4]["style"] == "link":
                            cvi_doc_addparagraph(cvi_docx, Cm(2), _s_api["detail"][_item4]["content"])
                        elif _s_api["detail"][_item4]["style"] == "table":
                            _rd_table = _s_api["detail"][_item4]["table"]
                            print(type(_rd_table))
                            cvi_doc_addtable(cvi_docx, _rd_table)
                            cvi_docx.add_paragraph('')  # 在表格后面添加换行
                        elif _s_api["detail"][_item4]["style"] == "List Bullet":
                            for _con in _s_api["detail"][_item4]["content"]:
                                _t_string = _con[0] + ": "
                                for i in range(len(_con)):
                                    if i == 0:
                                        pass
                                    elif i == 1:
                                        _t_string = _t_string + _con[i]
                                    else:
                                        _t_string = _t_string + ", " + _con[i]
                                cvi_doc_addparagraph(cvi_docx, Cm(2.7), _t_string, 'List Bullet')

            #
            # add chapter title 3
            #
            _m_string_tl3 = _m_chapter + "." + str(json_f[_item]["ch_tl3_3"]["tl_num"]) + " " + \
                            json_f[_item]["ch_tl3_3"]["title"]
            print(_m_string_tl3)
            _m_string_tl3 = json_f[_item]["ch_tl3_3"]["title"]
            cvi_doc_addheading(cvi_docx, 4, _m_string_tl3)
            if len(json_f[_item]["ch_tl3_3"]["section_start"]) != 0:
                cvi_doc_addparagraph(cvi_docx, Cm(2), json_f[_item]["ch_tl3_3"]["section_start"])
            for _api in json_f[_item]["ch_tl3_3"]["section_datastruct_list"]:
                print(_api)
                _m_content = _api["content"][0] + ": " + _api["content"][1]
                # print(_m_content)
                if _api["style"] == "List Bullet":
                    cvi_doc_addparagraph(cvi_docx, Cm(2.7), _m_content, 'List Bullet')
                else:
                    cvi_doc_addparagraph(cvi_docx, Cm(2), _m_content)

            # add apis description
            for _s_api in json_f[_item]["ch_tl3_3"]["section_datastructs"]:
                # print(_s_api)
                #cvi_doc_addheading(cvi_docx, 3, _s_api["title"])
                cvi_doc_addparagraph(cvi_docx, Cm(0),  _s_api["title"])
                for _item4 in _s_api["detail"]:
                    # print("wl mark: ", _item4)
                    cvi_doc_addparagraph(cvi_docx, Cm(2), "【" + _item4 + "】")
                    # print("type:", type(_s_api["detail"][_item4]))
                    if type(_s_api["detail"][_item4]) is list:
                        for _item5 in _s_api["detail"][_item4]:
                            # print(_item5)
                            if _item5["style"] == "List Bullet":
                                cvi_doc_addparagraph(cvi_docx, Cm(2.7), _item5["content"], 'List Bullet')
                            else:
                                cvi_doc_addparagraph(cvi_docx, Cm(2), _item5["content"])
                    elif type(_s_api["detail"][_item4]) is dict:
                        if _s_api["detail"][_item4]["style"] == "normal":
                            print(_item4)
                            print(_s_api["detail"][_item4]["content"])
                            cvi_doc_addparagraph(cvi_docx, Cm(2), _s_api["detail"][_item4]["content"])
                        elif _s_api["detail"][_item4]["style"] == "link":
                            cvi_doc_addparagraph(cvi_docx, Cm(2), _s_api["detail"][_item4]["content"])
                        elif _s_api["detail"][_item4]["style"] == "table":
                            _rd_table = _s_api["detail"][_item4]["table"]
                            print(type(_rd_table))
                            cvi_doc_addtable(cvi_docx, _rd_table)
                            cvi_docx.add_paragraph('')  # 在表格后面添加换行
                        elif _s_api["detail"][_item4]["style"] == "List Bullet":
                            for _con in _s_api["detail"][_item4]["content"]:
                                _t_string = _con[0] + ": "
                                for i in range(len(_con)):
                                    if i == 0:
                                        pass
                                    elif i == 1:
                                        _t_string = _t_string + _con[i]
                                    else:
                                        _t_string = _t_string + ", " + _con[i]
                                cvi_doc_addparagraph(cvi_docx, Cm(2.7), _t_string, 'List Bullet')

            cvi_docx.add_page_break()
                # print(_s_api["detail"])
            # print(len(json_f[_item]["ch_tl3_2"][""]))
            # cvi_doc_addheading(cvi_docx, 3, item3)
            #   _type = type(json_f[_item][item3])
            # print(_type)
            #   if isinstance(json_f[_item][item3], str):
            #       print(json_f[_item][item3])
            #       #cvi_doc_addparagraph(cvi_docx, Cm(2), json_f[_item][item3])
            #   else:
            #       for item4 in json_f[_item][item3]:
            #           print(item4)
            #           print(json_f[_item][item3][item4])
            #           if "attr" in json_f[_item][item3][item4]:
            #               print("true")
            #               cvi_doc_addparagraph(cvi_docx, Cm(2.7), u'CVI_MPI_ISP_SetCNRAttr：设置 CNR 属性参数',
            #                                    'List Bullet')
            #           else:
            #               print("false")


    cvi_isp_docx_save(cvi_docx, "./output/CVI ISP 开发参考(from json file).docx")