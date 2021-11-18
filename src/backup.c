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

void backup() {
    struct tm *tm;
    time_t t;
    char str_timestamp[100];

    char local_reporting_dir[100];
    strcpy(local_reporting_dir, reporting_dir);

    char local_backup_dir[100];
    strcpy(local_backup_dir, backup_dir);

    // get time and creating the directory file
    t = time(NULL);
    tm = localtime(&t);

    // format timestamp 
    strftime(str_timestamp, sizeof(str_timestamp), "%Y%m%d%H%M%S", tm);

    // add the timestamp to the directory
    strcat(local_backup_dir, str_timestamp);

    pid_t pid;

    // fork to use the cp command
    if((pid = fork()) == -1) {
        perror("Error forking");

        openlog("MANUFACTURING-DAEMON", LOG_PID | LOG_CONS, LOG_USER);
        syslog(LOG_INFO, "Error backing up");
        closelog();

        exit(EXIT_FAILURE);

    } else if(pid == 0) {
        char *cmd = "/bin/cp";
        char *args[] = {"cp", "-a", local_reporting_dir, local_backup_dir, NULL};
        execvp(cmd, args);

        // only runs if execvp fails
        perror("Error with backing up files");
        exit(EXIT_FAILURE);

    } else {
        int status;
        pid = wait(&status);

        if(WIFEXITED(status)) {
            // message queue
            mqd_t mq;
            char buffer[1024];
            mq = mq_open(QUEUE_NAME, O_WRONLY);
            mq_send(mq, "backup_success", 1024, 0);
            mq_close(mq);

            openlog("MANUFACTURING-DAEMON", LOG_PID | LOG_CONS, LOG_USER);
            syslog(LOG_INFO, "Backup successful");
            closelog();

        } else {
            mqd_t mq;
            char buffer[1024];
            mq = mq_open(QUEUE_NAME, O_WRONLY);
            mq_send(mq, "backup_failed", 1024, 0);
            mq_close(mq);

            openlog("baMANUFACTURING-DAEMON", LOG_PID | LOG_CONS, LOG_USER);
            syslog(LOG_INFO, "Backup failed");
            closelog();
        }
    }
}
