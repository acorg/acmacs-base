# -*- Python -*-
# license
# license.
# ======================================================================

import logging; module_logger = logging.getLogger(__name__)

# ----------------------------------------------------------------------

def encode(name):
    for char in "% :()!*';@&=+$,?#[]":
        name = name.replace(char, '%{:02X}'.format(ord(char)))
    return name

# ----------------------------------------------------------------------

def decode(name):
    return name

# ======================================================================
### Local Variables:
### eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
### End:
