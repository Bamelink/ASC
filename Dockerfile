FROM ubuntu:20.04

WORKDIR /work

RUN apt update

RUN apt install wget -y


# Zephyr install
RUN wget https://apt.kitware.com/kitware-archive.sh && bash kitware-archive.sh

ARG DEBIAN_FRONTEND=noninteractive

RUN apt install -y --no-install-recommends git cmake ninja-build gperf \
 ccache dfu-util device-tree-compiler wget \
 python3-dev python3-pip python3-setuptools python3-tk python3-wheel xz-utils file \
 make gcc gcc-multilib g++-multilib libsdl2-dev libmagic1

RUN pip3 install --user -U west
RUN echo $PATH
ENV PATH="$PATH:/root/.local/bin"
RUN echo $PATH

RUN mkdir /work/zephyrproject && cd /work/zephyrproject && git clone https://github.com/Bamelink/ASC.git
RUN cd /work/zephyrproject && west init .
RUN cd /work/zephyrproject/zephyr && git checkout 4256cd4 && cd .. && west update

RUN cd /work/zephyrproject && west zephyr-export

RUN cd /work/zephyrproject && pip3 install --user -r /work/zephyrproject/zephyr/scripts/requirements.txt


#Zephyr SDK
RUN wget https://github.com/zephyrproject-rtos/sdk-ng/releases/download/v0.15.0/zephyr-sdk-0.15.0_linux-x86_64.tar.gz
RUN wget -O - https://github.com/zephyrproject-rtos/sdk-ng/releases/download/v0.15.0/sha256.sum | shasum --check --ignore-missing

RUN tar xvf zephyr-sdk-0.15.0_linux-x86_64.tar.gz

RUN cd zephyr-sdk-0.15.0 && ./setup.sh -t all -h -c

#Cleanup
RUN rm kitware-archive.sh zephyr-sdk-0.15.0_linux-x86_64.tar.gz


# Needed for uROS
RUN pip3 install catkin_pkg lark-parser empy colcon-common-extensions