#!/usr/bin/env python3
# -*- coding: utf-8 -*-
__author__ = ''

#导入所需要的modul
from docx import Document
from docx.enum.text import WD_ALIGN_PARAGRAPH
from docx.enum.table import WD_TABLE_ALIGNMENT
from docx.shared import RGBColor
from docx.shared import Inches,Pt
from docx.shared import Cm
from docx.oxml.ns import qn
from docx.oxml.ns import nsdecls
from docx.oxml import parse_xml


software_docx = u'WPS' #WPS or WORD2016
doc_r_ver = u'v1.0'
doc_r_date = u'2019/12/31'


def cvi_doc_addheading(_doc, _level, _str):
    print('cvi_doc_addheading:', _str)
    _run = _doc.add_heading(level=_level).add_run(_str)
    _run.font.name = u'Microsoft JhengHei'
    _run._element.rPr.rFonts.set(qn('w:eastAsia'), u'Microsoft JhengHei')
	#小二：18，三号：16，四号：14，小四：12
    if _level == 0:
        _run.font.size = Pt(32)
    elif _level == 1:
        _run.font.size = Pt(18)
    elif _level == 2:
        _run.font.size = Pt(16)
    elif _level == 3:
        _run.font.size = Pt(14)
    else:
        _run.font.size = Pt(11)

def cvi_doc_addparagraph(_doc, _left_indent, _str, _style = None):
    print('cvi_doc_addparagraph:', _str)
    if _style != None:
        _paragraph = _doc.add_paragraph(_str, style = _style)
    else:
        _paragraph = _doc.add_paragraph(_str)
    _paragraph.paragraph_format.left_indent = _left_indent


def cvi_doc_addtable(_docx, _records):
    print('Add table size =[%d][%d]' % (len(_records), len(_records[0])))
    _row = len(_records)
    _col = len(_records[0])
    table = _docx.add_table(rows=_row, cols=_col, style="Table Grid")
    table.alignment = WD_TABLE_ALIGNMENT.RIGHT
    #table.alignment = WD_TABLE_ALIGNMENT.CENTER
    #table.alignment = WD_TABLE_ALIGNMENT.LEFT
    table.autofit = False
    # document.styles['Table Grid'].paragraph_format.left_indent = Cm(2)
    # table.cell(row, col).width = Cm(4) #表格宽度，该方法对 WPS 无效，对 Word2016 有效
    # table.rows[0].height = Cm(12) #表格高度
    if _col == 4:
        table.alignment = WD_TABLE_ALIGNMENT.CENTER
        table.columns[0].width = Cm(2.6)
        table.columns[1].width = Cm(2.6)
        table.columns[2].width = Cm(7.5)
        table.columns[3].width = Cm(2.6)
        shading_elm_0 = parse_xml(r'<w:shd {} w:fill="C0C0C0"/>'.format(nsdecls('w')))
        shading_elm_1 = parse_xml(r'<w:shd {} w:fill="C0C0C0"/>'.format(nsdecls('w')))
        shading_elm_2 = parse_xml(r'<w:shd {} w:fill="C0C0C0"/>'.format(nsdecls('w')))
        shading_elm_3 = parse_xml(r'<w:shd {} w:fill="C0C0C0"/>'.format(nsdecls('w')))
        table.rows[0].cells[0]._tc.get_or_add_tcPr().append(shading_elm_0)
        table.rows[0].cells[1]._tc.get_or_add_tcPr().append(shading_elm_1)
        table.rows[0].cells[2]._tc.get_or_add_tcPr().append(shading_elm_2)
        table.rows[0].cells[3]._tc.get_or_add_tcPr().append(shading_elm_3)
        i = 0
        for _ver, _date, _des, _name in _records:
            row_cells = table.rows[i].cells
            row_cells[0].text = _ver
            row_cells[1].text = _date
            row_cells[2].text = _des
            row_cells[3].text = _name
            i = i + 1
    if _col == 3:
        table.columns[0].width = Cm(2.7)
        table.columns[1].width = Cm(8)
        table.columns[2].width = Cm(2.7)
        shading_elm_0 = parse_xml(r'<w:shd {} w:fill="C0C0C0"/>'.format(nsdecls('w')))
        shading_elm_1 = parse_xml(r'<w:shd {} w:fill="C0C0C0"/>'.format(nsdecls('w')))
        shading_elm_2 = parse_xml(r'<w:shd {} w:fill="C0C0C0"/>'.format(nsdecls('w')))
        table.rows[0].cells[0]._tc.get_or_add_tcPr().append(shading_elm_0)
        table.rows[0].cells[1]._tc.get_or_add_tcPr().append(shading_elm_1)
        table.rows[0].cells[2]._tc.get_or_add_tcPr().append(shading_elm_2)
        i = 0
        for name, des, io in _records:
            row_cells = table.rows[i].cells
            row_cells[0].text = name
            row_cells[1].text = des
            row_cells[2].text = io
            i = i + 1
    elif _col == 2:
        table.columns[0].width = Cm(2.7)
        table.columns[1].width = Cm(10.7)
        shading_elm_0 = parse_xml(r'<w:shd {} w:fill="C0C0C0"/>'.format(nsdecls('w')))
        shading_elm_1 = parse_xml(r'<w:shd {} w:fill="C0C0C0"/>'.format(nsdecls('w')))
        table.rows[0].cells[0]._tc.get_or_add_tcPr().append(shading_elm_0)
        table.rows[0].cells[1]._tc.get_or_add_tcPr().append(shading_elm_1)
        i = 0
        for name, des in _records:
            row_cells = table.rows[i].cells
            row_cells[0].text = name
            row_cells[1].text = des
            i = i + 1

def cvi_isp_docx_add_chapter(_docx, _num, _module, _chapter):
    # 添加标题
    _docx.add_heading(str(_num) +  ' ISP', 0)
    # 添加二级标题
    cvi_doc_addheading(_docx, 2, str(_num) + '.1 ' + _module)
    cvi_doc_addheading(_docx, 3, str(_num) + '.1.1 功能描述')
    cvi_doc_addparagraph(_docx, Cm(2),
                         u'..........................................................................................')
    cvi_doc_addheading(_docx, 3, str(_num) + '.1.2 API 参考')
    cvi_doc_addparagraph(_docx, Cm(2.7), u'CVI_MPI_ISP_SetCNRAttr：设置 CNR 属性参数', 'List Bullet')
    cvi_doc_addparagraph(_docx, Cm(2.7), u'CVI_MPI_ISP_GetCNRAttr：获取 CNR 属性参数', 'List Bullet')
    cvi_doc_addheading(_docx, 3, u'CVI_MPI_ISP_SetCNRAttrt')
    cvi_doc_addparagraph(_docx, Cm(2),
                         u'【描述】\n设置 CNR 属性参数。\n【语法】\nCVI_S32 CVI_MPI_ISP_SetCNRAttr(VI_PIPE ViPipe, const ISP_CNR_ATTR_S *pstCNRAttr);\n【参数】')
    records = (
        ('参数名称', '描述', '输入/输出'),
        ('ViPipe', 'VI PIPE 号', '输入'),
        ('pstCNRAttr', 'pstCNRAttr 属性参数', '输入'),
    )
    cvi_doc_addtable(_docx, records)
    _docx.add_paragraph('')  # 在表格后面添加换行
    cvi_doc_addparagraph(_docx, Cm(2), u'【返回值】')
    records = (
        ('返回值', '描述'),
        ('0', '成功'),
        ('非 0', '失败，其值为错误码'),
    )
    cvi_doc_addtable(_docx, records)
    _docx.add_paragraph('')  # 在表格后面添加换行

    cvi_doc_addparagraph(_docx, Cm(2), '【需求】')
    cvi_doc_addparagraph(_docx, Cm(2.7), '头文件', 'List Bullet')
    cvi_doc_addparagraph(_docx, Cm(2.7), '库文件', 'List Bullet')
    cvi_doc_addparagraph(_docx, Cm(2), '【注意】\n无。')
    cvi_doc_addparagraph(_docx, Cm(2), '【距离】\n无')
    cvi_doc_addparagraph(_docx, Cm(2), '【相关主题】\n..............................')



def cvi_isp_docx_create():
    #document = Document("output/CV1835 Preliminary Datasheet V0.8_SC_TP.docx")
    document = Document()
    document.styles['Normal'].font.name = u'Microsoft JhengHei'
    document.styles['Normal']._element.rPr.rFonts.set(qn('w:eastAsia'), u'Microsoft JhengHei')
    document.styles['Normal'].font.size = Pt(11)
    """
    #添加封面页
    logo = document.add_picture('picture/logo1.png', width=Cm(8))
    logo.alignment = WD_TABLE_ALIGNMENT.CENTER
    cvi_doc_addheading(document, 1, u'\n\n\nCVITEK ISP\n开发参考')
    cvi_doc_addparagraph(document, Cm(0), u'\n\n\n\n')
    cvi_doc_addparagraph(document, Cm(0), u'文档版本：'+doc_r_ver)
    cvi_doc_addparagraph(document, Cm(0), u'发布日期：'+doc_r_date)

    #document.sections[0].header.paragraphs[0].text = u'这个是页眉'
    #document.sections[0].footer.paragraphs[0].text = u'北京晶视智能科技有限公司 保留一切权利'

    document.add_page_break() #分页符
    #添加版本记录页
    cvi_doc_addheading(document, 2, u'版本记录')
    records = (
        ('版本', '日期', '修订说明', '修订人'),
        (doc_r_ver, doc_r_date, '初稿', '王亮'),
        ('', '', '', '')
    )
    cvi_doc_addtable(document, records)
    """
    document.add_page_break()#分页符
    return document

def cvi_isp_docx_save(_docx, _name):
    # 将文档保存到demo.docx中
    #document.save('CVI ISP 开发参考.docx')
    _docx.save(_name)