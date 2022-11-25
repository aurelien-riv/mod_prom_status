#pragma once

#include "httpd.h"
#include "http_core.h"

#define SERVER_DISABLED SERVER_NUM_STATUS
#define MOD_STATUS_NUM_STATUS (SERVER_NUM_STATUS+1)

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
    int worker_status_count[SERVER_NUM_STATUS+1];
    int req_count;
    int byte_count;
} prom_status_httpd_metrics;
