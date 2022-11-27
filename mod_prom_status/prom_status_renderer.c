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
#include "scoreboard.h"
#include "prom_status_renderer.h"

void print_components(request_rec *r, prom_status_config *config, prom_status_http_mpm_config *mpm_config)
{
    ap_rputs("# HELP httpd_info Basic information on the HTTPd instance\n", r);
    ap_rputs("# TYPE httpd_info gauge\n", r);
    ap_rprintf(r,
        "httpd_info{version=\"%s\", name=\"%s\", mpm=\"%s\", maxservers=\"%d\", threads_per_child=\"%d\"} 1\n",
        ap_get_server_description(),
        ap_get_server_name(r),
        ap_show_mpm(),
        mpm_config->max_servers,
        mpm_config->threads_per_child
    );

    if (config->show_modules) {
        ap_rputs("# HELP httpd_module List of loaded modules\n", r);
        ap_rputs("# TYPE httpd_module gauge\n", r);
        for (int i = 0; ap_loaded_modules[i] != NULL; ++i) {
            ap_rprintf(r, "httpd_module{name=\"%s\"} 1\n", ap_find_module_short_name(i));
        }
    }
}

void print_traffic_metrics(request_rec *r, prom_status_httpd_metrics *metrics)
{
    ap_rputs("# HELP httpd_uptime_seconds Current uptime in seconds\n", r);
    ap_rputs("# TYPE httpd_uptime_seconds gauge\n", r);
    ap_rprintf(r, "httpd_uptime_seconds %d\n", metrics->uptime);

    ap_rputs("# HELP httpd_accesses_total Current total apache accesses\n", r);
    ap_rputs("# TYPE httpd_accesses_total counter\n", r);
    ap_rprintf(r, "httpd_accesses_total %d\n", metrics->req_count);

    ap_rputs("# HELP httpd_sent_bytes_total Current total kbytes sent\n", r);
    ap_rputs("# TYPE httpd_sent_bytes_total counter\n", r);
    ap_rprintf(r, "httpd_sent_bytes_total %d\n", metrics->byte_count);
}

void print_scoreboard_data(request_rec *r, prom_status_httpd_metrics *metrics, prom_status_http_mpm_config *mpm_config)
{
    ap_rputs("# HELP httpd_scoreboard HTTPd scoreboard statuses\n", r);
    ap_rputs("# TYPE httpd_scoreboard gauge\n", r);

    const char *tpl = "httpd_scoreboard{state=\"%s\", server=\"%d\"} %d\n";
    for (int i = 0; i < mpm_config->server_limit; ++i) {
        ap_rprintf(r, tpl, "open_slot",     i, metrics->worker_status_count[i * MOD_STATUS_STATUS_COUNT + SERVER_DEAD]);
        ap_rprintf(r, tpl, "idle",          i, metrics->worker_status_count[i * MOD_STATUS_STATUS_COUNT + SERVER_READY]);
        ap_rprintf(r, tpl, "startup",       i, metrics->worker_status_count[i * MOD_STATUS_STATUS_COUNT + SERVER_STARTING]);
        ap_rprintf(r, tpl, "read",          i, metrics->worker_status_count[i * MOD_STATUS_STATUS_COUNT + SERVER_BUSY_READ]);
        ap_rprintf(r, tpl, "reply",         i, metrics->worker_status_count[i * MOD_STATUS_STATUS_COUNT + SERVER_BUSY_WRITE]);
        ap_rprintf(r, tpl, "keepalive",     i, metrics->worker_status_count[i * MOD_STATUS_STATUS_COUNT + SERVER_BUSY_KEEPALIVE]);
        ap_rprintf(r, tpl, "dns",           i, metrics->worker_status_count[i * MOD_STATUS_STATUS_COUNT + SERVER_BUSY_DNS]);
        ap_rprintf(r, tpl, "closing",       i, metrics->worker_status_count[i * MOD_STATUS_STATUS_COUNT + SERVER_CLOSING]);
        ap_rprintf(r, tpl, "logging",       i, metrics->worker_status_count[i * MOD_STATUS_STATUS_COUNT + SERVER_BUSY_LOG]);
        ap_rprintf(r, tpl, "graceful_stop", i, metrics->worker_status_count[i * MOD_STATUS_STATUS_COUNT + SERVER_GRACEFUL]);
        ap_rprintf(r, tpl, "idle_cleanup",  i, metrics->worker_status_count[i * MOD_STATUS_STATUS_COUNT + SERVER_IDLE_KILL]);
//        ap_rprintf(r, tpl, "disabled",      i, metrics->worker_status_count[i * MOD_STATUS_STATUS_COUNT + SERVER_DISABLED]);
    }
}
