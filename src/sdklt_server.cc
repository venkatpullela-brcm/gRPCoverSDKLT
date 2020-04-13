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
 * File: sdklt_server.cc
 *
 * Description: 
 *      This file contains the SDKLT gRPC Server function handlers
 *
 * Authors: Kaushik, Koneru
 *          Anand, Akella
 *          Venkat, Pullela
 */

#include "sdklt_server.h"

bcma_sys_conf_t sys_conf, *isc;

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
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
using sdklt::Api;

extern int bcma_grpc_out_hook(bsl_meta_t *meta, const char *format, va_list args);

class SdkLTServiceImpl final : public Api::Service {

    Status bcmInit(ServerContext* context, const InitRequest *request,
            InitState *state) override {
        int rv = 0;
        int ndev = 0;
        int unit = request->unit();
        const char *config_file = "config.yml";
        std::cout << "******************************************" << std::endl;
        std::cout << "\t BCM Init RPC called" << std::endl;
        if (!isc) {
            isc = &sys_conf;
            bcma_sys_conf_init(isc);
            std::cout << " Successfully Initialized sys conf" << std::endl;
        }

        //rv = bcma_bslmgmt_init();
		rv = bcma_bslgrpc_init();
        if (SHR_FAILURE(rv)) {
            state->set_message("bsl mgmt init failed");
            return Status::OK;
        }

        rv = cli_init(isc);
        if (SHR_FAILURE(rv)) {
            state->set_message("CLI init failed");
            return Status::OK;
        }

        ndev = bcma_sys_conf_drd_init(isc);
        if (ndev < 0) {
            state->set_message("sys drd init failed");
            return Status::OK;
        }

        std::cout<<"Found: "<<ndev<<" devices" << std::endl;
        bcma_ha_init(true, false);
        for (unit = 0; unit < BCMDRD_CONFIG_MAX_UNITS; unit++) {
            if (!bcmdrd_dev_exists(unit)) {
                continue;
            }
            rv = bcma_ha_unit_open(unit, DEFAULT_HA_FILE_SIZE, true, false);
            if (SHR_FAILURE(rv)) {
                std::cout<<"Unable to open HA unit "<<unit<< "  Reason: "<<rv << std::endl;
            }
        }

        rv = bcmmgmt_init(false, config_file);
        if (SHR_FAILURE(rv)) {
            state->set_message("bsl mgmt init failed");
            return Status::OK;
        }
        state->set_message("Successfully initialized Unit");
        return Status::OK;

    }

    Status bcmShell(ServerContext* context, const ShellRequest *shell, ShellState *state) override {
        std::cout << "******************************************" << std::endl;
        std::cout << "\t BCM Shell RPC called" << std::endl;
        char *cstr = new char[shell->cmd().length()+1];

        std::cout<<"Command : "<<shell->cmd()<<" Len: "<<shell->cmd().length()<<std::endl;

        strcpy(cstr, shell->cmd().c_str());
        std::cout<<"Executing Command : "<<cstr<<std::endl;
        bcma_cli_bshell(shell->unit(), cstr);
        state->set_message("Executed Command");
        delete [] cstr;

        return Status::OK;
    }

    Status bcmShutdown(ServerContext* context, const ShutDownRequest *request, ShutDownState *state) override {
        std::cout << "******************************************" << std::endl;
        std::cout << "\t BCM Shutdown RPC called" << std::endl;
        std::cout << "graceful value: " << request->graceful() << std::endl;
        int rv = 0;
        rv = bcmmgmt_shutdown(request->graceful());
        if (SHR_FAILURE(rv)) {
            state->set_message("shutdown bcmmgmt_shutdown failed");
            state->set_success(false);
            return Status::OK;
        }
        state->set_message("Successfully shutdowned bcmgmt");
        state->set_success(true);
        return Status::OK;

    }

    Status openRead(ServerContext* context, const ReadRequest *read, ReadResponse  *response) override {
        std::cout << "******************************************" << std::endl;
        std::cout << "\t Open Read RPC called" << std::endl;
        std::cout << "device id value: " << read->device_id() << std::endl;
        std::cout << "role id value: " << read->role_id() << std::endl;

        std::cout << "lt_name value: " << read->lt_name() << std::endl;
        char *cstr = new char[read->lt_name().length()+1];
        std::cout<<"lt_name : "<< read->lt_name()<<" Len: "<<read->lt_name().length()<<std::endl;
        strcpy(cstr, read->lt_name().c_str());

        std::cout << "key: " << read->key() << std::endl;
        char *key = new char[read->key().length()+1];
        std::cout<<"key : "<< read->key()<<" Len: "<<read->key().length()<<std::endl;
        strcpy(key, read->key().c_str());

        int rv = 0;
        bcmlt_entry_handle_t entry_hdl;
        std::uint64_t vlan_id = 0;
        bcmlt_entry_info_t e_info;

        rv = bcmlt_entry_allocate(read->device_id(), cstr, &entry_hdl);
        if (SHR_FAILURE(rv)) {
            response->set_message(shr_errmsg(rv));
            response->set_success(false);
            return Status::OK;
        }

        std::cout << "entry hdl: " << entry_hdl << std::endl;

        /* Commit the entry synchronously */
        rv = bcmlt_custom_entry_commit(entry_hdl,
                                       BCMLT_OPCODE_TRAVERSE,
                                       BCMLT_PRIORITY_NORMAL);
        if (SHR_FAILURE(rv)) {
            response->set_message(shr_errmsg(rv));
            response->set_success(false);
            return Status::OK;
        }

        rv = bcmlt_entry_field_get(entry_hdl, key, &vlan_id);
        std::cout << "vlan_id: " << vlan_id << std::endl;
        if (SHR_FAILURE(rv)) {
            response->set_message(shr_errmsg(rv));
            response->set_success(false);
            return Status::OK;
        }
        /* Free the entry handle */
        rv = bcmlt_entry_free(entry_hdl);
        if (SHR_FAILURE(rv)) {
            response->set_message(shr_errmsg(rv));
            response->set_success(false);
            return Status::OK;
        }
        response->set_message("success");
        response->set_success(true);
        return Status::OK;

    }


    Status openWrite(ServerContext* context, const WriteRequest *write, WriteResponse  *response) override {
        std::cout << "******************************************" << std::endl;
        std::cout << "\t Open Write RPC called" << std::endl;
        std::cout << "device id value: " << write->device_id() << std::endl;
        std::cout << "role id value: " << write->role_id() << std::endl;
        int rv = 0;
        char *chr = "pktdev init";
        /* call the bcmlt CRUD based */

        bcmlt_entry_handle_t entry_hdl;

        // init the bshell by calling pktdev init
        bcma_cli_bshell(write->device_id(), chr);

        rv = bcmlt_entry_allocate(write->device_id(), "VLAN", &entry_hdl);
        if (SHR_FAILURE(rv)) {
            response->set_message(shr_errmsg(rv));
            response->set_success(false);
            return Status::OK;
        }
        std::cout << "entry hdl: " << entry_hdl << std::endl;

        rv = bcmlt_entry_field_add(entry_hdl, "VLAN_ID", 11);
        if (SHR_FAILURE(rv)) {
            response->set_message(shr_errmsg(rv));
            response->set_success(false);
            return Status::OK;
        }
        rv = bcmlt_entry_field_add(entry_hdl, "VLAN_STG_ID", 1);
        if (SHR_FAILURE(rv)) {
            response->set_message(shr_errmsg(rv));
            response->set_success(false);
            return Status::OK;
        }

        /* Commit the entry */
        rv = bcmlt_custom_entry_commit(entry_hdl,
                                       BCMLT_OPCODE_INSERT,
                                       BCMLT_PRIORITY_NORMAL);
        if (SHR_FAILURE(rv)) {
            response->set_message(shr_errmsg(rv));
            response->set_success(false);
            return Status::OK;
        }

        /* Free the entry handle */
        rv = bcmlt_entry_free(entry_hdl);
        if (SHR_FAILURE(rv)) {
            response->set_message(shr_errmsg(rv));
            response->set_success(false);
            return Status::OK;
        }

        response->set_message("success");
        response->set_success(true);
        return Status::OK;

    }

    static int cli_cmd_process_ctrlc(void *data)
    {
        bcma_cli_t *cli = (bcma_cli_t *)data;
        return bcma_cli_cmd_process(cli, cli->ibuf);

    }

    int cli_init(bcma_sys_conf_t *sc)
    {
        /* Initialize basic CLI */
        if (bcma_sys_conf_cli_init(sc) < 0) {
            return SHR_E_FAIL;
        }

        /* Enable CLI redirection in BSL output hook */
        bcma_bslmgmt_redir_hook_set(bcma_sys_conf_cli_redir_bsl);

        /* Add CLI commands for controlling the system log */
        bcma_bslcmd_add_cmds(sc->cli);
        bcma_bslcmd_add_cmds(sc->dsh);

        /* Add bcmlt commands */
        bcma_bcmltcmd_add_cmds(sc->cli);

        /* Add CLI command completion support */
        bcma_sys_conf_clirlc_init();

        /* Add CLI commands for base driver to debug shell */
        bcma_bcmbdcmd_add_cmicd_cmds(sc->dsh);
        bcma_bcmbdcmd_add_dev_cmds(sc->dsh);

        /* Add CLI commands for packet I/O driver */
        bcma_bcmpktcmd_add_cmds(sc->cli);

        /* Add BCMLT C interpreter (CINT) */
        bcma_cintcmd_add_cint_cmd(isc->cli);

        return SHR_E_NONE;
    }

    int bcmlt_custom_entry_commit(bcmlt_entry_handle_t entry_hdl,
                                  bcmlt_opcode_t op,
                                  bcmlt_priority_level_t prio)
    {
        int rv;
        bcmlt_entry_info_t entry_info;

        /* commit the entry sychronously */
        rv = bcmlt_entry_commit(entry_hdl, op, prio);
        if (SHR_FAILURE(rv)) {
            std::cout << "bcm entry commit failed error code: " << rv << " failure reason: "
            << shr_errmsg(rv) << std::endl;
            return rv;
        }

        rv = bcmlt_entry_info_get(entry_hdl, &entry_info);
        if (SHR_FAILURE(rv)) {
            std::cout << "bcmlt entry info get failed error code: " << rv <<  " failure reason: "
            << shr_errmsg(rv) << std::endl;
            return rv;
        }

        if (entry_info.status != SHR_E_NONE) {
            std::cout << "unit: " << entry_info.unit << " commit to LT: " << entry_info.table_name <<
            " failed status: " << shr_errmsg(entry_info.status) << "\n";
            return entry_info.status;
        }
        std::cout << "unit: " << entry_info.unit << " LT: " << entry_info.table_name << " commit success \n";
        return entry_info.status;
    }

    int bcmlt_custom_pt_entry_commit(bcmlt_entry_handle_t entry_hdl,
                                     bcmlt_pt_opcode_t op,
                                     bcmlt_priority_level_t prio)
    {
        int rv;
        bcmlt_entry_info_t entry_info;

        /* commit the entry sychronously */
        rv = bcmlt_pt_entry_commit(entry_hdl, op, prio);
        if (SHR_FAILURE(rv)) {
            std::cout << "bcm entry commit failed error code: " << rv << "failure reason: "
            << shr_errmsg(rv) << std::endl;
            return rv;
        }
        /* fetch info  */
        rv = bcmlt_entry_info_get(entry_hdl, &entry_info);
        if (SHR_FAILURE(rv)) {
            std::cout << "bcmlt entry info get failed error code: " << rv <<  " failure reason: "
            << shr_errmsg(rv) << std::endl;
            return rv;
        }

        if (entry_info.status != SHR_E_NONE) {
            std::cout << "unit: " << entry_info.unit << " commit to LT: " << entry_info.table_name <<
            " failed status: " << shr_errmsg(entry_info.status) << "\n";
            return entry_info.status;
        }
        std::cout << "unit: " << entry_info.unit << " PT: " << entry_info.table_name << " commit success \n";
    }

    Status RemoteShell(ServerContext* context, const ShellRequest *shell, ShellState *state) override {
        char *cstr = new char[shell->cmd().length()+1];
        cmdResp.clear();

        std::cout << "******************************************" << std::endl;
        std::cout << "\t Remote Shell" << std::endl;
        std::cout << "command (unit:"<<isc->cli->cur_unit<<") "<<shell->cmd()<<std::endl;

        strcpy(cstr, shell->cmd().c_str());

        memset(isc->cli->ibuf, 0, sizeof(isc->cli->ibuf));
        strcpy(isc->cli->ibuf, cstr);

        bcma_cli_ctrlc_exec(isc->cli, cli_cmd_process_ctrlc, (void *)isc->cli);
        state->set_message(cmdResp);
        delete [] cstr;
        return Status::OK;
    }

    int bcma_bslgrpc_init();

public:
	void appendReply(std::string reply);

    /* Per Connection Variables */
private:
	std::string		cmdResp;
};

void
SdkLTServiceImpl::appendReply(std::string reply)
{
    cmdResp.append(reply);
}

int
SdkLTServiceImpl::bcma_bslgrpc_init()
{
    bsl_config_t bsl_config;
    bcma_bslenable_init();

    bsl_config_t_init(&bsl_config);

    bsl_config.out_hook = bcma_grpc_out_hook;
    bsl_config.check_hook = NULL;
    bsl_init(&bsl_config);

    /* Initialize output hook */
    bcma_bslsink_init();

    /* Create console sink */
    bcma_bslcons_init();

    /* Create file sink */
    //bcma_bslfile_init();

    return 0;
}

SdkLTServiceImpl sdkltService;

int
bcma_grpc_out_hook(bsl_meta_t *meta, const char *format, va_list args)
{
    va_list args2;
    va_copy(args2, args);

    char buf[1+ vsnprintf(NULL, 0, format, args)];
    va_end(args);

    vsnprintf(buf, sizeof buf, format, args2);
    va_end(args2);

    sdkltService.appendReply(std::string(buf));
	return 0;
}

void
SdkLTgRPCServer(void) {
    std::string serverAddress("0.0.0.0:50051");

    grpc::EnableDefaultHealthCheckService(true);
    grpc::reflection::InitProtoReflectionServerBuilderPlugin();
    ServerBuilder builder;

    builder.AddListeningPort(serverAddress, grpc::InsecureServerCredentials());

    builder.RegisterService(&sdkltService);
    std::unique_ptr<Server> server(builder.BuildAndStart());
    std::cout<<"Listening on 50051...."<<std::endl;
    server->Wait();
}

int
main(int argc, char **argv)
{
	/* Trigger SDKLT gRPC Server */
    SdkLTgRPCServer();
    return 0;
}
