import re
from copy import deepcopy

from pycparser.c_generator import CGenerator
from pycparser.c_ast import *
# pycparserext is a Extensions for Eli Bendersky's pycparser
from pycparserext.ext_c_parser import GnuCParser


class _BasicParser(object):
    """ a modified version of pycparser.c_ast.NodeVisitor """
    _visit_method_cache  = {}
    _expand_method_cache = {}

    def visit(self, node, **kwargs):
        visitor = self._visit_method_cache.get(node.__class__.__name__, None)
        if visitor is None:
            method = 'visit_' + node.__class__.__name__
            visitor = getattr(self, method, self.nop_visit)
            self._visit_method_cache[node.__class__.__name__] = visitor

        return visitor(node, **kwargs)

    def nop_visit(self, node, **kwargs):
        pass

    def expand(self, node, **kwargs):
        expander = self._expand_method_cache.get(node.__class__.__name__, None)
        if expander is None:
            method = 'expand_' + node.__class__.__name__
            expander = getattr(self, method, self.nop_expand)
            self._expand_method_cache[node.__class__.__name__] = expander

        return expander(node, **kwargs)

    def nop_expand(self, node, **kwargs):
        if type(node) not in [Constant]:
            raise Exception(f'please implement expand_{node.__class__.__name__} method')
        return node


class StructParser(_BasicParser):
    def __init__(self):
        self.typedecl_nodes = dict()
        self.struct_nodes   = dict()
        self.union_nodes    = dict()
        self.enum_nodes     = dict()
        self.enum_map       = dict()
        self.node_alias     = dict()
        self.critical_enum_nodes = set()

    def append_nodes(self, nodes, name, node):
        if not name:
            return
        if not node.children() and node.__class__ not in [IdentifierType]:
            return
        if name in nodes and nodes[name].children():
            print(f'[Warning] {name} declared twice')
        else:
            nodes[name] = node

    def visit_FileAST(self, node):
        for c in node:
            self.visit(c)

    def visit_Typedef(self, node):
        self.visit(node.type)

    def visit_Decl(self, node):
        self.visit(node.type)

    def visit_TypeDecl(self, node):
        for c in node:
            self.typedecl_nodes[node.declname] = {
                'type'    : c.__class__.__name__,
                'Typedef' : Typedef(node.declname, [], ['typedef'], node),
                'children': c
            }
            self.visit(c, alias = node.declname)

    def visit_Union(self, node, alias = None):
        if alias and node.name:
            self.node_alias[alias] = node.name
        self.append_nodes(self.union_nodes, node.name, node)

    def visit_Struct(self, node, alias = None):
        if alias and node.name:
            self.node_alias[alias] = node.name
        self.append_nodes(self.struct_nodes, node.name, node)

    ''' fixme: enum is dealt with as integer identifier type here
		       see https://wiki.sophgo.com/x/IoAqC chapter-3, step-3, flaw description
		       a better way is to record all enum values and use them in ID encoder
    '''
    def visit_Enum(self, node, alias = None):
        if alias and node.name:
            self.node_alias[node.name] = alias
        self.append_nodes(self.enum_nodes, node.name, node)

        # parse enum values
        self.visit(node.values)

    ''' [visit_EnumeratorList, ... , visit_BinaryOp]
        parse enum values '''
    def visit_EnumeratorList(self, node):
        value = 0
        for c in node:
            value = self.visit(c, value = value)

    def visit_Enumerator(self, node, value):
        name = node.name
        value = self.visit(node.value) if node.value else value
        self.enum_map[name] = value
        return value + 1

    def visit_Constant(self, node):
        return eval(node.value)

    def visit_ID(self, node):
        return self.enum_map[node.name]

    def visit_BinaryOp(self, node):
        line = CGenerator().visit(node)
        try:
            if '<<' in line or '>>' in line:
                return 0
            return eval(line)
        except:
            print('=' * 60)
            print(f'[Warning] unable to eval "{line}"')
            print(f'          file: {node.coord.file} line: {node.coord.line}')
            return 0

    ''' [expand_Decl, ... , expand_ID]
        tree-like expand node '''

    def expand_Decl(self, node):
        node.type = self.expand(node.type)
        return node

    def expand_TypeDecl(self, node):
        node.type = self.expand(node.type)
        return node

    def expand_Struct(self, node):
        if not node.children():
            return self.find_node(node.name)
        _decls = []
        for c in node:
            _decls.append(self.expand(c))
        node.decls = _decls
        return node

    def expand_IdentifierType(self, node):
        return self.find_node(node.names[0]) or node

    def expand_Union(self, node):
        ''' the same rule as struct '''
        return self.expand_Struct(node)

    def expand_ArrayDecl(self, node):
        node.type = self.expand(node.type)
        node.dim  = self.expand(node.dim)
        return node

    def expand_BinaryOp(self, node):
        return Constant('int', str(self.visit(node)))

    def expand_ID(self, node):
        return Constant('int', str(self.visit(node)))

    ''' return None if not found
        Enum is a special case, it will not be expanded '''
    def find_node(self, name):
        if name in self.typedecl_nodes:
            ntype = self.typedecl_nodes[name]['type']
            node  = deepcopy(self.typedecl_nodes[name]['children'])
            if ntype in ['Struct', 'Union']:
                node.name = name
                return self.expand(node)
            if ntype == 'Enum':
                self.critical_enum_nodes.add(self.typedecl_nodes[name]['Typedef'])
                return None
            if ntype == 'IdentifierType':
                return self.find_node(node.names[0]) or node

        if name in self.struct_nodes:
            return self.expand(deepcopy(self.struct_nodes[name]))

        if name in self.union_nodes:
            return self.expand(deepcopy(self.union_nodes[name]))

        if name in self.enum_nodes:
            self.critical_enum_nodes.add(self.enum_nodes[name])
            return None

        return None

    def preprocess_text(self, text):
        # remove special attributes which are useless when parsing
        patterns = [
            [re.compile(r'__attribute__[^(]*\([^()]*\)'), ''],
            [re.compile(r'(__attribute__[^)]+)(\([^()]*\))'), r'\1']
        ]
        matched = True
        while matched:
            matched = False
            for pattern in patterns:
                _text = pattern[0].sub(pattern[1], text)
                if _text != text:
                    matched = True
                    text = _text

        text = text.replace('__signed__', 'signed')
        return text

    def parse_file(self, filename):
        with open(filename, 'r') as f:
            text = f.read()
        text = self.preprocess_text(text)

        parser = GnuCParser(lex_optimize=False)
        ast = parser.parse(text)
        self.visit(ast)

    def get_isp_struct_tree(self):
        node = self.typedecl_nodes['ISP_Parameter_Structures']['children']

        self.struct_isp_tree = self.expand(deepcopy(node))
        self.struct_isp_tree.name = 'ISP_Parameter_Structures'
        self.struct_isp_tree.str = 'struct_isp_tree'
        return self.struct_isp_tree

    def get_vpss_struct_tree(self):
        node = self.typedecl_nodes['VPSS_Parameter_Structures']['children']

        self.struct_vpss_tree = self.expand(deepcopy(node))
        self.struct_vpss_tree.name = 'VPSS_Parameter_Structures'
        self.struct_vpss_tree.str = 'struct_vpss_tree'
        return self.struct_vpss_tree

    def get_vo_struct_tree(self):
        node = self.typedecl_nodes['VO_Parameter_Structures']['children']

        self.struct_vo_tree = self.expand(deepcopy(node))
        self.struct_vo_tree.name = 'VO_Parameter_Structures'
        self.struct_vo_tree.str = 'struct_vo_tree'
        return self.struct_vo_tree
