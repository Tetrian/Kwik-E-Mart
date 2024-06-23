# Kwik-E-Mart
Progetto client-server per Laboratorio di Sistemi Operativi 2024
## Uso
### Server
Nella directory *server* eseguire:
```console
make server
docker-compose up -d
bin/server
```
### Client
Nella directory *client/src* eseguire:
```console
# Per eseguire il client GUI
python3 main.py
# Per eseguire un client automatizzato
python3 client.py
```
## Dipendenze
### Server
make\
docker-compose\
postgresql\
libq-dev\
pkg-config
### Client
Kivy
