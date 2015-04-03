#ifndef __JOBS_H__
#define __JOBS_H__

#include <sys/types.h>
#include "linked_list.h"

typedef struct {
    char rawcmd[1024];   // what the user typed in

    char fg;         // 1 - fg, 0 - bg
    char **fin;      // stdin  redirects
    char **fout;     // stdout redirects
    pid_t pgid;
    int job_id;
    char running;  // 1 - running, 0 - stopped
    char complete;   // 1 - complete, 0 - not complete

    int numcmds;    // # of executable cmds in this job
    char*** cmds;   // list of argvs for each cmd

    LIST *pid_list;
} job;

typedef struct {
    char complete; // 1 - complete, 0 - not complete
    pid_t pid;
    pid_t pgid;

    int fin;
    int fout;
} process;

typedef struct {
    char msg[1024];
    char status;  // 0 - stopped, 1 - finished
} message;

#endif

