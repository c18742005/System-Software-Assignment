#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include "backup.h"
#include "changes.h"
#include "locs.h"
#include "queue.h"
#include "transfers.h"

void signal_handler(int sig_no);
void lock_dir();
void unlock_dir();

int main(int argc, const char* argv[]) {
    // Step 1: Create orphan process
    int pid = fork();

    if(pid > 0) {
        //parent process
        exit(EXIT_SUCCESS);

    } else if(pid == 0) {
        // Child process
        openlog("MANUFACTURING-DAEMON", LOG_PID | LOG_CONS, LOG_USER);
        syslog(LOG_INFO, "Started tracking file changes");
        closelog();

        // Step 2: Elevate orphan process to session leader
        if(setsid() < 0) {
            exit(EXIT_FAILURE);
        };

        // Step 3: Set file mode creation mask to 0
        umask(0);

        // Step 4: Change current working directory to root
        if(chdir("/") < 0) {
            exit(EXIT_FAILURE);
        };

        // Step 5: Close all open file descriptors
        int x;
        for(x = sysconf(_SC_OPEN_MAX); x >= 0; x--) {
            close(x);
        }

        // implement message queue
        queue();

        // set up time to transfer files
        time_t now;
        struct tm transfer_time;
        double seconds;
        time(&now);
		
		transfer_time = *localtime(&now);
		
        transfer_time.tm_hour = 1;
        transfer_time.tm_min = 0;
        transfer_time.tm_sec = 0;
		

        // add signal handlers
        if(signal(SIGINT, signal_handler) == SIG_ERR) {
            openlog("MANUFACTURING-DAEMON", LOG_PID | LOG_CONS, LOG_USER);
            syslog(LOG_INFO, "SIGINT error");
            closelog();
        }

        // add sigusr1 handler
        if(signal(SIGUSR1, signal_handler) == SIG_ERR) {
            openlog("MANUFACTURING-DAEMON", LOG_PID | LOG_CONS, LOG_USER);
            syslog(LOG_INFO, "SIGUSR1 catch error");
            closelog();
        }

        while(1) {
            time(&now);
            seconds = difftime(now, mktime(&transfer_time));

            if(seconds == 0) {
                // begin transfer
                lock_dir();
                backup();
                transfer();
                unlock_dir();

            } else {
                changes();
            }
        
            sleep(1);
        }
    }

    return 0;
}

// Function to handle signal interrupts
void signal_handler(int sig_no) {
    if(sig_no == SIGINT) {
        openlog("MANUFACTURING-DAEMON", LOG_PID | LOG_CONS, LOG_USER);
        syslog(LOG_INFO, "SIGINT interrupt recieved");
        closelog();

    } else if (sig_no == SIGUSR1) {
        openlog("MANUFACTURING-DAEMON", LOG_PID | LOG_CONS, LOG_USER);
        syslog(LOG_INFO, "SIGUSR1 interrupt recieved");
        closelog();

        //backup and transfer
        lock_dir();
        backup();
        transfer();
        unlock_dir();
    }
}

// function that locks a directory
void lock_dir() {
    // Can read but cannot write
    char mode[4] = "0555";
    int i = strtol(mode, 0, 8);

    if(chmod(upload_dir, i) == 0){
        openlog("MANUFACTURING-DAEMON", LOG_PID | LOG_CONS, LOG_USER);
        syslog(LOG_INFO, "Directory locked");
        closelog();

    } else {
        openlog("MANUFACTURING-DAEMON", LOG_PID | LOG_CONS, LOG_USER);
        syslog(LOG_INFO, "Error locking directory");
        closelog();
    }
}

// function that unlocks directoy after update and backup
void unlock_dir() {
    // Allow read, write and execute
    char mode[4] = "0777";
    int i = strtol(mode, 0, 8);

    if(chmod(upload_dir, i) == 0) {
        openlog("MANUFACTURING-DAEMON", LOG_PID | LOG_CONS, LOG_USER);
        syslog(LOG_INFO, "Directory unlocked");
        closelog();

    } else {
        openlog("MANUFACTURING-DAEMON", LOG_PID | LOG_CONS, LOG_USER);
        syslog(LOG_INFO, "Error unlocking directory");
        closelog();
    }
}
