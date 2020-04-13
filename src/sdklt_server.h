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
 * File: sdklt_server.h
 *
 * Description: 
 *      This file contains the SDKLT gRPC Server function header files
 *
 * Authors: Kaushik, Koneru
 *          Anand, Akella
 *          Venkat, Pullela
 */

#ifndef __SDKLT_SERVER_H__
#define __SDKLT_SERVER_H__

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
#include <bcma/bsl/bcma_bslenable.h>
#include <bcma/bsl/bcma_bslsink.h>
#include <bcma/bsl/bcma_bslcons.h>

#ifdef __cplusplus
 }
#endif
#endif /* __SDKLT_SERVER_H__*/
