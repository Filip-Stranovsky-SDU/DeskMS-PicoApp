/*
 * Copyright (c) 2023 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#pragma once

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

// Using this url as we know the root cert won't change for a long time
#define TLS_CLIENT_SERVER "34.116.203.248"

//#define TLS_CLIENT_SERVER "fw-download-alias1.raspberrypi.com"
#define TLS_CLIENT_HTTP_REQUEST  "GET / HTTP/1.1\r\n" \
                                 "Host: " TLS_CLIENT_SERVER "\r\n" \
                                 "Upgrade: websocket\r\n" \
                                 "Connection: Upgrade\r\n" \
                                 "Sec-WebSocket-Key: %s\r\n" \
                                 "Sec-WebSocket-Version: 13\r\n" \
                                 "\r\n"

//#define TLS_CLIENT_HTTP_REQUEST  "GET Hello from pico\r\n" \
                                 "\r\n"
#define TLS_CLIENT_TIMEOUT_SECS  15

// This is the PUBLIC root certificate exported from a browser
// Note that the newlines are needed
#define TLS_ROOT_CERT_OK "-----BEGIN CERTIFICATE-----\n\
MIIDJDCCAgygAwIBAgIUWBruXmAqWyTQbIX0FtYabVDNVDQwDQYJKoZIhvcNAQEL\n\
BQAwGTEXMBUGA1UEAwwOMTAuMjMzLjE4Ny4xMDAwHhcNMjUxMTMwMTEzMDE4WhcN\n\
MjYxMTMwMTEzMDE4WjAZMRcwFQYDVQQDDA4xMC4yMzMuMTg3LjEwMDCCASIwDQYJ\n\
KoZIhvcNAQEBBQADggEPADCCAQoCggEBAK1BSnZ3eylI2sz/jmiLsiMXg6h+oo+4\n\
wCRiIxJSyD/OrrqvYym5SKxqWOkr2HffmUHzQMbbp7L+JWSlTSmVsgmFiq3an8AJ\n\
p+h9NsCBUYWRqKLi4QivkCTZkqiLjKDDynnwb9b/LYR33ujyjbsnDT6j5g/mcYcH\n\
UbvVlIgSGwHYYpA1GsgvLwk/sQTOn9v5g27p5VLWuNkO2xmetvtTXFC59XAXsgqM\n\
zDJT9nvVdUHtJ68sXJpkOE7h5E0rxTzG+2zb4E9yTylsmPzCmC6Rmftrb8h3bD55\n\
xeHDtgMSeLehZtcX+d45Ik+k9mqCO+XdfpiPavGUqxQtagcyKe6ha8UCAwEAAaNk\n\
MGIwHQYDVR0OBBYEFM5WYvqKD9HyVe3N28dKNow93cM/MB8GA1UdIwQYMBaAFM5W\n\
YvqKD9HyVe3N28dKNow93cM/MA8GA1UdEwEB/wQFMAMBAf8wDwYDVR0RBAgwBocE\n\
Cum7ZDANBgkqhkiG9w0BAQsFAAOCAQEAl74cuVtq7DuFVSlDxr8RaZApIr/D9P+K\n\
fEn7UAo6PDcF4DleTWzvFNsOiID/EeQAcf2/JELUwrXYW7jvFIEfDx+pwDSDRZGi\n\
sTuMNiLe/YIqORkVs9ErvuKbOIG/97GSO8FmJyYfyOzUBjQO09kij8nqwv0tMEXp\n\
0HkbsVsQTPGAOR7r4ANJq/kQ30JphX6NZa4Bbq0cdVmB1R01hkFgK/hWx2qHxtV8\n\
rHWRuwmfdqeEUb4j/llUVmuyh5vOl3ICYlmR1E59EjZ1+iGlhtipurC8T9kG3+VW\n\
CscpdAPB6lOMKMNRhswm6wrHRPZKIpaq4Zv6ySrtoKNgtCgqCcHH3A==\n\
-----END CERTIFICATE-----\n"

// This is a test certificate
#define TLS_ROOT_CERT_BAD "-----BEGIN CERTIFICATE-----\n\
MIIDezCCAwGgAwIBAgICEAEwCgYIKoZIzj0EAwIwgasxCzAJBgNVBAYTAkdCMRAw\n\
DgYDVQQIDAdFbmdsYW5kMR0wGwYDVQQKDBRSYXNwYmVycnkgUEkgTGltaXRlZDEc\n\
MBoGA1UECwwTUmFzcGJlcnJ5IFBJIEVDQyBDQTElMCMGA1UEAwwcUmFzcGJlcnJ5\n\
IFBJIEludGVybWVkaWF0ZSBDQTEmMCQGCSqGSIb3DQEJARYXc3VwcG9ydEByYXNw\n\
YmVycnlwaS5jb20wHhcNMjExMjA5MTMwMjIyWhcNNDYxMjAzMTMwMjIyWjA6MQsw\n\
CQYDVQQGEwJHQjErMCkGA1UEAwwiZnctZG93bmxvYWQtYWxpYXMxLnJhc3BiZXJy\n\
eXBpLmNvbTBZMBMGByqGSM49AgEGCCqGSM49AwEHA0IABJ6BQv8YtNiNv7ibLtt4\n\
lwpgEr2XD4sOl9wu/l8GnGD5p39YK8jZV0j6HaTNkqi86Nly1H7YklzbxhFy5orM\n\
356jggGDMIIBfzAJBgNVHRMEAjAAMBEGCWCGSAGG+EIBAQQEAwIGQDAzBglghkgB\n\
hvhCAQ0EJhYkT3BlblNTTCBHZW5lcmF0ZWQgU2VydmVyIENlcnRpZmljYXRlMB0G\n\
A1UdDgQWBBRlONP3G2wTERZA9D+VxJABfiaCVTCB5QYDVR0jBIHdMIHagBQnpjMi\n\
oWHiuFARuYKcRtaYcShcBaGBvaSBujCBtzELMAkGA1UEBhMCR0IxEDAOBgNVBAgM\n\
B0VuZ2xhbmQxEjAQBgNVBAcMCUNhbWJyaWRnZTEdMBsGA1UECgwUUmFzcGJlcnJ5\n\
IFBJIExpbWl0ZWQxHDAaBgNVBAsME1Jhc3BiZXJyeSBQSSBFQ0MgQ0ExHTAbBgNV\n\
BAMMFFJhc3BiZXJyeSBQSSBSb290IENBMSYwJAYJKoZIhvcNAQkBFhdzdXBwb3J0\n\
QHJhc3BiZXJyeXBpLmNvbYICEAAwDgYDVR0PAQH/BAQDAgWgMBMGA1UdJQQMMAoG\n\
CCsGAQUFBwMBMAoGCCqGSM49BAMCA2gAMGUCMEHerJRT0WmG5tz4oVLSIxLbCizd\n\
//SdJBCP+072zRUKs0mfl5EcO7dXWvBAb386PwIxAL7LrgpJroJYrYJtqeufJ3a9\n\
zVi56JFnA3cNTcDYfIzyzy5wUskPAykdrRrCS534ig==\n\
-----END CERTIFICATE-----\n"


