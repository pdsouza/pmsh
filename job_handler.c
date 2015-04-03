#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include "job_handler.h"
#include "jobs.h"
#include "linked_list.h"

/**
* Generate a pgid for a new job by returning the smallest
* positive integer not taken by jobs in job_list
*
* @param job_list   the list of jobs to generate the id for
* @return           the next valid job id
*/
int gen_job_id(LIST *job_list) {
    int i = 1, cnt = -1;
    int job_id = 1;
    while(cnt != 0) {
        cnt = 0;
        for(i=0;i<job_list->size;i++) {
            job *j = (job *)get(job_list, i);
            if(j->job_id == job_id) {
                ++cnt;
                ++job_id;
                break;
            }
        }
    }
    return job_id;
}

/**
* Set the process group id of all processes in the given 
* job to the given pgid.
*
* @param j      the job which needs to have its processes set
* @param pgid   the pgid to set it to
* @return       simply returns the pgid
*/
pid_t set_pgid(job *j, pid_t pgid) {
    int i;
    LIST *pid_list = j->pid_list;

    for(i=0;i<pid_list->size;i++) {
        pid_t pid = *(pid_t *)get(pid_list, i);
        setpgid(pid, pgid);
    }
    j->pgid = pgid;

    return pgid;
}

/**
* Gets the current foreground job in the job list if any
*
* @param job_list   the job list to search
* @return           the foreground job
*/
job* get_fg(LIST *job_list) {
    if (job_list->size > 0) {
        NODE *node = job_list->head;

        while (node != NULL && !(((job *)node->node_ptr)->fg)) {
            node = node->next;
        }

        if (node != NULL) {
            return (job *)node->node_ptr;
        } else {
            return (job *)NULL;
        }
    } else {
        return (job *)NULL;
    }
}

/**
* Prints the list of jobs running in the background
*
* @param job_list   the list of jobs to traverse
*/
void print_bg(LIST *job_list) {
    int max = 0;
    int i = 0;
    NODE *node = job_list->head;

    if (job_list->size == 0) {
        printf("There are currently no background jobs running.\n");
        return;
    }

    while (node != NULL) {
        int id = ((job *)node->node_ptr)->job_id;

        if (id > max) {
            max = id;
        }

        node = node->next;
    }

    for (i = 1; i <= max; i++) {
        job *j = get_job_jid(job_list, i);

        if (j != NULL && !j->fg && !j->complete) {
            printf("[%d] %s (%s)\n", j->job_id, j->rawcmd, j->running ? "Running" : "Stopped");
        }
    }
}

/**
* Gets a job by its pgid
*
* @param job_list   the job list to search through
* @param pgid       the pgid to match against
* @return           the job with the matching pgid
*/
job* get_job_pgid(LIST *job_list, pid_t pgid) {
    if (job_list->size > 0) {
        NODE *node = job_list->head;

        while (node != NULL && (((job *)node->node_ptr)->pgid) != pgid) {
            node = node->next;
        }

        if (node != NULL) {
            return (job *)node->node_ptr;
        } else {
            return (job *)NULL;
        }
    } else {
        return (job *)NULL;
    }
}

/**
* Gets a job by its job id
*
* @param job_list   the job list to search through
* @param job_id     the job id to match against
* @return           the job with the matching job id
*/
job* get_job_jid(LIST *job_list, int job_id) {
    if (job_list->size > 0) {
        NODE *node = job_list->head;

        while (node != NULL && (((job *)node->node_ptr)->job_id) != job_id) {
            node = node->next;
        }

        if (node != NULL) {
            return (job *)node->node_ptr;
        } else {
            return (job *)NULL;
        }
    } else {
        return (job *)NULL;
    }
}

/**
* Determines whether or not a job contains the pid
*
* @param process_list   the job's process list to check
* @param pid            the pid to look for
* @return               1 - exists, 0 - does not exist
*/
static int get_job_pid(LIST *process_list, pid_t pid) {
    if (process_list->size > 0) {
        NODE *node = process_list->head;

        while (node != NULL && (((process *)node->node_ptr)->pid) != pid) {
            node = node->next;
        }

        if (node != NULL) {
            return 1;
        } else {
            return 0;
        }
    } else {
        return 0;
    }
}

/**
* Gets the job that contains the pid
*
* @param job_list   the job list to search through
* @param pid        the process id to look for
* @return           the job that contains the process id
*/
job* get_job(LIST *job_list, pid_t pid) {
    if (job_list->size > 0) {
        NODE *node = job_list->head;

        while (node != NULL && !get_job_pid(((job *)node->node_ptr)->pid_list, pid)) {
            node = node->next;
        }

        if (node != NULL) {
            return (job *)node->node_ptr;
        } else {
            return (job *)NULL;
        }
    } else {
        return (job *)NULL;
    }
}

/**
* Deletes a job from the list based on pgid
*
* @param job_list   the job list to delete from
* @param pgid       the pgid to delete
* @return           the job that was deleted
*/
job* del_job(LIST *job_list, pid_t pgid) {
    if (job_list->size > 0) {
        NODE *node = job_list->head;

        while (node != NULL && (((job *)node->node_ptr)->pgid) != pgid) {
            node = node->next;
        }

        if (node != NULL) {
            job *j = (job *)del_node(job_list, node);
            free(node);
            return j;
        } else {
            return (job *)NULL;
        }
    } else {
        return (job *)NULL;
    }
}

/**
* Sets a process within a job to complete as well as the entire job
*
* @param j      the job to loop through
* @param pid    the process to set complete
* @return       0 - success, -1 - error
*/
int set_complete(job *j, pid_t pid) {
    int i = 0;
    char incomplete = 1;
    int r = -1;

    NODE *node = (NODE *)j->pid_list->head;

    for (i = 0; i < j->numcmds; i++) {
        process *p = (process *)node->node_ptr;

        if (p->pid == pid) {
            p->complete = 1;
            r = 0;
        }

        incomplete &= p->complete;
        node = node->next;
    }
    
    j->complete = incomplete;
    return r;
}

/**
* Frees the memory taken up by the job but not the commands within the job
*
* @params job   the job to free
*/
void free_job_cmd(job *job) {
    int i = 0;

    for (i = 0; i < job->numcmds; i++) {
        free(job->cmds[i]);
    }

    free(job->fin);
    free(job->fout);
    free(job->cmds);
    free_list(job->pid_list);
    free(job);
}

/**
* Frees the memory taken up by the job
*
* @param job    the job to free
*/
void free_job(job *job) {
    int i = 0;
    int a = 0;

    for (i = 0; i < job->numcmds; i++) {
        free(job->fin[i]);
        free(job->fout[i]);

        while (job->cmds[i][a] != (char *)NULL) {
            free(job->cmds[i][a]);
            a++;
        }
        a = 0;

        free(job->cmds[i]);
    }

    free(job->fin);
    free(job->fout);
    free(job->cmds);
    free_list(job->pid_list);
    free(job);
}

/**
* Send a job to the background
*
* @param job_list   the job list that contains the job
* @param pgid       the pgid of the job to send back
*/
void send_back(LIST *job_list, pid_t pgid) {
    job *j = del_job(job_list, pgid);
    j->fg = 0;
    push(job_list, JOB, (void *)j);

    if (tcsetpgrp(STDIN_FILENO, getpgid(0)) == -1) {
        perror("tcsetpgrp");
        _exit(EXIT_FAILURE);
    }
}

/**
* Sends a SIGCONT to specified job
*
* @param job_list   the job list that contains the job
* @param job_id     the job id of the job to bring forward
* @param fgcmd      whether the job should be brought forward or not
* @return           the job that was affected
*/
job* bring_forward(LIST *job_list, int job_id, char fgcmd) {
    job *j = (job *)NULL;

    if (job_list->size > 0) {
        if (job_id > 0) {
            j = get_job_jid(job_list, job_id);
        } else {
            if (fgcmd) {
                j = (job *)job_list->tail->node_ptr;
            } else {
                NODE *node = job_list->tail;

                while (node != NULL && (((job *)node->node_ptr)->running)) {
                    node = node->previous;
                }

                if (node != NULL) {
                    j = (job *)node->node_ptr;
                }
            }
        }

        if (j != NULL) {
            if (fgcmd) {
                j->fg = 1;

                if (tcsetpgrp(STDIN_FILENO, j->pgid) == -1) {
                    perror("tcsetpgrp");
                    _exit(EXIT_FAILURE);
                }
            } else {
                j->fg = 0;
            }

            if (killpg(j->pgid, SIGCONT) == -1) {
                perror("kill");
                _exit(EXIT_FAILURE);
            }
        }
    }

    return j;
}

