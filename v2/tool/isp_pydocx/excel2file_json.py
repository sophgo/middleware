#!/usr/bin/env python3
# -*- coding: utf-8 -*-
__author__ = ''

# import jinja2
from jinja2 import Template
import pandas as pd
import math
import re
import sys
import json
import copy
import time
import baiduTranslate
import wordninja

IsTranslateDescription = False
IsAddParamDescription = False

def text_create(name):
    _path = "./output/"  # 新创建的txt文件的存放路径
    full_path = _path + name # 也可以创建一个.doc的word文档
    file = open(full_path, 'a+', encoding='utf-8')
    # file.close()
    return file

class Param2RegParser:
	def __init__(self, excel_file, tab):
		self.excel_file = excel_file
		self.df = pd.read_excel(self.excel_file, tab)

	def get_pd(self):
		return self.df

class CodeGenerator:
	def _type(self, val_max, val_min):
		print(type(val_max)," ",type(val_min))
		if val_max == 1 and val_min == 0:
			# return "bool"
			return "CVI_U8"
		elif val_min < 0:
			return "CVI_S" + str(math.ceil(math.ceil(math.log2(max(val_max+1, -val_min) + 1)+1) / 8) * 8)
		else:
			return "CVI_U" + str(math.ceil(math.ceil(math.log2(val_max + 1)) / 8) * 8)

	def format(self, df, module, chapter_x=None):
		print("-"*10 + " start " + type(self).__name__ + "-"*10)

		# only process rows if API parameter has value
		# self._format(df.loc[~df["API parameter"].isnull()], module, tab)
		if chapter_x == None:
			self._format(df, module)
		else:
			self._format(df, module, chapter_x)

		print("-"*10 + " end " + type(self).__name__ + "-"*10)

class CodeGenerator_cvi_comm_isp_h(CodeGenerator):
	def __init__(self, time):
		# self.tempfile = "template/sample.c"
		# with open(self.tempfile) as file:
		#     self.template = Template(file.read())
		self.time = time
		template_separator = \
"""
//-----------------------------------------------------------------------------
//  {{module}}
//-----------------------------------------------------------------------------
"""
		template_manual = \
"""
typedef struct _ISP_{{tab}}_MANUAL_ATTR_S {
	{%- for _, row in df.iterrows() %}
{%- if ("CVI_BOOL" in row["Type"]) or ("CVI_S" in row["Type"]) %}
	{{row["Type"]}} {{row["API parameter"]}}; /*RW; Range:[{{"%d"%(row['low value']|int)}}, {{"%d"%(row['high value']|int)}}]*/
{%- else %}
	{{row["Type"]}} {{row["API parameter"]}}; /*RW; Range:[{{"0x%x"%(row['low value']|int)}}, {{"0x%x"%(row['high value']|int)}}]*/
{%- endif %}
{%- if IsAddParamDescription %}
	 /* {{row['parameter description use']}} */
{%- endif %}
	{%- endfor %}
} ISP_{{tab}}_MANUAL_ATTR_S;
"""
		template_auto = \
"""
typedef struct _ISP_{{tab}}_AUTO_ATTR_S {
	{%- for _, row in df.iterrows() %}
{%- if ("CVI_BOOL" in row["Type"]) or ("CVI_S" in row["Type"]) %}
	{{row["Type"]}} {{row["API parameter"]}}[ISP_AUTO_ISO_STRENGTH_NUM]; /*RW; Range:[{{"%d"%(row['low value']|int)}}, {{"%d"%(row['high value']|int)}}]*/
{%- else %}
	{{row["Type"]}} {{row["API parameter"]}}[ISP_AUTO_ISO_STRENGTH_NUM]; /*RW; Range:[{{"0x%x"%(row['low value']|int)}}, {{"0x%x"%(row['high value']|int)}}]*/
{%- endif %}
{%- if IsAddParamDescription %}
	 /* {{row['parameter description use']}} */
{%- endif %}
	{%- endfor %}
} ISP_{{tab}}_AUTO_ATTR_S;
"""
		template_auto_ccm = \
"""
typedef struct _ISP_{{tab}}_AUTO_ATTR_S {
	{%- for _, row in df.iterrows() %}
{%- if ("CVI_BOOL" in row["Type"]) or ("CVI_S" in row["Type"]) %}
	{{row["Type"]}} {{row["API parameter"]}}; /*RW; Range:[{{"%d"%(row['low value']|int)}}, {{"%d"%(row['high value']|int)}}]*/
{%- else %}
	{{row["Type"]}} {{row["API parameter"]}}; /*RW; Range:[{{"0x%x"%(row['low value']|int)}}, {{"0x%x"%(row['high value']|int)}}]*/
{%- endif %}
{%- if IsAddParamDescription %}
	 /* {{row['parameter description use']}} */
{%- endif %}
	{%- endfor %}
} ISP_{{tab}}_AUTO_ATTR_S;
"""
		template = \
"""
typedef struct _ISP_{{tab}}_ATTR_S {
{%- if hasAuto0 %}
	{%- for _, row in df.iterrows() %}
{%- if ("CVI_BOOL" in row["Type"]) or ("CVI_S" in row["Type"]) %}
	{{row["Type"]}} {{row["API parameter"]}}; /*RW; Range:[{{"%d"%(row['low value']|int)}}, {{"%d"%(row['high value']|int)}}]*/
{%- else %}
	{{row["Type"]}} {{row["API parameter"]}}; /*RW; Range:[{{"0x%x"%(row['low value']|int)}}, {{"0x%x"%(row['high value']|int)}}]*/
{%- endif %}
{%- if IsAddParamDescription %}
	 /* {{row['parameter description use']}} */
{%- endif %}
	{%- endfor %}
{%- endif %}
{%- if hasAuto1 %}
	ISP_OP_TYPE_E enOpType;
	ISP_{{tab}}_MANUAL_ATTR_S stManual;
	ISP_{{tab}}_AUTO_ATTR_S stAuto;
{%- endif %}
} ISP_{{tab}}_ATTR_S;
"""
		self.template = Template(template)
		self.template_auto = Template(template_auto)
		self.template_manual = Template(template_manual)
		self.template_separator = Template(template_separator)
		self.template_auto_ccm = Template(template_auto_ccm)

	def _format(self, df, module, _chapter_tmp):
		print('create file:cvi_comm_isp.h')
		txt_file = text_create('cvi_comm_isp' + self.time + '.h')
		print(self.template_separator.render(module=module))
		txt_file.write(self.template_separator.render(module=module))

		_chapter_tmp["ch_tl3_3"]["section_datastruct_list"] = []

		for api in df['API name'].unique():
			if str(api) == 'nan':
				continue
			tab = re.search("CVI_ISP_(Set|Get|Query)(.*)$", api).group(2)
			if('Attr' in api):
				tab = tab.replace('Attr','')
			df2 = df[df['API name'] == api]

			has_auto0 = len(df2.loc[df2["auto"]==0].index) > 0
			has_auto1 = len(df2.loc[df2["auto"]==1].index) > 0
			
			if has_auto1:
				if tab == "CCM":
					txt_file.write(self.template_manual.render(df=df2.loc[df2["auto"] == 2], module=module, tab=tab) + "\n")
					txt_file.write(self.template_auto_ccm.render(df=df2.loc[df2["auto"] == 1], module=module, tab=tab) + "\n")
				else:
					#print(self.template_manual.render(df=df2.loc[df2["auto"]==1], module=module, tab=tab))
					txt_file.write(self.template_manual.render(df=df2.loc[df2["auto"]==1], module=module, tab=tab) + "\n")
					#print(self.template_auto.render(df=df2.loc[df2["auto"]==1], module=module, tab=tab))
					txt_file.write(self.template_auto.render(df=df2.loc[df2["auto"]==1], module=module, tab=tab) + "\n")
			#print(self.template.render(df=df2.loc[df2["auto"]==0], module=module, tab=tab, hasAuto0=has_auto0, hasAuto1=has_auto1))
			txt_file.write(self.template.render(df=df2.loc[df2["auto"]==0], module=module, tab=tab, hasAuto0=has_auto0, hasAuto1=has_auto1) + "\n")
			datastruct_list = [
				{
					"style": "List Bullet",
					"link": "",
					"content": ["ISP_" + tab + "_MANUAL_ATTR_S", "定义数据格式"]
				},
				{
					"style": "List Bullet",
					"link": "",
					"content": ["ISP_" + tab + "_AUTO_ATTR_S", "定义数据格式"]
				},
				{
					"style": "List Bullet",
					"link": "",
					"content": ["ISP_" + tab + "_ATTR_S", "定义数据格式"]
				}
			]
			_chapter_tmp["ch_tl3_3"]["section_datastruct_list"].append(datastruct_list[0])
			_chapter_tmp["ch_tl3_3"]["section_datastruct_list"].append(datastruct_list[1])
			_chapter_tmp["ch_tl3_3"]["section_datastruct_list"].append(datastruct_list[2])
			_datastruct_manual = copy.deepcopy(_chapter_tmp["ch_tl3_3"]["section_datastructs"][0])
			_datastruct_manual["title"] = "ISP_" + tab + "_MANUAL_ATTR_S"
			_datastruct_manual["detail"]["定义"]["content"] = self.template_manual.render(df=df2.loc[df2["auto"] == 1], module=module, tab=tab)
			_datastruct_auto = copy.deepcopy(_chapter_tmp["ch_tl3_3"]["section_datastructs"][0])
			_datastruct_auto["title"] = "ISP_" + tab + "_AUTO_ATTR_S"
			_datastruct_auto["detail"]["定义"]["content"] = self.template_auto.render(df=df2.loc[df2["auto"] == 1], module=module, tab=tab)
			_datastruct = copy.deepcopy(_chapter_tmp["ch_tl3_3"]["section_datastructs"][0])
			_datastruct["title"] = "ISP_" + tab + "_ATTR_S"
			_datastruct["detail"]["定义"]["content"] = self.template.render(df=df2.loc[df2["auto"] == 0], module=module, tab=tab)
			_table_manual = [["成员名称", "描述"],]
			_table_auto = [["成员名称", "描述"],]
			_table = [["成员名称", "描述"],]
			haveManualAutoParam = False
			haveCommonParam = False
			for _, row in df2.loc[df2["auto"]==1].iterrows():
				#print(row["API parameter"], "  ",row["parameter description"])
				if(pd.isna(row["low value"]) or pd.isna(row["high value"])):
					range = ''
				else:
					if("CVI_BOOL" in row["Type"]):
						range = "\n取值范围： [%d, %d]" %(int(row["low value"]), int(row["high value"])) + "\n数据类型： %s" %(row["Type"])
					else:
						range = "\n取值范围： [0x%x, 0x%x]" %(int(row["low value"]), int(row["high value"])) + "\n数据类型： %s" %(row["Type"])
				_table_manual.append([row["API parameter"], row["parameter description"] + range])
				_table_auto.append([row["API parameter"], row["parameter description"] + range])
				haveManualAutoParam = True
			for _, row in df2.loc[df2["auto"]==0].iterrows():
				if(pd.isna(row["low value"]) or pd.isna(row["high value"])):
					range = ''
				else:
					if("CVI_BOOL" in row["Type"]):
						range = "\n取值范围： [%d, %d]" %(int(row["low value"]), int(row["high value"])) + "\n数据类型： %s" %(row["Type"])
					else:
						range = "\n取值范围： [0x%x, 0x%x]" %(int(row["low value"]), int(row["high value"])) + "\n数据类型： %s" %(row["Type"])
				_table.append([row["API parameter"], row["parameter description"] + range])
				haveCommonParam = True
			_table.append(["enOpType", "选择手动或自动模式"])
			_table.append(["stManual", "手动参数"])
			_table.append(["stAuto", "自动参数"])
			if(haveManualAutoParam):
				_datastruct_manual["detail"]["成员"]["table"] = _table_manual
				_datastruct_auto["detail"]["成员"]["table"] = _table_auto
				_chapter_tmp["ch_tl3_3"]["section_datastructs"].append(_datastruct_manual)
				_chapter_tmp["ch_tl3_3"]["section_datastructs"].append(_datastruct_auto)
			if (haveCommonParam):
				_datastruct["detail"]["成员"]["table"] = _table
				_chapter_tmp["ch_tl3_3"]["section_datastructs"].append(_datastruct)
		_chapter_tmp["ch_tl3_3"]["section_datastructs"].pop(0)
		return

class CodeGenerator_mpi_isp_h(CodeGenerator):
	def __init__(self):
		template_set = """CVI_S32 CVI_ISP_Set{{tab}}Attr(VI_PIPE ViPipe, const ISP_{{tab}}_ATTR_S *pst{{tab}}Attr);"""
		template_get = """CVI_S32 CVI_ISP_Get{{tab}}Attr(VI_PIPE ViPipe, ISP_{{tab}}_ATTR_S *pst{{tab}}Attr);"""
		self.template_set = Template(template_set)
		self.template_get = Template(template_get)

	def _format(self, df, module, _chapter_tmp):
		print('module name: ' + module)
		txt_file = text_create('mpi_isp_' + module + '.h')
		#df = df[df["UI Level"].isin([0, 1])]
		for api in df['API name'].unique():
			print("wangliang mark",api)
			tab = re.search("CVI_ISP_(Set|Get|Query)(.*)(Attr)?$", api).group(2)
			print( self.template_set.render(df=df, module=module, tab=tab))
			print( self.template_get.render(df=df, module=module, tab=tab))
			txt_file.write(self.template_set.render(df=df, module=module, tab=tab) + "\n")
			txt_file.write(self.template_get.render(df=df, module=module, tab=tab) + "\n")
		#
		#
		_chapter_tmp["ch_tl3_2"]["section_api_list"] = []
		for api in df['API name'].unique():
			tab = re.search("CVI_ISP_(Set|Get|Query)(.*)(Attr)?$", api).group(2)
			#
			# template for chapter_section2: api list
			tmp1 = {
				"style": "List Bullet",
				"link": "",
				"content": ["CVI_ISP_Set" + tab + "Attr", "设置" + tab + "属性参数"]
			}
			tmp2 = {
				"style": "List Bullet",
				"link": "",
				"content": ["CVI_ISP_Get" + tab + "Attr", "获取" + tab + "属性参数"]
			}
			#
			#
			_chapter_tmp["ch_tl3_2"]["section_api_list"].append(tmp1)
			_chapter_tmp["ch_tl3_2"]["section_api_list"].append(tmp2)

		#_chapter_tmp["ch_tl3_2"]["section_apis"] = []
		for api in df['API name'].unique():
			tab = re.search("CVI_ISP_(Set|Get|Query)(.*)(Attr)?$", api).group(2)
			_des_dt_set = copy.deepcopy(_chapter_tmp["ch_tl3_2"]["section_apis"][0])
			_des_dt_get = copy.deepcopy(_chapter_tmp["ch_tl3_2"]["section_apis"][0])
			#
			#Set API
			_des_dt_set["title"] = "CVI_ISP_Set" + tab + "Attr"
			_des_dt_set["detail"]["描述"]["content"] = "设置 " + tab + " 属性参数"
			_des_dt_set["detail"]["语法"]["content"] = self.template_set.render(df=df, module=module, tab=tab)
			_des_dt_set["detail"]["语法"]["link"] = "ISP_" + tab + "_ATTR_S"
			_des_dt_set["detail"]["参数"]["table"].pop(2)
			_des_dt_set["detail"]["参数"]["table"].insert(2, ["pst" + tab + "Attr", "pst" + tab +"Attr 属性参数", "输入"])
			_des_dt_set["detail"]["相关主题"][0]["content"] = "CVI_ISP_Get" + tab + "Attr"
			# _des_dt_set["detail"]["相关主题"]["content"] = "CVI_ISP_Get" + tab + "Attr"
			#
			#Get API
			_des_dt_get["title"] = "CVI_ISP_Get" + tab + "Attr"
			_des_dt_get["detail"]["描述"]["content"] = "获取 " + tab + " 属性参数"
			_des_dt_get["detail"]["语法"]["content"] = self.template_get.render(df=df, module=module, tab=tab)
			_des_dt_get["detail"]["语法"]["link"] = "ISP_" + tab + "_ATTR_S"
			_des_dt_get["detail"]["参数"]["table"].pop(2)
			_des_dt_get["detail"]["参数"]["table"].insert(2, ["pst" + tab + "Attr", "pst" + tab + "Attr 属性参数", "输出"])
			_des_dt_get["detail"]["相关主题"][0]["content"] = "CVI_ISP_Set" + tab + "Attr"
			# _des_dt_get["detail"]["相关主题"]["content"] = "CVI_ISP_Set" + tab + "Attr"
			#
			#
			_chapter_tmp["ch_tl3_2"]["section_apis"].append(_des_dt_set)
			_chapter_tmp["ch_tl3_2"]["section_apis"].append(_des_dt_get)
		_chapter_tmp["ch_tl3_2"]["section_apis"].pop(0)

		return

class CodeGenerator_isp_param_c(CodeGenerator):
	def __init__(self):
		template = """
	[ISP_IQ_BLOCK_{{module|upper}}] =
		{
			.paramToReg = isp_{{module|lower}}_param_reg,
			.regFlush = isp_iqBlock_reg_flush,
			.ispCfgInvalid = 0,
		},
"""
		template_static_cfg = """
struct cvi_vip_isp_{{module|lower}}_config {{module|lower}}Cfg;	
"""
		template_param_reg = """
CVI_S32 isp_{{module|lower}}_param_reg(VI_PIPE ViPipe, CVI_S32 iso, CVI_S32 colorTemp, ISP_IQ_BLOCK_LIST_E blockIdx)
{
	u8 opType = 0;

	// non-auto
{%- for _, row in df.loc[df["auto"]==0].iterrows() %}
	isp_param_get(ViPipe, {{row["isp_param_def.h"]}}, (void *)&({{module|lower}}Cfg.{{row["cvi_vip_tun_cfg.h"]}}));
{%- endfor %}

	// manual
{%- for _, row in df.loc[df["auto"]==1].iterrows() %}
	isp_param_get(ViPipe, {{row["isp_param_def.h"]}}, (void *)&({{module|lower}}Cfg.{{row["cvi_vip_tun_cfg.h"]}}));
{%- endfor %}

	// auto
	// TODO implement this

	// fixed
{%- for _, row in df.loc[df["auto"]==9].iterrows() %}
	{{module|lower}}Cfg.{{row["cvi_vip_tun_cfg.h"]}} = {{row["default value"]}};
{%- endfor %}

	return 0;
}
"""
		self.template = Template(template)
		self.template_param_reg = Template(template_param_reg)
		self.template_static_cfg = Template(template_static_cfg)

	def _format(self, df, module):
		txt_file_name = 'isp_param_' + module + '.c'
		print(txt_file_name)
		txt_file = text_create(txt_file_name)
		# df = df[df["UI Level"].isin([0, 1])]
		# df["type"] = [self._type(row["high value"], row["low value"]) for _, row in df.iterrows()]
		df = df[df["UI Level"].isin([0,1,9])]
		#df['type'] = df.apply(lambda row: self._type(row['high value'], row['low value']), axis=1)
		df["type"] = df["Type"]
		print(self.template_static_cfg.render(df=df, module=module))
		txt_file.write(self.template_static_cfg.render(df=df, module=module))
		print(self.template.render(df=df, module=module))
		txt_file.write(self.template.render(df=df, module=module))
		print(self.template_param_reg.render(df=df.loc[~df["cvi_vip_tun_cfg.h"].isnull()], module=module))
		txt_file.write(self.template_param_reg.render(df=df.loc[~df["cvi_vip_tun_cfg.h"].isnull()], module=module))
		return

# TODO optype for set and gets
class CodeGenerator_mpi_isp_c(CodeGenerator):
	def __init__(self):
		template = """
CVI_S32 CVI_ISP_Set{{tab}}Attr(VI_PIPE ViPipe, const ISP_{{tab}}_ATTR_S *pst{{tab}}Attr)
{
	CVI_S32 i = 0;

	// non-auto attributes
{%- for _, row in df.loc[df["auto"]==0].iterrows() %}
	isp_param_set(ViPipe, {{row["isp_param_def.h"]}}, pst{{tab}}Attr->{{row["API parameter"]}});
{%- endfor %}

	// manual attributes
{%- for _,row in df.loc[df["auto"]==1].iterrows() %}
	isp_param_set(ViPipe, {{row["isp_param_def.h"]}}, pst{{tab}}Attr->stManual.{{row["API parameter"]}});
{%- endfor %}

	// auto attributes
	for (i = 0; i < ISP_AUTO_ISO_STRENGTH_NUM; i++) {
{%- for _,row in df.loc[df["auto"]==1].iterrows() %}
		isp_param_set(ViPipe, {{row["isp_param_def.h"]|replace("_MANUAL_", "_AUTO_")}} + i, pst{{tab}}Attr->stAuto.{{row["API parameter"]}}[i]);
{%- endfor %}
	}

	isp_iq_invalid_set(ViPipe, ISP_IQ_BLOCK_{{module|upper}});
}

CVI_S32 CVI_ISP_Get{{tab}}Attr(VI_PIPE ViPipe, ISP_{{tab}}_ATTR_S *pst{{tab}}Attr)
{
	CVI_S32 i = 0;

	// non-auto parameters
{%- for _, row in df.loc[df["auto"]==0].iterrows() %}
	isp_param_get(ViPipe, {{row["isp_param_def.h"]}}, (void *)&(pst{{tab}}Attr->{{row["API parameter"]}}));
{%- endfor %}

	// manual parameters
{%- for _, row in df.loc[df["auto"]==1].iterrows() %}
	isp_param_get(ViPipe, {{row["isp_param_def.h"]}}, (void *)&(pst{{tab}}Attr->stManual.{{row["API parameter"]}}));
{%- endfor %}

	// auto parameters
	for (i = 0; i < ISP_AUTO_ISO_STRENGTH_NUM; i++) {
{%- for _, row in df.loc[df["auto"]==1].iterrows() %}
		isp_param_get(ViPipe, {{row["isp_param_def.h"]|replace("_MANUAL_", "_AUTO_")}} + i, (void *)&(pst{{tab}}Attr->stAuto.{{row["API parameter"]}}[i]));
{%- endfor %}
	}

	// TODO should add fixed parameters?
}
"""
		# TODO parse array

		self.template = Template(template)

	def _format(self, df, module):
		df = df[df["UI Level"].isin([0, 1])]
		#df['type'] = df.apply(lambda row: self._type(row['high value'], row['low value']), axis=1)
		df["type"] = df["Type"]
		# df = df[df["UI Level"].isin([0, 1])]
		# print(self.template.render(df=df, module=module))
		# print(self.template_param_reg.render(df=df, module=module))
		# return

		df = df[df["UI Level"].isin([0, 1])]
		#df["type"] = [self._type(row["high value"], row["low value"]) for _, row in df.iterrows()]
		df["type"] = df["Type"]
		print('module name: ' + module)
		txt_file = text_create('mpi_isp_' + module + '.c')
		for api in df['API name'].unique():
			tab = re.search("CVI_ISP_(Set|Get|Query)(.*)(Attr)?$", api).group(2)
			df2 = df[df['API name'] == api]
			print(self.template.render(df=df2, module=module, tab=tab))
			txt_file.write(self.template.render(df=df2, module=module, tab=tab))
		return
class CodeGenerator_param_check_c(CodeGenerator):
	def __init__(self, time):
		self.time = time
		self.dataType = {"CVI_U8":{"low value":0,"high value":255,"prtStr":"0x%x"},
						 "CVI_U16":{"low value":0,"high value":65535,"prtStr":"0x%x"},
						 "CVI_BOOL": {"low value": 0, "high value": 1,"prtStr":"%d"},
						 "CVI_S16": {"low value": -32768, "high value": 32767,"prtStr":"%d"},
						 "CVI_U32": {"low value": 0, "high value": 4294967295,"prtStr":"0x%lx"},
						 "CVI_S32": {"low value": -2147483648, "high value": 2147483647,"prtStr":"%d"},
						 }
		template = """
{%- set ns = namespace(isNeedPrintAno=1, isNeedPrintSubAno=1, isNeedPrintRightBracket=0, isNeedPrintSubRightBracket=0) %}
CVI_S32 CVI_ISP_Set{{tab}}Attr(VI_PIPE ViPipe, const ISP_{{tab}}_ATTR_S *pst{{tab}}Attr)
{
	CVI_U16 i = 0;

{#- ----------------------------common:variable-----------------------------------#}
{%- if df.loc[df["auto"]==1].shape[0] > 0 %}
{%- if ns.isNeedPrintAno == 1 %}
	//common: check variable range
{%- set ns.isNeedPrintAno = 0 %}
{%- endif %}
	if (pst{{tab}}Attr->enOpType >= OP_TYPE_BUTT) {
		ISP_DEBUG(LOG_ERR, "Invalid enOpType %d", pst{{tab}}Attr->enOpType);
{%- if debug != 1 %}
		return CVI_FAILURE_ILLEGAL_PARAM;
{%- endif %}
	}
{%- endif %}
{%- for _,row in df.loc[df["auto"]==0].iterrows() %}
{%- if '[' not in row["API parameter"] and (row["Type"] in datatype)%}
{%- if (row["high value"] != datatype[row["Type"]]["high value"]) and (row["low value"] != datatype[row["Type"]]["low value"]) %}
{%- if ns.isNeedPrintAno == 1 %}
	//common: check variable range
{%- set ns.isNeedPrintAno = 0 %}
{%- endif %}
	if (pst{{tab}}Attr->{{row["API parameter"]}} < {{datatype[row["Type"]]["prtStr"]%(row['low value']|int)}} || pst{{tab}}Attr->{{row["API parameter"]}} > {{datatype[row["Type"]]["prtStr"]%(row['high value']|int)}}) {
		ISP_DEBUG(LOG_ERR, "Invalid pst{{tab}}Attr->{{row["API parameter"]}} {{datatype[row["Type"]]["prtStr"]}}",
			pst{{tab}}Attr->{{row["API parameter"]}});
{%- if debug != 1 %}
		return CVI_FAILURE_ILLEGAL_PARAM;
{%- endif %}
	}
{%- elif (row["high value"] != datatype[row["Type"]]["high value"]) and (row["low value"] == datatype[row["Type"]]["low value"]) %}
{%- if ns.isNeedPrintAno == 1 %}
	//common: check variable range
{%- set ns.isNeedPrintAno = 0 %}
{%- endif %}
	if (pst{{tab}}Attr->{{row["API parameter"]}} > {{datatype[row["Type"]]["prtStr"]%(row['high value']|int)}}) {
		ISP_DEBUG(LOG_ERR, "Invalid pst{{tab}}Attr->{{row["API parameter"]}} {{datatype[row["Type"]]["prtStr"]}}",
			pst{{tab}}Attr->{{row["API parameter"]}});
{%- if debug != 1 %}
		return CVI_FAILURE_ILLEGAL_PARAM;
{%- endif %}
	}
{%- endif %}
{%- endif %}
{%- endfor %}
{#- ----------------------------common:variable array-----------------------------------#}
{%- set ns.isNeedPrintAno=1 %}
{%- for _,row in df.loc[df["auto"]==0].iterrows() %}
{%- if '[' in row["API parameter"] and (row["Type"] in datatype) %}
{%- set value = re.search(".*\[(.*)\]", row["API parameter"]).group(1) %}
{%- set paramName = re.search("(.*)\[.*", row["API parameter"]).group(1) %}
{%- if (row["high value"] != datatype[row["Type"]]["high value"]) and (row["low value"] != datatype[row["Type"]]["low value"]) %}
{%- if ns.isNeedPrintAno == 1 %}
	//common: check variable array range
	for (i = 0; i < ; ++i) {
{%- set ns.isNeedPrintAno = 0 %}
{%- set ns.isNeedPrintRightBracket = 1%}
{%- endif %}
		if ((i < {{value}}) && (pst{{tab}}Attr->{{paramName}}[i] < {{datatype[row["Type"]]["prtStr"]%(row['low value']|int)}} || (pst{{tab}}Attr->{{paramName}}[i] > {{datatype[row["Type"]]["prtStr"]%(row['high value']|int)}})) {
			ISP_DEBUG(LOG_ERR, "Invalid pst{{tab}}Attr->{{paramName}}[%d] {{datatype[row["Type"]]["prtStr"]}}",
				i, pst{{tab}}Attr->{{paramName}}[i]);
{%- if debug != 1 %}
			return CVI_FAILURE_ILLEGAL_PARAM;
{%- endif %}
		}
{%- elif (row["high value"] != datatype[row["Type"]]["high value"]) and (row["low value"] == datatype[row["Type"]]["low value"]) %}
{%- if ns.isNeedPrintAno == 1 %}
	//common: check variable array range
	for (i = 0; i < ; ++i) {
{%- set ns.isNeedPrintAno = 0 %}
{%- set ns.isNeedPrintRightBracket = 1%}
{%- endif %}
		if ((i < {{value}}) && (pst{{tab}}Attr->{{paramName}}[i] > {{datatype[row["Type"]]["prtStr"]%(row['high value']|int)}})) {
			ISP_DEBUG(LOG_ERR, "Invalid pst{{tab}}Attr->{{paramName}}[%d] {{datatype[row["Type"]]["prtStr"]}}",
				i, pst{{tab}}Attr->{{paramName}}[i]);
{%- if debug != 1 %}
			return CVI_FAILURE_ILLEGAL_PARAM;
{%- endif %}
		}
{%- endif %}
{%- endif %}
{%- endfor %}
{%- if ns.isNeedPrintRightBracket == 1 %}
	}
{%- set ns.isNeedPrintRightBracket = 0 %}
{%- endif %}
{#- ----------------------------manual:variable-----------------------------------#}
{%- set ns.isNeedPrintAno = 1 %}
{%- for _,row in df.loc[df["auto"]==1].iterrows() %}
{%- if '[' not in row["API parameter"] and (row["Type"] in datatype) %}
{%- if (row["high value"] != datatype[row["Type"]]["high value"]) and (row["low value"] != datatype[row["Type"]]["low value"]) %}
{%- if ns.isNeedPrintAno == 1 %}
	//manual: check variable range
{%- set ns.isNeedPrintAno = 0 %}
{%- endif %}
	if (pst{{tab}}Attr->stManual.{{row["API parameter"]}} < {{datatype[row["Type"]]["prtStr"]%(row['low value']|int)}} || pst{{tab}}Attr->stManual.{{row["API parameter"]}} > {{datatype[row["Type"]]["prtStr"]%(row['high value']|int)}}) {
		ISP_DEBUG(LOG_ERR, "Invalid pst{{tab}}Attr->stManual.{{row["API parameter"]}} {{datatype[row["Type"]]["prtStr"]}}",
			pst{{tab}}Attr->stManual.{{row["API parameter"]}});
{%- if debug != 1 %}
		return CVI_FAILURE_ILLEGAL_PARAM;
{%- endif %}
	}
{%- elif (row["high value"] != datatype[row["Type"]]["high value"]) and (row["low value"] == datatype[row["Type"]]["low value"]) %}
{%- if ns.isNeedPrintAno == 1 %}
	//manual: check variable range
{%- set ns.isNeedPrintAno = 0 %}
{%- endif %}
	if (pst{{tab}}Attr->stManual.{{row["API parameter"]}} > {{datatype[row["Type"]]["prtStr"]%(row['high value']|int)}}) {
		ISP_DEBUG(LOG_ERR, "Invalid pst{{tab}}Attr->stManual.{{row["API parameter"]}} {{datatype[row["Type"]]["prtStr"]}}",
			pst{{tab}}Attr->stManual.{{row["API parameter"]}});
{%- if debug != 1 %}
		return CVI_FAILURE_ILLEGAL_PARAM;
{%- endif %}
	}
{%- endif %}
{%- endif %}
{%- endfor %}
{#- ----------------------------manual:variable array-----------------------------------#}
{%- set ns.isNeedPrintAno = 1 %}
{%- for _,row in df.loc[df["auto"]==1].iterrows() %}
{%- if '[' in row["API parameter"] and (row["Type"] in datatype) %}
{%- set value = re.search(".*\[(.*)\]", row["API parameter"]).group(1) %}
{%- set paramName = re.search("(.*)\[.*", row["API parameter"]).group(1) %}
{%- if (row["high value"] != datatype[row["Type"]]["high value"]) and (row["low value"] != datatype[row["Type"]]["low value"]) %}
{%- if ns.isNeedPrintAno == 1 %}
	//manual: check variable array range
	for (i = 0; i < ; ++i) {
{%- set ns.isNeedPrintAno = 0 %}
{%- set ns.isNeedPrintRightBracket = 1 %}
{%- endif %}
		if ((i < {{value}}) && (pst{{tab}}Attr->stManual.{{paramName}}[i] < {{datatype[row["Type"]]["prtStr"]%(row['low value']|int)}} || pst{{tab}}Attr->stManual.{{paramName}}[i] > {{datatype[row["Type"]]["prtStr"]%(row['high value']|int)}})) {
			ISP_DEBUG(LOG_ERR, "Invalid pst{{tab}}Attr->stManual.{{paramName}}[%d] {{datatype[row["Type"]]["prtStr"]}}",
				i, pst{{tab}}Attr->stManual.{{paramName}}[i]);
{%- if debug != 1 %}
			return CVI_FAILURE_ILLEGAL_PARAM;
{%- endif %}
		}
{%- elif (row["high value"] != datatype[row["Type"]]["high value"]) and (row["low value"] == datatype[row["Type"]]["low value"]) %}
{%- if ns.isNeedPrintAno == 1 %}
	//manual: check variable array range
	for (i = 0; i < ; ++i) {
{%- set ns.isNeedPrintAno = 0 %}
{%- set ns.isNeedPrintRightBracket = 1 %}
{%- endif %}
		if ((i < {{value}}) && (pst{{tab}}Attr->stManual.{{paramName}}[i] > {{datatype[row["Type"]]["prtStr"]%(row['high value']|int)}})) {
			ISP_DEBUG(LOG_ERR, "Invalid pst{{tab}}Attr->stManual.{{paramName}}[%d] {{datatype[row["Type"]]["prtStr"]}}",
				i, pst{{tab}}Attr->stManual.{{paramName}}[i]);
{%- if debug != 1 %}
			return CVI_FAILURE_ILLEGAL_PARAM;
{%- endif %}
		}
{%- endif %}
{%- endif %}
{%- endfor %}
{%- if ns.isNeedPrintRightBracket == 1 %}
	}
{%- set ns.isNeedPrintRightBracket = 0 %}
{%- endif %}
{#- ----------------------------auto:variable + variable array-----------------------------------#}
{%- set ns.isNeedPrintAno = 1 %}
{%- set ns.isNeedPrintSubAno = 1 %}
{%- for _,row in df.loc[df["auto"]==1].iterrows() %}
{%- if '[' not in row["API parameter"] and (row["Type"] in datatype) %}
{%- if (row["high value"] != datatype[row["Type"]]["high value"]) and (row["low value"] != datatype[row["Type"]]["low value"]) %}
{%- if ns.isNeedPrintAno == 1 %}
	for (i = 0; i < ISP_AUTO_ISO_STRENGTH_NUM; ++i) {
{%- set ns.isNeedPrintAno = 0 %}
{%- set ns.isNeedPrintRightBracket = 1 %}
{%- endif %}
{%- if ns.isNeedPrintSubAno == 1 %}
		//auto: check variable range
{%- set ns.isNeedPrintSubAno = 0 %}		
{%- endif %}
		if (pst{{tab}}Attr->stAuto.{{row["API parameter"]}}[i] < {{datatype[row["Type"]]["prtStr"]%(row['low value']|int)}} || pst{{tab}}Attr->stAuto.{{row["API parameter"]}}[i] > {{datatype[row["Type"]]["prtStr"]%(row['high value']|int)}}) {
			ISP_DEBUG(LOG_ERR, "Invalid pst{{tab}}Attr->stAuto.{{row["API parameter"]}}[%d] {{datatype[row["Type"]]["prtStr"]}}",
				i, pst{{tab}}Attr->stAuto.{{row["API parameter"]}}[i]);
{%- if debug != 1 %}
			return CVI_FAILURE_ILLEGAL_PARAM;
{%- endif %}
		}
{%- elif (row["high value"] != datatype[row["Type"]]["high value"]) and (row["low value"] == datatype[row["Type"]]["low value"]) %}
{%- if ns.isNeedPrintAno == 1 %}
	for (i = 0; i < ISP_AUTO_ISO_STRENGTH_NUM; ++i) {
{%- set ns.isNeedPrintAno = 0 %}
{%- set ns.isNeedPrintRightBracket = 1 %}
{%- endif %}
{%- if ns.isNeedPrintSubAno == 1 %}
		//auto: check variable range
{%- set ns.isNeedPrintSubAno = 0 %}		
{%- endif %}
		if (pst{{tab}}Attr->stAuto.{{row["API parameter"]}}[i] > {{datatype[row["Type"]]["prtStr"]%(row['high value']|int)}}) {
			ISP_DEBUG(LOG_ERR, "Invalid pst{{tab}}Attr->stAuto.{{row["API parameter"]}}[%d] {{datatype[row["Type"]]["prtStr"]}}",
				i, pst{{tab}}Attr->stAuto.{{row["API parameter"]}}[i]);
{%- if debug != 1 %}
			return CVI_FAILURE_ILLEGAL_PARAM;
{%- endif %}
		}
{%- endif %}
{%- endif %}
{%- endfor %}
{%- set ns.isNeedPrintSubAno = 1 %}
{%- for _,row in df.loc[df["auto"]==1].iterrows() %}
{%- if '[' in row["API parameter"] and (row["Type"] in datatype) %}
{%- set value = re.search(".*\[(.*)\]", row["API parameter"]).group(1) %}
{%- set paramName = re.search("(.*)\[.*", row["API parameter"]).group(1) %}
{%- if (row["high value"] != datatype[row["Type"]]["high value"]) and (row["low value"] != datatype[row["Type"]]["low value"]) %}
{%- if ns.isNeedPrintAno == 1 %}
	for (i = 0; i < ISP_AUTO_ISO_STRENGTH_NUM; ++i) {
{%- set ns.isNeedPrintAno = 0 %}
{%- set ns.isNeedPrintRightBracket = 1 %}
{%- endif %}
{%- if ns.isNeedPrintSubAno == 1 %}
		//auto: check variable array range
		CVI_U16 j = 0;

		for (j = 0; j < ; ++j) {
{%- set ns.isNeedPrintSubAno = 0 %}
{%- set ns.isNeedPrintSubRightBracket = 1 %}
{%- endif %}
			if ((j < {{value}}) && (pst{{tab}}Attr->stAuto.{{paramName}}[j][i] < {{datatype[row["Type"]]["prtStr"]%(row['low value']|int)}} || pst{{tab}}Attr->stAuto.{{paramName}}[j][i] > {{datatype[row["Type"]]["prtStr"]%(row['high value']|int)}})) {
				ISP_DEBUG(LOG_ERR, "Invalid pst{{tab}}Attr->stAuto.{{paramName}}[%d][%d] {{datatype[row["Type"]]["prtStr"]}}",
					j, i, pst{{tab}}Attr->stAuto.{{paramName}}[j][i]);
{%- if debug != 1 %}
				return CVI_FAILURE_ILLEGAL_PARAM;
{%- endif %}
			}
{%- elif (row["high value"] != datatype[row["Type"]]["high value"]) and (row["low value"] == datatype[row["Type"]]["low value"]) %}
{%- if ns.isNeedPrintAno == 1 %}
	for (i = 0; i < ISP_AUTO_ISO_STRENGTH_NUM; ++i) {
{%- set ns.isNeedPrintAno = 0 %}
{%- set ns.isNeedPrintRightBracket = 1 %}
{%- endif %}
{%- if ns.isNeedPrintSubAno == 1 %}
		CVI_U16 j = 0;
		//auto: check variable array range
		for (j = 0; j < ; ++j) {
{%- set ns.isNeedPrintSubAno = 0 %}
{%- set ns.isNeedPrintSubRightBracket = 1 %}
{%- endif %}
			if ((j < {{value}}) && (pst{{tab}}Attr->stAuto.{{paramName}}[j][i] > {{datatype[row["Type"]]["prtStr"]%(row['high value']|int)}})) {
				ISP_DEBUG(LOG_ERR, "Invalid pst{{tab}}Attr->stAuto.{{paramName}}[%d][%d] {{datatype[row["Type"]]["prtStr"]}}",
					j, i, pst{{tab}}Attr->stAuto.{{paramName}}[j][i]);
{%- if debug != 1 %}
				return CVI_FAILURE_ILLEGAL_PARAM;
{%- endif %}
			}
{%- endif %}
{%- endif %}
{%- endfor %}		
{%- if ns.isNeedPrintSubRightBracket == 1 %}
		}
{%- set ns.isNeedPrintSubRightBracket = 0 %}
{%- endif %}
{%- if ns.isNeedPrintRightBracket == 1 %}
	}
{%- set ns.isNeedPrintRightBracket = 0 %}
{%- endif %}
}
"""
		# TODO parse array
		self.template = Template(template)
	def _format(self, df, module):
		print('create file:param_check.c')
		txt_file = text_create('param_check' + self.time + '.c')
		for api in df['API name'].unique():
			tab = re.search("CVI_ISP_(Set|Get|Query)(.*)$", api).group(2)
			if ('Attr' in api):
				tab = tab.replace('Attr', '')
			df2 = df[df['API name'] == api]
			#print(df.loc[df["auto"]==1].shape[0])
			# {%- if "CVI_BOOL" in df[] %}
			# 	{{row["Type"]}} {{row["API parameter"]}}; /*RW; Range:[{{"%d"%(row['low value']|int)}}, {{"%d"%(row['high value']|int)}}]*/
			# {%- else %}
			# 	{{row["Type"]}} {{row["API parameter"]}}; /*RW; Range:[{{"0x%x"%(row['low value']|int)}}, {{"0x%x"%(row['high value']|int)}}]*/
			# {%- endif %}
			txt_file.write(self.template.render(df=df2, module=module, tab=tab, datatype=self.dataType, re=re, debug=0))
		return

def open_json(_template_json):
	#with open("./resource/template.json", "r", encoding='utf-8') as _f:
	with open(_template_json, "r", encoding='utf-8') as _f:
		_file_json = json.load(_f)
		return _file_json

excel_file = "resource/1880v2_API_parameters_to_registers_mapping_list.xlsx"
# Calibration
module_list_calibration = (
	"BlackLevel",
	"DPC",
	"MeshShading",
	"CCM",
	"Noise profile",
)
# NR
module_list_nr = (
	"BNR",
	"YNR",
	"CNR",
	"TNR",
	"Crosstalk",
)
# Constrast & EE
module_list_constrast = (
	"Demosaic",
	"Sharpen",
	"Gamma",
	"DCI",
	"Dehaze",
)
# Color
module_list_color = (
	"Hsv",
	"ColorTone",
	"Saturation",
	"CAC",
)
# 3A
module_list_3a = (
	# AE
	"ExposureAttr",
	"ExposureInfo",
	"WDRExposureAttr",
	"AE Route",

	# WB
	"WBAttr",
	"WBInfo",

	# Statiscs
	"AE statistics",
	"AWB statistics",
	"Get AWB Statistic data",
	"Get AF Statistic data",
	"HI_AE Statistic",
	"HI_AWB statistics",
	# "HI_FOCUS statistics",	# TODO
)
# ALL
module_list_all = (
	"MeshShading",
	"HSV",
	"DPC",
	"TNR",
	"CCM",
    "Gamma",
	"YNR",
	"Dehaze",
	"CNR",
	"CAC",
	"BNR",
	"Sharpen",
	"CLut",
	"BlackLevel",
	"ColorTone",
	"DCI",
	"Demosaic",
	"Crosstalk",
	"FSWDR",
	"DRC",
)
module_list_temp = (
	"MeshShading",
)
module_list = module_list_all	#2
# module_list = module_list_calibration + module_list_nr + module_list_constrast + module_list_color + module_list_3a + module_list_wdr
if __name__ == '__main__':
	json_file = open_json("./resource/template.json")
	translateObj = baiduTranslate.Translate()
	myTime =  time.strftime('%H_%M_%S', time.localtime(time.time()))
	for module in module_list:
		#Gets the processed dataframe
		useableList = ['API name','API parameter','UI Level','Type','auto']
		df = pd.read_excel(excel_file, module)
		df = df.rename(columns=lambda x: x.strip().replace('\n','').replace(".1",''))
		print(df.columns.values)
		df['API name'] = df['API name'].fillna(method='ffill')
		df = df.loc[df["UI Level"].isin([0, 1]) & ~df["API parameter"].isnull()]
		df = df.drop_duplicates(['API name','API parameter'], 'first')
		#low value, high value, default value, parameter description are duplicated in the excel, so we need to get what we need
		of = pd.DataFrame(df[useableList])
		of['low value'] = df.iloc[:, list(df.columns.values).index('low value')]
		of['high value'] = df.iloc[:, list(df.columns.values).index('high value')]
		of['default value'] = df.iloc[:, list(df.columns.values).index('default value')]
		of['parameter description'] = df.iloc[:, list(df.columns.values).index('parameter description')]
		if(IsTranslateDescription):
			of['parameter description use'] = of['parameter description'].apply(translateObj.getTranslateRes)
		else:
			pattern = re.compile('.{33}')
			of['parameter description'] = of['parameter description'].apply(lambda x: x.strip().replace('\n','').replace('\r','') if type(x) == type('') else '')
			of['parameter description use'] = of['parameter description'].apply(lambda x: '\n\t'.join([x[i:i+33] for i in range(0, len(x), 33)]))
		df = of
		df.to_csv('test2.csv', encoding='utf_8_sig')

		json_file["chapter_" + str(module_list.index(module) + 1)] = copy.deepcopy(json_file["chapter_x"])
		chapter_tmp = json_file["chapter_" + str(module_list.index(module) + 1)]
		chapter_tmp["chapter_num_main"] = module_list.index(module) + 1
		chapter_tmp["module"] = module

		CodeGenerator_cvi_comm_isp_h(myTime).format(df, module, chapter_tmp)
		#CodeGenerator_isp_param_c().format(df, module)
		CodeGenerator_param_check_c(myTime).format(df, module)
		
	json_file.pop("chapter_x")
	with open("./resource/new.json", "w", encoding='utf-8') as _json_file:
		json.dump(json_file, _json_file, ensure_ascii=False)
	print("ok")

