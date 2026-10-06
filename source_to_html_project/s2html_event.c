#include "s2html_event.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

typedef enum
{
    PSTATE_IDLE,
    PSTATE_PREPROCESSOR_DIRECTIVE,
    PSTATE_HEADER_FILE,
    PSTATE_RESERVE_KEYWORD,
    PSTATE_NUMERIC_CONSTANT,
    PSTATE_STRING,
    PSTATE_SINGLE_LINE_COMMENT,
    PSTATE_MULTI_LINE_COMMENT,
    PSTATE_ASCII_CHAR
} parser_state_e;

static parser_state_e state = PSTATE_IDLE;

static pevent_t event;
static char buffer[PEVENT_DATA_SIZE];

static int is_data_type_keyword(const char *word)
{
    const char *list[] =
    {
        "const", "volatile", "static", "signed", "unsigned",
        "int", "float", "double", "char", "void", "struct",
        "union", "enum", "typedef", "long", "short"
    };

    size_t i;
    for (i = 0; i < sizeof(list) / sizeof(list[0]); i++)
        if (strcmp(word, list[i]) == 0)
            return 1;

    return 0;
}

static int is_non_data_keyword(const char *word)
{
    const char *list[] =
    {
        "return", "if", "else", "for", "while", "do",
        "switch", "case", "default", "break", "continue",
        "goto", "sizeof"
    };

    size_t i;
    for (i = 0; i < sizeof(list) / sizeof(list[0]); i++)
        if (strcmp(word, list[i]) == 0)
            return 1;

    return 0;
}

static int is_reserved_keyword(const char *word)
{
    if (is_data_type_keyword(word))
        return PROPERTY_DATA_TYPE_KEYWORD;

    if (is_non_data_keyword(word))
        return PROPERTY_NON_DATA_KEYWORD;

    return PROPERTY_NONE;
}

static void reset_event(void)
{
    memset(&event, 0, sizeof(event));
}

static void set_event(pevent_e type, int property, const char *data, int len)
{
    reset_event();

    event.type = type;
    event.property = property;

    if (len >= PEVENT_DATA_SIZE)
        len = PEVENT_DATA_SIZE - 1;

    if (len > 0)
        memcpy(event.data, data, (size_t)len);

    event.data[len] = '\0';
    event.length = len;
}

static pevent_t *make_eof(void)
{
    set_event(PEVENT_EOF, PROPERTY_NONE, "", 0);
    return &event;
}

static pevent_t *parse_preprocessor(FILE *fp, int first)
{
    int c;
    int len = 0;

    buffer[len++] = (char)first;

    while ((c = fgetc(fp)) != EOF)
    {
        if (c == '\n')
        {
            ungetc(c, fp);
            break;
        }

        if (len < PEVENT_DATA_SIZE - 1)
            buffer[len++] = (char)c;
    }

    set_event(PEVENT_PREPROCESSOR_DIRECTIVE, PROPERTY_NONE, buffer, len);
    return &event;
}

static pevent_t *parse_comment(FILE *fp, int first)
{
    int c;
    int len = 0;

    buffer[len++] = (char)first;

    c = fgetc(fp);

    if (c == '/')
    {
        buffer[len++] = '/';

        while ((c = fgetc(fp)) != EOF)
        {
            if (len < PEVENT_DATA_SIZE - 1)
                buffer[len++] = (char)c;

            if (c == '\n')
                break;
        }

        set_event(PEVENT_SINGLE_LINE_COMMENT, PROPERTY_NONE, buffer, len);
        return &event;
    }

    if (c == '*')
    {
        buffer[len++] = '*';

        int previous = 0;

        while ((c = fgetc(fp)) != EOF)
        {
            if (len < PEVENT_DATA_SIZE - 1)
                buffer[len++] = (char)c;

            if (previous == '*' && c == '/')
                break;

            previous = c;
        }

        set_event(PEVENT_MULTI_LINE_COMMENT, PROPERTY_NONE, buffer, len);
        return &event;
    }

    if (c != EOF)
        ungetc(c, fp);

    set_event(PEVENT_REGULAR_EXP, PROPERTY_NONE, buffer, len);
    return &event;
}

static pevent_t *parse_string(FILE *fp, int quote)
{
    int c;
    int len = 0;
    int escaped = 0;

    buffer[len++] = (char)quote;

    while ((c = fgetc(fp)) != EOF)
    {
        if (len < PEVENT_DATA_SIZE - 1)
            buffer[len++] = (char)c;

        if (escaped)
        {
            escaped = 0;
            continue;
        }

        if (c == '\\')
        {
            escaped = 1;
            continue;
        }

        if (c == '"')
            break;
    }

    set_event(PEVENT_STRING, PROPERTY_NONE, buffer, len);
    return &event;
}

static pevent_t *parse_char_literal(FILE *fp, int quote)
{
    int c;
    int len = 0;
    int escaped = 0;

    buffer[len++] = (char)quote;

    while ((c = fgetc(fp)) != EOF)
    {
        if (len < PEVENT_DATA_SIZE - 1)
            buffer[len++] = (char)c;

        if (escaped)
        {
            escaped = 0;
            continue;
        }

        if (c == '\\')
        {
            escaped = 1;
            continue;
        }

        if (c == '\'')
            break;
    }

    set_event(PEVENT_ASCII_CHAR, PROPERTY_NONE, buffer, len);
    return &event;
}

static pevent_t *parse_number(FILE *fp, int first)
{
    int c;
    int len = 0;

    buffer[len++] = (char)first;

    while ((c = fgetc(fp)) != EOF)
    {
        if (isalnum((unsigned char)c) || c == '.' || c == '_')
        {
            if (len < PEVENT_DATA_SIZE - 1)
                buffer[len++] = (char)c;
        }
        else
        {
            ungetc(c, fp);
            break;
        }
    }

    set_event(PEVENT_NUMERIC_CONSTANT, PROPERTY_NONE, buffer, len);
    return &event;
}

static pevent_t *parse_word(FILE *fp, int first)
{
    int c;
    int len = 0;
    int property;

    buffer[len++] = (char)first;

    while ((c = fgetc(fp)) != EOF)
    {
        if (isalnum((unsigned char)c) || c == '_')
        {
            if (len < PEVENT_DATA_SIZE - 1)
                buffer[len++] = (char)c;
        }
        else
        {
            ungetc(c, fp);
            break;
        }
    }

    buffer[len] = '\0';
    property = is_reserved_keyword(buffer);

    if (property)
    {
        set_event(PEVENT_RESERVE_KEYWORD, property, buffer, len);
    }
    else
    {
        set_event(PEVENT_REGULAR_EXP, PROPERTY_NONE, buffer, len);
    }

    return &event;
}

static pevent_t *parse_header(FILE *fp)
{
    int c;
    int len = 0;
    int user_header = 0;

    buffer[len++] = '<';

    while ((c = fgetc(fp)) != EOF)
    {
        if (c == '>')
        {
            if (len < PEVENT_DATA_SIZE - 1)
                buffer[len++] = (char)c;
            break;
        }

        if (c == '"')
            user_header = 1;

        if (len < PEVENT_DATA_SIZE - 1)
            buffer[len++] = (char)c;
    }

    set_event(
        PEVENT_HEADER_FILE,
        user_header ? PROPERTY_USER_HEADER : PROPERTY_STANDARD_HEADER,
        buffer,
        len
    );

    return &event;
}

pevent_t *get_parser_event(FILE *fp)
{
    int c;

    if (fp == NULL)
        return NULL;

    while ((c = fgetc(fp)) != EOF)
    {
        if (c == '#')
            return parse_preprocessor(fp, c);

        if (c == '/')
            return parse_comment(fp, c);

        if (c == '"')
            return parse_string(fp, c);

        if (c == '\'')
            return parse_char_literal(fp, c);

        if (c == '<')
        {
            /*
             * Treat '<...>' as a header only when it looks like
             * a header name. This keeps normal comparison operators
             * as regular text.
             */
            int next = fgetc(fp);

            if (next != EOF &&
                (isalpha((unsigned char)next) || next == '_' || next == '.'))
            {
                ungetc(next, fp);
                return parse_header(fp);
            }

            if (next != EOF)
                ungetc(next, fp);

            set_event(PEVENT_REGULAR_EXP, PROPERTY_NONE, "<", 1);
            return &event;
        }

        if (isdigit((unsigned char)c))
            return parse_number(fp, c);

        if (isalpha((unsigned char)c) || c == '_')
            return parse_word(fp, c);

        /*
         * For spaces, operators and other symbols, return the
         * character as regular source text.
         */
        buffer[0] = (char)c;
        buffer[1] = '\0';
        set_event(PEVENT_REGULAR_EXP, PROPERTY_NONE, buffer, 1);
        return &event;
    }

    state = PSTATE_IDLE;
    return make_eof();
}
