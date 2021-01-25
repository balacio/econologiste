#include <pgmspace.h>

#define SECRET
#define THINGNAME "MyNewESP32"

const char WIFI_SSID[] = "Livebox-Ahmed";
const char WIFI_PASSWORD[] = "Ahmed06110";
const char AWS_IOT_ENDPOINT[] = "a1xuj8bqvjhb5r-ats.iot.eu-west-3.amazonaws.com";

// Amazon Root CA 1
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
MIIDWTCCAkGgAwIBAgIUVsH2JA+d8ScBViFUy8cUXn7bmpUwDQYJKoZIhvcNAQEL
BQAwTTFLMEkGA1UECwxCQW1hem9uIFdlYiBTZXJ2aWNlcyBPPUFtYXpvbi5jb20g
SW5jLiBMPVNlYXR0bGUgU1Q9V2FzaGluZ3RvbiBDPVVTMB4XDTIwMDQyMTE1MDc0
NVoXDTQ5MTIzMTIzNTk1OVowHjEcMBoGA1UEAwwTQVdTIElvVCBDZXJ0aWZpY2F0
ZTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBANCIL54BhhAdNNBBKk//
HPKWeqoDNU36PilVMPm/z8WJ7QQea834tW32hmHEF2Ak6J93oytz7hnWPuuXY7jv
xfKZ5R7wcB/6NWrGm+yA8MN9lINXApcCFGOAJ+jB+X2CbmMEoVd99dLZA7NS1pfJ
WsAOF8zFc4eRj0zfeFMhIS0lDr5QbI0964PF6oFubr2QB0IzmzkeKQxdn9OEOABW
2AnLTKRaizvGKXpjDuU85nnCoNM/A6Mgn5hjsKCMJrWFk5+E7mpQaEFvexmSdOVw
vVW1vf8OoOB4RmVWM6T400Kk6EGBtDxxQ1WCNk8TYmXlQuQkVjLFhwpl9RvoWHzQ
ND8CAwEAAaNgMF4wHwYDVR0jBBgwFoAUOxEnL9f1fRHtRxuiHdnNZKmvFUswHQYD
VR0OBBYEFPQ/d4MzO25435QwEoJhF2/fja5eMAwGA1UdEwEB/wQCMAAwDgYDVR0P
AQH/BAQDAgeAMA0GCSqGSIb3DQEBCwUAA4IBAQAeKmY3c2kSCO2B3HGFG2NCrfON
Cdqi+I1c3olnDBp7kmXV2DBFKTgduHeLZaUpBQCvx4hB5u32sFXnUZNzoP+/5vYg
9Wqn2CvtQtKjGmpgNvgmZaasr/1yQOdCs3hEOBwrdhoYI8faKuX1FbfKG+osJVuJ
B06ZflUo84OEKEQVm4/J5NFgMNDfD9YMvNOjo+80bfR6c3u/8MrDI9qSDWr93v+p
Qq/1G4W8zGa7ZSFU9NW2f4Lv5/Ol09F3bJHg8XiR3u4elyQr0+opi60WXGJPIvO+
npATj5KRHCt1Czn7j4qNYmjVCJ3seYthRCdBlHpSB4vzewcEY9n2FN2fLJke
-----END CERTIFICATE-----

)KEY";

// Device Private Key
static const char AWS_CERT_PRIVATE[] PROGMEM = R"KEY(
-----BEGIN RSA PRIVATE KEY-----
MIIEpAIBAAKCAQEA0IgvngGGEB000EEqT/8c8pZ6qgM1Tfo+KVUw+b/PxYntBB5r
zfi1bfaGYcQXYCTon3ejK3PuGdY+65djuO/F8pnlHvBwH/o1asab7IDww32Ug1cC
lwIUY4An6MH5fYJuYwShV3310tkDs1LWl8lawA4XzMVzh5GPTN94UyEhLSUOvlBs
jT3rg8XqgW5uvZAHQjObOR4pDF2f04Q4AFbYCctMpFqLO8YpemMO5TzmecKg0z8D
oyCfmGOwoIwmtYWTn4TualBoQW97GZJ05XC9VbW9/w6g4HhGZVYzpPjTQqToQYG0
PHFDVYI2TxNiZeVC5CRWMsWHCmX1G+hYfNA0PwIDAQABAoIBAQCLxWxdNNo9/MWE
fNk2i2FEvFX1lHbrEDsQCzjrfYRu/DawVw+dHAHHdcC20O92EucU0+S7YFYMXU0J
Lgy6TYRZSouHRFNzK4sYSY0DZZ16+rZLnktvGga+dozJdxKZ7I2+ssDN2vCk5a6U
SIbrcBvytL+BEiuQh6EMPpDw7zjEf2doRZMInEElP5VDqUCtMgY8AwidiN7blvqc
6gLgYrisatVCWGZuV/bKxTgVYTpgFWnt5psqs5nGHE3iZIqpS4qmkwEVa2zOfFAc
MLgBNslfHcigIohkFP/zegSR2yToOfqOieFXWKiizo4KNkMBnZxdjfiCY2iLXfiY
2jWdPIEJAoGBAOvxm/TicG9qdNLJF6H27H+Qzf95b51v4j1a3pBpUfmdw8yh8POU
VyQVQsqBTzcFrRTNWLiziFD5L7ZlDYMyJcrKuQTMVsdgb95C2gkE5bBASmXiwpJo
X4SX+DN2vxtjBfzoGEJYP0zpilX9kAEvgMv8VNMyyLfFEHEVBDdEe63NAoGBAOJC
EPoa5lMRYX0+4jY3dMDSuwv+5MhqlzegI4gYCc99f4zA/zBVZIk1/YW5D1S6v4Kk
6kpMg/uNqqTqGDhCJE/ExCDoxcnTbjjHBujlhXhu6S+8VDZi5gaBBEZcO3BYnQtF
bnUhXr/tuuZ8UqVesbrwWi3h0TY+k5mbxUHFlb47AoGBAIQRfcaZaW40HKhQP5Y9
bSwKlgRGqR9rHSxSzPnwuh4LHSpSC1mM0yozbGuF7h1SZAuc1eitNcR9dyOVdPbW
/yiASQhSiNCZPoUezoT15Mu/cOWPBHKHUdAacKf99AMUbqmeENYyJ1yZ7QEtKYpA
WMvVtnVxm8Ch4g4qQW3eO4yRAoGACZyE3BsIbb7vRULGmUcBzq8lAv1w+pKMmfBT
khRzVj0ejfwk2wjFnI6Oln+mBxNnz0TyLvXRJX5/IUJQhRNKN5lEO6GnxUsWA5Nw
17PoV5VekoiNi0HoQ553cAW7AKrDAReR+afRPpX6uo/Ncvjy094h9pEBMiy2WHAl
kQWztsECgYAM4EdVX36tp7qyJ3X3sd4tKaTLkCWjFnkNHjKVou+/GATjT5gb1IF7
ZxoCq6Tn6X042QobFLio+lqJX3v7NX1o5OKqoXaBOU4hBCK6kxAqhfvsj1UQD4Zz
4gpsyVLbn6a2zHkHaOE95wT2aaAUS+R4aP3cj1rHN+VXVPkunPgmJA==
-----END RSA PRIVATE KEY-----

)KEY";