/*
 * SPDX-FileCopyrightText: 2022 Jarosław Wierzbicki
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>

#include "stacktrace.h"

#define RD_PIPE 0
#define WR_PIPE 1
#define CHILD_READY 0xc0

sig_atomic_t signalled = 0;

void signal_handler(int signal)
{
    // printf("signal: %d\n", signal);
    dump_stack();

    signalled = 1;
}

static void foo3_with_very_long_name_123456789(void)
{
    dump_stack();
}

static void foo2(void)
{
    foo3_with_very_long_name_123456789();
}

static void foo1(void)
{
    foo2();
}

int main(void)
{
    int pipefd[2];

    pipe(pipefd);
    pid_t pid = fork();
    if (pid == -1) {
        fprintf(stderr, "fail to spawn child process (err=%s)\n",
                strerror(errno));
        return EXIT_FAILURE;
    } else if (pid == 0) {
        // child process
        close(pipefd[RD_PIPE]);

        struct sigaction sa;
        sa.sa_handler = signal_handler;

        int ret = sigaction(SIGUSR1, &sa, NULL);
        if (ret == -1) {
            fprintf(stderr, "failed to install signal handler (err=%s)\n",
                    strerror(errno));
        }

        char msg = CHILD_READY;
        write(pipefd[WR_PIPE], &msg, sizeof(msg));
        close(pipefd[WR_PIPE]);

        printf("stack trace from signal handler\n");
        while (!signalled) {
            sleep(1);
        }
    } else {
        // parent process
        close(pipefd[WR_PIPE]);

        char msg;
        while(read(pipefd[RD_PIPE], &msg, sizeof(msg)) > 0);
        close(pipefd[RD_PIPE]);

        if (kill(pid, SIGUSR1) == -1) {
            fprintf(stderr, "failed to send signal to process %d (err=%s)\n",
                    pid, strerror(errno));
            return EXIT_FAILURE;
        }

        int status;
        pid = wait(&status);
        if (status != EXIT_SUCCESS) {
            fprintf(stderr, "child process %d returned with status %d\n",
                    pid, status);
            return EXIT_FAILURE;
        }

        // now call the test function
        printf("normal stack trace\n");
        foo1();
    }

    return EXIT_SUCCESS;
}
