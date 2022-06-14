# iRODS Training

This repository holds the training manuals and example codes used in the
iRODS Training sessions led by the iRODS Consortium.

## Beginner

The Beginner training is self contained within a single document.  It leads the student through a scenario and provides step-by-step instructions to explore the breadth of basic iRODS functionality.

It can be found in the ./beginner directory.

The prerequisites for building the PDF are `make`, `pdflatex`, `memoir.cls`, and `isodate.sty` which can be provided in Ubuntu with

```
apt-get install -y make texlive-latex-base texlive-latex-recommended texlive-latex-extra
```

`make` will then generate the PDF.

`make clean` will remove any generated files.

Alternatively, or on OSes not derived from Debian, Docker may be installed and the PDF conversion achieved with: `sudo bash pdf_from_latex.sh`.

## Advanced

The Advanced training assumes a working knowledge and understanding of basic iRODS functionality and terminology.  It is not as self contained, but provides the code examples used by the instructors.  The code examples should be executable.

These materials can be found in the ./advanced directory.

