# ProxyPOP3 - Grupo 3 - Protocolos de Comunicación 2018 2C

# Instrucciones de compilación
En el directorio principal del proyecto realizar:
```sh
$ cmake .
$ make all
```
Esto va a generar tres ejecutables:
  - stripmime
  - pop3filter
  - pop3ctl


# Ejecución

Para ejecutar pop3filter:
```sh
$ ./pop3filter <opciones> [origin server]
```

Para ejecutar pop3ctl:
```sh
$ ./pop3ctl [Management Port] [Management Address]
```
**Contraseña de administrador: 1234**

Para ejecutar stripmime (utiliza las variables FILTER_MEDIAS y FILTER_MESSAGE):
```sh
$ ./stripmime
```

# Informe
En el directorio principal del proyecto se encuentra el informe en formato pdf detallando la implementación del proyecto. 
