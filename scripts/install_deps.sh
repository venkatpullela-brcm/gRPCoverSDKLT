#!/bin/bash

# immediately exit if a program returns non zero output
set -xe

python_version='3.6.9'
protobuf_branch='master'
ubuntu_release=`lsb_release -s -r`
grpc_branch='tags/v1.28.0'
usr_local_path='/usr/local'
libyaml_dest_folder='/home/vagrant/libyaml'
cmake_version='3.17.0'

if [[ "${ubuntu_release}" > "18" ]]; then
    # logic taken from \
    # https://github.com/p4lang/behavioral-model/blob/master/install_deps.sh
    # This older package libssl1.0-dev enables compiling Thrift 0.9.2
    # on Ubuntu 18.04.  Package libssl-dev exists, but Thrift 0.9.2
    # fails to compile when it is installed.
    LIBSSL_DEV="libssl1.0-dev"
else
    LIBSSL_DEV="libssl-dev"
fi

#Get the number of cores to speed up the compilation process
num_cores=`grep -c ^processor /proc/cpuinfo`


set_time_zone()
{
    ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone
    sudo apt-get -y install tzdata
    dpkg-reconfigure --frontend ${DEBIAN_FRONTEND} tzdata
}


install_packages_deps()
{
    echo "Installing package dependencies"
    sudo apt-get update
    sudo apt-get -qq -y install gcc
    sudo apt-get -qq -y install g++
    sudo apt-get -qq -y install ntpdate
    sudo apt-get -qq -y install make
    sudo apt-get -qq -y install git
    sudo apt-get -qq -y install build-essential
    sudo apt-get -qq -y install apt-transport-https
    sudo apt-get -qq -y install lsb-release
    sudo apt-get -qq -y install ca-certificates
    sudo apt-get -qq -y install checkinstall
    sudo apt-get -qq -y install wget curl
    sudo apt-get -qq -y install cscope
    sudo apt-get -qq -y install tcpdump traceroute
    sudo apt-get -qq -y install python-dev
    sudo apt-get -qq -y install python3-dev
    sudo apt-get -qq -y install libc-ares-dev
    # will remove python2 pip install after 1st Jan 2020
    curl https://bootstrap.pypa.io/get-pip.py -o /tmp/get-pip.py
    python /tmp/get-pip.py
    sudo pip install --upgrade pip
    sudo pip install --upgrade setuptools

}

install_cmake()
{
    cd /tmp
    wget https://github.com/Kitware/CMake/releases/download/v${cmake_version}/cmake-${cmake_version}.tar.gz
    tar -zxvf cmake-${cmake_version}.tar.gz
    cd cmake-${cmake_version}
    ./bootstrap --prefix='/usr'
    make -j${num_cores}
    sudo make install
    cmake --version
    if [[ ! $? -eq 0 ]]; then
        echo "failed to install cmake"
        exit 1
    fi
    cd $HOME && rm -rf /tmp/cmake-${cmake_version}
}

check_cmake_and_install()
{

    echo "installing cmake-${cmake_version}"
    cmd=$(cmake --version)
    echo "cmake version: ${cmd}"
    exp_cmake_version="3.13"
    regex='.*([0-9]+\.[0-9]+)\.[0-9]+'
    if [[ $cmd =~ $regex ]]; then
        echo "current version: ${BASH_REMATCH[1]}"
        cmake_ver=${BASH_REMATCH[1]}
        if [[ "`echo "$cmake_ver < $exp_cmake_version" | bc`" -eq 1 ]]; then
            echo "cmake current version is less than expected version"
            sudo apt-get -qq -y remove cmake
            install_cmake
        else
            echo "cmake current version is greather than expected version"
        fi
    else
        install_cmake
    fi
}

install_grpc_deps()
{
    echo "installing grpc sw dependencies"
    sudo apt-get update
    sudo apt-get -qq -y install build-essential
    sudo apt-get -qq -y install autoconf
    sudo apt-get -qq -y install libtool
    sudo apt-get -qq -y install pkg-config
}


install_protobuf_deps()
{
    echo "installing protobuf sw dependencies"
    sudo apt-get update
    sudo apt-get -qq -y install autoconf
    sudo apt-get -qq -y install automake
    sudo apt-get -qq -y install libtool
    sudo apt-get -qq -y install curl
    sudo apt-get -qq -y install make
    sudo apt-get -qq -y install g++
    sudo apt-get -qq -y install unzip
}

install_bazel()
{
    sudo apt install curl
    curl https://bazel.build/bazel-release.pub.gpg | sudo apt-key add -
    echo "deb [arch=amd64] https://storage.googleapis.com/bazel-apt stable jdk1.8" | \
        sudo tee /etc/apt/sources.list.d/bazel.list
    sudo apt update && sudo apt install bazel
}


install_grpc()
{
    install_grpc_deps
    check_cmake_and_install
    dest_folder="/home/vagrant/grpc"
    if [[ -d ${dest_folder} ]]; then
        rm -rf ${dest_folder}
    fi
    git clone https://github.com/grpc/grpc.git ${dest_folder} &
    b=$!
    echo "checking cloning status"
    wait $b && echo OK || exit 1
    cd ${dest_folder}
    git submodule update --init --recursive
    git checkout ${grpc_branch}
    mkdir -p cmake/build
    cd cmake/build
    cmake ../.. -DgRPC_INSTALL=ON              \
              -DCMAKE_BUILD_TYPE=Release       \
              -DgRPC_CARES_PROVIDER=module    \
              -DgRPC_PROTOBUF_PROVIDER=module \
              -DgRPC_SSL_PROVIDER=module      \
              -DgRPC_ZLIB_PROVIDER=module
    make -j${num_cores}
    sudo make install
    sudo ldconfig
    cd ../../
    install_protobuf ${dest_folder}/third_party/protobuf
    cmd=$(which grpc_cpp_plugin)
    if [[ -z ${cmd} ]]; then
        echo "Unable to find grpc_cpp_plugin: ${cmd}"
        exit 1
    fi
    sudo python3 -m pip install grpcio
    sudo python3 -m pip install grpcio-tools
}


install_protobuf()
{
    protobuf_path=""
    protoc_path="${usr_local_path}/bin/protoc"
    install_protobuf_deps
    if [[ -z "$1" ]]; then
        echo "args are empty"
        if [[ ! -f ${protoc_path} ]]; then
            dest_folder='/tmp/protobuf'
            if [[ -d "${dest_folder}" ]]; then
                rm -rf ${dest_folder}
            fi
            git clone --recursive https://github.com/protocolbuffers/protobuf.git ${dest_folder}
            cd ${dest_folder}
            git checkout ${protobuf_branch}
            make_protobuf
            cd $HOME
            rm -rf ${dest_folder}
        else
            echo "protobuf already installed: ${protoc_path}"
        fi
    else
        protobuf_path=$1
        echo "protobuf package folder: ${protobuf_path}"
        if [[ ! -f ${protoc_path} ]]; then
            cd ${protobuf_path}
            make_protobuf
        else
            echo "protobuf already installed: ${protoc_path}"
        fi
    fi
}

check_protoc_version()
{
    cmd=$(protoc --version)
    echo "Protoc version: $cmd"
}

make_protobuf()
{
    echo "building protobuf binary"
    ./autogen.sh
    ./configure --prefix="${usr_local_path}"
    make -j${num_cores}
    make check
    sudo make install
    # refresh shared library cache.
    sudo ldconfig
    # install python module
    cd python
    sudo python3 setup.py install
}

install_python_deps()
{
    # install_python
    echo "Installing python dependencies"
    sudo apt-get -qq -y install python3-dev
    sudo apt-get -qq -y install python3-pip
    sudo apt-get -qq -y install virtualenv
    sudo apt-get -qq -y install libyaml-dev
    sudo apt-get -qq -y install cmake
    sudo apt-get -qq -y install libreadline-gplv2-dev
    sudo apt-get -qq -y install libncursesw5-dev
    sudo apt-get -qq -y install ${LIBSSL_DEV}
    sudo apt-get -qq -y install libsqlite3-dev
    sudo apt-get -qq -y install tk-dev
    sudo apt-get -qq -y install libgdbm-dev
    sudo apt-get -qq -y install libc6-dev
    sudo apt-get -qq -y install libbz2-dev
    sudo pip3 install --upgrade pip
    sudo pip3 install --upgrade setuptools
}

install_python()
{
    # installing python
    echo "installing python"
    install_python_deps
    cd /tmp
    wget https://www.python.org/ftp/python/${python_version}/Python-${python_version}.tar.xz
    tar xvf Python-${python_version}.tar.xz
    cd Python-${python_version}/
    ./configure
    sudo make altinstall
    rm /tmp/Python-${python_version}.tar.xz
}

set_yaml_env()
{
    git clone https://github.com/yaml/libyaml.git ${libyaml_dest_folder}
    export YAML=${libyaml_dest_folder}
    export YAML_LIBDIR=${YAML}/src/.libs
}

install_sdklt()
{
    # install pyyaml for python2.7
    pip2 install pyyaml
    echo "installing bcm sdklt"
    export SDK=${sdklt_dest_folder}/src
    git clone https://github.com/Broadcom-Network-Switching-Software/SDKLT.git ${sdklt_dest_folder}
    cd ${sdklt_dest_folder}/src/appl/demo
    make -s TARGET_PLATFORM=${target_platform}
}

export_paths()
{
    echo "export YAML=${libyaml_dest_folder}" >> /home/vagrant/.profile
    echo "export YAML_LIBDIR=${YAML}/src/.libs" >> /home/vagrant/.profile
    echo "export SDK=${sdklt_dest_folder}/src" >> /home/vagrant/.profile
}

main()
{
    install_packages_deps
    set_time_zone
    install_python_deps
    # install grpc dependencies
    install_grpc
    set_yaml_env
    install_sdklt
    export_paths
}

export DEBIAN_FRONTEND=noninteractive
sdklt_dest_folder="/home/vagrant/sdklt"
target_platform='native_thsim'
TZ='America/Los_Angeles'
main
