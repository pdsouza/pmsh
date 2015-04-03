#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "../linked_list.h"
#include "../jobs.h"
#include "../parser.h"

#define CMD1 "cat < /proc/cpuinfo | head -5 > ./cpuinfo"
#define CMD2 "ls -al | head -5"
#define CMD3 "pwd"
#define CMD5 "echo 10 > ./info"

int main(int argc, char **argv) {
    LIST *jl = init_list();
    //job *jb1 = parse(CMD1);
    //job *jb2 = parse(CMD2);
    //job *jb3 = parse(CMD3);
    job *jb5 = parse(CMD5);
    
    /*job *jb4 = (job *)malloc(sizeof(job));
    
    char **fin_test = (char **)malloc(sizeof(char *) * 2);

    fin_test[0] = (char *)malloc(sizeof(char) * 10);
    fin_test[1] = (char *)malloc(sizeof(char) * 10);

    char **fout_test = (char **)malloc(sizeof(char *) * 2);
    
    fout_test[0] = (char *)malloc(sizeof(char) * 10);
    fout_test[1] = (char *)malloc(sizeof(char) * 10);

    jb4->fin = fin_test;
    jb4->fout = fout_test;

    jb4->numcmds = 2;

    char ***cmd_test = (char ***)malloc(sizeof(char **) * 2);

    cmd_test[0] = (char **)malloc(sizeof(char *) * 2);
    cmd_test[1] = (char **)malloc(sizeof(char *) * 2);

    cmd_test[0][0] = (char *)malloc(sizeof(char) * 2);
    cmd_test[1][0] = (char *)malloc(sizeof(char) * 2);

    cmd_test[0][0][0] = 'a';
    cmd_test[1][0][0] = 'b';
    cmd_test[0][1] = (char *)NULL;
    cmd_test[1][1] = (char *)NULL;

    jb4->cmds = cmd_test;

    int *i = (int *)malloc(sizeof(int));
    *i = 9;
    LIST *l = init_list();
    push(l, PROCESS, (void *)i);

    int *j = (int *)malloc(sizeof(int));
    *j = 7;
    push(l, PROCESS, (void *)j);

    jb4->pid_list = l;*/
    
    //push(jl, JOB, (void *)jb1);
    //push(jl, JOB, (void *)jb2);
    //push(jl, JOB, (void *)jb3);

    //push(jl, JOB, (void *)jb4);
    push(jl, JOB, (void *)jb5);
    free_list(jl);

    /*int *i = (int *)malloc(sizeof(int));
    *i = 9;
    LIST *l = init_list();
    push(l, PROCESS, (void *)i);

    free_list(l);*/

    return 0;
}

