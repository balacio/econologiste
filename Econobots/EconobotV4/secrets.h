#include <pgmspace.h>

#define SECRET
#define THINGNAME "EconobotDemo"

//const char WIFI_SSID[] = "Freebox-92E6B4";
//const char WIFI_PASSWORD[] = "0618339894";
const char WIFI_SSID[] = "Livebox-Ahmed";
const char WIFI_PASSWORD[] = "Ahmed06110";
const char ENDPOINT[] = "a1xuj8bqvjhb5r-ats.iot.eu-west-3.amazonaws.com";

// Amazon Root CA 1 - Don't delete - Never changes
static const char AWS_CERT_CA[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF
ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6
b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL
MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv
b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj
ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM
9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw
IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6
VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L
93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm
jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC
AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA
A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI
U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs
N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv
o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU
5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy
rqXRfboQnoZsG4q5WTP468SQvvG5
-----END CERTIFICATE-----
)EOF";

// Device Certificate
static const char AWS_CERT_CRT[] PROGMEM = R"KEY(
-----BEGIN CERTIFICATE-----
MIIDWTCCAkGgAwIBAgIUUKw66pCmBKA8NC4KK1BOZt3j0igwDQYJKoZIhvcNAQEL
BQAwTTFLMEkGA1UECwxCQW1hem9uIFdlYiBTZXJ2aWNlcyBPPUFtYXpvbi5jb20g
SW5jLiBMPVNlYXR0bGUgU1Q9V2FzaGluZ3RvbiBDPVVTMB4XDTIwMDQyNjEyMzc1
NloXDTQ5MTIzMTIzNTk1OVowHjEcMBoGA1UEAwwTQVdTIElvVCBDZXJ0aWZpY2F0
ZTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAOFiwkIA0qnRrF5e9JMg
rIOb0pM/GhOnyZmO9bGIfdQj9PSKgFo6akhhzHonf02oSmdsFk9EB/byzbHE8sPe
2eGUL+oDkCh4W6kPQuDvyVFDJ7i9FljaddIpVhCqO3Kf/Sak2jkHUlg6SKk+kV5s
an5j7axiv9POqg6DU2HzAOClC1mgFe9a/sdKodjqocxsgLsUmMuOIBYq9IF9vgZt
jT7ykkeTQKIx0q5OkvnzvNHIjf5bPw2SLKQ9reDZYtFNhh5NX2pL7/66HpA/pBWc
SXK+gCr8R5/sBZUJzXL6kOvGFr3N/xhvcGU7Sjc0tri73UAGd0zVjO8cPY6+ypkj
BUcCAwEAAaNgMF4wHwYDVR0jBBgwFoAUo9dqdigcbu/RWosbgSjWpzuKrP0wHQYD
VR0OBBYEFDUhyzPlueyiYYYB3KYyjphOlIGEMAwGA1UdEwEB/wQCMAAwDgYDVR0P
AQH/BAQDAgeAMA0GCSqGSIb3DQEBCwUAA4IBAQApyI2ItGIgxlcxuOTR0wlO4Q3+
oFF3WrXAdBarrne3kPfew2630lpgYM5N4szOKWgqZNDI5UeknwvBc8Yj3pPv/+8b
nCTvbCFNURbYXGTcxvJcOljgUWcJCX4BqUduIk1HQB7sZcbu4fPaLId3+v1udSNS
48rdpflY+wFL8ih4mSW1xXEulYkvuAfHNAJdgY6xH++QwYOwtQjz0unyl2dpC3lY
eVgekZoYh+FAu4y7kVQ9PMEbkpZ12U6Qg4ZmDD7PNDQk/9iJijmPVOaK4N2wRuey
SHvOSV9qCHRZcippeLKeQrhySdBdgj+F4A8oWqD0LeX4lNCKDnggoaIIgktx
-----END CERTIFICATE-----
)KEY";

// Device Private Key
static const char AWS_CERT_PRIVATE[] PROGMEM = R"KEY(
-----BEGIN RSA PRIVATE KEY-----
MIIEpAIBAAKCAQEA4WLCQgDSqdGsXl70kyCsg5vSkz8aE6fJmY71sYh91CP09IqA
WjpqSGHMeid/TahKZ2wWT0QH9vLNscTyw97Z4ZQv6gOQKHhbqQ9C4O/JUUMnuL0W
WNp10ilWEKo7cp/9JqTaOQdSWDpIqT6RXmxqfmPtrGK/086qDoNTYfMA4KULWaAV
71r+x0qh2OqhzGyAuxSYy44gFir0gX2+Bm2NPvKSR5NAojHSrk6S+fO80ciN/ls/
DZIspD2t4Nli0U2GHk1fakvv/roekD+kFZxJcr6AKvxHn+wFlQnNcvqQ68YWvc3/
GG9wZTtKNzS2uLvdQAZ3TNWM7xw9jr7KmSMFRwIDAQABAoIBABewMYqOW5XNm7gv
crUj8f3DGtavAyads+qXw+rs1Yy+Wrh49hSFnzYMUVllXVPkJC3Khm2+ULf7vPK6
KGm9ulijIoYN6dDXFYsnN3g8u4P28u3PsdGU5F06SvllauoZNpohKW5k/pqXRZvE
H7M04dx21DwSBOdWCTmNQ+4/hBAYFh+q+INOnwHbEFo+InQcKSkbSsiOElyxNQuZ
M7U1+SBmFntEOYyb5VID6n6k8I37PmvJh+23jSoaUOHnD6eTRUcRAuFXYAI1RmdM
WM5iP/ywMXhFByR7K9JMT8ZnzfcOpncW0feSAF9g0Xf+VWIspXIHeVRh4rPCO+HM
bhlwlcECgYEA9epnooS4V6ivEA2JeahmKceD1NBT64WDadzGHETR8uegIf6LjxxH
F+orBOoducySYApwCjk3G5wbsYfwgjkb2reSDzbQs7Xqq9XZ2fubmIIR08oUT386
TCxMBCLCHuvXsBdOIkZUXHcsd2R2Ww4xgTxJx2P2E/br0T660fxs1GMCgYEA6qDV
bwVHSXF8RJbO/m5ojz+EIhdxGkwi3k+/F0KWA/QLHxBv4HhVsjilUegD1IdM6QIx
POKllsSokI66b6kIdTNbgEfpyqVhae+EaRIomAEFWIO/2gqbHbP154o3F/ORufyH
QazKzR1J9eC5ssO862BSHl9jOSGht/tD5o3k5s0CgYEAgplzYm8jy4INPvRBf3e8
khGgZMQi3WDxsqYKHHc8HMmAVK1vhphY9U1qfVisYsFPN2wzpmpOZv5YAseUJkSY
E2cMTcIpwlcpgwq2gLrMHoltWFHoyXNjDZM057lh6LQNvzoRR8icAuNTnTCgXqlC
B0VyVU80H9Qx1tV+uAhb78sCgYBQWRGeOlWGPhnLN0XddZvS6nJqZlKyKb6LGQ4c
fg33fpxMYAmf+jWGmcVrfiPJpSfeZ4TLD+aa8e+zyVzpHh6qXBMKpBXY8regWoii
nrhODZNyE3QgzB8Alf8w8nEZf6y431TGVdEBWvVYAUQ2hrdsd4jHo+zLxMunwKEp
FBB+RQKBgQDlUkM0aVIyTwpy7ISrHTx/nl5Ru8iMiBe77DLlol6B2jQ2v/GnFvEp
CMkCHLyfFQ3IkIutpwxEbak2K8Wi2oAYzChndBxSIghTsWRlfzQ8QkgvQvUSVqfj
/A0qKWT2zZR75GZm5SyQQQpebMPvvG9QsryaNsyIvLwf3why8nYieA==
-----END RSA PRIVATE KEY-----
)KEY";
