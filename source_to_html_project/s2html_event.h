#ifndef S2HTML_EVENT_H
#define S2HTML_EVENT_H

#include <stdio.h>

#define PEVENT_DATA_SIZE 1024

typedef enum
{
    PEVENT_PREPROCESSOR_DIRECTIVE,
    PEVENT_RESERVE_KEYWORD,
    PEVENT_NUMERIC_CONSTANT,
    PEVENT_STRING,
    PEVENT_HEADER_FILE,
    PEVENT_REGULAR_EXP,
    PEVENT_SINGLE_LINE_COMMENT,
    PEVENT_MULTI_LINE_COMMENT,
    PEVENT_ASCII_CHAR,
    PEVENT_EOF
} pevent_e;

typedef enum
{
    PROPERTY_NONE = 0,
    PROPERTY_USER_HEADER,
    PROPERTY_STANDARD_HEADER,
    PROPERTY_DATA_TYPE_KEYWORD,
    PROPERTY_NON_DATA_KEYWORD
} pevent_property_e;

typedef struct
{
    pevent_e type;
    int property;
    int length;
    char data[PEVENT_DATA_SIZE];
} pevent_t;

pevent_t *get_parser_event(FILE *fp);

#endif
