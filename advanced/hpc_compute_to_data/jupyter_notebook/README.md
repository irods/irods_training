
Sequence for building.
---------------------

Part one: the published image.

   - Built the time-consuming part of the docker image (the conda install is the culprit!):
     ```
     docker build -t dwmoore/intermediate-notebook -f Dockerfile .
     ```
   - Pushed dwmoore/intermediate-notebook to Docker Hub.

Part two: the local GID/UID customized image

   - 
     ```
     docker build -t testimages/jupyter-digital-filter . -f Dockerfile.2 --build-arg irods_uid=$(id -u irods) --build-arg irods_gid=$(id -g irods)
     ```

