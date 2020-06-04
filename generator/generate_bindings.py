#! /bin/python3

from pathlib import Path
import json
import xml
import sys


skipcategories = ["macros"]


def generate_lambda_binding(f):
    lmb = "[]("

    parg = None
    args = []
    callargs = []

    for i, arg in enumerate(f["args"]):

        if arg["type"].endswith("**"):
            callargs.append("&" + arg["name"])
            parg = arg
            continue

        callargs.append(arg["name"])
        args.append(arg["type"] + " " + arg["name"])

    lmb += ", ".join(args)
    lmb += ") {\n"
    lmb += "\t\t" + f"{parg['type'][:-1]} {parg['name']} = nullptr;" + "\n"

    if f["ret"] == "bool":
        lmb += "\t\t" + f"bool ok = {f['name']}(" + ", ".join(callargs) + ");\n"
        lmb += "\t\t" + f"return ok ? {parg['name']} : nullptr;"
    elif f["ret"] == "size_t":
        lmb += "\t\t" + f"size_t count = {f['name']}(" + ", ".join(callargs) + ");\n"
        lmb += "\n\t\t" + f"std::vector<{parg['type'][:-1]}> c(count);" + "\n"
        lmb += "\t\t" + f"std::copy_n(std::addressof({parg['name']}), count, c.begin());" + "\n"
        lmb += "\t\treturn c;"
    else:
        print(f"Unhandled lambda for {f['name']}")

    lmb += "\n\t}"

    n = f["name"]
    return "\n\t" + f'm.def("{n}", {lmb});\n'


def generate_handles(catobj, src):
    for h in catobj["handles"]:
        src.append("\t" + f'pybind11::class_<{h}>(m, "{h}");')

    src.append("")


def generate_enums(catobj, src):

    for e in catobj["enums"]:
        sl = len(e["states"])

        if not sl:
            continue

        n = e["name"]

        if n.endswith("Flags"):
            src.append("\t" + f'pybind11::enum_<{n}>(m, "{n}", pybind11::arithmetic())')
        else:
            src.append("\t" + f'pybind11::enum_<{n}>(m, "{n}")')

        for s in e["states"]:
            src.append("\t\t" + f'.value("{s}", {n}::{s})')

        src.append("\t\t.export_values();")
        src.append("")

    src.append("")


def generate_functions(catobj, src):
    needsvector = False

    for f in catobj["functions"]:
        n = f["name"]

        index = -1

        for i, arg in enumerate(f["args"]):
            if arg["type"].endswith("**"):
                needsvector = True
                index = i
                break

        if index != -1:
            src.append(generate_lambda_binding(f))
        else:
            src.append("\t" + f'm.def("{n}", &{n});')

    src.append("}")

    if needsvector:  # We need vector for C <-> Python array translation
        src.insert(2, "#include <vector>")
        src.insert(2, "#include <algorithm>")
        src.insert(2, "#include <pybind11/stl.h>")


def generate_category(categoryname, catobj, outputdir):
    src = [f'#include "rdapi_{categoryname}.h"',
           f'#include "rdapi_all.h"',
           "\n",
           f"void bind{categoryname.capitalize()}(pybind11::module& m) {{"]

    generate_handles(catobj, src)
    generate_enums(catobj, src)
    generate_functions(catobj, src)

    with open(Path(outputdir, f"rdapi_{categoryname}.cpp"), "w") as f:
        f.write("\n".join(src))


def generate_rdpython(jsondoc, outputdir):
    with open(Path(outputdir, f"rdapi_rdpython.cpp"), "w") as f:
        f.write("#include <pybind11/pybind11.h>\n")

        for c, obj in jsondoc["categories"].items():
            if c.lower() in skipcategories:
                continue

            f.write(f'#include "rdapi_{c.lower()}.h"' + "\n")

        f.write("\n")
        f.write(f"void bindRDPython(pybind11::module& m) {{" + "\n")

        for c, obj in jsondoc["categories"].items():
            if c.lower() in skipcategories:
                continue

            f.write("\t" + f"bind{c.lower().capitalize()}(m);" + "\n")

        f.write("}")


def generate_bindings(docfilepath, outputdir):
    with open(docfilepath, "r") as f:
        jsondoc = json.load(f)

    outdir = Path(outputdir)
    outdir.mkdir(parents=True, exist_ok=True)

    for category, obj in jsondoc["categories"].items():
        lccategory = category.lower()

        if lccategory in skipcategories:
            continue

        with open(Path(outdir, f"rdapi_{lccategory}.h"), "w") as f:
            f.write("#pragma once\n\n")
            f.write("#include <pybind11/pybind11.h>\n\n")
            f.write(f"void bind{category}(pybind11::module& m);")

        generate_category(lccategory, obj, outputdir)

    generate_rdpython(jsondoc, outputdir)


if __name__ == "__main__":
    generate_bindings(sys.argv[1], sys.argv[2])
