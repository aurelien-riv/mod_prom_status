FROM httpd:2.4 AS builder
RUN apt update && apt -y install apache2-dev
COPY ./mod_prom_status/ /root/mod_prom_status/
COPY ./mod_prom_cache_status/ /root/mod_prom_cache_status/
RUN cd /root/mod_prom_status/ && apxs -i -c *.c
RUN cd /root/mod_prom_cache_status/ && apxs -i -I /root/mod_prom_status/ -c *.c


FROM httpd:2.4
RUN apt update && apt install curl
COPY --from=builder /usr/local/apache2/modules/mod_prom_status.so /usr/local/apache2/modules/mod_prom_status.so
COPY --from=builder /usr/local/apache2/modules/mod_prom_cache_status.so /usr/local/apache2/modules/mod_prom_cache_status.so
COPY ./docker-demo/httpd/prom-status.conf /usr/local/apache2/conf/extra/prom-status.conf

RUN sed -i 's%LoadModule mpm_event_module%#LoadModule mpm_event_module%'     /usr/local/apache2/conf/httpd.conf
#RUN sed -i 's%#LoadModule mpm_prefork_module%LoadModule mpm_prefork_module%' /usr/local/apache2/conf/httpd.conf
RUN sed -i 's%#LoadModule mpm_worker_module%LoadModule mpm_worker_module%'   /usr/local/apache2/conf/httpd.conf

RUN echo "LoadModule cache_module modules/mod_cache.so" >> /usr/local/apache2/conf/httpd.conf
RUN echo "LoadModule cache_disk_module modules/mod_cache_disk.so" >> /usr/local/apache2/conf/httpd.conf
RUN echo "LoadModule slotmem_shm_module modules/mod_slotmem_shm.so" >> /usr/local/apache2/conf/httpd.conf
RUN echo "LoadModule prom_status_module modules/mod_prom_status.so" >> /usr/local/apache2/conf/httpd.conf
RUN echo "LoadModule prom_cache_status_module modules/mod_prom_cache_status.so" >> /usr/local/apache2/conf/httpd.conf
RUN echo "Include conf/extra/prom-status.conf" >> /usr/local/apache2/conf/httpd.conf
