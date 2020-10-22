FROM node:12.3.1-alpine
ARG DIRECTORY

ADD package.json package-lock.json  /tmp/
RUN cd /tmp && npm install
RUN mkdir -p /var/www/src && cd /var/www && ln -s /tmp/node_modules && ln -s /tmp/package.json && ln -s /tmp/package-lock.json
WORKDIR /var/www
COPY src ./src