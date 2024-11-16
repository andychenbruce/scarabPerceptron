FROM ubuntu:20.04

RUN apt update
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y cmake make git g++ python3 wget zlib1g-dev libsnappy-dev libconfig++-dev

# COPY . /app/

WORKDIR /root
RUN wget -nc https://software.intel.com/sites/landingpage/pintool/downloads/pin-3.15-98253-gb56e429b1-gcc-linux.tar.gz
RUN tar --no-same-owner -xzvf pin-3.15-98253-gb56e429b1-gcc-linux.tar.gz

ENV PIN_ROOT /root/pin-3.15-98253-gb56e429b1-gcc-linux
ENV SCARAB_ENABLE_PT_MEMTRACE 1
ENV LD_LIBRARY_PATH /root/pin-3.15-98253-gb56e429b1-gcc-linux/extras/xed-intel64/lib
ENV LD_LIBRARY_PATH /root/pin-3.15-98253-gb56e429b1-gcc-linux/intel64/runtime/pincrt:$LD_LIBRARY_PATH

COPY infra/cse220/run_exp_using_descriptor.py /usr/local/bin
COPY infra/cse220/run_cse220.sh /usr/local/bin
RUN sed -i '1s/^/#!\/usr\/bin\/env python3\n/' /usr/local/bin/run_exp_using_descriptor.py

COPY andy.sh /root

# WORKDIR /app/src/
# RUN make
