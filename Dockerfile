FROM nginx:alpine
WORKDIR /usr/share/nginx/html
COPY Weather.html ./index.html
