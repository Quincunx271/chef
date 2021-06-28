import gdb
import gdb.printing

_RE_TYPE = gdb.lookup_type('chef::re')


class ValuePtrPrinter(gdb.printing.PrettyPrinter):
    def __init__(self, val):
        self.val = val

    def to_string(self):
        return str(self.val['value']['_M_t']['_M_t']['_M_head_impl'].dereference())


_VARIANT_TYPES = ['chef::re_empty', 'chef::re_cat', 'chef::re_union',
                  'chef::re_lit', 'chef::re_star', 'chef::re_char_class']
_VARIANT_TYPES = [gdb.lookup_type(t) for t in _VARIANT_TYPES]


class ReUnionPrinter(gdb.printing.PrettyPrinter):
    def __init__(self, val):
        self.val = val

    def to_string(self):
        vec = self.val['pieces']
        v = vec['_M_impl']
        start = v['_M_start']
        finish = v['_M_finish']
        num_elems = int(finish - start)
        return '|'.join(str(start[i]) for i in range(num_elems))


class ReCatPrinter(gdb.printing.PrettyPrinter):
    def __init__(self, val):
        self.val = val

    def to_string(self):
        vec = self.val['pieces']
        v = vec['_M_impl']
        start = v['_M_start']
        finish = v['_M_finish']
        num_elems = int(finish - start)
        return ''.join('(' + str(start[i]) + ')' for i in range(num_elems))


class ReStarPrinter(gdb.printing.PrettyPrinter):
    def __init__(self, val):
        self.val = val

    def to_string(self):
        return '(' + str(self.val['value']) + ')*'


class ReLitPrinter(gdb.printing.PrettyPrinter):
    def __init__(self, val):
        self.val = val

    def to_string(self):
        return str(self.val['value'])


class ReCharClassPrinter(gdb.printing.PrettyPrinter):
    def __init__(self, val):
        self.val = val

    def to_string(self):
        return '[]'


class ReEmptyPrinter(gdb.printing.PrettyPrinter):
    def __init__(self, val):
        self.val = val

    def to_string(self):
        return '<EMPTY>'


class RePrinter(gdb.printing.PrettyPrinter):
    def __init__(self, val):
        self.val = val

    def to_string(self):
        variant = self.val['value']
        index = variant['_M_index']
        if index not in range(len(_VARIANT_TYPES)):
            return '{{{BAD-RE}}'
        type = _VARIANT_TYPES[index]
        r = variant['_M_u'].address
        p = type.pointer()
        return str(r.cast(p).dereference())


class RePointerPrinter(gdb.printing.PrettyPrinter):
    def __init__(self, val):
        self.val = val

    def to_string(self):
        return 'pointer to re ' + (str(self.val.dereference()) if self.val else 'nullptr')


def build_pretty_printer():
    pp = gdb.printing.RegexpCollectionPrettyPrinter("chef")
    pp.add_printer('chef::re', r'^chef::re$', RePrinter)
    # pp.add_printer('chef::re*', r'^(const\s*)?chef::re\s*(const\s*)?\*$', RePointerPrinter)
    pp.add_printer('chef::detail::value_ptr',
                   r'^chef::detail::value_ptr', ValuePtrPrinter)
    pp.add_printer('chef::re_union', r'^chef::re_union$', ReUnionPrinter)
    pp.add_printer('chef::re_cat', r'^chef::re_cat$', ReCatPrinter)
    pp.add_printer('chef::re_star', r'^chef::re_star$', ReStarPrinter)
    pp.add_printer('chef::re_lit', r'^chef::re_lit$', ReLitPrinter)
    pp.add_printer('chef::re_char_clas',
                   r'^chef::re_char_clas$', ReCharClassPrinter)
    pp.add_printer('chef::re_empty', r'^chef::re_empty$', ReEmptyPrinter)
    return pp


def pp_p_re(val):
    if str(val.type) in ('chef::re *', 'const chef::re *'):
        return RePointerPrinter(val)

def register_pps():
    gdb.pretty_printers.append(pp_p_re)
