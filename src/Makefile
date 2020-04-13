#
# Copyright 2019
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
# IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
# DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
# OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
# THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#
#
# Authors: Kaushik, Koneru
#

ifndef SDK
nosdk:; @echo 'The $$SDK environment variable is not set'; exit 1
endif

ifndef TARGET_PLATFORM_DIR
TARGET_PLATFORM_DIR = $(SDK)/appl/make
endif

ifndef TARGET_PLATFORM

help:
	@echo
	@echo Broadcom SDKLT gRPC DEMO application makefile.
	@echo
	@echo Please specify a target platform, e.g.:
	@echo
	@echo make TARGET_PLATFORM=native_thsim $(MAKECMDGOALS)
	@echo make TARGET_PLATFORM=xlr_linux $(MAKECMDGOALS)
	@echo

clean: help

else
include $(TARGET_PLATFORM_DIR)/$(TARGET_PLATFORM).mk

# Build the SDK library from here
SDKLIB = $(SDK)/appl/sdklib

# Build the Linux kernel modules from here
SDKMOD = $(SDK)/appl/linux

# Place the library build objects here
SDK_BLDDIR = $(SDKLIB)/build/$(TARGET_PLATFORM)

# Install SDK header files here
SDK_INCDIR = $(SDK_BLDDIR)/include/sdklt

# Install SDK library files here
SDK_LIBDIR = $(SDK_BLDDIR)/lib

SDK_INCLUDES += -I$(SDK_INCDIR)

HOST_SYSTEM = $(shell uname | cut -f 1 -d_)
SYSTEM ?= $(HOST_SYSTEM)
CXX = g++
CPPFLAGS += `pkg-config --cflags protobuf grpc`
CXXFLAGS += -std=c++11 -ggdb -O0 $(SDK_INCLUDES)
ifeq ($(SYSTEM),Darwin)
LDFLAGS += -L/usr/local/lib `pkg-config --libs protobuf grpc++`\
           -pthread\
           -lgrpc++_reflection\
           -ldl -lrt
else
LDFLAGS += -L/usr/local/lib `pkg-config --libs protobuf grpc++`\
           -pthread\
           -Wl,--no-as-needed -lgrpc++_reflection -Wl,--as-needed\
           -ldl -lrt
endif

include $(SDK)/appl/make/yaml.mk
LDFLAGS += $(YAML_LDFLAGS)
LDFLAGS += $(YAML_LDLIBS) $(OPENSRC_LIBS)
CMAKE_C_FLAGS +="-ggdb -O0"

PROTOC = protoc
GRPC_CPP_PLUGIN = grpc_cpp_plugin
GRPC_CPP_PLUGIN_PATH ?= `which $(GRPC_CPP_PLUGIN)`
SDK_LDFLAGS = $(SDK)/appl/sdklib/build/$(TARGET_PLATFORM)/lib/libsdklt.a
SDK_LDFLAGS += $(YAML_LDFLAGS) $(OPENSRC_LIBS)
PROTOS_PATH = ../proto
BUILD_DIR := ../build

vpath sdklt.proto $(PROTOS_PATH)

all: system-check sdklt_server sdklt_client clean_pb 

sdklt_client: sdklt.pb.o sdklt.grpc.pb.o sdklt_client.o 
	mkdir -p $(BUILD_DIR)
	$(CXX) $^ $(LDFLAGS) -o $(BUILD_DIR)/$@

sdklt_server: sdklt.pb.o sdklt.grpc.pb.o sdklt_server.o
	mkdir -p $(BUILD_DIR)
	$(CXX) $^ $(SDK_LDFLAGS) $(SDK_INCLUDES) $(LDFLAGS) -o $(BUILD_DIR)/$@

clean_pb:
	rm -f *.pb.cc *.pb.h *.o

.PRECIOUS: sdklt.grpc.pb.cc
sdklt.grpc.pb.cc: sdklt.proto
	mkdir -p $(BUILD_DIR)
	$(PROTOC) -I $(PROTOS_PATH) --grpc_out=. --plugin=protoc-gen-grpc=$(GRPC_CPP_PLUGIN_PATH) $<

.PRECIOUS: sdklt.pb.cc
sdklt.pb.cc: sdklt.proto
	mkdir -p $(BUILD_DIR)
	$(PROTOC) -I $(PROTOS_PATH) --cpp_out=. $<

clean:
	rm -rf $(BUILD_DIR)


# The following is to test your system and ensure a smoother experience.
# They are by no means necessary to actually compile a grpc-enabled software.

PROTOC_CMD = which $(PROTOC)
PROTOC_CHECK_CMD = $(PROTOC) --version | grep -q libprotoc.3
PLUGIN_CHECK_CMD = which $(GRPC_CPP_PLUGIN)
HAS_PROTOC = $(shell $(PROTOC_CMD) > /dev/null && echo true || echo false)
ifeq ($(HAS_PROTOC),true)
HAS_VALID_PROTOC = $(shell $(PROTOC_CHECK_CMD) 2> /dev/null && echo true || echo false)
endif
HAS_PLUGIN = $(shell $(PLUGIN_CHECK_CMD) > /dev/null && echo true || echo false)

SYSTEM_OK = false
ifeq ($(HAS_VALID_PROTOC),true)
ifeq ($(HAS_PLUGIN),true)
SYSTEM_OK = true
endif
endif

system-check:
ifneq ($(HAS_VALID_PROTOC),true)
	@echo " DEPENDENCY ERROR"
	@echo
	@echo "You don't have protoc 3.0.0 installed in your path."
	@echo "Please install Google protocol buffers 3.0.0 and its compiler."
	@echo "You can find it here:"
	@echo
	@echo "   https://github.com/google/protobuf/releases/tag/v3.0.0"
	@echo
	@echo "Here is what I get when trying to evaluate your version of protoc:"
	@echo
	-$(PROTOC) --version
	@echo
	@echo
endif
ifneq ($(HAS_PLUGIN),true)
	@echo " DEPENDENCY ERROR"
	@echo
	@echo "You don't have the grpc c++ protobuf plugin installed in your path."
	@echo "Please install grpc. You can find it here:"
	@echo
	@echo "   https://github.com/grpc/grpc"
	@echo
	@echo "Here is what I get when trying to detect if you have the plugin:"
	@echo
	-which $(GRPC_CPP_PLUGIN)
	@echo
	@echo
endif
ifneq ($(SYSTEM_OK),true)
	@false
endif
endif
