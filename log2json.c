// Copyright 2025 dah4k
// SPDX-License-Identifier: EPL-2.0

#define _GNU_SOURCE     /* needed for Linux madvise() */

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

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

typedef struct ReadBuffer {
    void *data;
    size_t data_len;
} ReadBuffer_t;

void ReadBuffer_init(ReadBuffer_t *self, const char *pathname)
{
    int fd = open(pathname, O_RDONLY);
    if (-1 == fd) {
        fprintf(stderr, "cannot open '%s': %s\n", pathname, strerror(errno));
        exit(EXIT_FAILURE);
    }

    struct stat sb;
    int rc = fstat(fd, &sb);
    if (-1 == rc) {
        fprintf(stderr, "cannot fstat '%s': %s\n", pathname, strerror(errno));
        exit(EXIT_FAILURE);
    }

    const size_t data_len = sb.st_size;
    const int prot = PROT_READ;
    const int flags = MAP_FILE | MAP_PRIVATE | MAP_POPULATE;
    void *data = mmap(NULL, data_len, prot, flags, fd, 0);
    if (MAP_FAILED == data) {
        fprintf(stderr, "cannot mmap: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    close(fd);  /* no longer needs the file descriptor after mmap */

    rc = madvise(data, data_len, MADV_SEQUENTIAL|MADV_WILLNEED);
    if (-1 == rc) {
        fprintf(stderr, "cannot madvise '%p': %s\n", data, strerror(errno));
        exit(EXIT_FAILURE);
    }

    self->data = data;
    self->data_len = data_len;
}

void ReadBuffer_deinit(ReadBuffer_t *self)
{
    munmap(self->data, self->data_len);
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        usage(argv[0]);
        exit(EXIT_FAILURE);
    }
    const char * pathname = argv[1];

    ReadBuffer_t memfile;
    ReadBuffer_init(&memfile, pathname);
    fprintf(stderr, "[DEBUG] %s is %lu bytes\n", pathname, memfile.data_len);
    ReadBuffer_deinit(&memfile);

    return EXIT_SUCCESS;
}
