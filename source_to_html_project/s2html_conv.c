#include "s2html_conv.h"

#include <stdio.h>

static void html_escape(FILE *fp, const char *text)
{
    while (*text)
    {
        switch (*text)
        {
            case '&':
                fputs("&amp;", fp);
                break;
            case '<':
                fputs("&lt;", fp);
                break;
            case '>':
                fputs("&gt;", fp);
                break;
            case '"':
                fputs("&quot;", fp);
                break;
            default:
                fputc(*text, fp);
        }

        text++;
    }
}

static const char *class_for_event(const pevent_t *event)
{
    switch (event->type)
    {
        case PEVENT_PREPROCESSOR_DIRECTIVE:
            return "preprocess_dir";

        case PEVENT_SINGLE_LINE_COMMENT:
        case PEVENT_MULTI_LINE_COMMENT:
            return "comment";

        case PEVENT_STRING:
            return "string";

        case PEVENT_HEADER_FILE:
            return "header_file";

        case PEVENT_NUMERIC_CONSTANT:
            return "numeric_constant";

        case PEVENT_ASCII_CHAR:
            return "ascii_char";

        case PEVENT_RESERVE_KEYWORD:
            if (event->property == PROPERTY_DATA_TYPE_KEYWORD)
                return "reserved_key1";

            if (event->property == PROPERTY_NON_DATA_KEYWORD)
                return "reserved_key2";

            return "reserved_key3";

        default:
            return NULL;
    }
}

void html_begin(FILE *fp, int type)
{
    if (fp == NULL || type != HTML_OPEN)
        return;

    fprintf(fp,
        "<!DOCTYPE html>\n"
        "<html lang=\"en-US\">\n"
        "<head>\n"
        "    <title>Source to HTML</title>\n"
        "    <meta charset=\"UTF-8\">\n"
        "    <link rel=\"stylesheet\" href=\"styles.css\">\n"
        "</head>\n"
        "<body style=\"background-color:lightgrey;\">\n"
        "<pre>\n"
    );
}

void source_to_html(FILE *fp, const pevent_t *event)
{
    const char *class_name;

    if (fp == NULL || event == NULL)
        return;

    if (event->type == PEVENT_EOF)
        return;

    class_name = class_for_event(event);

    if (class_name != NULL)
    {
        fprintf(fp, "<span class=\"%s\">", class_name);
        html_escape(fp, event->data);
        fprintf(fp, "</span>");
    }
    else
    {
        html_escape(fp, event->data);
    }
}

void html_end(FILE *fp, int type)
{
    if (fp == NULL || type != HTML_CLOSE)
        return;

    fprintf(fp, "</pre>\n</body>\n</html>\n");
}
