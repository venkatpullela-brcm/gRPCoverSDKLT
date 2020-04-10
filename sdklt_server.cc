#include <iostream>
#include <memory>
#include <string>

/* GRPC HEADER */
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>
#include <grpcpp/ext/proto_server_reflection_plugin.h>

#include "sdklt.grpc.pb.h" 

/* BCM Headers */
#ifdef __cplusplus 
 extern "C" {
#endif
#include <unistd.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>

#include <shr/shr_debug.h>
#include <shr/shr_error.h>

#include <bcmdrd/bcmdrd_dev.h>
#include <bcmdrd/bcmdrd_feature.h>
#include <bcmbd/bcmbd.h>
#include <bcmlrd/bcmlrd_init.h>
#include <bcmmgmt/bcmmgmt.h>
#include <bcmmgmt/bcmmgmt_sysm_default.h>
#include <bcmlt/bcmlt.h>

#include <bcma/bsl/bcma_bslmgmt.h>
#include <bcma/bsl/bcma_bslcmd.h>
#include <bcmltd/bcmltd_lt_types.h>
#include <bcma/bcmlt/bcma_bcmltcmd.h>
#include <bcma/bcmpc/bcma_bcmpccmd.h>
#include <bcma/bcmbd/bcma_bcmbdcmd_cmicd.h>
#include <bcma/bcmbd/bcma_bcmbdcmd_dev.h>
#include <bcma/bcmpkt/bcma_bcmpktcmd.h>
#include <bcma/cint/bcma_cint_cmd.h>
#include <bcma/ha/bcma_ha.h>
#include <bcma/sys/bcma_sys_conf_sdk.h>
#include <bcma/cli/bcma_cli_bshell.h>

#ifdef __cplusplus
 }
#endif

static bcma_sys_conf_t sys_conf, *isc;

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

        rv = bcma_bslmgmt_init();
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
        std::uint64_t data = 0;

        rv = bcmlt_entry_allocate(read->device_id(), cstr, &entry_hdl);
        if (SHR_FAILURE(rv)) {
            response->set_message(shr_errmsg(rv));
            response->set_success(false);
            return Status::OK;
        }

        /* Commit the entry synchronously */
        rv = bcmlt_custom_entry_commit(entry_hdl, BCMLT_OPCODE_LOOKUP,
                                       BCMLT_PRIORITY_NORMAL);
        if (SHR_FAILURE(rv)) {
            response->set_message(shr_errmsg(rv));
            response->set_success(false);
            return Status::OK;
        }

        rv = bcmlt_entry_field_get(entry_hdl, key, &data);
        std::cout << "data: " << data << std::endl;
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

        rv = bcmlt_entry_field_add(entry_hdl, "VLAN_ID", 11);
        if (SHR_FAILURE(rv)) {
            response->set_message(shr_errmsg(rv));
            response->set_success(false);
            return Status::OK;
        } 

        /* Commit the entry */
        rv = bcmlt_custom_entry_commit(entry_hdl, BCMLT_OPCODE_INSERT, BCMLT_PRIORITY_NORMAL);
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

};


void
SdkLTServer() {
    std::string server_address("0.0.0.0:50051");
    SdkLTServiceImpl sdklt_service;

    grpc::EnableDefaultHealthCheckService(true);
    grpc::reflection::InitProtoReflectionServerBuilderPlugin();
    ServerBuilder builder;

    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());

    builder.RegisterService(&sdklt_service);
    std::unique_ptr<Server> server(builder.BuildAndStart());
    std::cout<<"Listening on 50051...."<<std::endl;
    server->Wait();
}

int 
main()
{
    SdkLTServer();
    return 0;
}
