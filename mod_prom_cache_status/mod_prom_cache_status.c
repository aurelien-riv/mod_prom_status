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

#include "httpd.h"
#include "http_core.h"
#include "http_protocol.h"
#include "http_request.h"
#include "ap_mpm.h"
#include "mod_cache.h"

// FIXME the module is instantiated in multiple threads, we need to make this memory area unique
// for instance using slotmem (see mod_proxy_balancer and mod_heartmonitor)
static int metrics[AP_CACHE_INVALIDATE+1] = {0};

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

static int prom_cache_status_handler(request_rec *r)
{
    if (!r->handler || strcmp(r->handler, "prom-cache-status"))
    {
      return (DECLINED);
    }

    ap_rputs("# HELP httpd_cache_statistics mod_cache statistics\n", r);
    ap_rputs("# TYPE httpd_cache_statistics counter\n", r);
    ap_rprintf(r, "httpd_cache_statistics{status=\"%s\"} %d\n", AP_CACHE_HIT_ENV,        metrics[AP_CACHE_HIT]);
    ap_rprintf(r, "httpd_cache_statistics{status=\"%s\"} %d\n", AP_CACHE_REVALIDATE_ENV, metrics[AP_CACHE_REVALIDATE]);
    ap_rprintf(r, "httpd_cache_statistics{status=\"%s\"} %d\n", AP_CACHE_MISS_ENV,       metrics[AP_CACHE_MISS]);
    ap_rprintf(r, "httpd_cache_statistics{status=\"%s\"} %d\n", AP_CACHE_INVALIDATE_ENV, metrics[AP_CACHE_INVALIDATE]);

    return OK;
}

static int prom_cache_status_listener(cache_handle_t *h, request_rec *r,
        apr_table_t *headers, ap_cache_status_e status, const char *reason)
{
    ++metrics[status];
    return OK;
}


static void register_hooks(apr_pool_t *pool)
{
    ap_hook_handler(prom_cache_status_handler, NULL, NULL, APR_HOOK_MIDDLE);
    cache_hook_cache_status(prom_cache_status_listener, NULL, NULL, APR_HOOK_MIDDLE);
}
