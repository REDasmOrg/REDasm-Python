#! /bin/python3

from pathlib import Path
import json
import xml
import sys


skipcategories = ["macros"]

def check_struct_proxies(catobj):
    structproxies = {}

    for s in catobj["structs"]:
        for f in s["fields"]:
            if not f["callback"]:
                continue
            structproxies.setdefault(s["name"], [])
            structproxies[s["name"]].append(f)

    return structproxies


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


def generate_structs(catobj, src):
    structproxies = check_struct_proxies(catobj)

    for s in catobj["structs"]:
        fl = len(s["fields"])

        if not fl:
            continue

        n = s["name"]

        proxy = structproxies.get(n, None)

        if proxy:
            src.append("\t" + f'pybind11::class_<Py{n}>(m, "{n}")')
        else:
            src.append("\t" + f'pybind11::class_<{n}>(m, "{n}")')

        src.append("\t\t" + f'.def(pybind11::init<>())')

        for i, f in enumerate(s["fields"]):
            last = i == (fl - 1)
            fn = f["name"]

            if f["arraysize"]:
                if last:
                    src.append("\t\t" + f'.def_readonly("{fn}", &{n}::{fn});')
                else:
                    src.append("\t\t" + f'.def_readonly("{fn}", &{n}::{fn})')
            elif f["callback"]:
                if last:
                    src.append("\t\t" + f'.def_property("{fn}", &Py{n}::get{fn.capitalize()}, &Py{n}::set{fn.capitalize()}, pybind11::return_value_policy::reference);')
                else:
                    src.append("\t\t" + f'.def_property("{fn}", &Py{n}::get{fn.capitalize()}, &Py{n}::set{fn.capitalize()}, pybind11::return_value_policy::reference)')
            else:
                if last:
                    src.append("\t\t" + f'.def_readwrite("{fn}", &{n}::{fn});')
                else:
                    src.append("\t\t" + f'.def_readwrite("{fn}", &{n}::{fn})')

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
            src.append("\t" + f'm.def("{n}", &{n}, pybind11::return_value_policy::reference);')

    src.append("}")

    if needsvector:  # We need vector for C <-> Python array translation
        src.insert(2, "#include <vector>")
        src.insert(2, "#include <algorithm>")
        src.insert(2, "#include <pybind11/stl.h>")


def generate_struct_proxies(catobj, src):
    structproxies = check_struct_proxies(catobj)
    if not structproxies:
        return

    src.insert(2, "#include <string>")

    for name, field in structproxies.items():
        src.append(f"struct Py{name}: public {name} {{")

        for f in field:  # Typedefs
            cb = catobj["callbacks"][f["type"]]
            argtypes = [a["type"] for a in cb["args"][1:]]
            src.append("\t" + f"typedef std::function<{cb['ret']}(" + ", ".join(argtypes) + f")> Py{f['type']};")

        src.append("")

        for f in field:  # Field Proxy
            src.append("\t" + f"pybind11::object py_{f['name']}{{pybind11::none()}}; // {f['type']}")

        src.append("")

        for f in field:  # Field Getters
            src.append("\t" + f"pybind11::object get{f['name'].capitalize()}() {{ return py_{f['name']}; }}")

        src.append("")

        for f in field:  # Field Setters
            cb = catobj["callbacks"][f["type"]]
            src.append("\t" + f"void set{f['name'].capitalize()}(const pybind11::object& arg) {{ ")
            src.append("\t\t" + f"py_{f['name']} = arg;\n")
            src.append("\t\tif(arg) {")

            if cb["args"][0]["type"].startswith("const "):
                thistype = f"const Py{name}*"
            else:
                thistype = f"Py{name}*"

            thisarg = cb["args"][0]["name"]
            args = ", ".join([f"{a['type']} {a['name']}" for a in cb["args"]])
            callargs = ", ".join([a["name"] for a in cb["args"]][1:])

            if cb["ret"] == "const char*":  # Workaround for const char* callbacks
                src.append("\t\t\t" + f"{f['name']} = []({args}) {{")
                src.append("\t\t\t\t" + f"static std::string res;")
                src.append("\t\t\t\t" + f"res = static_cast<{thistype}>({thisarg})->py_{f['name']}({callargs}).cast<std::string>(); ")
                src.append("\t\t\t\t" + f"return res.c_str();")
                src.append("\t\t\t};")
            elif cb["ret"] == "void":
                src.append("\t\t\t" + f"{f['name']} = []({args}) {{ static_cast<{thistype}>({thisarg})->py_{f['name']}({callargs}); }};")
            else:
                src.append("\t\t\t" + f"{f['name']} = []({args}) -> {cb['ret']} {{ return static_cast<{thistype}>({thisarg})->py_{f['name']}({callargs}).cast<{cb['ret']}>(); }};")

            src.append("\t\t}")
            src.append("\t\telse")
            src.append(f"\t\t\t{f['name']} = nullptr;")
            src.append("\t}\n")

        src.append("};\n")

    src.append("")


def generate_category(categoryname, catobj, outputdir):
    src = [f'#include "rdapi_{categoryname}.h"',
           f'#include "rdapi_all.h"',
           "\n"]

    generate_struct_proxies(catobj, src)
    src.append(f"void bind{categoryname.capitalize()}(pybind11::module& m) {{")

    generate_handles(catobj, src)
    generate_enums(catobj, src)
    generate_structs(catobj, src)
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
