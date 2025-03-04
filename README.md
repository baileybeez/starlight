# Starlight

Starlight is a web server for the [gemini protocol](https://geminiprotocol.net/). 

> [!WARNING]
> This software is far from complete and may crash or be missing features. Keep your expectations low.

## Getting Started

1. create certificate and key files
```console 
$ openssl req -x509 -newkey rsa:4096 -sha256 -days 3650 -nodes \
    -keyout starlight.key -out starlight.crt -subj "/CN=localhost"
```

2. setup your starlight.ini file
```ini
PORT=1965
CERT=/path/to/starlight.CRT
KEY=/path/to/starlight.key
ROOT=/path/to/content
```

3. launch starlight 
```console
$ make
$ cd bin
$ ./starlight
```