import argparse

from cvi_bin_parser import StructParser
from cvi_bin_generator import HeaderCodeGenerator, \
                          CmpCodeGenerator,    \
                          ParseCodeGenerator,  \
                          DumpCodeGenerator,   \
                          MacroCodeGenerator
from cvi_bin_encoder import StructEncoder

def gencode(args):
    parser = StructParser()

    parser.parse_file(args.header)

    node_alias       = parser.node_alias
    enum_nodes       = parser.critical_enum_nodes
    struct_isp_tree  = parser.get_isp_struct_tree()
    struct_vpss_tree = parser.get_vpss_struct_tree()
    struct_vo_tree   = parser.get_vo_struct_tree()
    tree_list = [struct_isp_tree, struct_vpss_tree, struct_vo_tree]

    encoder = StructEncoder(profile=args.profile)
    encoder.encode(tree_list)

    # critical codes
    macroCG = MacroCodeGenerator(encoder.data)
    macroCG.gen_code(args.macro_code)

    parseCG = ParseCodeGenerator(tree_list)
    parseCG.gen_code(args.parse_code)

    entryCG = DumpCodeGenerator(tree_list, node_alias=node_alias)
    entryCG.gen_code(args.dump_code)

    # optional codes, for debug
    if args.cmp_code:
        cmpCG = CmpCodeGenerator(tree_list)
        cmpCG.gen_code(args.cmp_code)

    if args.header_code:
        headerCG = HeaderCodeGenerator(tree_list, enum_nodes=enum_nodes)
        headerCG.gen_code(args.header_code)

if __name__ == "__main__":
    parser = argparse.ArgumentParser()

    parser.add_argument('header', type=str, help='preprocessed header file')
    parser.add_argument('--profile', type=str,
                        default='python/cvi_bin_profile.json')
    parser.add_argument('--macro_code', type=str, help='output macro file',
                        default='include/cvi_bin_macros.autogen.h')
    parser.add_argument('--parse_code', type=str, help='output parse file',
                        default='src/cvi_bin_parser.autogen.c')
    parser.add_argument('--dump_code', type=str, help='output dump file',
                        default='src/cvi_bin_dumper.autogen.c')
    parser.add_argument('--cmp_code', type=str, help='output compare file',
                        default='')
    parser.add_argument('--header_code', type=str, help='output header file',
                        default='python/cvi_bin_struct.h')

    args = parser.parse_args()

    gencode(args)