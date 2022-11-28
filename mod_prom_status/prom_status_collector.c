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

#include "http_protocol.h"
#include "http_log.h"
#include "scoreboard.h"
#include "prom_status_collector.h"

prom_status_httpd_metrics *load_http_metrics(request_rec *r, prom_status_http_mpm_config *mpm_config)
{
    if (!ap_exists_scoreboard_image()) {
        ap_log_rerror(APLOG_MARK, APLOG_ERR, 0, r, APLOGNO(01237) "Server status unavailable in inetd mode");
        return NULL;
    }

    prom_status_httpd_metrics *metrics = (prom_status_httpd_metrics *)apr_pcalloc(r->pool, sizeof(prom_status_httpd_metrics));
    metrics->worker_status_count = (int *) apr_pcalloc(r->pool, sizeof(int) * mpm_config->server_limit * MOD_STATUS_STATUS_COUNT);

    metrics->uptime = (apr_uint32_t) apr_time_sec(apr_time_now() - ap_scoreboard_image->global->restart_time);

    for (int i = 0; i < mpm_config->server_limit; ++i) {
        for (int k = 0; k < mpm_config->thread_limit; ++k) {
            worker_score *ws_record = apr_palloc(r->pool, sizeof *ws_record);

            ap_copy_scoreboard_worker(ws_record, i, k);

            int res = ws_record->status;

            if ((i >= mpm_config->max_servers || k >= mpm_config->threads_per_child) && (res == SERVER_DEAD)) {
                ++metrics->worker_status_count[i * MOD_STATUS_STATUS_COUNT + SERVER_DISABLED];
            } else {
                ++metrics->worker_status_count[i * MOD_STATUS_STATUS_COUNT + res];
            }

            if (ws_record->access_count != 0 || (res != SERVER_READY && res != SERVER_DEAD)) {
                metrics->req_count += ws_record->access_count;
                metrics->byte_count += ws_record->bytes_served;
            }
        }
    }

    return metrics;
}

