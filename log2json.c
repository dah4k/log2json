// Copyright 2025 dah4k
// SPDX-License-Identifier: EPL-2.0

#define _GNU_SOURCE     /* needed for Linux madvise() */

#include <errno.h>
#include <fcntl.h>
#include <libgen.h>     /* POSIX basename() and dirname() */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

void usage(const char* progname)
{
    fprintf(stderr,
"Usage: %s INPUT_LOG_FILE OUTPUT_JSON_FILE\n"
"\n"
"Parse input log file and print the JSON-ified entries to STDOUT.\n"
"The grammar of the expected input is:\n"
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

///////////////////////////////////////////////////////////////////////////
//  InputFile
///////////////////////////////////////////////////////////////////////////
typedef struct InputFile {
    void *data;
    size_t data_len;
} InputFile_t;

void InputFile_init(InputFile_t *self, const char *pathname)
{
    int fd = open(pathname, O_RDONLY);
    if (-1 == fd) {
        fprintf(stderr, "fatal open '%s': %s\n", pathname, strerror(errno));
        exit(EXIT_FAILURE);
    }

    struct stat sb;
    int rc = fstat(fd, &sb);
    if (-1 == rc) {
        fprintf(stderr, "fatal fstat '%s': %s\n", pathname, strerror(errno));
        exit(EXIT_FAILURE);
    }

    const size_t DataLen = sb.st_size;
    const int Prot = PROT_READ;
    const int Flags = MAP_FILE | MAP_PRIVATE | MAP_POPULATE;
    void *data = mmap(NULL, DataLen, Prot, Flags, fd, 0);
    if (MAP_FAILED == data) {
        fprintf(stderr, "fatal mmap: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    close(fd);  /* no longer needs the file descriptor after mmap */

    const int Advice = MADV_SEQUENTIAL | MADV_WILLNEED;
    rc = madvise(data, DataLen, Advice);
    if (-1 == rc) {
        fprintf(stderr, "fatal madvise '%p': %s\n", data, strerror(errno));
        exit(EXIT_FAILURE);
    }

    self->data = data;
    self->data_len = DataLen;
}

void InputFile_deinit(InputFile_t *self)
{
    munmap(self->data, self->data_len);
}

///////////////////////////////////////////////////////////////////////////
//  OutputFile
///////////////////////////////////////////////////////////////////////////
typedef struct OutputFile {
    FILE *ostream;
    char *in_progress_pathname;
    char *done_pathname;
} OutputFile_t;

void OutputFile_init(OutputFile_t *self,  const char *pathname)
{
    char *done_pathname = strdup(pathname);
    if (NULL == done_pathname) {
        fprintf(stderr, "fatal strdup '%s': %s\n",
                pathname, strerror(errno));
        exit(EXIT_FAILURE);
    }

    char *in_progress_pathname = malloc(
            strlen(".") + strlen("/.") + strlen(pathname));
    if (NULL == in_progress_pathname) {
        fprintf(stderr, "fatal malloc: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    in_progress_pathname[0] = '\0';

    char *tmp = strdup(pathname);
    if (NULL == tmp) {
        fprintf(stderr, "fatal strdup '%s': %s\n",
                pathname, strerror(errno));
        exit(EXIT_FAILURE);
    }
    strcat(in_progress_pathname, dirname(tmp));
    strcat(in_progress_pathname, "/.");
    free(tmp);

    tmp = strdup(pathname);
    if (NULL == tmp) {
        fprintf(stderr, "fatal strdup '%s': %s\n",
                pathname, strerror(errno));
        exit(EXIT_FAILURE);
    }
    strcat(in_progress_pathname, basename(tmp));
    free(tmp);

    FILE *ostream = NULL;
    {
        ostream = fopen(in_progress_pathname, "w");
        if (NULL == ostream) {
            fprintf(stderr, "fatal fopen '%s': %s\n",
                    in_progress_pathname, strerror(errno));
            exit(EXIT_FAILURE);
        }
    }

    self->ostream = ostream;
    self->in_progress_pathname = in_progress_pathname;
    self->done_pathname = done_pathname;
}

void OutputFile_deinit(OutputFile_t *self)
{
    int rc = fclose(self->ostream);
    if (EOF == rc) {
        fprintf(stderr, "warning fclose '%s': %s\n",
                self->in_progress_pathname, strerror(errno));
    }

    // NOTE: Use link(2) to avoid overwriting existing file
    rc = link(self->in_progress_pathname, self->done_pathname);
    if (-1 == rc) {
        fprintf(stderr, "warning link-rename '%s' to '%s': %s\n",
                self->in_progress_pathname,
                self->done_pathname, strerror(errno));
    } else {
        // Rename was successful, in_progress_pathname may be deleted.
        rc = unlink(self->in_progress_pathname);
        if (-1 == rc) {
            fprintf(stderr, "warning unlink-delete '%s': %s\n",
                    self->in_progress_pathname, strerror(errno));
        }
    }

    free(self->in_progress_pathname);
    free(self->done_pathname);
}

size_t OutputFile_write(OutputFile_t *self, const uint8_t* data, size_t data_len)
{
    return fwrite(data, sizeof(uint8_t), data_len, self->ostream);
}

///////////////////////////////////////////////////////////////////////////
//  Processor
///////////////////////////////////////////////////////////////////////////

static void testCopyFile(OutputFile_t *destination, const InputFile_t *source)
{
    size_t written = OutputFile_write(destination, source->data, source->data_len);
    fprintf(stderr, "[DEBUG] copied %lu bytes\n", written);
}


///////////////////////////////////////////////////////////////////////////
//  Main
///////////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[])
{
    if (argc != 3) {
        usage(argv[0]);
        exit(EXIT_FAILURE);
    }
    const char *InputPathName = argv[1];
    const char *OutputPathName= argv[2];

    InputFile_t memfile;
    InputFile_init(&memfile, InputPathName);

    OutputFile_t jsonfile;
    OutputFile_init(&jsonfile, OutputPathName);

    testCopyFile(&jsonfile, &memfile);

    OutputFile_deinit(&jsonfile);
    InputFile_deinit(&memfile);
    return EXIT_SUCCESS;
}
