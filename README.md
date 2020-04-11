# README #

This is the Basic Version of "SDKLT over gRPC" in C++.

### What is this repository for? ###

* Quick summary
  This has basic code of initializing the Chip

* Version
  1.0

### How do I get set up? ###

* Installation
There are two ways to build the OpenNPL GRPC and SDKLT:
1. Virtual machine (vagrant) - (Recommended method)
2. Docker (WIP)

#### Building virtual machine using Vagrant and Virtualbox ####
1. Install Oracle VirtualBox downloading it following website
```
https://www.virtualbox.org/wiki/Downloads
```

2. Install Vagrant on any of the following operating system MAC OSX, Linux OS and Windows 10.
```
Follow the guide of installation guide
https://www.vagrantup.com/downloads.html

Note: Make sure you download vagrant version >= 2.0.x
```

3. Install the vagrant plugin at vagrant-disksize
```
vagrant plugin install vagrant-disksize
```

4. For provisioning the virtual machine execute the following command
```
cd gRPCoverSDKLT/vagrant
vagrant up
```

5. The bootstrap shell script will install the dependencies such as protobuf, grpc, sdklt, python3, etc.
After the virtual machine is provisioned successfully. Login to the Virtual machine using following command.
```
vagrant ssh
```
Please Note: open two terminal one for client other for server

6. After you are done using the Virtual Machine (VM). The VM can be halted or powered down using this command.
```
vagrant halt
```

After this step please move on to the topic "Running the application"

#### Instructions to build OpenNPL and SDKLT using docker container  ####

1. Base container uses the latest Ubuntu image. It installs following dependencies
such as protobuf and grpc.
```
cd devops/docker
docker build -t base-container:1.0 -f base-container/Dockerfile .
```

2. Building sdklt-image. It install sdklt image and builds dependencies specific to demo application.
The docker container currently only supports the TARGET_SYSTEM=native_thsim. For xlr_linux the user has
to modify the dockerfile install crosscomplier specific to xlr_linux target system.
For more info please go through the building of bcm sdklt
https://github.com/Broadcom-Network-Switching-Software/SDKLT/wiki/Building-the-Demo-App.
```
cd devops
docker build -t sdklt-image:1.0 -f sdklt-image/Dockerfile .
```

3. Running the sdklt-image exec bash prompt
```
docker run -it sdklt-image:1.0 bash
```

4. Launching the sldklt binary inside docker container
```
root@275f160172fb:/sdklt/src/appl/demo/build/native_thsim# ./sdklt
SDKLT Demo Application (c) 2018 Broadcom Ltd.
Release 0.9 built on Tue, 10 Mar 2020 23:42:36 -0700
Found 1 device.
Bus error
```

5. Used gdb to debug why the Bus error is happening. Need further debugging WIP.
```
(gdb)
SDKLT Demo Application (c) 2018 Broadcom Ltd.
Release 0.9 built on Tue, 10 Mar 2020 23:42:36 -0700
Found 1 device.
[New Thread 0x7f6b55cba700 (LWP 137)]
[New Thread 0x7f6b55cad700 (LWP 138)]
[New Thread 0x7f6b55ca0700 (LWP 139)]
[New Thread 0x7f6b55c93700 (LWP 140)]
[New Thread 0x7f6b55c8a700 (LWP 141)]
[New Thread 0x7f6b55c81700 (LWP 142)]
[New Thread 0x7f6b55ade700 (LWP 143)]
[New Thread 0x7f6b55ad1700 (LWP 144)]
[New Thread 0x7f6b54080700 (LWP 145)]

Thread 4 "SHR_SYSM_INST" received signal SIGBUS, Bus error.
[Switching to Thread 0x7f6b55ca0700 (LWP 139)]
__memset_erms () at ../sysdeps/x86_64/multiarch/memset-vec-unaligned-erms.S:141
141	../sysdeps/x86_64/multiarch/memset-vec-unaligned-erms.S: No such file or directory.
(gdb)
```

### Running the application ###

* Summary of set up
    * change directory to /home/vagrant/gRPCoverSDKLT by sshing to VM using vagrant ssh
    ```
    cd /home/vagrant/gRPCoverSDKLT
    ```
    * Now in this directory execute `make TARGET_PLATFORM=native_thsim` to compile
    ```
    make TARGET_PLATFORM=native_thsim
    ```
    * This will generate two object files sdklt_client and sdklt_server

* How to run tests
    * After executing compiling, two object files "sdklt_client & sdklt_server" will be formed. By Default, sdklt_server is listening on 50051 port.

* Deployment instructions

### Contribution guidelines ###

* Writing tests
* Code review
* Other guidelines

### Who do I talk to? ###

* Repo owner or admin
* Other community or team contact