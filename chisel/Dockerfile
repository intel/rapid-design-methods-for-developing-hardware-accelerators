FROM openjdk:8 as withverilator

RUN \
  apt-get update && \
  apt-get -y install make autoconf flex bison software-properties-common && \
  apt-get install -y \
     gcc-6 g++-6 gcc-6.base && \
  update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-6 100 && \
  update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-6 100 && \  
  git clone http://git.veripool.org/git/verilator && \
  (cd verilator; git pull && git checkout verilator_3_886 && \
  autoconf && ./configure && make && make install)

FROM withverilator as chisel3

ENV SBT_VERSION 0.13.16

RUN \
  curl -L -o sbt-$SBT_VERSION.deb http://dl.bintray.com/sbt/debian/sbt-$SBT_VERSION.deb && \
  dpkg -i sbt-$SBT_VERSION.deb && \
  rm sbt-$SBT_VERSION.deb && \
  apt-get update && \
  apt-get install sbt && \
  sbt sbtVersion


WORKDIR /ChiselRoot
ADD ./modifyVersion.csh /ChiselRoot

RUN \
  git clone https://github.com/ucb-bar/chisel3.git && \
  git clone https://github.com/ucb-bar/firrtl.git && \
  git clone https://github.com/ucb-bar/firrtl-interpreter.git && \
  git clone https://github.com/ucb-bar/chisel-testers.git && \
  (cd firrtl; git checkout 3dd921f38f298c7c4aa338e14ac43bc77c652e8c) && \
  (cd chisel3; git checkout 8168a8eea6c3465966081c5acd0347e09791361c) && \
  (cd firrtl-interpreter; git checkout b99bad1cc841b525516823fee870b3837a430c88) && \
  (cd chisel-testers; git checkout f1f2645690de370063af01f86c5fe6e49a462f3b) && \
  apt-get -y install tcsh && \
  ./modifyVersion.csh && \
  (cd firrtl; sbt assembly publishLocal) && \
  (cd chisel3; sbt publishLocal) && \
  (cd firrtl-interpreter; sbt publishLocal) && \
  (cd chisel-testers; sbt publishLocal)

FROM chisel3 as hldchisel

WORKDIR /ChiselRoot
ADD . /ChiselRoot

RUN \
  (cd reporters; sbt publishLocal) && \
  (cd testutil; sbt publishLocal) && \
  (cd pipe_insert_transform; sbt publishLocal) && \
  (cd designutils; sbt publishLocal) && \
  (cd accio; sbt publishLocal) && \
  (cd dataflow_building_blocks; sbt publishLocal)
