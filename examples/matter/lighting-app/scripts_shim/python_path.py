"""
Minimal shim for Pigweed's python_path module.

connectedhomeip/scripts/codegen_paths.py and codegen.py import PythonPath from
the Pigweed submodule (third_party/pigweed/repo).  When Pigweed is not
initialised we provide this drop-in replacement so the build works without
checking out the full Pigweed tree.

PythonPath(subdir, relative_to=__file__) is a context manager that temporarily
prepends <dirname(relative_to)>/<subdir> to sys.path, then restores it on exit.
"""

import os
import sys


class PythonPath:
    def __init__(self, subdir, relative_to=None):
        if relative_to is not None:
            base = os.path.dirname(os.path.abspath(relative_to))
        else:
            base = os.getcwd()
        self._path = os.path.join(base, subdir)
        self._added = False

    def __enter__(self):
        if self._path not in sys.path:
            sys.path.insert(0, self._path)
            self._added = True
        return self

    def __exit__(self, *_exc):
        if self._added:
            try:
                sys.path.remove(self._path)
            except ValueError:
                pass
        return False
