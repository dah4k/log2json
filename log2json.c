// Copyright 2025 dah4k
// SPDX-License-Identifier: EPL-2.0

#include <stdio.h>
#include <stdlib.h>

typedef struct {
    const char *key;
    const char *value;
} KeyValue_t;

void usage(const char* progname)
{
    fprintf(stderr,
"Usage: %s INPUT_LOG_FILE\n"
"\n"
"Parse input log file and print the JSON-ified entries to STDOUT.\n"
"The expected grammar of the input is:\n"
"\n"
"    LOG_ENTRY ::= KEY '=' VALUE [ SPACE KEY '=' VALUE ]* NEWLINE\n"
"    KEY ::= TEXT\n"
"    VALUE ::= TEXT | '\"' QUOTED_TEXT '\"'\n"
"    SPACE ::= 0x20\n"
"    NEWLINE ::= 0x0A\n"
"    TEXT ::= WORD_CHARACTER+\n"
"    QUOTED_TEXT ::= [ WORD_CHARACTER | WHITESPACE | '\\\\' | '\\\"' ]+\n"
"\n"
"Known Issues:\n"
"- Not implemented yet.\n"
"- Quoted text value containing invalid UTF-8 code points.\n",
    progname);
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        usage(argv[0]);
        exit(EXIT_FAILURE);
    }

    KeyValue_t kv;
    kv.key = "TODO";
    kv.value = "Implement log2json";
    printf("{\"%s\": \"%s\"}\n", kv.key, kv.value);

    return EXIT_SUCCESS;
}
