#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "../job_handler.h"
#include "../jobs.h"
#include "../linked_list.h"

job* tstjob(pid_t pgid);

int main() {
    LIST *jlist = init_list();

    assert(gen_job_id(jlist) == 1);
    assert(get_fg(jlist) == (job *)NULL);
    assert(jlist->size == 0);

    //print_bg(jlist);
    
    job *jb1 = tstjob(1);
    push(jlist, JOB, jb1);
    assert(gen_job_id(jlist) == 2);
    assert(get_fg(jlist) == jb1);

    //print_bg(jlist);

    job *jb2 = tstjob(3);
    push(jlist, JOB, jb2);
    assert(gen_job_id(jlist) == 2);

    job *jb3 = tstjob(2);
    push(jlist, JOB, jb3);
    assert(gen_job_id(jlist) == 4);

    push(jlist, JOB, tstjob(7));
    assert(gen_job_id(jlist) == 4);

    print_bg(jlist);

    pop(jlist);
    assert(gen_job_id(jlist) == 4);

    assert(get_job_pgid(jlist, 11) == (job *)NULL);
    assert(get_job_pgid(jlist, 7) == jb2);
    assert(get_job_pgid(jlist, 3) == jb3);
    assert(get_job(jlist, 3) == jb2);
    assert(del_job(jlist, 7) == jb2);
    assert(get_job(jlist, 3) == (job *)NULL);
    assert(get_job(jlist, 55) == jb3);
    assert(get_job(jlist, 10) == (job *)NULL);
    assert(set_complete(jb2, 3) == 0);
    assert(set_complete(jb2, 4) == -1);
    assert(set_complete(jb3, 55) == 0);
    assert(((process *)jb2->pid_list->head->node_ptr)->complete == 0);
    assert(jb2->complete == 0);
    assert(jb3->complete == 1);
    assert(jlist->size == 2);
    assert(get_job_jid(jlist, 1) == jb1);
    assert(get_job_jid(jlist, 11) == (job *)NULL);

    printf("[job_handler_tst] All tests pass!\n");
    return 0;
}

job* tstjob(int job_id) {
    job *j = (job *) malloc(sizeof(job));
    j->job_id = job_id;

    if (job_id == 1) {
        j->fg = 1;
        j->rawcmd[0] = 't';
        j->rawcmd[1] = '\0';
    } else if (job_id == 3) {
        j->fg = 0;
        j->rawcmd[0] = 'b';
        j->rawcmd[1] = 'a';
        j->rawcmd[2] = '\0';
    } else {
        j->fg = 0;
    }

    if (job_id == 3) {
        int i = 0;
        j->pgid = 7;
        LIST *l = init_list();

        for (i = 0; i < 4; i++) {
            process *p = (process *)malloc(sizeof(process));

            p->complete = 0;
            p->pid = i;
            push(l, PROCESS, (void *)p);
        }
        
        j->numcmds = 4;
        j->pid_list = l;
    } else if (job_id == 2) {
        int i = 0;
        j->pgid = 3;
        LIST *l = init_list();

        for (i = 0; i < 4; i++) {
            process *p = (process *)malloc(sizeof(process));

            p->complete = 1;
            p->pid = i + 55;
            push(l, PROCESS, (void *)p);
        }
        
        j->numcmds = 4;
        j->pid_list = l;
    } else {
        int i = 0;
        LIST *l = init_list();

        for (i = 0; i < 3; i++) {
            process *p = (process *)malloc(sizeof(process));

            p->complete = 1;
            p->pid = i + 100;
            push(l, PROCESS, (void *)p);
        }
        
        j->numcmds = 3;
        j->pid_list = l;
    }

    return j;
}

