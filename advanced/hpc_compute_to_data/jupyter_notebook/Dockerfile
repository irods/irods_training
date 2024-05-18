FROM jupyter/base-notebook

USER root

COPY lpfilter.ipynb /home/jovyan/work/.

COPY mymodule/ /home/jovyan/work/mymodule/

USER jovyan

RUN conda init

RUN conda install -y -c conda-forge matplotlib numpy

RUN jupyter trust /home/jovyan/work/lpfilter.ipynb

RUN mkfifo /tmp/fifo

CMD cat /tmp/fifo
