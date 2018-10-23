#ifndef PROXYPOP3_INPUT_PARSER_H
#define PROXYPOP3_INPUT_PARSER_H

#define DELIM "."
#define BACKLOG 10

struct options{
    int port;                           // -p
    char * error_file;                  // -e
    char * listen_address;              // -l
    char * management_address;          // -L
    int management_port;                // -o
    char * replacement_msg;             // -m
    char * filtered_media_types;        // -M
    char * origin_server;               // This is the argument origin_server, it's not an option
    int origin_port;                    // -P
    char * filter_command;              // -t
};

typedef struct options * options;

typedef int file_descriptor;

extern options parameters;

/* Chequea que no se haya llamado a -v o -h. Llama a las funciones
 * para validar el formato de las opciones y de los argumentos */
int parse_input(int, char **);

/* Imprime un manual de ayuda si el formato del input es erroneo */
void print_usage();

/* Imprime ayuda/version segun la opcion ingresada */
void print_help();
void print_version();

/* Valida el formato del input. Es decir: -opcion argumento ... -opcion argumento o_server */
int validate_options(int, char **);

/* Libera estructuras utilizadas */
void free_options(char **, int);

/* Valida el formato de los argumentos ingresados */
int validate_parameters(char *, char *);

int validate_address(char *);
int validate_error_file(char *);
int validate_message(char *);
int validate_media_type(char *);
int validate_port(char *);
int validate_transformation(char *);

int validate_origin_server_argument(char *);

/* Util para validar origin server */
int is_valid_ip(char *);
int valid_digit(char *);

/* Inicializa en la estructura de parametros los valores default */
options initialize_values();

/* Setea los parametros segun lo ingresado como opcion */
options set_options_values(int, char **);

#endif //PROXYPOP3_INPUT_PARSER_H
