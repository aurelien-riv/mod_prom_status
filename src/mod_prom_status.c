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
 * Most of this code comes from https://github.com/apache/httpd/blob/ccf9197926e25d11d8786df3dd39cb1b931f11b5/modules/generators/mod_status.c
 */

/**
 * Prometheus Status Module - (Will) Display lots of internal data about how Apache is
 * performing and the state of all children processes in a Prometheus-compatible language.
 */

#include "httpd.h"
#include "http_core.h"
#include "http_protocol.h"
#include "http_request.h"
#include "scoreboard.h"
#include "ap_mpm.h"

#define MOD_STATUS_NUM_STATUS (SERVER_NUM_STATUS+1)
#define SERVER_DISABLED SERVER_NUM_STATUS

static int server_limit, thread_limit, threads_per_child, max_servers, is_async;

static void register_hooks(apr_pool_t *pool);

module AP_MODULE_DECLARE_DATA prom_status_module =
{
    STANDARD20_MODULE_STUFF,
    NULL,            // Per-directory configuration handler
    NULL,            // Merge handler for per-directory configurations
    NULL,            // Per-server configuration handler
    NULL,            // Merge handler for per-server configurations
    NULL,            // Any directives we may have for httpd
    register_hooks   // Our hook registering function
};

static int prom_status_init(apr_pool_t *p, apr_pool_t *plog, apr_pool_t *ptemp, server_rec *s)
{
    ap_mpm_query(AP_MPMQ_HARD_LIMIT_THREADS, &thread_limit);
    ap_mpm_query(AP_MPMQ_HARD_LIMIT_DAEMONS, &server_limit);
    ap_mpm_query(AP_MPMQ_MAX_THREADS, &threads_per_child);
    /* work around buggy MPMs */
    if (threads_per_child == 0)
        threads_per_child = 1;
    ap_mpm_query(AP_MPMQ_MAX_DAEMONS, &max_servers);
    ap_mpm_query(AP_MPMQ_IS_ASYNC, &is_async);

    return OK;
}

static int prom_status_pre_config(apr_pool_t *p, apr_pool_t *plog, apr_pool_t *ptemp)
{
    /* When mod_status is loaded, default our ExtendedStatus to 'on'
     * other modules which prefer verbose scoreboards may play a similar game.
     * If left to their own requirements, mpm modules can make do with simple
     * scoreboard entries.
     */
    ap_extended_status = 1;
    return OK;
}

static void print_scoreboard_data(request_rec *r)
{
    int *worker_status_count = apr_pcalloc(r->pool, (SERVER_NUM_STATUS+1) * sizeof(int));

    for (int i = 0; i < server_limit; ++i) {
        for (int j = 0; j < thread_limit; ++j) {
            worker_score *ws_record = apr_palloc(r->pool, sizeof *ws_record);
            ap_copy_scoreboard_worker(ws_record, i, j);
            int res = ws_record->status;

            if ((i >= max_servers || j >= threads_per_child) && (res == SERVER_DEAD))
                ++worker_status_count[SERVER_DISABLED];
            else
                ++worker_status_count[res];
        }
    }

    ap_rprintf(r, "httpd_scoreboard{state='open_slot'} %d\n", worker_status_count[SERVER_DEAD]);
    ap_rprintf(r, "httpd_scoreboard{state='idle'} %d\n", worker_status_count[SERVER_READY]);
    ap_rprintf(r, "httpd_scoreboard{state='startup'} %d\n", worker_status_count[SERVER_STARTING]);
    ap_rprintf(r, "httpd_scoreboard{state='read'} %d\n", worker_status_count[SERVER_BUSY_READ]);
    ap_rprintf(r, "httpd_scoreboard{state='reply'} %d\n", worker_status_count[SERVER_BUSY_WRITE]);
    ap_rprintf(r, "httpd_scoreboard{state='keepalive'} %d\n", worker_status_count[SERVER_BUSY_KEEPALIVE]);
    ap_rprintf(r, "httpd_scoreboard{state='dns'} %d\n", worker_status_count[SERVER_BUSY_DNS]);
    ap_rprintf(r, "httpd_scoreboard{state='closing'} %d\n", worker_status_count[SERVER_CLOSING]);
    ap_rprintf(r, "httpd_scoreboard{state='logging'} %d\n", worker_status_count[SERVER_BUSY_LOG]);
    ap_rprintf(r, "httpd_scoreboard{state='graceful_stop'} %d\n", worker_status_count[SERVER_GRACEFUL]);
    ap_rprintf(r, "httpd_scoreboard{state='idle_cleanup'} %d\n", worker_status_count[SERVER_IDLE_KILL]);
    //ap_rprintf(r, "httpd_scoreboard{state='disabled'} %d\n", worker_status_count[SERVER_DISABLED]);
}

static int prom_status_handler(request_rec *r)
{
    if (!r->handler || strcmp(r->handler, "prom-status"))
    {
      return (DECLINED);
    }

    print_scoreboard_data(r);

    return OK;
}

static void register_hooks(apr_pool_t *pool)
{
    ap_hook_handler(prom_status_handler, NULL, NULL, APR_HOOK_MIDDLE);
    ap_hook_pre_config(prom_status_pre_config, NULL, NULL, APR_HOOK_LAST);
    ap_hook_post_config(prom_status_init, NULL, NULL, APR_HOOK_MIDDLE);
}
