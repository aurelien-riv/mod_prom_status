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
#include "ap_slotmem.h"
#include "httpd.h"
#include "http_core.h"
#include "http_protocol.h"
#include "http_request.h"
#include "http_main.h"
#include "http_log.h"
#include "mod_cache.h"
#include "mod_prom_status.h"

static const ap_slotmem_provider_t *storage = NULL;
static ap_slotmem_instance_t *slotmem = NULL;

static volatile int *metrics;

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
    ap_log_rerror(APLOG_MARK, APLOG_ERR, 0, r, "miss: %d", metrics[AP_CACHE_MISS]);

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
    // This isn't thread safe, if two increments on the same status are done simultaneously, one increment is lost.
    // It shouldn't change the shape of the graphs, and a mutex would slow HTTPd down so I think it's OK.
    ++metrics[status];
    return OK;
}

static void prom_cache_status_child_init(apr_pool_t *p, server_rec *s)
{
    storage->dptr(slotmem, 1, (void *)&metrics);
    ap_log_error(APLOG_MARK, APLOG_EMERG, 0, s, APLOGNO(01205) "prom_cache_status init: %d", metrics[AP_CACHE_HIT]);
}

static int prom_cache_status_post_config(apr_pool_t *p, apr_pool_t *plog, apr_pool_t *ptemp, server_rec *s)
{
    if (ap_state_query(AP_SQ_MAIN_STATE) == AP_SQ_MS_CREATE_CONFIG) {
        storage = ap_lookup_provider(AP_SLOTMEM_PROVIDER_GROUP, "shm", AP_SLOTMEM_PROVIDER_VERSION);
        if (!storage) {
            ap_log_error(
                APLOG_MARK, APLOG_EMERG, 0, s, APLOGNO(02284)
                "failed to lookup provider 'shm' for '%s', maybe you need to load mod_slotmem_shm?",
                AP_SLOTMEM_PROVIDER_GROUP
            );
            return !OK;
        }
        storage->create(&slotmem, "mod_prom_cache_status", sizeof(int), AP_CACHE_INVALIDATE+1, AP_SLOTMEM_TYPE_PREGRAB, p);
        if (!slotmem) {
            ap_log_error(APLOG_MARK, APLOG_EMERG, 0, s, APLOGNO(02285) "slotmem_create for status failed");
            return !OK;
        }

        if (storage->num_slots(slotmem) == 0) {
            // FIXME memory leak
            metrics = (volatile int *) calloc(sizeof(int), AP_CACHE_INVALIDATE+1);
            storage->put(slotmem, 0, (unsigned char *)metrics, (AP_CACHE_INVALIDATE+1) * sizeof(int));
        }
    }

    return OK;
}

static void register_hooks(apr_pool_t *pool)
{
    ap_hook_post_config(prom_cache_status_post_config, NULL, NULL, APR_HOOK_LAST);
    ap_hook_child_init(prom_cache_status_child_init, NULL, NULL, APR_HOOK_MIDDLE);
    cache_hook_cache_status(prom_cache_status_listener, NULL, NULL, APR_HOOK_MIDDLE);
    ap_hook_prom_status_hook(prom_cache_status_handler, NULL, NULL, APR_HOOK_MIDDLE);
}
