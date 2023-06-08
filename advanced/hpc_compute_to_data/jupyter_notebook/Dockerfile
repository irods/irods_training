FROM jupyter/base-notebook

ARG  irods_uid=997
ENV  IRODS_UID ${irods_uid}
ARG  irods_gid=998
ENV  IRODS_GID ${irods_gid}

USER root

RUN apt-get update && apt-get install -y vim less

RUN groupadd -g $IRODS_GID irods && usermod -aG irods jovyan 

RUN sed -i "s/jovyan:x:[0-9]*:[0-9]*\(.*\)/jovyan:x:${IRODS_UID}:${IRODS_GID}\1/" /etc/passwd

ADD lpfilter.ipynb /home/jovyan/work/.

COPY mymodule/ /home/jovyan/work/mymodule/

RUN chown jovyan.users /home/jovyan/work/lpfilter.ipynb

COPY mymodule/ /home/jovyan/work/mymodule

RUN chown -R jovyan.users /home/jovyan/work/mymodule

RUN chown -R ${IRODS_UID}:${IRODS_GID} /home/jovyan && chown -R ${IRODS_UID}:${IRODS_GID} /opt/conda

USER jovyan

RUN conda init

RUN conda install -y -c conda-forge matplotlib numpy

RUN jupyter trust /home/jovyan/work/lpfilter.ipynb

CMD [ '/bin/bash' ]
