Un site web permettant de piloter des volets roulants et lumières, via des relais connectés à un micro-contrôleur ESP32.

La carte utilisée est une LolinD32, connectée par WIFI.
Les fichiers HTML, CSS et JavaScript (frontend) sont stockés sur une partition SPIFF, à flasher sur la carte.
La LolinD32 est connectée à une carte de 8 relais de type optocoupleur ou solid state, selon la puissance nécessaire aux volets et/ou éclairages 230V.
On peut aussi connecter des MOSFET pour commander un ruban LED en 12V PWM (IRF520 par exemple).
Le serveur (backend) est écrit en C++ avec le framework Arduino. Il est nécessaire d'ajouter un fichier "src/secrets.h" pour déclarer les identifiants WIFI (ssid, password).

La compilation et le flash sont faits à l'aide de l'extension PlatformIO de VSCode.
