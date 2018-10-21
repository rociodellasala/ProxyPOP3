#include "include/media_types_utils.h"

int check_mime_not_in(char * mime, char * media_types) {
    char * regexmr = "[a-zA-Z0-9]+\\/[a-zA-Z0-9]+*(, [a-zA-Z0-9]+\\/[a-zA-Z0-9]+)*";
    printf("ACA \n");
    if (checkAgainstRegex(mime, regexmr)) {
        return -1;
    }
    printf("ACA 0\n");
    return parse(mime, media_types);
}

int parse(char *  mime, char * media_types) {
    int i = 0;
    int ret;
    int j;
    int size = countMediaRanges(media_types, ',');
    const char s[] = ",";
    char *token;
    char *str[size];

    token = strtok(media_types, s);
    str[i++] = token;

    while (token != NULL) {
        token = strtok(NULL, s);
        str[i++] = token;
    }

    i--;

    regex_t *regexArray = createRegexArray(str, i);
    printf("ACA 1\n");
    ret = validate(mime, regexArray, i);
    printf("ACA 2\n");
    free(mime);

    for (j = 0; j < i; j++) {
        regfree(&regexArray[j]);
    }

    regfree(regexArray);
    free(regexArray);
    free(mime);

    return ret;
}

int validate(char * mime, regex_t * regexArray, int size) {
    int i;
    int maxSize 		= strlen(mime);
    char * regex 		= "[a-zA-Z0-9]+\\/[a-zA-Z0-9]+(; charset=UTF-8|ISO-8859-1)?";
    regmatch_t * match 	= malloc(maxSize * sizeof(regmatch_t));

    if (checkAgainstRegex(mime, regex)) {
        free(match);
        return 0;
    }

    for (i = 0; i < size; i++) {
        if(regexec(&regexArray[i], mime, 1, match, REG_EXTENDED) == 0) {
            free(match);
            return 1;
        }
    }

    free(match);
    return -1;
}

/* Checks if the input is malformed like "hola mundo" */
int checkAgainstRegex(const char * str, const char * regexpr) {
    int maxSize 		=  strlen(str);
    int ret 			= 0;
    regex_t * compiled 	= malloc(sizeof(regex_t));
    regmatch_t * match 	= malloc(maxSize * sizeof(regmatch_t));
    
    printf("ACAAAAAAAA\n");
    
    if (!regexec(compiled, str, 1, match, REG_EXTENDED) == 0) {
        ret = 1;
    }

    printf("ACAAAAAAAA\n");
    regfree(compiled);
    free(compiled);
    free(match);

    return ret;
}

regex_t * createRegexArray(char ** expressions, const int size) {
    int compileFlags, i;
    const char * delim 		= "/";
    char * type, * subtype;
    char * regex 			= malloc(REGEXMAXLENGTH * sizeof(char));
    regex_t * compiledExp 	= malloc(size * sizeof(regex_t));

    for (i = 0; i < size; i++) {
        type = strtok(expressions[i], delim);
        subtype = strtok(NULL, delim);
        createRegex(type, subtype, regex);
        compileFlags = regcomp(&compiledExp[i], regex, REG_EXTENDED);
        if(compileFlags != 0)
            fputs("Error during creating regex compiler: ", stdout);
    }

    free(regex);

    return compiledExp;
}

/* Creates from each media-range a regular expression in order to
compare with each media-type in a more efficiently way */
void createRegex(const char * type, const char * subtype, char * regex) {
    if(strcmp(type, "*") == 0)
        strcpy(regex,"[a-zA-Z0-9]+\\/[a-zA-Z0-9]+");
    else if(strcmp(subtype, "*") == 0){
        strcpy(regex, type);
        strcat(regex, "\\/[a-zA-Z0-9]+");
    } else {
        strcpy(regex, type);
        strcat(regex, "\\/");
        strcat(regex, subtype);
    }
}

/* Returns the quantity of media-ranges of the argument */
int countMediaRanges(char * s, char c) {
    int count 		= 0;
    char * sTemp 	= s;

    while (*sTemp) {
        if (*sTemp++ == c)
            count++;
    }

    return count;
}