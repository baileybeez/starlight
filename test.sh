
#openssl x509 -in bin/starlight.crt -text -noout

echo "gemini://localhost" | openssl s_client -quiet -crlf -connect localhost:1965
#echo "gemini://localhost" | openssl s_client -crlf -connect localhost:1965
