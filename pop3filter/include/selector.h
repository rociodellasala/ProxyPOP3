#ifndef PROXYPOP3_SELECTOR_H
#define PROXYPOP3_SELECTOR_H

#include <sys/time.h>
#include <stdbool.h>

/*
 * selector.c - un muliplexor de entrada salida
 *
 * Un selector permite manejar en un único hilo de ejecución la entrada salida
 * de file descriptors de forma no bloqueante.
 *
 * Esconde la implementación final (select(2) / poll(2) / epoll(2) / ..)
 *
 * El usuario registra para un file descriptor especificando:
 *  1. un handler: provee funciones callback que manejarán los eventos de
 *     entrada/salida
 *  2. un interés: que especifica si interesa leer o escribir.
 *
 * Es importante que los handlers no ejecute tareas bloqueantes ya que demorará
 * el procesamiento del resto de los descriptores.
 *
 * Si el handler requiere bloquearse por alguna razón (por ejemplo realizar
 * una resolución de DNS utilizando getaddrinfo(3)), tiene la posiblidad de
 * descargar el trabajo en un hilo notificará al selector que el resultado del
 * trabajo está disponible y se le presentará a los handlers durante
 * la iteración normal. Los handlers no se tienen que preocupar por la
 * concurrencia.
 *
 * Dicha señalización se realiza mediante señales, y es por eso que al
 * iniciar la librería `selector_init' se debe configurar una señal a utilizar.
 *
 * Todos métodos retornan su estado (éxito / error) de forma uniforme.
 * Puede utilizar `selector_error' para obtener una representación human
 * del estado. Si el valor es `SELECTOR_IO' puede obtener información adicional
 * en errno(3).
 *
 * El flujo de utilización de la librería es:
 *  - iniciar la libreria `selector_init'
 *  - crear un selector: `selector_new'
 *  - registrar un file descriptor: `selector_register_fd'
 *  - esperar algún evento: `selector_iteratate'
 *  - destruir los recursos de la librería `selector_close'
 */
typedef struct fdselector * fd_selector;

/* Valores de retorno */
typedef enum {
    /* Llamada exitosa */
            SELECTOR_SUCCESS  = 0,
    /* No pudimos alocar memoria */
            SELECTOR_ENOMEM   = 1,
    /* Llegamos al límite de descriptores que la plataforma puede manejar */
            SELECTOR_MAXFD    = 2,
    /* Argumento ilegal */
            SELECTOR_IARGS    = 3,
    /* Descriptor ya está en uso */
            SELECTOR_FDINUSE  = 4,
    /* I/O error check errno */
            SELECTOR_IO       = 5,
} selector_status;

/* Retorna una descripción humana del fallo */
const char * selector_error(selector_status);

/* Opciones de inicialización del selector */
struct selector_init {
    /* Señal a utilizar para notificaciones internas */
    const int signal;

    /* Tiempo máximo de bloqueo durante `selector_iteratate' */
    struct timespec select_timeout;
};

/* Inicializa la librería */
selector_status selector_init(const struct selector_init *);

/* Deshace la incialización de la librería */
selector_status selector_close(void);

/* Instancia un nuevo selector. Retorna NULL si no puede instanciar  */
fd_selector selector_new(size_t);

/* Destruye un selector creado por _new. Tolera NULLs */
void selector_destroy(fd_selector);

/**
 * Intereses sobre un file descriptor (quiero leer, quiero escribir, …)
 *
 * Son potencias de 2, por lo que se puede requerir una conjunción usando el OR
 * de bits.
 *
 * OP_NOOP es útil para cuando no se tiene ningún interés.
 */
typedef enum {
    OP_NOOP    = 0,
    OP_READ    = 1 << 0,
    OP_WRITE   = 1 << 2,
} fd_interest;

/* Argumento de todas las funciones callback del handler */
struct selector_key {
    /** el selector que dispara el evento */
    fd_selector s;
    /** el file descriptor en cuestión */
    int         fd;
    /** dato provisto por el usuario */
    void *      data;
};

/* Manejador de los diferentes eventos.. */
typedef struct fd_handler {
    void (*handle_read)      (struct selector_key * key);
    void (*handle_write)     (struct selector_key * key);
    void (*handle_block)     (struct selector_key * key);

    /* Llamado cuando se se desregistra el fd. Seguramente deba liberar los recusos alocados en data */
    void (*handle_close)     (struct selector_key * key);

} fd_handler;

/**
 * Registra en el selector `s' un nuevo file descriptor `fd'.
 *
 * Se especifica un `interest' inicial, y se pasa handler que manejará
 * los diferentes eventos. `data' es un adjunto que se pasa a todos
 * los manejadores de eventos.
 *
 * No se puede registrar dos veces un mismo fd.
 *
 * @return 0 si fue exitoso el registro.
 */
selector_status selector_register(fd_selector, int, const fd_handler *, fd_interest, void *);

/* Desregistra un file descriptor del selector */
selector_status selector_unregister_fd(fd_selector, int);

/* Permite cambiar los intereses para un file descriptor */
selector_status selector_set_interest(fd_selector, int, fd_interest);

/* Permite cambiar los intereses para un file descriptor */
selector_status selector_set_interest_key(struct selector_key *, fd_interest);


/* Se bloquea hasta que hay eventos disponible y los despacha. Retorna luego de cada iteración, o al llegar al timeout */
selector_status selector_select(fd_selector);

/**
 * Método de utilidad que activa O_NONBLOCK en un fd.
 *
 * retorna -1 ante error, y deja detalles en errno.
 */
int selector_fd_set_nio(int);

/* Notifica que un trabajo bloqueante terminó */
selector_status selector_notify_block(fd_selector, int);

#endif //PROXYPOP3_SELECTOR_H

