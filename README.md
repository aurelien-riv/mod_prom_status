# mod_prom_status

This module reports some Apache HTTPd metrics, like mod_status, formatted like any Prometheus exporter.

It avoid having to use apache_exporter as a translator between mod_status and prometheus.  