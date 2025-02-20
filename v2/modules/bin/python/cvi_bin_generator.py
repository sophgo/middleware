from cvi_bin_code_template import *
from pycparser.c_ast import *
from pycparser.c_generator import CGenerator

'''
tokens : a list of dicts like {
    'type': ['unsigned', 'char'] # type of a variable
    'name': 'au8Weight'          # variable name
    'dims': ['37', '37']         # dimensions of an array, [] if not array
}   # the example represents 'unsigned char au8Weight[15][17]'
'''
def gen_macro(tokens):
    return '__'.join(['_'.join(token['type']) + '_' + token['name'] + \
                      ''.join('_x' + d for d in token['dims']) for token in tokens])

class _BasicGenerator(object):
    """ a modified version of pycparser.c_ast.NodeVisitor """
    _method_cache = None

    def visit(self, node, **kwargs):
        if self._method_cache is None:
            self._method_cache = {}

        visitor = self._method_cache.get(node.__class__.__name__, None)
        if visitor is None:
            method = 'visit_' + node.__class__.__name__
            visitor = getattr(self, method, self.generic_visit)
            self._method_cache[node.__class__.__name__] = visitor

        return visitor(node, **kwargs)

    def generic_visit(self, node, **kwargs):
        if type(node) not in [TypeDecl, Decl]:
            raise Exception(f'please implement visit_{node.__class__.__name__} method')
        for c in node:
            self.visit(c, **kwargs)

class HeaderCodeGenerator():
    def __init__(self, tree_list, enum_nodes) -> None:
        for tree in tree_list:
            self.__setattr__(tree.str, tree)
        self.enum_nodes = enum_nodes

    def gen_code(self, filename):
        cgenerator = CGenerator()
        fp = open(filename, 'w')
        fp.write(C_CODE_HEADER_TEMPLATE.format(
            filename = __file__.split('modules/')[1],
            enum_def = ';\n\n'.join([cgenerator.visit(node) for node in self.enum_nodes]),
            struct_def = cgenerator.visit(self.struct_isp_tree),
            struct_vpss_def = cgenerator.visit(self.struct_vpss_tree),
            struct_vo_def = cgenerator.visit(self.struct_vo_tree)
        ))

        fp.close()

class CmpCodeGenerator(_BasicGenerator):
    def __init__(self, tree_list) -> None:
        for tree in tree_list:
            self.__setattr__(tree.str, tree)

    def iter_arr(self, dims):
        if dims:
            for i in range(int(dims[0])):
                for sub_arr in self.iter_arr(dims[1:]):
                    yield f'[{i}]' + sub_arr
        else:
            yield ''

    def iter_ref(self, tokens):
        if len(tokens) == 1:
            yield tokens[0]['name']
        else:
            ref = tokens[0]['name']
            dims = tokens[0]['dims']
            for arr in self.iter_arr(dims):
                for sub_ref in self.iter_ref(tokens[1:]):
                    yield ref + arr + '.' + sub_ref

    def gen_code(self, filename):
        line_template = lambda ref, dim, id: \
            f"if (memcmp(&p1->{ref}{'[0]'*dim}, &p2->{ref}{'[0]'*dim}, sizeof(p1->{ref}))) return {id};"

        self.cmp_entrys = []
        self.visit(self.struct_isp_tree)
        lines = []
        id = 1
        for entry in self.cmp_entrys:
            for ref in self.iter_ref(entry):
                dim = len(entry[-1]['dims'])
                lines.append('\t' + line_template(ref, dim, id))
                id += 1
        compare_code = '\n'.join(lines)

        self.cmp_entrys = []
        self.visit(self.struct_vpss_tree)
        lines = []
        for entry in self.cmp_entrys:
            for ref in self.iter_ref(entry):
                dim = len(entry[-1]['dims'])
                lines.append('\t' + line_template(ref, dim, id))
                id += 1
        compare_vpss_code = '\n'.join(lines)

        self.cmp_entrys = []
        self.visit(self.struct_vo_tree)
        lines = []
        for entry in self.cmp_entrys:
            for ref in self.iter_ref(entry):
                dim = len(entry[-1]['dims'])
                lines.append('\t' + line_template(ref, dim, id))
                id += 1
        compare_vo_code = '\n'.join(lines)

        fp = open(filename, 'w')
        fp.write(C_CODE_CMP_TEMPLATE.format(
            filename = __file__.split('modules/')[1],
            compare_code = compare_code,
            compare_vpss_code = compare_vpss_code,
            compare_vo_code = compare_vo_code
        ))

        fp.close()

    def visit_Struct(self, node, prefixs = []):
        for c in node:
            # type is unused in this class
            cprefixs = prefixs + [{'type': [], 'name': c.name, 'dims': []}]
            self.visit(c, prefixs = cprefixs)

    def visit_IdentifierType(self, node, prefixs = []):
        self.cmp_entrys.append(prefixs)

    def visit_Enum(self, node, prefixs = []):
        self.cmp_entrys.append(prefixs)

    def visit_Union(self, node, prefixs = []):
        self.cmp_entrys.append(prefixs)

    def visit_ArrayDecl(self, node, prefixs = []):
        prefixs[-1]['dims'].append(node.dim.value)
        self.visit(node.type, prefixs = prefixs)

class ParseCodeGenerator(_BasicGenerator):
    ROOTNAME   = 'pst'

    def __init__(self, tree_list) -> None:
        for tree in tree_list:
            self.__setattr__(tree.str, tree)
        self.indent_level = 1

    @property
    def _indent(self):
        return '\t' * self.indent_level

    def count_dim(self, tokens):
        dim = 0
        for token in tokens:
            dim += len(token['dims'])
        return dim

    def gen_ref(self, tokens):
        ref = ''
        dim_id = 1
        for token in tokens[: -1]:
            ref += token['name']
            ref += ''.join([f'[i{i + dim_id}]' for i in range(len(token['dims']))])
            dim_id += len(token['dims'])
            ref += '.'
        return ref + tokens[-1]['name']

    def visit_Struct(self, node, prefixs = []):
        depth = len(prefixs) + 1
        self.max_depth = max(self.max_depth, depth)
        if prefixs:
            prefixs[-1]['type'] = [node.name]

            self.codes.append(self._indent + f'case {gen_macro(prefixs)}: {{')
            self.indent_level += 1

            dim_cnt = self.count_dim(prefixs[:-1])
            for i, dim in enumerate(prefixs[-1]['dims']):
                idx = i + dim_cnt + 1
                self.codes.append(self._indent + f'const size_t len{idx} = {dim};')
                self.codes.append(self._indent + f'const size_t unitsize{idx} = pentry_d{depth - 1}->unitsize;')

        self.codes.append(self._indent + f'while (pentry_d{depth} != pentry_end_d{depth}) {{')
        self.indent_level += 1

        # these 3 lies are unnecessary if the node does not contain any struct
        idx = len(self.codes)
        self.codes.append(self._indent + f'pentry_d{depth + 1} = pentry_d{depth} + 1;')
        self.codes.append(self._indent + f'pentry_end_d{depth + 1} = pentry_d{depth} + pentry_d{depth}->offset;')
        self.codes.append(self._indent + f'pdata_d{depth + 1} = pdata_d{depth};')

        self.codes.append(self._indent + f'switch (pentry_d{depth}->ID) {{')

         # if the node contains a struct, it will be true after iteration
        self.has_struct = False

        for c in node:
            cprefixs = prefixs + [{'type': [], 'name': c.name, 'dims': []}]
            self.visit(c, prefixs = cprefixs)

        # remove the unnecessary lines
        if not self.has_struct:
            del self.codes[idx:idx + 3]

        self.has_struct = True

        self.codes.append(self._indent + '}')   # end of switch
        self.codes.append(self._indent + f'pdata_d{depth} = pdata_d{depth} + pentry_d{depth}->size;')
        self.codes.append(self._indent + f'pentry_d{depth} = pentry_d{depth} + pentry_d{depth}->offset;')
        self.indent_level -= 1
        self.codes.append(self._indent + '}')   # end of while

        if prefixs:
            self.codes.append(self._indent + 'break; }') # end of case
            self.indent_level -= 1

    def visit_IdentifierType(self, node, prefixs = []):
        prefixs[-1]['type'] = node.names
        depth = len(prefixs)
        self.max_depth = max(self.max_depth, depth)
        vartype = ' '.join(node.names)
        ref = self.ROOTNAME + '->' + self.gen_ref(prefixs)
        dataptr = f'pdata_d{len(prefixs)}'
        pre_dimcnt = self.count_dim(prefixs[:-1])
        for i in range(1, pre_dimcnt + 1):
            dataptr += f' + i{i} * unitsize{i}'

        if prefixs[-1]['dims']:
            assignment = f'memcpy(&{ref + "[0]"*len(prefixs[-1]["dims"])}, {dataptr}, sizeof({ref}));'
        else:
            assignment = f'{ref} = *({vartype}*)({dataptr});'
        self.gen_case(gen_macro(prefixs), assignment, pre_dimcnt)

    def visit_Union(self, node, prefixs = []):
        prefixs[-1]['type'] = [node.name]
        depth = len(prefixs)
        self.max_depth = max(self.max_depth, depth)
        ref = self.ROOTNAME + '->' + self.gen_ref(prefixs)

        # fixme: union is dealt with as fixed size identifier type here
        #        see https://wiki.sophgo.com/x/IoAqC chapter-3, step-3, flaw description
        #        a better way is to deal with union as struct
        assignment = f'memcpy(&{ref + "[0]"*len(prefixs[-1]["dims"])}, pdata_d{depth}, sizeof({ref}));'
        self.gen_case(gen_macro(prefixs), assignment, self.count_dim(prefixs[:-1]))

    def visit_ArrayDecl(self, node, prefixs = []):
        prefixs[-1]['dims'].append(node.dim.value)
        self.visit(node.type, prefixs = prefixs)

    def gen_case(self, macro, assignment, dimcnt = 0):
        self.codes.append(self._indent + f'case {macro}:')
        self.indent_level += 1
        for d in range(1, dimcnt + 1):
            self.codes.append(self._indent + f'for (size_t i{d} = 0; i{d} < len{d}; i{d}++) {{')
            self.indent_level += 1

        self.codes.append(self._indent + assignment)

        for d in range(1, dimcnt + 1):
            self.indent_level -= 1
            self.codes.append(self._indent + '}')

        self.codes.append(self._indent + 'break;')
        self.indent_level -= 1

    def gen_vardefinition(self, max_depth):
        self.indent_level = 1
        vardef_codes = []
        gen_names = lambda name, d: [f'{name}_d{i}' for i in range(1, d + 1)]
        gen_unuse = lambda name, d: [f'UNUSED({name}_d{i});' for i in range(1, d + 1)]
        vardefs = [
            ['Entry' , 'pentry'],
            ['Entry' , 'pentry_end'],
            ['CVI_U8', 'pdata'],
        ]

        vardef_codes.append(self._indent + 'CVI_BIN_INDEX_HEADER *pheader = (CVI_BIN_INDEX_HEADER *)(buf + indexOffset);')
        for vardef in vardefs:
            vtype, vname = vardef
            vnames = ', '.join(['{:15s}'.format('*' + v) for v in gen_names(vname, max_depth)])
            vardef_codes.append(self._indent + '{:8s}'.format(vtype) + vnames + ';')
            vardef_codes.append(self._indent + ' '.join(gen_unuse(vname, max_depth)) + '\n')
        return vardef_codes

    def gen_process(self):
        self.codes = []		# codes for parsing
        self.max_depth = 0	# max depth of struct

        self.codes.append(self._indent + 'pentry_d1     = (Entry*)((CVI_U8*)buf + pheader->offsetIspEntry);')
        self.codes.append(self._indent + 'pentry_end_d1 = (Entry*)((CVI_U8*)buf + pheader->offsetVpssEntry);')
        self.codes.append(self._indent + 'pdata_d1      = (CVI_U8*)((CVI_U8*)buf + pheader->offsetData[id]);')

        self.codes.append('')
        self.visit(self.struct_isp_tree)
        return self.codes, self.max_depth

    def gen_process_vpss(self):
        self.codes = []		# codes for parsing
        self.max_depth = 0	# max depth of struct

        self.codes.append(self._indent + 'pentry_d1     = (Entry*)((CVI_U8*)buf + pheader->offsetVpssEntry);')
        self.codes.append(self._indent + 'pentry_end_d1 = (Entry*)((CVI_U8*)buf + pheader->offsetVoEntry);')
        self.codes.append(self._indent + 'pdata_d1      = (CVI_U8*)((CVI_U8*)buf + pheader->offsetData[id]);')

        self.codes.append('')
        self.visit(self.struct_vpss_tree)
        return self.codes, self.max_depth

    def gen_process_vo(self):
        self.codes = []		# codes for parsing
        self.max_depth = 0	# max depth of struct

        self.codes.append(self._indent + 'pentry_d1     = (Entry*)((CVI_U8*)buf + pheader->offsetVoEntry);')
        self.codes.append(self._indent + 'pentry_end_d1 = (Entry*)((CVI_U8*)buf + pheader->size[0] + indexOffset);')
        self.codes.append(self._indent + 'pdata_d1      = (CVI_U8*)((CVI_U8*)buf + pheader->offsetData[id]);')

        self.codes.append('')
        self.visit(self.struct_vo_tree)
        return self.codes, self.max_depth

    def gen_code(self, filename):
        process_codes, max_depth = self.gen_process()
        vardef_codes = self.gen_vardefinition(max_depth)
        parse_code = '\n'.join(vardef_codes + process_codes)

        process_codes, max_depth = self.gen_process_vpss()
        vardef_codes = self.gen_vardefinition(max_depth)
        parse_vpss_code = '\n'.join(vardef_codes + process_codes)

        process_codes, max_depth = self.gen_process_vo()
        vardef_codes = self.gen_vardefinition(max_depth)
        parse_vo_code = '\n'.join(vardef_codes + process_codes)

        fp = open(filename, 'w')
        fp.write(C_CODE_PARSER_TEMPLATE.format(
            filename = __file__.split('modules/')[1],
            var = self.ROOTNAME,
            entry_decl_code = C_ENTRY_DECL_CODE,
            parse_code = parse_code,
            parse_vpss_code = parse_vpss_code,
            parse_vo_code = parse_vo_code
        ))
        fp.close()

class DumpCodeGenerator(_BasicGenerator):
    def __init__(self, tree_list, node_alias) -> None:
        for tree in tree_list:
            self.__setattr__(tree.str, tree)
        self.node_alias = node_alias
        self.entrys = []
        self.root_type = ''

    def get_vartype(self, types, quals = ''):
        vartype = ' '.join(types) if isinstance(types, list) else types
        if vartype in self.node_alias:
            return vartype
        elif quals:
            return quals + ' ' + vartype
        return vartype

    def visit_TypeDecl(self, node, **kwargs):
        return self.visit(node.type, **kwargs)

    def visit_Decl(self, node, **kwargs):
        return self.visit(node.type, **kwargs)

    def visit_Struct(self, node, prefixs = [], previd = -1):
        if prefixs:
            vartype = self.get_vartype(node.name, 'struct')
            prefixs[-1]['type'] = [node.name]
            varname = prefixs[-1]['name']

            if len(prefixs) > 1:
                parent_vartype = self.get_vartype(prefixs[-2]['type'], 'struct')
            else:
                parent_vartype = self.root_type

            if previd >= 0:
                self.entrys[previd]['size'][0] = f'offsetof({parent_vartype}, {varname})'

            idx = len(self.entrys)
            offset = ['', f'offsetof({parent_vartype}, {varname})']
            self.append_entrys(gen_macro(prefixs), '', offset, f'sizeof({vartype})')
        else:
            vartype = node.name

        entry_cnt = 1
        child_prev_id = -1
        for c in node:
            cprefixs = prefixs + [{'type': [], 'name': c.name, 'dims': []}]
            _child_prev_id = len(self.entrys)
            entry_cnt += self.visit(c, prefixs = cprefixs, previd = child_prev_id)
            child_prev_id = _child_prev_id

        if prefixs:
            self.entrys[idx]['offset'] = f'{entry_cnt}'

        self.entrys[child_prev_id]['size'][0] = f'sizeof({vartype})'

        return entry_cnt

    def visit_IdentifierType(self, node, prefixs = [], previd = -1):
        prefixs[-1]['type'] = node.names

        if len(prefixs) > 1:
            parent_vartype = self.get_vartype(prefixs[-2]['type'], 'struct')
        else:
            parent_vartype = self.root_type

        varname = prefixs[-1]['name']
        vartype = ' '.join(node.names)
        if previd >= 0:
            self.entrys[previd]['size'][0] = f'offsetof({parent_vartype}, {varname})'

        offset = ['', f'offsetof({parent_vartype}, {varname})']
        self.append_entrys(gen_macro(prefixs), '1', offset, f'sizeof({vartype})')
        return 1

    def visit_ArrayDecl(self, node, prefixs = [], previd = -1):
        prefixs[-1]['dims'].append(node.dim.value)
        return self.visit(node.type, prefixs = prefixs, previd = previd)

    def visit_Union(self, node, prefixs = [], previd = -1):
        prefixs[-1]['type'] = [node.name]
        parent_vartype = ' '.join(prefixs[-2]['type'])
        varname = prefixs[-1]['name']
        vartype = node.name

        if previd >= 0:
            self.entrys[previd]['size'][0] = f'offsetof({parent_vartype}, {varname})'

        offset = ['', f'offsetof({parent_vartype}, {varname})']
        self.append_entrys(gen_macro(prefixs), '1', offset, f'sizeof({vartype})')

        return 1

    def append_entrys(self, id, offset, size, arrlen):
        self.entrys.append({
            'id': id,
            'offset': offset,
            'size': size,
            'arrlen': arrlen
        })

    def gen_code(self, filename):
        self.root_type = 'ISP_Parameter_Structures'
        entry_cnt = self.visit(self.struct_isp_tree)
        entry_cnt -= 1 # remove the entry of 'ISP_Parameter_Structures'

        self.root_type = 'VPSS_Parameter_Structures'
        entry_vpss_cnt = self.visit(self.struct_vpss_tree)
        entry_vpss_cnt -= 1 # remove the entry of 'VPSS_Parameter_Structures'

        self.root_type = 'VO_Parameter_Structures'
        entry_vo_cnt = self.visit(self.struct_vo_tree)
        entry_vo_cnt -= 1 # remove the entry of 'VO_Parameter_Structures'

        fp = open(filename, 'w')
        fp.write(C_CODE_ENTRY_TEMPLATE.format(
            filename = __file__.split('modules/')[1],
            entry_decl_code = C_ENTRY_DECL_CODE,
            entry_size      = entry_cnt * 8,
            entry_vpss_size   = entry_vpss_cnt * 8,
            entry_vo_size   = entry_vo_cnt * 8,
            entry_def_codes = ',\n'.join([self.gen_entry(entry) for entry in self.entrys])
        ))

        fp.close()

    def gen_entry(self, entry):
        return f'''\t{{
        {entry["id"]},
        {entry["offset"]},
        {entry["size"][0]} - {entry["size"][1]},
        {entry["arrlen"]}\n\t}}'''

class MacroCodeGenerator(_BasicGenerator):
    def __init__(self, data) -> None:
        self.data = data
        self.macros = []

    def _gen_macros(self, datalist, prefixs = []):
        for data in datalist:
            cprefixs = prefixs + [data]
            self.macros.append([gen_macro(cprefixs), data['ID']])
            self._gen_macros(data['children'], cprefixs)

    def gen_code(self, filename):
        self._gen_macros(self.data)

        fp = open(filename, 'w')
        fp.write(C_CODE_MACRO_TEMPLATE.format(
            filename = __file__.split('modules/')[1],
            macro_defines = '\n'.join([f'#define {m[0]} 0x{m[1]:04x}' for m in self.macros])
        ))
        fp.close()