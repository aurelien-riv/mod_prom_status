# Apache HTTPd modules for Prometheus monitoring

Using apache_exporter to translate mod_status into a Prometheus understandable file was nice, but had the disadvantage to
add a component per server in the monitoring stack. Also, it is limited by what mod_status can export.

## mod_prom_status

This module reports some Apache HTTPd metrics, like mod_status, formatted like any Prometheus exporter.

## mod_prom_cache_status

This module listens on the cache_status hook of mod_cache in order to monitor the cache efficiency. 

**Not ready yet.** (the stats are stored per thread, not globally)

## Demonstration

Clone the repository and run the docker-compose.yml, then browse the dashboards:
- http://localhost:3000/d/rYdddlPWk/mod_prom_status?orgId=1&refresh=15s
- http://localhost:3000/d/rAzRu9O4z/mod_prom_cache_status?orgId=1&refresh=15s

![grafana_mod_prom_status](https://github.com/aurelien-riv/mod_prom_status/blob/main/docker-demo/.screenshots/grafana_mod_prom_status.png?raw=true)
![grafana_mod_prom_cache_status](https://github.com/aurelien-riv/mod_prom_status/blob/main/docker-demo/.screenshots/grafana_mod_prom_cache_status.png?raw=true)
