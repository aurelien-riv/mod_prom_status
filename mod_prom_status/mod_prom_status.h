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
#include "apr_hooks.h"
#include "ap_config.h"

#if defined(WIN32)
#define PROM_STATUS_DECLARE(type)            __declspec(dllexport) type __stdcall
#define PROM_STATUS_DECLARE_NONSTD(type)     __declspec(dllexport) type
#define PROM_STATUS_DECLARE_DATA             __declspec(dllexport)
#else
#define PROM_STATUS_DECLARE(type)            type
#define PROM_STATUS_DECLARE_NONSTD(type)     type
#define PROM_STATUS_DECLARE_DATA
#endif

APR_DECLARE_EXTERNAL_HOOK(prom_status, PROM_STATUS, void, prom_status_hook, (request_rec *r))
