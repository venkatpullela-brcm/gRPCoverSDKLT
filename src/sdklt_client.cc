/*
 * Copyright 2019
 *
 * Permission is hereby granted, free of charge, to any person obtaining a 
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation 
 * the rights to use, copy, modify, merge, publish, distribute, sublicense, 
 * and/or sell copies of the Software, and to permit persons to whom the 
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included 
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF 
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR 
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR 
 * THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * File: sdklt_client.cc
 *
 * Description: 
 *      This file contains the SDKLT gRPC Client function handlers
 *
 * Authors: Kaushik, Koneru
 *          Anand, Akella
 *          Venkat, Pullela
 */

#include <iostream>
#include <memory>
#include <string>

#include <grpcpp/grpcpp.h>
#include "sdklt.grpc.pb.h"


using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using sdklt::Api;
using sdklt::InitRequest;
using sdklt::InitState;
using sdklt::ShellRequest;
using sdklt::ShellState;
using sdklt::ShutDownRequest;
using sdklt::ShutDownState;
using sdklt::WriteRequest;
using sdklt::WriteResponse;
using sdklt::ReadRequest;
using sdklt::ReadResponse;
//using sdklt::Uint128;

 struct read_map {
    std::uint64_t device_id;
    std::uint64_t role_id;
    std::string lt_name;
    std::string key;
} rmap;

class sdkltClient {
    public:
        sdkltClient(std::shared_ptr<Channel> channel)
            : stub_(Api::NewStub(channel)) {}

        std::string bcmInit(const int unit) {
            InitRequest request;
            InitState state;
            ClientContext context;
            Status status;

            request.set_unit(unit);
            
            status = stub_->bcmInit(&context, request, &state);
            if (!status.ok()) {
                return "Communication Failed";
            }

            return state.message();
        }

        std::string bcmShutdown(bool graceful) {
            ShutDownRequest request;
            ShutDownState state;
            ClientContext context;
            Status status;

            request.set_graceful(graceful);
            
            status = stub_->bcmShutdown(&context, request, &state);
            if (!status.ok()) {
                return "Communication Failed";
            }

            return state.message();
        }

        std::string bshell(int unit, std::string cmd) {
            ShellRequest shell;
            ShellState shellRet;
            ClientContext context;
            Status status;

            shell.set_unit(unit);
            shell.set_cmd(cmd);
            status = stub_->bcmShell(&context, shell, &shellRet);
            if (!status.ok()) {
                return "Communication Failed";
            }
            return shellRet.message();
        }

        std::string openWrite(uint unit) {
            WriteRequest request;
            WriteResponse response;
            ClientContext context;
            Status status;

            request.set_device_id(unit);
            request.set_role_id(1);
            //request.set_allocated_election_id(election_id);
            status = stub_->openWrite(&context, request, &response);
            if (!status.ok()) {
                return "Communication Failed";
            }
            return response.message();
        }

        std::string openRead(read_map rmp) {
            ReadRequest request;
            ReadResponse response;
            ClientContext context;
            Status status;

            request.set_device_id(rmp.device_id);
            request.set_role_id(rmp.role_id);
            request.set_lt_name(rmp.lt_name);
            request.set_key(rmp.key);
            //request.set_allocated_election_id(election_id);
            status = stub_->openRead(&context, request, &response);
            if (!status.ok()) {
                return "Communication Failed";
            }
            return response.message();
        }

		std::string rShell(int unit, std::string cmd) {
			ShellRequest shell;
			ShellState shellRet;
			ClientContext context;
			Status status;

			shell.set_unit(unit);
			shell.set_cmd(cmd);


			status = stub_->RemoteShell(&context, shell, &shellRet);
			if (!status.ok()) {
				return "Communication Failed";
			}
			return shellRet.message();

		}


    private:
        std::unique_ptr<Api::Stub> stub_;
};


int
main() {
    sdkltClient sdklt_client(grpc::CreateChannel("0.0.0.0:50051", grpc::InsecureChannelCredentials()));
    std::string cmd;

    std::cout << "******************************************" << std::endl;   
    std::cout<<"Initializing chip"<<std::endl;
    std::cout<<sdklt_client.bcmInit(0)<<std::endl;
    
    /* Calling the openWrite method */
    std::cout << "******************************************" << std::endl;
    std::cout<<"openWrite method"<<std::endl;
    std::cout<<sdklt_client.openWrite(0)<<std::endl;
    
    /* TODO: Check for Proper Initialization, Below call is triggering network device init failed */
    std::cout << "******************************************" << std::endl;
    std::cout<<"sending CLI: lt VLAN lookup VLAN_ID=11"<<std::endl;
    std::cout<<sdklt_client.bshell(0, "lt VLAN lookup VLAN_ID=11")<<std::endl;;
    
    /* Calling the OpenRead method*/
    std::cout << "******************************************" << std::endl;
    std::cout<<"OpenRead"<<std::endl;
    read_map rmp;
    rmp.device_id = 0;
    rmp.role_id = 1;
    rmp.lt_name = "VLAN";
    rmp.key = "VLAN_ID";
    std::cout<<sdklt_client.openRead(rmp)<<std::endl;;

    /* Getting in Remote Shell */
    std::cout << "******************************************" << std::endl;
    while (1) {
        std::cout<<"rShell.0>";
        getline(std::cin, cmd);
        if (cmd.length() == 0) {
            continue;
        }
        if (cmd.compare("quit") == 0) {
            break;
        }
        std::cout<<sdklt_client.rShell(0, cmd);
    }
    std::cout << "******************************************" << std::endl;

    /* Calling the shutdown */
    std::cout << "******************************************" << std::endl;
    std::cout<<"bcmmgmt shutdown"<<std::endl;
    bool flag = true;
    std::cout<<sdklt_client.bcmShutdown(flag)<<std::endl;
    std::cout << "******************************************" << std::endl;

    return 0;
}
