# -*- Python -*-
# license
# license.
# ======================================================================

import shutil
from pathlib import Path
import logging; module_logger = logging.getLogger(__name__)
from . import json

# ----------------------------------------------------------------------

def backup_file(path :Path):
    if path.exists():
        backup_dir = path.resolve().parent.joinpath(".backup")
        backup_dir.mkdir(exist_ok=True)

        version = 1
        def gen_name():
            nonlocal version, backup_dir, path
            version += 1
            return backup_dir.joinpath("{}.~{:03d}~{}".format(path.stem, version - 1, path.suffix))

        while True:
            newname = gen_name()
            # module_logger.debug('backup_file %s %r', newname, newname.exists())
            if not newname.exists():
                # path.rename(newname)
                module_logger.debug('backup_file %s -> %s', path, newname)
                shutil.copy(str(path), str(newname))
                break

# ----------------------------------------------------------------------

def read_pydata(path :Path):
    text = read_text(path)
    if text.strip()[0] == '{':
        text = 'data = ' + text
    data = {}
    exec(text, data)
    return data['data']

# ----------------------------------------------------------------------

def read_json(path :Path):
    return json.loads(read_text(path))

# ----------------------------------------------------------------------

def read_text(path :Path):
    import lzma, bz2
    filename = str(path)
    for opener in [lzma.LZMAFile, bz2.BZ2File, open]:
        f = opener(filename, "rb")
        try:
            f.peek(1)
            break
        except:
            pass
    else:
        raise RuntimeError("Cannot read " + filename)
    return f.read().decode("utf-8")

# ----------------------------------------------------------------------

def write_binary(path :Path, data :bytes):
    if path.suffix == ".xz":
        import lzma
        opener = lzma.LZMAFile
    elif path.suffix == ".bz2":
        import bz2
        opener = bz2.BZ2File
    else:
        opener = open
    if isinstance(data, str):
        data = data.encode("utf-8")
    with opener(str(path), "wb") as f:
        f.write(data)

# ======================================================================
### Local Variables:
### eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
### End:
