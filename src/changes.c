#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <sys/stat.h>
#include <unistd.h>
#include "changes.h"
#include "locs.h"

int pid;
int pipefd1[2];
int pipefd2[2];

void get_changes();

void changes() {
    int file_desc;

    // dup() creates a copy of the file descriptor 
    // use lowest numbered unused file descriptor for new descriptor
    int stdout_copy = dup(STDOUT_FILENO);

    FILE *file = fopen(changes_dir, "a");

    // Examine the argument stream and returns integer file descriptor
    file_desc = fileno(file);

    // dup2() uses the file descriptor number specified in file_desc
    // If file_desc was previously open, silently close before use
    dup2(file_desc, STDOUT_FILENO);
    close(file_desc);

    // retrieve changes made to files in the backup directory
    get_changes();

    // clear stdoutput
    fflush(stdout);
    dup2(stdout_copy, STDOUT_FILENO);
    close(stdout_copy);
}

// Function to retrieve changes in files using the find command and awk 
// Redirects output to a stored file
void get_changes() {
    char data[4096];

    // pipe awk
    if(pipe(pipefd1) == -1) {
        perror("Error initialising pipe");
        exit(EXIT_FAILURE);
    }

    // get find command
    if((pid = fork()) == -1) {
        openlog("MANUFACTURING-DAEMON", LOG_PID | LOG_CONS, LOG_USER);
        syslog(LOG_INFO, "changes.c: Error forking");
        closelog();

        perror("Error forking");
        exit(EXIT_FAILURE);

    } else if(pid == 0) {
        dup2(pipefd1[1], 1);

        close(pipefd1[0]);
        close(pipefd1[1]);

        // execute find command, finding files modified in the last 24 hours
        execlp("find", "find", upload_dir, "-cmin", "-0.017", "-type", "f", "-ls", NULL);

        // if exec returns then log an error
        perror("Error with find command");
        exit(EXIT_FAILURE);
    }

    // pipe awk and sort
    if(pipe(pipefd2) == -1) {
        perror("Error initialising pipe");
        exit(EXIT_FAILURE);
    }

    // fork awk
    if((pid = fork()) == -1) {
        openlog("MANUFACTURING-DAEMON", LOG_PID | LOG_CONS, LOG_USER);
        syslog(LOG_INFO, "changes.c: Error forking");
        closelog();

        perror("Error forking");
        exit(EXIT_FAILURE);

    } else if(pid == 0) {
        // pipe 1
        dup2(pipefd1[0], 0);

        // pipe2
        dup2(pipefd2[1], 1);

        // close pipes
        close(pipefd1[0]);
        close(pipefd1[1]);
        close(pipefd2[0]);
        close(pipefd2[1]);

        // execute awk command
        execlp("awk", "awk", "{print $5,$8,$9,$10,$11}", NULL);

        // if exec command returns then log error
        perror("awk exec command failed");
        exit(EXIT_FAILURE);
    }

    // close fd's
    close(pipefd1[0]);
    close(pipefd1[1]);

    // fork sort
    if((pid = fork()) == -1) {
        openlog("MANUFACTURING-DAEMON", LOG_PID | LOG_CONS, LOG_USER);
        syslog(LOG_INFO, "changes.c: Error forking for sort");
        closelog();

        perror("Error forking for sort");
        exit(EXIT_FAILURE);
        
    } else if(pid == 0) {
        // pipe 2
        dup2(pipefd2[0], 0);

        close(pipefd2[0]);
        close(pipefd2[1]);

        execlp("sort", "sort", "-u", NULL);

        // if exec command fails, log error
        perror("Error with sort command");
        exit(EXIT_FAILURE);

    } else {
        close(pipefd2[1]);
        int nbytes = read(pipefd2[0], data, sizeof(data));
        printf("%.*s", nbytes, data);
        close(pipefd2[0]);
    }
}
