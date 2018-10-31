#ifndef PROXYPOP3_REQUEST_H
#define PROXYPOP3_REQUEST_H

#include "response.h"

#define CMD_SIZE    5
#define PARAM_SIZE  (40 * 2)

enum pop3_cmd_id {
    error = -1,
    /**
     * Sintaxis: USER identificacion
     *
     * Estado POP3: AUTORIZACION
     * Descripcion: Este comando permite identificarse con el nombre de usuario.
     * Este comando debe preceder al comando PASS.
     */
    user,
    /**
     * Sintaxis: PASS contraseña
     *
     * Estado POP3: AUTORIZACION
     * Descripcion: Este comando permite ingresar la contraseña para
     * terminar la autenticacion.
     */
    pass,



    /**
     * Sintaxis: RETR numero_mensaje
     *
     * Estado POP3: TRANSACCION
     * Descripcion: Este comando permite obtener el mensaje especificado por el
     * numero
     */
    retr,
    /**
     * Sintaxis: LIST
     *
     * Estado POP3: TRANSACCION
     * Descripcion: Este comando permite listar todos los mensajes con su numero
     * identificador seguido de su tamaño
     */
    list,
    /**
     * Sintaxis: STAT
     *
     * Estado POP3: TRANSACCION
     * Descripcion: Este comando permite obtener el numero y el tamaño
     * total de todos los mensajes
     */
    stat,
    /**
     * Sintaxis: DELE numero_mensaje
     *
     * Estado POP3: TRANSACCION
     * Descripcion: Este comando permite borrar el mensaje especificado por el
     * numero una vez terminada la sesion.
     */
    dele,
    /**
     * Sintaxis: NOOP
     *
     * Estado POP3: TRANSACCION
     * Descripcion: Este comando permite mantener la conexion abieta en caso
     * de inactividad
     */
    noop,
    /**
     * Sintaxis: TOP numero_mensaje n_lineas
     *
     * Estado POP3: TRANSACCION
     * Descripcion: Este comando permite obtener las primeras n lineas del mensaje
     * especificado por el numero. El servidor enviara los encabezados del mensaje,
     * una linea en blanco y finalmente las primeras n lineas del mensaje
     */
    top,
    /**
     * Sintaxis: RSET
     *
     * Estado POP3: TRANSACCION
     * Descripcion: Este comando permite recuperar los mensajes borrados
     */
    rset,



    /**
     * Sintaxis: QUIT
     *
     * Estado POP3: AUTORIZACION Y TRANSACCION
     * Descripcion: Este comando solicita la salida del servidor POP3.
     */
    quit,
    /**
     * Sintaxis: CAPA
     *
     * Estado POP3: AUTORIZACION Y TRANSACCION
     * Descripcion: Este comando devuelve una lista de capacidades
     * soportadas por el servidor POP3
     */
    capa,
};

struct pop3_request_cmd {
    const enum pop3_cmd_id 	        id;
    const char *			        name;
};

struct pop3_request {
    struct pop3_request_cmd *       cmd;
    char *                          args;
    const struct pop3_response *    response;
};

const struct pop3_request_cmd * get_cmd(const char *);

struct pop3_request * new_request(const struct pop3_request_cmd *, char *);

void destroy_request(struct pop3_request *);

#endif //PROXYPOP3_REQUEST_H
