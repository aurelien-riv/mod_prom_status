# Apache HTTPd modules for Prometheus monitoring

Using apache_exporter to translate mod_status into a Prometheus understandable file was nice, but had the disadvantage to
add a component per server in the monitoring stack. Also, it is limited by what mod_status can export.

## mod_prom_status

This module reports some Apache HTTPd metrics, like mod_status, formatted like any Prometheus exporter.

It must be loaded before any mod_prom_*_status.

## mod_prom_cache_status

This module listens on the cache_status hook of mod_cache in order to monitor the cache efficiency. 

You must load mod_cache, mod_slotmem_shm and mod_prom_status **before** mod_prom_cache_status.


## Demonstration

Clone the repository and run the docker-compose.yml, then browse the dashboards:
- http://localhost:3000/d/rYdddlPWk/mod_prom_status?orgId=1&refresh=15s
- http://localhost:3000/d/rAzRu9O4z/mod_prom_cache_status?orgId=1&refresh=15s

![grafana_mod_prom_status](https://github.com/aurelien-riv/mod_prom_status/blob/main/docker-demo/.screenshots/grafana_mod_prom_status.png?raw=true)
![grafana_mod_prom_cache_status](https://github.com/aurelien-riv/mod_prom_status/blob/main/docker-demo/.screenshots/grafana_mod_prom_cache_status.png?raw=true)

## Build

### Linux
```bash
apxs -c mod_prom_status/*.c
apxs -c -I mod_prom_status/ mod_prom_cache_status/*.c
```

### Windows
```cmd
"C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Tools\MSVC\14.34.31933\bin\Hostx64\x64\cl.exe" -I D:\Apache2448\include /c /MD /W3 /O2 /D WIN32 /D _WINDOWS /D NDEBUG *.c
"C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Tools\MSVC\14.34.31933\bin\Hostx64\x64\link.exe" /subsystem:windows /dll /libpath:"D:\Apache2448\lib" libapr-1.lib libhttpd.lib libaprutil-1.lib  *.obj
```