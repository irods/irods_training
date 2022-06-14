#!/bin/bash
CMDDIR=$(dirname "$0")
cd "$CMDDIR"
docker build --quiet=true --rm -t pdf_from_latex -f Dockerfile.pdflatex . || {
   echo "Docker must be installed for this script to work." >&2
   exit 1
}
docker run --rm -v `pwd`:/tmp -t pdf_from_latex *.tex
make clean
