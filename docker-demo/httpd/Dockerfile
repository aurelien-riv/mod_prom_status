FROM httpd:2.4 AS builder
RUN apt update && apt -y install apache2-dev
RUN mkdir /root/mod_prom_status/
COPY ./src/ /root/mod_prom_status/
RUN cd /root/mod_prom_status/ && apxs -i -a -c *.c


FROM httpd:2.4
COPY --from=builder /usr/local/apache2/modules/mod_prom_status.so /usr/local/apache2/modules/mod_prom_status.so
COPY ./docker-demo/httpd/prom-status.conf /usr/local/apache2/conf/extra/prom-status.conf
RUN echo "LoadModule prom_status_module modules/mod_prom_status.so" >> /usr/local/apache2/conf/httpd.conf
RUN echo "Include conf/extra/prom-status.conf" >> /usr/local/apache2/conf/httpd.conf