import json
import os

from pycparser.c_ast import *

class _BasicEncoder(object):
    """ a modified version of pycparser.c_ast.NodeVisitor """
    _method_cache = None

    def visit(self, node, **kwargs):
        if self._method_cache is None:
            self._method_cache = {}

        visitor = self._method_cache.get(node.__class__.__name__, None)
        if visitor is None:
            method = 'visit_' + node.__class__.__name__
            visitor = getattr(self, method, self.basic_visit)
            self._method_cache[node.__class__.__name__] = visitor

        return visitor(node, **kwargs)

    def basic_visit(self, node, **kwargs):
        if type(node) not in [Decl, TypeDecl, Constant]:
            raise Exception(f'please implement visit_{node.__class__.__name__} method')
        for c in node:
            self.visit(c, **kwargs)

class _PrettyJsonEncoder(json.JSONEncoder):
    def iterencode(self, o, _one_shot=False):
        oneline_field = False
        for chunk in super().iterencode(o, _one_shot=_one_shot):
            if 'type' in chunk:
                oneline_field = True
            if oneline_field:
                if 'children' in chunk:
                    oneline_field = False

                chunk = chunk.replace('\n', '').replace(' ', '').replace(',', ', ')
            yield chunk

class StructEncoder(_BasicEncoder):
    def __init__(self, profile) -> None:
        self.profile = profile
        self.data = []

    def find_entry(self, data, typ, name, dims):
        for d in data:
            if d['type'] == typ and d['name'] == name and d['dims'] == dims:
                return d['ID']
        print(f'[add] entry type: {typ} name: {name} dims: {dims}')
        return -1

    def append_data(self, data, typ, name, dims):
        data.append({
            'ID': len(data),
            'type': typ, 'name': name, 'dims': dims,
            'children' : []
        })

    def visit_Struct(self, node, data, prefixs = []):
        if prefixs:
            prefixs[-1]['type'] = [node.name]

            typ, name, dims = prefixs[-1]['type'], prefixs[-1]['name'], prefixs[-1]['dims']

            idx = self.find_entry(data, typ, name, dims)
            if idx == -1:
                self.append_data(data, typ, name, dims)
                idx = len(data) - 1

            cdata = data[idx]['children']
        else:
            cdata = data

        for c in node:
            cprefixs = prefixs + [{'type': [], 'name': c.name, 'dims': []}]
            self.visit(c, data = cdata, prefixs = cprefixs)

    def visit_IdentifierType(self, node, data, prefixs):
        prefixs[-1]['type'] = node.names

        typ, name, dims = prefixs[-1]['type'], prefixs[-1]['name'], prefixs[-1]['dims']
        idx = self.find_entry(data, typ, name, dims)
        if idx == -1:
            self.append_data(data, typ, name, dims)

    def visit_Union(self, node, data, prefixs):
        prefixs[-1]['type'] = [node.name]

        typ, name, dims = prefixs[-1]['type'], prefixs[-1]['name'], prefixs[-1]['dims']
        idx = self.find_entry(data, typ, name, dims)
        if idx == -1:
            self.append_data(data, typ, name, dims)

    def visit_ArrayDecl(self, node, data, prefixs):
        prefixs[-1]['dims'].append(node.dim.value)
        self.visit(node.type, data = data, prefixs = prefixs)

    def read_profile(self):
        if os.path.exists(self.profile):
            with open(self.profile, 'r') as f:
                self.data = json.load(f)

    def write_profile(self):
        with open(self.profile, 'w') as f:
            json.dump(self.data, f, indent=4, cls=_PrettyJsonEncoder)

    def encode(self, tree_list):
        self.read_profile()

        for tree in tree_list:
            self.visit(tree, data = self.data)

        self.write_profile()
