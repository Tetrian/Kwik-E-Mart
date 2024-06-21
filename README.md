# Kwik-E-Mart
Progetto client-server per Laboratorio di Sistemi Operativi 2024
## Uso
### Server
Nella directory *server* eseguire:
#+begin_src bash
make server
docker-compose up -d
bin/server
#+end_src
### Client
Nella directory *client/src* eseguire:
#+begin_src bash
# Per eseguire l'interfaccia grafica
python3 main.py
# Per eseguire un client automatizzato
python3 client.py
#+end_src
## Dipendenze
### Server
make\
docker-compose\
postgresql\
libq-dev\
pkg-config
### Client
Kivy
