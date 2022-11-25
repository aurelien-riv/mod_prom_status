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
#include "ap_mpm.h"
#include "prom_status_collector.h"

static void register_hooks(apr_pool_t *pool);

module AP_MODULE_DECLARE_DATA prom_status_module =
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

static prom_status_http_mpm_config mpm_config = {
    .server_limit = 0,
    .thread_limit = 0,
    .threads_per_child = 0,
    .max_servers = 0
};

static int prom_status_init(apr_pool_t *p, apr_pool_t *plog, apr_pool_t *ptemp, server_rec *s)
{
    ap_mpm_query(AP_MPMQ_HARD_LIMIT_THREADS, &mpm_config.thread_limit);
    ap_mpm_query(AP_MPMQ_HARD_LIMIT_DAEMONS, &mpm_config.server_limit);
    ap_mpm_query(AP_MPMQ_MAX_THREADS, &mpm_config.threads_per_child);
    /* work around buggy MPMs */
    if (mpm_config.threads_per_child == 0)
        mpm_config.threads_per_child = 1;
    ap_mpm_query(AP_MPMQ_MAX_DAEMONS, &mpm_config.max_servers);

    return OK;
}

static int prom_status_pre_config(apr_pool_t *p, apr_pool_t *plog, apr_pool_t *ptemp)
{
    /* When mod_prom_status is loaded, default our ExtendedStatus to 'on'
     * other modules which prefer verbose scoreboards may play a similar game.
     * If left to their own requirements, mpm modules can make do with simple
     * scoreboard entries.
     */
    ap_extended_status = 1;
    return OK;
}

static prom_status_config *parse_arguments(request_rec *r)
{
    prom_status_config *config = (prom_status_config *)apr_pcalloc(r->pool, sizeof(prom_status_config));

    if (r->args) {
        const char *loc = ap_strstr_c(r->args, "modules");
        if (loc != NULL) {
            config->show_modules = 1;
        }
    }

    return config;
}

static int prom_status_handler(request_rec *r)
{
    if (!r->handler || strcmp(r->handler, "prom-status"))
    {
      return (DECLINED);
    }

    prom_status_config *config = parse_arguments(r);
    print_components(r, config);
    print_scoreboard_data(r, &mpm_config);

    return OK;
}

static void register_hooks(apr_pool_t *pool)
{
    ap_hook_handler(prom_status_handler, NULL, NULL, APR_HOOK_MIDDLE);
    ap_hook_pre_config(prom_status_pre_config, NULL, NULL, APR_HOOK_LAST);
    ap_hook_post_config(prom_status_init, NULL, NULL, APR_HOOK_MIDDLE);
}
