#! /bin/python3

from pathlib import Path
import glob, sys, os

SKIP_PATHS = []
API_FOLDER = "rdapi"


def can_skip(header: str):
    for skippath in SKIP_PATHS:
        return skippath in header


def generate_header(filename: str, searchpath: str):
    relpathidx = len(searchpath) + 1
    headers = glob.glob(str(Path(searchpath, API_FOLDER, "**", "*.h*")), recursive=True)

    with open(filename, "w") as f:
        f.write("#pragma once\n\n")

        for header in headers:
            if can_skip(header): continue
            relheader = header[relpathidx:]
            f.write(f"#include <{relheader}>\n")


if __name__ == "__main__":
    generate_header(sys.argv[1], sys.argv[2])
