
#openssl x509 -in bin/starlight.crt -text -noout

# simple testing against starlight
echo "gemini://localhost" | openssl s_client -quiet -crlf -connect localhost:1965
