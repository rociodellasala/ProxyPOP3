#ifndef PROXYPOP3_MEDIA_TYPES_UTILS_H
#define PROXYPOP3_MEDIA_TYPES_UTILS_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <regex.h>

#define REGEXMAXLENGTH 20

int check_mime_not_in(char *, char *);
int parse(char * , char *);
int validate(char *, regex_t *, int);
int checkAgainstRegex(const char *, const char *);
regex_t * createRegexArray(char **, const int);
void createRegex(const char *, const char *, char *);
int countMediaRanges(char *, char);

#endif //PROXYPOP3_MEDIA_TYPES_UTILS_H
