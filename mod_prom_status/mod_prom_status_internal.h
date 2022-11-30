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

#pragma once

#include "httpd.h"
#include "http_core.h"


typedef struct {
    int show_modules;
} prom_status_config;

typedef struct {
    int server_limit;
    int thread_limit;
    int threads_per_child;
    int max_servers;
} prom_status_http_mpm_config;

typedef struct {
    apr_uint32_t uptime;
    int *worker_status_count;
    int req_count;
    int byte_count;
} prom_status_httpd_metrics;