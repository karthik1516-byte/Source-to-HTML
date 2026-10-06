#ifndef S2HTML_CONV_H
#define S2HTML_CONV_H

#include <stdio.h>
#include "s2html_event.h"

#define HTML_OPEN  1
#define HTML_CLOSE 2

void html_begin(FILE *fp, int type);
void source_to_html(FILE *fp, const pevent_t *event);
void html_end(FILE *fp, int type);

#endif
