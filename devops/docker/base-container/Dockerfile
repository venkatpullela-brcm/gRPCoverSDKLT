FROM ubuntu:latest

# define variables
ARG usr_local_path='/usr/local'
ARG protoc_path="${usr_local_path}/bin/protoc"
ARG protobuf_dest_folder='/tmp/protobuf'
ARG protobuf_branch='v3.6.1'
ARG grpc_branch='v1.17.2'
ARG cmake_version='3.17.0'
ENV TZ=America/Los_Angeles
ENV DEBIAN_FRONTEND=noninteractive
ARG grpc_dest_folder="/tmp/grpc"

RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone
RUN apt-get -y install tzdata &&\
    dpkg-reconfigure --frontend ${DEBIAN_FRONTEND} tzdata

# Install.
RUN apt-get update && \
    apt-get -y upgrade && \
    apt-get install -y gcc &&\
    apt-get install -y g++ &&\
    apt-get install -y make &&\
    apt-get install -y git &&\
    apt-get install -y autoconf &&\
    apt-get install -y automake &&\
    apt-get install -y libtool &&\
    apt-get install -y build-essential && \
    apt-get install -y cmake &&\
    apt-get install -y curl htop man unzip vim wget && \
    rm -rf /var/lib/apt/lists/*

# Installing python3 and virtualenv
RUN apt-get update &&\
    apt-get install -y software-properties-common &&\
    add-apt-repository ppa:deadsnakes/ppa && \
    apt-get update &&\
    apt-get install -y python3.6 python3.6-dev python3-pip &&\
    apt-get install -y virtualenv &&\
    python3.6 -m pip install pip --upgrade &&\
    pip3 install setuptools wheel

# Installing protobuf
RUN git clone --recursive https://github.com/protocolbuffers/protobuf.git ${protobuf_dest_folder} &&\
    cd ${protobuf_dest_folder} && \
    git checkout -b ${protobuf_branch} &&\
    ./autogen.sh &&\
    ./configure --prefix="${usr_local_path}" &&\
    make &&\
    make check &&\
    make install &&\
    ldconfig &&\
    cd python &&\
    python3.6 setup.py install &&\
    rm -rf ${protobuf_dest_folder}

# Installing cmake-3.17.0
RUN apt-get --qq -y remove cmake &&\
    cd /tmp &&\
    wget https://github.com/Kitware/CMake/releases/download/v${cmake_version}/cmake-${cmake_version}.tar.gz &&\
    tar -zxvf cmake-${cmake_version}.tar.gz &&\
    cd cmake-${cmake_version} &&\
    ./bootstrap --prefix='/usr' &&\
    make &&\
    make install &&\
    cmake --version &&\
    rm -rf /tmp/cmake-${cmake_version}.tar.gz &&\
    rm -rf /tmp/cmake-${cmake_version}

# Installing gRPC
RUN git clone https://github.com/grpc/grpc.git ${grpc_dest_folder} &&\
    cd ${grpc_dest_folder} &&\
    git checkout tags/${grpc_branch} &&\
    git submodule update --init --recursive &&\
    mkdir -p cmake/build &&\
    cd cmake/build &&\
    cmake -DgRPC_INSTALL=ON -DgRPC_BUILD_TESTS=OFF\
          -DgRPC_PROTOBUF_PROVIDER=package \
          -DgRPC_ZLIB_PROVIDER=package \
          -DgRPC_CARES_PROVIDER=package \
          -DgRPC_SSL_PROVIDER=package \
          -DCMAKE_BUILD_TYPE=Release ../../ &&\
    make &&\
    make install &&\
    ldconfig &&\
    cd ../../ &&\
    python3.6 -m pip install grpcio &&\
    rm -rf {grpc_dest_folder}

# installing grpcio-tools
RUN python3.6 -m pip install grpcio-tools

ADD scripts/entrypoint.sh /usr/local/bin/entrypoint.sh
RUN chmod +x /usr/local/bin/entrypoint.sh
ENTRYPOINT [ "entrypoint.sh" ]