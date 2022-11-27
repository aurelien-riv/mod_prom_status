/**
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

/**
 * Aurélien Rivière <aurelien.riv@gmail.com> 2022
 */

#include "ap_config.h"
#include "httpd.h"
#include "http_core.h"
#include "http_protocol.h"
#include "http_request.h"
#include "http_log.h"
#include "mod_cache.h"
#include "mod_prom_status.h"

static volatile int metrics[AP_CACHE_INVALIDATE+1] = {0};

static void register_hooks(apr_pool_t *pool);

module AP_MODULE_DECLARE_DATA prom_cache_status_module =
{
    STANDARD20_MODULE_STUFF,
    NULL,            // Per-directory configuration handler
    NULL,            // Merge handler for per-directory configurations
    NULL,            // Per-server configuration handler
    NULL,            // Merge handler for per-server configurations
    NULL,            // Any directives we may have for httpd
    register_hooks,  // Our hook registering function
    0                // flags
};

static void prom_cache_status_handler(request_rec *r)
{
    // FIXME when httpd spawns a new process the counter is partially reset:
    /*
    192.168.240.3 - - [26/Nov/2022:20:23:55 +0000] "GET /cache-metrics HTTP/1.1" 200 286
    [Sat Nov 26 20:24:00.246652 2022] [:error] [pid 9:tid 140362744030976] [client 192.168.240.3:36116] AH01237: miss: 523
    192.168.240.3 - - [26/Nov/2022:20:24:00 +0000] "GET /cache-metrics HTTP/1.1" 200 286
    [Sat Nov 26 20:24:05.253403 2022] [:error] [pid 10:tid 140363037644544] [client 192.168.240.3:33660] AH01237: miss: 256
    */
    ap_log_rerror(APLOG_MARK, APLOG_ERR, 0, r, APLOGNO(01237) "miss: %d", metrics[AP_CACHE_MISS]);

    ap_rputs("# HELP httpd_cache_statistics mod_cache statistics\n", r);
    ap_rputs("# TYPE httpd_cache_statistics counter\n", r);
    ap_rprintf(r, "httpd_cache_statistics{status=\"%s\"} %d\n", AP_CACHE_HIT_ENV,        metrics[AP_CACHE_HIT]);
    ap_rprintf(r, "httpd_cache_statistics{status=\"%s\"} %d\n", AP_CACHE_REVALIDATE_ENV, metrics[AP_CACHE_REVALIDATE]);
    ap_rprintf(r, "httpd_cache_statistics{status=\"%s\"} %d\n", AP_CACHE_MISS_ENV,       metrics[AP_CACHE_MISS]);
    ap_rprintf(r, "httpd_cache_statistics{status=\"%s\"} %d\n", AP_CACHE_INVALIDATE_ENV, metrics[AP_CACHE_INVALIDATE]);
}

static int prom_cache_status_listener(cache_handle_t *h, request_rec *r,
        apr_table_t *headers, ap_cache_status_e status, const char *reason)
{
    // if metrics wasn't volatile, each thread would have the same pointer but a different value on the pointer data.
    // However, it doesn't mean this code is thread safe. It isn't, if two increments on the same status are done
    // simultaneously, one increment is lost. I don't  the shape of the graphs and the percentage will be impacted by
    // these data race, so for now I won't put a mutex that would slow down HTTPd.
    ap_log_rerror(APLOG_MARK, APLOG_ERR, 0, r, APLOGNO(01237) "inc: %d", metrics[AP_CACHE_MISS]);

    ++metrics[status];
    return OK;
}

static void register_hooks(apr_pool_t *pool)
{
    cache_hook_cache_status(prom_cache_status_listener, NULL, NULL, APR_HOOK_MIDDLE);
    ap_hook_prom_status_hook(prom_cache_status_handler, NULL, NULL, APR_HOOK_MIDDLE);
}
