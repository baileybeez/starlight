# Starlight
Starlight is a web server for the gemini protocol. 

🔴 IMPORTANT 🔴 This software is far from complete and may crash or be missing features.

## Getting Started

1. create certificate and key files
```
openssl req -x509 -newkey rsa:4096 -sha256 -days 3650 -nodes \
    -keyout starlight.key -out starlight.crt -subj "/CN=localhost"
```
2. setup your starlight.ini file
```
PORT=1965
CERT=/path/to/starlight.CRT
KEY=/path/to/starlight.key
ROOT=/path/to/content
```

3. launch starlight 
```
./starlight
```