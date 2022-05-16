FROM ubuntu:latest
RUN apt-get -y update && apt-get install -y
RUN apt-get -y install sudo clang cmake wget pkg-config
COPY . /polaris
ADD scripts /scripts
WORKDIR /polaris
RUN bash /scripts/install_cpputest.sh
RUN mkdir /polaris/docker-build && \
          cd /polaris/docker-build && \
          cmake ../ && \
          make -j9