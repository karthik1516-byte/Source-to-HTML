#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "s2html_event.h"
#include "s2html_conv.h"

static void make_output_name(const char *source, char *output, size_t size)
{
    const char *dot;
    size_t base_len;

    dot = strrchr(source, '.');

    if (dot != NULL)
        base_len = (size_t)(dot - source);
    else
        base_len = strlen(source);

    if (base_len + 5 >= size)
    {
        fprintf(stderr, "Output filename is too long\n");
        exit(EXIT_FAILURE);
    }

    memcpy(output, source, base_len);
    output[base_len] = '\0';
    strcat(output, ".html");
}

int main(int argc, char *argv[])
{
    FILE *sfp;
    FILE *dfp;
    char output_name[1024];
    pevent_t *event;

    if (argc != 2)
    {
        printf("Insufficient arguments\n");
        printf("Usage: %s filename.c\n", argv[0]);
        return EXIT_FAILURE;
    }

    sfp = fopen(argv[1], "r");
    if (sfp == NULL)
    {
        perror("Unable to open source file");
        return EXIT_FAILURE;
    }

    make_output_name(argv[1], output_name, sizeof(output_name));

    dfp = fopen(output_name, "w");
    if (dfp == NULL)
    {
        perror("Unable to create HTML file");
        fclose(sfp);
        return EXIT_FAILURE;
    }

    html_begin(dfp, HTML_OPEN);

    do
    {
        event = get_parser_event(sfp);

        if (event == NULL)
        {
            fprintf(stderr, "Parser error\n");
            fclose(sfp);
            fclose(dfp);
            return EXIT_FAILURE;
        }

        source_to_html(dfp, event);

    } while (event->type != PEVENT_EOF);

    html_end(dfp, HTML_CLOSE);

    fclose(sfp);
    fclose(dfp);

    printf("Generated: %s\n", output_name);

    return EXIT_SUCCESS;
}
