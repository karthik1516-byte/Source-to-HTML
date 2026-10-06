# Source to HTML Converter

A multi-file C project that reads a C source file character-by-character,
classifies source constructs, converts parser events into HTML `<span>`
elements, and uses CSS classes for syntax-style highlighting.

## Project flow

C source -> parser -> pevent_t -> HTML converter -> CSS -> highlighted HTML

## Files

- `s2html_main.c` - main application and file handling
- `s2html_event.c` - lexical parser and event generation
- `s2html_event.h` - event types and `pevent_t`
- `s2html_conv.c` - HTML generation and event-to-HTML mapping
- `s2html_conv.h` - HTML constants and declarations
- `styles.css` - syntax highlighting styles
- `sample.c` - sample input

## Compile

```bash
gcc -Wall -Wextra -std=c11 s2html_main.c s2html_event.c s2html_conv.c -o s2html
```

## Run

```bash
./s2html sample.c
```

This creates:

```text
sample.html
```

Open `sample.html` in a browser. Keep `styles.css` in the same directory
as the generated HTML file.

## Important learning point

The parser and HTML conversion layers are separated. The parser creates a
`pevent_t`; the converter decides how that event is represented in HTML.
