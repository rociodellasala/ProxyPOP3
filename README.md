# ProxyPOP3
# Grupo 3 - Protocolos de Comunicación 2018 2C

## Códigos fuentes
En el directorio proxy_pop3 se encuentra el codigo fuente del servidor proxy. En el directorio admin_client se encuentra el codigo fuente del cliente de administracion. En el directorio mime_filter se encuentra el codigo fuente del programa transformador de tipos MIME. Se adjunta en un archivo CMake para compilar el trabajo.

## Informe
En el directorio principal del proyecto se encuentra el informe en formato pdf detallando la implementación del proyecto. 

## Instrucciones de compilación
En el directorio principal del proyecto realizar:
```sh
$ cmake .
$ make all
```
Esto va a generar tres ejecutables:
  - stripmime
  - pop3filter
  - pop3ctl

## Ejecución
Para ejecutar pop3filter:
```sh
$ ./pop3filter <opciones> [Origin Server]
```

Para ejecutar pop3ctl:
```sh
$ ./pop3ctl [Management Port] [Management Address]
```
**Contraseña de administrador: protosgrupo3**

Para ejecutar stripmime (utiliza las variables FILTER_MEDIAS y FILTER_MESSAGE):
```sh
$ ./stripmime
```
## Integrantes
- Della Sala, Rocío
- Giorgi, María Florencia
- Rodriguez, Ariel Andrés
- Santoflaminio, Alejandro
