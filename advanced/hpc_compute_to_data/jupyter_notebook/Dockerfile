FROM jupyter/base-notebook

RUN conda init

RUN conda install -y -c conda-forge matplotlib numpy

USER jovyan

COPY mymodule/ /home/jovyan/work/mymodule/
#RUN chown jovyan.users /home/jovyan/work/lpfilter.ipynb
ADD lpfilter.ipynb /home/jovyan/work/.
RUN jupyter trust /home/jovyan/work/lpfilter.ipynb
COPY mymodule/ /home/jovyan/work/mymodule
#RUN chown -R jovyan.users /home/jovyan/work/mymodule

