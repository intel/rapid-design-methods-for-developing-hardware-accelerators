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

ENV SBT_VERSION 1.1.1

RUN \
  curl -L -o sbt-$SBT_VERSION.deb http://dl.bintray.com/sbt/debian/sbt-$SBT_VERSION.deb && \
  dpkg -i sbt-$SBT_VERSION.deb && \
  rm sbt-$SBT_VERSION.deb && \
  apt-get update && \
  apt-get install sbt && \
  sbt sbtVersion

RUN \
  apt-get -y install graphviz

RUN \
  git clone https://github.com/intel/rapid-design-methods-for-developing-hardware-accelerators.git hld

WORKDIR /hld/chisel

RUN \
  git clone https://github.com/freechipsproject/chisel3.git && \
  git clone https://github.com/freechipsproject/firrtl.git && \
  git clone https://github.com/freechipsproject/firrtl-interpreter.git && \
  git clone https://github.com/freechipsproject/chisel-testers.git && \
  git clone https://github.com/freechipsproject/treadle.git && \
  (cd firrtl; git checkout ed709571876b68e4982d11db15d236752713b6a1) && \
  (cd chisel3; git checkout 822a256891ef7aeb6e79aa7aeb569a152e5e37d4) && \
  (cd firrtl-interpreter; git checkout de2302f4fe7e61638e5a7e5d6aa6bbf73a59473c) && \
  (cd treadle; git checkout 5e2678da19b7613ad767df4514797207d26700fe) && \
  (cd chisel-testers; git checkout 5bbe52cdac7fa304a7d2aef75e82bac753971437) && \
  (cd firrtl; sbt assembly publishLocal) && \
  (cd chisel3; sbt publishLocal) && \
  (cd firrtl-interpreter; sbt publishLocal) && \
  (cd treadle; sbt publishLocal) && \
  (cd chisel-testers; sbt publishLocal)

FROM chisel3 as hldchisel

WORKDIR /hld/chisel

RUN \
  git pull && \
  (cd reporters; sbt publishLocal) && \
  (cd testutil; sbt publishLocal) && \
  (cd pipe_insert_transform; sbt publishLocal) && \
  (cd designutils; sbt publishLocal) && \
  (cd accio; sbt publishLocal) && \
  (cd dataflow_building_blocks; sbt publishLocal)
