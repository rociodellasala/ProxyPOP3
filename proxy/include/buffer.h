#ifndef PROXYPOP3_BUFFER_H
#define PROXYPOP3_BUFFER_H

#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>  // size_t, ssize_t

/**
 * buffer.c - buffer con acceso directo (útil para I/O) que mantiene
 *            mantiene puntero de lectura y de escritura.
 *
 *
 * Para esto se mantienen dos punteros, uno de lectura
 * y otro de escritura, y se provee funciones para
 * obtener puntero base y capacidad disponibles.
 *
 * R=0
 * ↓
 * +---+---+---+---+---+---+
 * |   |   |   |   |   |   |
 * +---+---+---+---+---+---+
 * ↑                       ↑
 * W=0                     limit=6
 *
 * Invariantes:
 *    R <= W <= limit
 *
 * Se quiere escribir en el bufer cuatro bytes.
 *
 * ptr + 0 <- buffer_write_ptr(b, &wbytes), wbytes=6
 * n = read(fd, ptr, wbytes)
 * buffer_write_adv(b, n = 4)
 *
 * R=0
 * ↓
 * +---+---+---+---+---+---+
 * | H | O | L | A |   |   |
 * +---+---+---+---+---+---+
 *                 ↑       ↑
 *                W=4      limit=6
 *
 * Quiero leer 3 del buffer
 * ptr + 0 <- buffer_read_ptr, wbytes=4
 * buffer_read_adv(b, 3);
 *
 *            R=3
 *             ↓
 * +---+---+---+---+---+---+
 * | H | O | L | A |   |   |
 * +---+---+---+---+---+---+
 *                 ↑       ↑
 *                W=4      limit=6
 *
 * Quiero escribir 2 bytes mas
 * ptr + 4 <- buffer_write_ptr(b, &wbytes=2);
 * buffer_write_adv(b, 2)
 *
 *            R=3
 *             ↓
 * +---+---+---+---+---+---+
 * | H | O | L | A |   | M |
 * +---+---+---+---+---+---+
 *                         ↑
 *                         limit=6
 *                         W=4
 * Compactación a demanda
 * R=0
 * ↓
 * +---+---+---+---+---+---+
 * | A |   | M |   |   |   |
 * +---+---+---+---+---+---+
 *             ↑           ↑
 *            W=3          limit=6
 *
 * Leo los tres bytes, como R == W, se auto compacta.
 *
 * R=0
 * ↓
 * +---+---+---+---+---+---+
 * |   |   |   |   |   |   |
 * +---+---+---+---+---+---+
 * ↑                       ↑
 * W=0                     limit=6
 */

typedef struct buffer buffer;

struct buffer {
    uint8_t * data;

    /* Límite superior del buffer. inmutable */
    uint8_t * limit;

    /* Puntero de lectura */
    uint8_t * read;

    /* Puntero de escritura */
    uint8_t * write;
};

/* Inicializa el buffer sin utilizar el heap */
void buffer_init(buffer *, size_t, uint8_t *);

/* Retorna un puntero donde se pueden escribir hasta `*nbytes`.
 * Se debe notificar mediante la función `buffer_write_adv' */
uint8_t * buffer_write_ptr(buffer *, size_t *);

void buffer_write_adv(buffer *, ssize_t);

uint8_t * buffer_read_ptr(buffer *, size_t *);
void buffer_read_adv(buffer *, ssize_t);

/* Obtiene un byte del buffer */
uint8_t buffer_read(buffer *);

/* Escribe un byte en el buffer */
void buffer_write(buffer *, uint8_t);

/* Compacta el buffer */
void buffer_compact(buffer *);

/* Reinicia todos los punteros */
void buffer_reset(buffer *);

/* Retorna true si hay bytes para leer del buffer */
bool buffer_can_read(buffer *);

/* Retorna true si se pueden escribir bytes en el buffer */
bool buffer_can_write(buffer *);

#endif  //PROXYPOP3_BUFFER_H
