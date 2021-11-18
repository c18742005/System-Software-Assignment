#include <mqueue.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include "locs.h"

#define QUEUE_NAME "/queue"

void get_transfers();

void transfer() {
    // get files to be transferred
    get_transfers();

    // stop process shortly to run this function
    sleep(5);

    char local_transfers_dir[100];
    strcpy(local_transfers_dir, transfers_dir);

    char local_reporting_dir[100];
    strcpy(local_reporting_dir, reporting_dir);

    FILE *fp;

    int i = 0;
    int num_lines = 0;

    fp = fopen(local_transfers_dir, "r");
    
    if(fp == NULL) {
        openlog("MANUFACTURING-DAEMON", LOG_PID | LOG_CONS, LOG_USER);
        syslog(LOG_INFO, "Could not open file for reading");
        closelog();
        exit(EXIT_FAILURE);
    }

    char records[10][100];

    while(fscanf(fp, "%s", records[i]) != EOF) {
        i++;
    }

    fclose(fp);

    num_lines = i;

    for(int i = 0; i < num_lines; i++) {
        pid_t pid = fork();

        if(pid == 0) {
            char *command = "/bin/cp";
            char *arguments[] = {"cp", "-f", records[i], local_reporting_dir, NULL};
            execvp(command, arguments);

        } else if(pid == -1) {
            openlog("MANUFACTURING-DAEMON", LOG_PID | LOG_CONS, LOG_USER);
            syslog(LOG_INFO, "transfers.c: Error forking");
            closelog();
        }
    }
}

// function to get changes to files
void get_transfers() {
    int pid;
    int pipefd[2];
    int file_desc;
    char data[4096];

    FILE *file = fopen(transfers_dir, "w");

    file_desc = fileno(file);

    dup2(file_desc, STDOUT_FILENO);
    close(file_desc);

    // used for IPC
    pipe(pipefd);

    if((pid = fork()) == -1) {
        perror("Error forking");
        exit(EXIT_FAILURE);

    } else if(pid == 0) {
        close(pipefd[0]);
        close(pipefd[1]);
        
        execlp("find", "find", upload_dir, "-mtime", "-1", "-type", "f", NULL);

        perror("Error with exec find function");
        exit(EXIT_FAILURE);

    }
    else {
        int status;
        pid = wait(&status);

        close(pipefd[1]);

        int nbytes = read(pipefd[0], data, sizeof(data));
        printf("%.*s", nbytes, data);
        close(pipefd[0]);

        if(WIFEXITED(status)) {
            //get message queue and send log
            mqd_t mq;
            char buffer[1024];
            mq = mq_open(QUEUE_NAME, O_WRONLY);
            mq_send(mq, "Transfer_successful", 1024, 0);
            mq_close(mq);

            openlog("MANUFACTURING-DAEMON", LOG_PID | LOG_CONS, LOG_USER);
            syslog(LOG_INFO, "Transfer was successful");
            closelog();
        } else {
            // get message queue and send log
            mqd_t mq;
            char buffer[1024];
            mq = mq_open(QUEUE_NAME, O_WRONLY);
            mq_send(mq, "Transfer_failed", 1024, 0);
            mq_close(mq);

            openlog("MANUFACTURING-DAEMON", LOG_PID | LOG_CONS, LOG_USER);
            syslog(LOG_INFO, "Transfer failed");
            closelog();
        }
    }
}
