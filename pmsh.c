/*************************************************************************
* PMShell
*
* Copyright (c) 2012, Preetam D'Souza & Max Guo
* 
* Originally published 9.22.2012
*************************************************************************/

#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "tokenizer.h"
#include "parser.h"
#include "jobs.h"
#include "job_handler.h"
#include "linked_list.h"
#include "history.h"

#define SHELL "(pmsh)"
#define PROMPT "$"
#define SUCC_MSG  "Process completed on time.\n"
#define OFLOW_MSG "Input too long, try again.\n"
#define UNCMD_MSG "Command not found.\n"
#define EXIT_MSG  "quit\n"

#define BUFFER_SIZE 1 << 10
#define STRING_SIZE(s) (sizeof(s)/sizeof(char))

pid_t shell_pgid;
pid_t pid;
LIST *job_list;
LIST *msg_q;

static char pwd[BUFFER_SIZE];

void handler(int signum, siginfo_t *info, void *context);
void set_handler();
void exec_process(char *cmd, char **args, pid_t pgid, int fin, int fout, char fg);
void pflush();
void print_jobs();
void jwait(job *j);
void do_fg(char **argv);
void do_bg(char **argv);
static void do_cd(char **argv);
static void sync_pwd();
void cleanup();

int main(int argc, char** argv) {
    ssize_t cmd_size;
    int i;
    int pipefd[2], fin, fout;
    char buf[BUFFER_SIZE], hist[BUFFER_SIZE];
    char *cmd;
    char **args;
    job *j;
    job_list = init_list();
    msg_q = init_list();
    pid_t pgid;

    /* initial setup */
    shell_pgid = getpgid(0);
    sync_pwd();

    // TODO: investigate termios.h for handing arrow keys

    /* disable (ignore) job control signals */
    signal(SIGTTOU, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
    signal(SIGTERM, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    signal(SIGINT,  SIG_IGN);

    set_handler();

    /*** Shell loop ***/
    for(;;) {

        /* Print out queued messages */
        for(i=0;i<msg_q->size;i++) {
            message *m = (message *) get(msg_q, i);
            printf("%s: %s\n", m->status ? "Finished" : "Stopped", m->msg);
        }

        // clear the queue
        while(msg_q->size > 0)
            free(del(msg_q,0));

        /* TODO: shell print macro */
        printf("%s %s %s ", SHELL, pwd, PROMPT);
        fflush(stdout);

        do 
            cmd_size = read(STDIN_FILENO, buf, BUFFER_SIZE);
        while(cmd_size == -1 && errno == EINTR); // ignore system call interrupts

        if(cmd_size == -1) {
            perror("read");
            continue;
        } else if(cmd_size == 0) { // EOF (quit)
            write(STDOUT_FILENO, EXIT_MSG, STRING_SIZE(EXIT_MSG));
            cleanup();
            _exit(EXIT_SUCCESS);
        } else if(cmd_size == 1 && buf[0] == '\n') {
            continue;
        }

        if(buf[cmd_size-1] != '\n') { // overflow 
            write(STDOUT_FILENO, OFLOW_MSG, STRING_SIZE(OFLOW_MSG));
            pflush();
            continue;
        }

        buf[cmd_size-1] = '\0'; // strip the newline

        add_history(buf);

        j = parse(buf);
        if(j == (job *) NULL) {
            printf("Invalid redirections you moron!\n");
            continue;
        } 

        args = j->cmds[0];
        if (!my_strcmp(args[0], "fg")) {
            do_fg(args);
            free_job(j);
            continue;
        } else if (!my_strcmp(args[0], "bg")) {
            do_bg(args);
            free_job(j);
            continue;
        } else if (!my_strcmp(args[0], "jobs")) {
            print_bg(job_list);
            free_job(j);
            continue;
        } else if (!my_strcmp(args[0], "cd")) {
            do_cd(args);
            free_job(j);
            continue;
        } else if (!my_strcmp(args[0], "history")) {
            if (get_history(hist, BUFFER_SIZE, 1) == NULL) {
                printf("***** LAST ITEM IN HISTORY = NULL\n");
            } else {
                printf("***** LAST ITEM IN HISTORY = %s\n", hist);
            }
            free_job(j);
            continue;
        }

        j->job_id = gen_job_id(job_list);
        j->running = 1;
        j->complete = 0;
        push(job_list, JOB, (void *)j);

        pgid = 0; // set the job pgid to be the first child's pid

        fin = STDIN_FILENO;
        for(i=0;i<(j->numcmds);i++) {
            args = j->cmds[i];
            cmd = args[0];

            if(i + 1 < (j->numcmds)) { // not last process in job
                //printf("Creating a pipe!\n");
                if(pipe(pipefd) == -1) {
                    perror("pipe");
                    exit(EXIT_FAILURE);
                }
                fout = pipefd[1];
            }
            else
                fout = STDOUT_FILENO;

            /*printf("Forking %s\n", cmd);
            k = 0;
            while( args[k] != (char *) NULL ) {
                printf("\targv[%d] = %s\n", k, args[k]);
                ++k;
            }*/

            pid = fork();
            if(pid == -1) {
                perror("fork");
                continue;
            }

            /*** CHILD ***/
            if(!pid) {

                // <
                if(j->fin[i] != (char *) NULL) {
                    if( (fin = open(j->fin[i], O_RDONLY)) == -1 ) {
                        perror("open");
                    }
                }
                // >
                if(j->fout[i] != (char *) NULL) {
                    if( (fout = open(j->fout[i], O_WRONLY | O_CREAT | O_TRUNC, 0644)) == -1 ) {
                        perror("open");
                    }
                }

                exec_process(cmd, args, pgid, fin, fout, j->fg); 
            }
            
            /*** PARENT ***/
            else {
                if(!pgid) { // set to first child's pid (process group leader)
                    pgid = pid;
                    j->pgid = pgid;
                    //printf("Set job's pgid to %d\n", pgid);
                }

                process *proc = malloc(sizeof(process));
                proc->complete = 0;
                proc->pid = pid;
                proc->pgid = pgid;
                push(j->pid_list, PROCESS, (void *)proc);
                
                if( setpgid(pid, pgid) == -1 ) {
                    perror("setpgid");
                    _exit(EXIT_FAILURE);
                }

            }

            if(fin != STDIN_FILENO)
                close(fin);
            if(fout != STDOUT_FILENO)
                close(fout);

            fin = pipefd[0];
        }

        //print_jobs();

        if(j->fg) {  // foreground

            // give terminal control to job
            if( tcsetpgrp(STDIN_FILENO, pgid) == -1 ) {
                perror("tcsetpgrp");
                _exit(EXIT_FAILURE);
            }

            // wait for job to finish
            jwait(j);

            // give pmsh terminal control again
            if( tcsetpgrp(STDIN_FILENO, shell_pgid) == -1 ) {
                perror("tcsetpgrp");
                _exit(EXIT_FAILURE);
            }
        }
        else  {      // background
            printf("Running: %s\n", j->rawcmd);
        }
    }
}

/** SIGCHLD handler **/
void handler(int signum, siginfo_t *info, void *context) {
    int cpid, status;

    cpid = info->si_pid; // sending pid
    //printf("Got a SIGCHLD from %d!\n", cpid);

    // loop to catch simultaneous SIGCHLDs
    while( (cpid = waitpid(-1, &status, WNOHANG | WUNTRACED | WCONTINUED)) > 0 ) {

        //printf("HANDLER Got %d from waitpid\n", cpid);
        job *j = get_job(job_list, cpid);
        if(j == NULL) {
            printf("Error: get_job returned null\n");
            _exit(EXIT_FAILURE);
        }

        if(WIFEXITED(status)) { // completed
            if( set_complete(j, cpid) == -1 ) {
                printf("HANDLER set_complete went awry!\n");
                _exit(EXIT_FAILURE);
            }
        }
        else if(WIFSIGNALED(status)) { // terminated
            set_complete(j, cpid);
            printf("\n");
            //printf("TERMINATED: %d\n", WTERMSIG(status));
        }
        else if(WIFSTOPPED(status)) { // stopped
            j->running = 0;

            if(j->fg) {
                send_back(job_list, j->pgid);
                printf("\nStopped: %s\n", j->rawcmd);
            } else {

                // queue stopped message
                message *m = malloc(sizeof(message));
                copy_to_buf(j->rawcmd, m->msg);
                m->status = 0;
                push(msg_q, PROCESS, (void *)m);
            }
        }
        else if(WIFCONTINUED(status)) { // continued
            j->running = 1;
            //printf("CONTINUED\n");
        }

        if(j->complete) {
            j->running = 0;
            if(!j->fg) {
                // queue finished message
                message *m = malloc(sizeof(message));
                copy_to_buf(j->rawcmd, m->msg);
                m->status = 1;
                push(msg_q, PROCESS, (void *)m);

                free_job(del_job(job_list, j->pgid));
            }
        }
    }

    //printf("HANDLER Done with waitpid loop\n");
}

void set_handler() {
    struct sigaction action;
    sigset_t mask;

    sigemptyset(&mask);
    sigaddset(&mask, SIGCHLD);
    action.sa_sigaction = handler;
    action.sa_mask = mask;
    action.sa_flags = SA_SIGINFO;

    sigaction(SIGCHLD, &action, NULL);
}

void exec_process(char *cmd, char **args, pid_t pgid, int fin, int fout, char fg) {
    int err;

    /* set the pgid */

    if( setpgid(0, pgid) == -1 ) {
        perror("setpgid");
        _exit(EXIT_FAILURE);
    }

    //printf("%s new pgid: %d\n", args[0],getpgid(0));
    if( fg && tcsetpgrp(STDIN_FILENO, getpgid(0)) == -1 ) {
        perror("tcsetpgrp");
        _exit(EXIT_FAILURE);
    }

    /* set stdin, stdout */

    if(fin != STDIN_FILENO) {
        dup2(fin, STDIN_FILENO);
        close(fin);
    }

    if(fout != STDOUT_FILENO) {
        dup2(fout, STDOUT_FILENO);
        close(fout);
    }

    /* reset signal handlers to default */
    signal(SIGTTOU, SIG_DFL);
    signal(SIGTTIN, SIG_DFL);
    signal(SIGTERM, SIG_DFL);
    signal(SIGTSTP, SIG_DFL);
    signal(SIGINT,  SIG_DFL);
    signal(SIGCHLD, SIG_DFL);

    err = execvp(cmd, args);
    if(err == -1) {
        err = errno;
        if(err == ENOENT || err == ENOTDIR) // no file or dir for cmd
            write(STDOUT_FILENO, UNCMD_MSG, STRING_SIZE(UNCMD_MSG));
        else
            perror("execvp");

        free_list(job_list);
        free_list(msg_q);

        _exit(EXIT_FAILURE);
    }
}

/** Flush stdin **/
void pflush() {
    char tmp[1] = {0};
    while(tmp[0] != '\n')
        read(STDIN_FILENO, tmp, 1);
}

void print_jobs() {
    int i,k;
    printf("--------------------------------------------------------------------------\n");
    printf("Job list size = %d\n", job_list->size);
    for(i=0;i<(job_list->size);i++) {
        job *j = ((job *) get(job_list, i));
        printf("job_list[%d]: %s j->fg: %d, j->pgid: %d, j->running: %d, j->complete: %d\n", i, j->rawcmd, j->fg, j->pgid, j->running, j->complete);
        for(k=0;k<(j->numcmds);k++) {
            process *p = (process *) get(j->pid_list, k);
            printf("\tProcess %d: %s p->pid: %d, p->pgid: %d, p->complete: %d\n", k, j->cmds[k][0], p->pid, p->pgid, p->complete);
        }
    }
    printf("--------------------------------------------------------------------------\n");
}

/** Wait on fg job **/
void jwait(job *j) {
    if (j == NULL)
        return;

    //printf("Waiting on %s (j->running = %d)...\n", j->rawcmd, j->running);

    while(j->running)
        pause();

    if(j->complete) {
        free_job(del_job(job_list, j->pgid));
    }
}

void do_fg(char **argv) {
    int num_toks = 0;

    while(argv[num_toks] != NULL) ++num_toks;

    job *fg_job = (job *)NULL;

    if (num_toks > 2) {
        printf("Invalid command.\n");
    } else if (num_toks == 1) {
        fg_job = bring_forward(job_list, 0, 1);
    } else {
        fg_job = bring_forward(job_list, my_atoi(argv[1]), 1);
    }

    if(fg_job != NULL) {
        fg_job->running = 1;
        printf("Restarting: %s\n", fg_job->rawcmd);
    }
    jwait(fg_job);
    
    if(tcsetpgrp(STDIN_FILENO, shell_pgid) == -1) {
        perror("tcsetpgrp");
        _exit(EXIT_FAILURE);
    }
}

void do_bg(char **argv) {
    int num_toks = 0;

    while(argv[num_toks] != NULL) ++num_toks;

    job *bg_job = (job *)NULL;

    if (num_toks > 2) {
        printf("Invalid command.\n");
    } else if (num_toks == 1) {
        bg_job = bring_forward(job_list, 0, 0);
    } else {
        bg_job = bring_forward(job_list, my_atoi(argv[1]), 0);
    }

    if (bg_job != NULL)
        printf("Running: %s\n", bg_job->rawcmd);
}

static void do_cd(char **argv) {
    int num_toks = 0;

    while(argv[num_toks] != NULL) ++num_toks;

    if (num_toks > 1) {
        if (chdir(argv[1]) == -1) {
            perror("cd");
            return;
        }

        sync_pwd();
    }
}

static void sync_pwd() {
    if (getcwd(pwd, BUFFER_SIZE) == NULL) {
        /* TODO: increase buffer size if necessary */
        perror("getcwd");
        exit(EXIT_FAILURE);
    }
}

void cleanup() {
    if(job_list->size > 0) {
        NODE *node = job_list->head;

        while(node != NULL) {
            job *j = (job *) node->node_ptr;
            if( killpg(j->pgid, SIGKILL) == -1 ) {
                perror("kill");
                _exit(EXIT_FAILURE);
            }
            node = node->next;
        }
    }

    free_list(job_list);
    free_list(msg_q);
}
