# create certificates for testing on localhost
openssl req -x509 -newkey rsa:4096 -sha256 -days 3650 -nodes \
    -keyout certs/starlight.key -out certs/starlight.crt -subj "/CN=localhost"