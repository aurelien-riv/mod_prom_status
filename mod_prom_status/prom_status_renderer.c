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

void print_components(request_rec *r, prom_status_config *config)
{
    ap_rputs("# HELP httpd_info Basic information on the HTTPd instance\n", r);
    ap_rputs("# TYPE httpd_info gauge\n", r);
    ap_rprintf(r,
        "httpd_info{version=\"%s\", name=\"%s\", mpm=\"%s\"} 1\n",
        ap_get_server_description(),
        ap_get_server_name(r),
        ap_show_mpm()
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

void print_scoreboard_data(request_rec *r, prom_status_httpd_metrics *metrics)
{
    ap_rputs("# HELP httpd_scoreboard HTTPd scoreboard statuses\n", r);
    ap_rputs("# TYPE httpd_scoreboard gauge\n", r);
    ap_rprintf(r, "httpd_scoreboard{state=\"open_slot\"} %d\n", metrics->worker_status_count[SERVER_DEAD]);
    ap_rprintf(r, "httpd_scoreboard{state=\"idle\"} %d\n", metrics->worker_status_count[SERVER_READY]);
    ap_rprintf(r, "httpd_scoreboard{state=\"startup\"} %d\n", metrics->worker_status_count[SERVER_STARTING]);
    ap_rprintf(r, "httpd_scoreboard{state=\"read\"} %d\n", metrics->worker_status_count[SERVER_BUSY_READ]);
    ap_rprintf(r, "httpd_scoreboard{state=\"reply\"} %d\n", metrics->worker_status_count[SERVER_BUSY_WRITE]);
    ap_rprintf(r, "httpd_scoreboard{state=\"keepalive\"} %d\n", metrics->worker_status_count[SERVER_BUSY_KEEPALIVE]);
    ap_rprintf(r, "httpd_scoreboard{state=\"dns\"} %d\n", metrics->worker_status_count[SERVER_BUSY_DNS]);
    ap_rprintf(r, "httpd_scoreboard{state=\"closing\"} %d\n", metrics->worker_status_count[SERVER_CLOSING]);
    ap_rprintf(r, "httpd_scoreboard{state=\"logging\"} %d\n", metrics->worker_status_count[SERVER_BUSY_LOG]);
    ap_rprintf(r, "httpd_scoreboard{state=\"graceful_stop\"} %d\n", metrics->worker_status_count[SERVER_GRACEFUL]);
    ap_rprintf(r, "httpd_scoreboard{state=\"idle_cleanup\"} %d\n", metrics->worker_status_count[SERVER_IDLE_KILL]);
    //ap_rprintf(r, "httpd_scoreboard{state=\"disabled\"} %d\n", metrics->worker_status_count[SERVER_DISABLED]);
}
