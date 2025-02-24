# create certificates for testing on localhost
openssl req -x509 -newkey rsa:4096 -sha256 -days 3650 -nodes \
    -keyout starlight.key -out starlight.crt -subj "/CN=localhost"