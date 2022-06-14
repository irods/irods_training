#!/bin/bash
CMDDIR=$(dirname "$0")
cd "$CMDDIR"
echo -n "Building image..." >&2
docker build --quiet=true --rm -t pdf_from_latex -f Dockerfile.pdflatex . || {
   echo "Docker must be installed for this script to work." >&2
   exit 1
}
echo
echo "Converting TEX to PDF ..." >&2
docker run --rm -v `pwd`:/tmp -t pdf_from_latex *.tex
make clean
