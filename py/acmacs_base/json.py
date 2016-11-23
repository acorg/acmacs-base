# -*- Python -*-
# license
# license.
# ======================================================================

import json
import logging; module_logger = logging.getLogger(__name__)

# ----------------------------------------------------------------------

def _json_simple(d):
    r = True
    if isinstance(d, dict):
        r = not any(isinstance(v, (list, tuple, set, dict)) for v in d.values()) and len(d) < 10
    elif isinstance(d, (tuple, list)):
        r = not any(isinstance(v, (list, tuple, set, dict)) for v in d)
    return r

# ----------------------------------------------------------------------

def dumps(data, separators=[',', ':'], indent=None, compact=True, sort_keys=False, simple=_json_simple):
    # module_logger.info('json.dumps: {!r}'.format(data))
    if indent is not None and compact:
        data = _json_dumps(data, indent=indent, indent_increment=indent, simple=simple)
    else:
        data = json.dumps(data, separators=separators, indent=indent, default=serializable_to_json, sort_keys=sort_keys)
    return data

# ----------------------------------------------------------------------

class JSONEncoder (json.JSONEncoder):

    def default(self, o):
        if isinstance(o, Path):
            r = str(o)
        # elif hasattr(o, "json"):
        #     r = o.json()
        else:
            r = "<" + str(type(o)) + " : " + repr(o) + ">"
        return r

# ----------------------------------------------------------------------

def _json_dumps(data, indent=2, indent_increment=None, simple=_json_simple, toplevel=True):
    """More compact dumper with wide lines."""

    def end(symbol, indent):
        if indent > indent_increment:
            r = "{:{}s}{}".format("", indent - indent_increment, symbol)
        else:
            r = symbol
        return r

    if indent_increment is None:
        indent_increment = indent
    r = []
    if simple(data):
        if isinstance(data, set):
            r.append(json.dumps(sorted(data), sort_keys=True, cls=JSONEncoder))
        else:
            r.append(json.dumps(data, sort_keys=True, cls=JSONEncoder))
    else:
        if isinstance(data, dict):
            if toplevel:
                r.append("{{{:<{}s}\"_\":\"-*- js-indent-level: {} -*-\",".format("", indent_increment - 1, indent_increment))
            else:
                r.append("{")
            for no, k in enumerate(sorted(data), start=1):
                comma = "," if no < len(data) else ""
                r.append("{:{}s}{}: {}{}".format("", indent, json.dumps(k, cls=JSONEncoder), _json_dumps(data[k], indent + indent_increment, indent_increment, simple=simple, toplevel=False), comma))
            r.append(end("}", indent))
        elif isinstance(data, (tuple, list)):
            r.append("[")
            for no, v in enumerate(data, start=1):
                comma = "," if no < len(data) else ""
                r.append("{:{}s}{}{}".format("", indent, _json_dumps(v, indent + indent_increment, indent_increment, simple=simple, toplevel=False), comma))
            r.append(end("]", indent))
    return "\n".join(r)

# ----------------------------------------------------------------------

loads = json.loads

# ======================================================================
### Local Variables:
### eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
### End:
