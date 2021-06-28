import gdb
import gdb.printing

import sys, os
sys.path.append(os.path.dirname(__file__))
import chef.v1

gdb.printing.register_pretty_printer(gdb.current_objfile(), chef.v1.build_pretty_printer())
chef.v1.register_pps()
