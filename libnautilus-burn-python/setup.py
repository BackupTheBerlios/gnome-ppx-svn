from distutils.core import setup
from distutils.sysconfig import get_python_lib
import sys, os
if os.system ("make"):
	sys.exit (1)
setup (name = 'nautilus_burn',
       version = '1.0',
       packages = ['nautilus_burn'],
       data_files = [(get_python_lib(), ['nautilus_burn.pyd'])])
