# import jinja2
from jinja2 import Template
import pandas as pd
import math
import re

class Param2RegParser:
	def __init__(self, excel_file, tab):
		self.excel_file = excel_file
		self.df = pd.read_excel(self.excel_file, tab)

	def get_pd(self):
		return self.df

class CodeGenerator:
	def _type(self, max, min):
		if max == 1 and min == 0:
			# return "bool"
			return "CVI_U32"
		elif min < 0:
			return "CVI_S" + str(math.ceil(math.ceil(math.log2(math.max(max+1,-min) + 1)+1) / 8) * 8)
		else:
			return "CVI_U" + str(math.ceil(math.ceil(math.log2(max + 1)) / 8) * 8)

	def format(self, df, module):
		print("-"*10 + " start " + type(self).__name__ + "-"*10);

		# only process rows if API parameter has value
		# self._format(df.loc[~df["API parameter"].isnull()], module, tab)
		self._format(df, module)

		print("-"*10 + " end " + type(self).__name__ + "-"*10);

class CodeGenerator_cvi_comm_isp_h(CodeGenerator):
	def __init__(self):
		# self.tempfile = "template/sample.c"
		# with open(self.tempfile) as file:
		#     self.template = Template(file.read())
		template_separator = """
//-----------------------------------------------------------------------------
//  {{module}}
//-----------------------------------------------------------------------------
"""
		template_manual = """
typedef struct cviISP_{{tab}}_MANUAL_ATTR_S {
	{%- for _, row in df.iterrows() %}
	{{row["type"]}} {{row["API parameter"]}};
	{%- endfor %}
} ISP_{{tab}}_MANUAL_ATTR_S;
"""

		template_auto = """
typedef struct cviISP_{{tab}}_AUTO_ATTR_S {
    {%- for _, row in df.iterrows() %}
	{{row["type"]}} {{row["API parameter"]}}[ISP_AUTO_ISO_STRENGTH_NUM];
    {%- endfor %}
} ISP_{{tab}}_AUTO_ATTR_S;
"""

		template = """
typedef struct cviISP_{{tab}}_ATTR_S {
{%- if hasAuto0 %}
    {%- for _, row in df.iterrows() %}
	{{row["type"]}} {{row["API parameter"]}};
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

	def _format(self, df, module):
		df = df.loc[df["UI Level"].isin([0, 1]) & ~df["API parameter"].isnull()]
		# .loc[~df["API parameter"].isnull()]
		# df = df[df["UI Level"].isin([0, 1])]
		df["type"] = [self._type(row["high value"], row["low value"]) for _, row in df.iterrows()]

		print(self.template_separator.render(module=module))
		for api in df['API name'].unique():
			if str(api) == 'nan':
				continue
			tab = re.search("CVI_MPI_ISP_Set(.*)(Attr)$", api).group(1)
			df2 = df[df['API name'] == api]

			has_auto0 = len(df2.loc[df2["auto"]==0].index) > 0
			has_auto1 = len(df2.loc[df2["auto"]==1].index) > 0
			if has_auto1:
				print(self.template_manual.render(df=df2.loc[df2["auto"]==1], module=module, tab=tab))
				print(self.template_auto.render(df=df2.loc[df2["auto"]==1], module=module, tab=tab))
			print(self.template.render(df=df2.loc[df2["auto"]==0], module=module, tab=tab, hasAuto0=has_auto0, hasAuto1=has_auto1))
		return

class CodeGenerator_mpi_isp_h(CodeGenerator):
	def __init__(self):
		template = """
CVI_S32 CVI_MPI_ISP_Set{{tab}}Attr(VI_PIPE ViPipe, const ISP_{{tab}}_ATTR_S *pst{{tab}}Attr);
CVI_S32 CVI_MPI_ISP_Get{{tab}}Attr(VI_PIPE ViPipe, ISP_{{tab}}_ATTR_S *pst{{tab}}Attr);
"""

		self.template = Template(template)

	def _format(self, df, module):
		# df = df[df["UI Level"].isin([0, 1])]
		for api in df['API name'].unique():
			tab = re.search("CVI_MPI_ISP_Set(.*)(Attr)$", api).group(1)
			print(self.template.render(df=df, module=module, tab=tab))
		return

class CodeGenerator_isp_param_c(CodeGenerator):
	def __init__(self):
		template = """
	[ISP_IQ_BLOCK_{{module|upper}}] =
		{
			.paramToReg = isp_{{module|lower}}_param_reg,
			.regFlush = isp_iqBlock_reg_flush,
			.ispCfgInvalid = {0},
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
		# df = df[df["UI Level"].isin([0, 1])]
		# df["type"] = [self._type(row["high value"], row["low value"]) for _, row in df.iterrows()]
		df = df[df["UI Level"].isin([0,1,9])]
		df['type'] = df.apply(lambda row: self._type(row['high value'], row['low value']), axis=1)

		print(self.template_static_cfg.render(df=df, module=module))
		print(self.template.render(df=df, module=module))
		print(self.template_param_reg.render(df=df.loc[~df["cvi_vip_tun_cfg.h"].isnull()], module=module))
		return

# TODO optype for set and gets
class CodeGenerator_mpi_isp_c(CodeGenerator):
	def __init__(self):
		template = """
CVI_S32 CVI_MPI_ISP_Set{{tab}}Attr(VI_PIPE ViPipe, const ISP_{{tab}}_ATTR_S *pst{{tab}}Attr)
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

CVI_S32 CVI_MPI_ISP_Get{{tab}}Attr(VI_PIPE ViPipe, ISP_{{tab}}_ATTR_S *pst{{tab}}Attr)
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
		df['type'] = df.apply(lambda row: self._type(row['high value'], row['low value']), axis=1)

		# df = df[df["UI Level"].isin([0, 1])]
		# print(self.template.render(df=df, module=module))
		# print(self.template_param_reg.render(df=df, module=module))
		# return

		df = df[df["UI Level"].isin([0, 1])]
		df["type"] = [self._type(row["high value"], row["low value"]) for _, row in df.iterrows()]

		for api in df['API name'].unique():
			tab = re.search("CVI_MPI_ISP_Set(.*)(Attr)$", api).group(1)
			df2 = df[df['API name'] == api]
			print(self.template.render(df=df2, module=module, tab=tab))
		return



if __name__ == '__main__':
	from jinja2 import Template
	import pandas as pd
	import math
	import re

	excel_file = "resource/1880v2_API_parameters_to_registers_mapping_list.xlsx"

	# TODO set tab name of excel file
	# module = "BNR"
	# module = "YNR"
	module = "CNR"

	#
	if 1:
		df = pd.read_excel(excel_file, module)
		df = df.rename(columns=lambda x: x.strip().replace("\n", ""))
		df['API name'] = df['API name'].fillna(method='ffill')     # must before apply strip()
		df = df.apply(lambda x: x.str.strip() if x.dtype == "object" else x)

		if 1:
			# define parameter
			f = CodeGenerator_cvi_comm_isp_h()
			f.format(df, module)

		if 1:
			# parameter2reg
			f = CodeGenerator_isp_param_c()
			f.format(df, module)

		if 1:
			# define mpi
			f = CodeGenerator_mpi_isp_h()
			f.format(df, module)

		if 1:
			# implement mpi
			f = CodeGenerator_mpi_isp_c()
			f.format(df, module)

	print("ok")
