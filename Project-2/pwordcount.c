#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define READ_END  0
#define WRITE_END 1

#define CHUNK_SIZE 4096

static void print_usage(void) {
    printf("Please enter a file name.\n");
    printf("Usage: ./pwordcount <file_name>\n");
}

/*
 * Count words in a buffer of bytes.
 * A "word" is any maimum run of non-whitespace characters.
 * This idea will count numbers as words too (e.g., "6", "gr8").
 */
static int count_words_in_bytes(const unsigned char *buf, ssize_t n, int *in_word) {
    int count = 0;

    for (ssize_t i = 0; i < n; i++) {
        if (isspace(buf[i])) {
            *in_word = 0;
        } else {
            if (*in_word == 0) {
                count++;
                *in_word = 1;
            }
        }
    }
    return count;
}

/* Robust write: write all bytes unless an error occurs. Returns 0 on success, -1 on error. */
static int write_all(int fd, const void *data, size_t len) {
    const unsigned char *p = (const unsigned char *)data;
    size_t total = 0;

    while (total < len) {
        ssize_t w = write(fd, p + total, len - total);
        if (w < 0) {
            if (errno == EINTR) continue;
            return -1;
        }
        total += (size_t)w;
    }
    return 0;
}

/* Robust read: retry on EINTR. */
static ssize_t read_retry(int fd, void *buf, size_t len) {
    for (;;) {
        ssize_t r = read(fd, buf, len);
        if (r < 0 && errno == EINTR) continue;
        return r;
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        print_usage();
        return 0;
    }

    const char *filename = argv[1];

    int pipe1[2]; // Parent -> Child (file bytes)
    int pipe2[2]; // Child  -> Parent (wordcount int)

    if (pipe(pipe1) == -1) {
        perror("pipe");
        return 1;
    }
    if (pipe(pipe2) == -1) {
        perror("pipe");
        close(pipe1[0]); close(pipe1[1]);
        return 1;
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        close(pipe1[0]); close(pipe1[1]);
        close(pipe2[0]); close(pipe2[1]);
        return 1;
    }

    if (pid == 0) {
        /* ---------------- Child (Process 2) ---------------- */
        // Close unused ends
        close(pipe1[WRITE_END]); // child reads from pipe1
        close(pipe2[READ_END]);  // child writes to pipe2

        printf("Process 2 finishes receiving data from Process 1 ...\n");

        unsigned char buf[CHUNK_SIZE];
        int total_words = 0;
        int in_word = 0;

        // Read until EOF (parent closes write end)
        for (;;) {
            ssize_t r = read_retry(pipe1[READ_END], buf, sizeof(buf));
            if (r < 0) {
                perror("read");
                close(pipe1[READ_END]);
                close(pipe2[WRITE_END]);
                _exit(1);
            }
            if (r == 0) break; // EOF
            total_words += count_words_in_bytes(buf, r, &in_word);
        }

        close(pipe1[READ_END]);

        printf("Process 2 is counting words now ...\n");
        printf("Process 2 is sending the result back to Process 1 ...\n");

        // Send integer result back to parent
        if (write_all(pipe2[WRITE_END], &total_words, sizeof(total_words)) != 0) {
            perror("write");
            close(pipe2[WRITE_END]);
            _exit(1);
        }

        close(pipe2[WRITE_END]);
        _exit(0);
    } else {
        /* ---------------- Parent (Process 1) ---------------- */
        // Close unused ends
        close(pipe1[READ_END]);  // parent writes to pipe1
        close(pipe2[WRITE_END]); // parent reads from pipe2

        printf("Process 1 is reading file \"%s\" now ...\n", filename);

        FILE *fp = fopen(filename, "rb");
        if (!fp) {
            perror("fopen");
            // Important: close pipe write end so child doesn't hang
            close(pipe1[WRITE_END]);
            close(pipe2[READ_END]);
            waitpid(pid, NULL, 0);
            return 1;
        }

        printf("Process 1 starts sending data to Process 2 ...\n");

        unsigned char buf[CHUNK_SIZE];
        for (;;) {
            size_t n = fread(buf, 1, sizeof(buf), fp);
            if (n > 0) {
                if (write_all(pipe1[WRITE_END], buf, n) != 0) {
                    perror("write");
                    fclose(fp);
                    close(pipe1[WRITE_END]);
                    close(pipe2[READ_END]);
                    waitpid(pid, NULL, 0);
                    return 1;
                }
            }

            if (n < sizeof(buf)) {
                if (ferror(fp)) {
                    perror("fread");
                    fclose(fp);
                    close(pipe1[WRITE_END]);
                    close(pipe2[READ_END]);
                    waitpid(pid, NULL, 0);
                    return 1;
                }
                // EOF
                break;
            }
        }

        fclose(fp);

        // Close write-end to signal EOF to child
        close(pipe1[WRITE_END]);

        // Receive integer count
        int total_words = 0;
        ssize_t r = read_retry(pipe2[READ_END], &total_words, sizeof(total_words));
        if (r < 0) {
            perror("read");
            close(pipe2[READ_END]);
            waitpid(pid, NULL, 0);
            return 1;
        }
        if (r != (ssize_t)sizeof(total_words)) {
            fprintf(stderr, "Error: unexpected read size from child.\n");
            close(pipe2[READ_END]);
            waitpid(pid, NULL, 0);
            return 1;
        }

        close(pipe2[READ_END]);

        printf("Process 1: The total number of words is %d.\n", total_words);

        // Ensure child is fully done
        waitpid(pid, NULL, 0);
        return 0;
    }
}