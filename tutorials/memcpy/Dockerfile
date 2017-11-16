FROM ubuntu:16.04 as hld_env

RUN \
  apt-get update && \
  apt-get -y install wget && \
  apt-get -y install build-essential gcc g++ python3 && \
  wget http://accellera.org/images/downloads/standards/systemc/systemc-2.3.1.tgz && \
  tar xvfz systemc-2.3.1.tgz && \
  cd systemc-2.3.1 && \
  mkdir build && \
  cd build && \
  ../configure && \
  make && \
  make install

RUN \
  apt-get -y install git && \
  git clone https://github.com/google/googletest && \
  cd googletest/googletest/make && \
  make

FROM hld_env as tutorial_memcpy

ENV SC_DIR=/systemc-2.3.1
ENV GTEST_DIR=/googletest/googletest

RUN \
  git clone https://github.com/intel/rapid-design-methods-for-developing-hardware-accelerators.git hld && \
  cd hld && \
  cd tutorials/memcpy/systemc && \
  make

CMD (cd hld/tutorials/memcpy/systemc/acc_build; ./acc_test)
  
