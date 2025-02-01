// Copyright 2025 dah4k
// SPDX-License-Identifier: EPL-2.0

#include <stdio.h>
#include <stdlib.h>

typedef struct {
    const char *key;
    const char *value;
} KeyValue_t;

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    KeyValue_t kv;
    kv.key = "TODO";
    kv.value = "Implement log2json";
    printf("{\"%s\": \"%s\"}\n", kv.key, kv.value);

    return EXIT_SUCCESS;
}
