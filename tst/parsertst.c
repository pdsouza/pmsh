#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "../jobs.h"
#include "../parser.h"

/* getstdin test strings */

// pos
#define STDIN1 "cat < test1 | head test2"
#define STDIN2 "head > test2 | cat < test1"
// neg
#define STDIN3 "cat < test1 < test2"

/* getstdout test strings */
#define STDOUT1 "cat > test1 | head test2"
#define STDOUT2 "cat > test1 | head < test2"

#define STDOUT3 "cat > test1 > test2"

#define PIPES1 "cat > test | head > blah"
#define PIPES2 "cat > test | head | tail"

#define CMDS1 "cat"
#define CMDS2 "cat test"
#define CMDS3 "cat > test"
#define CMDS4 "cat < test1 | head test2 | tail test3"

#define CMDS5 "cat < test1 | < head test2 | tail test3"

// bg
#define BG1 "sleep 10&"

#define ATOI1 "5"
#define ATOI2 "-5"
#define ATOI3 "    5"
#define ATOI4 "    -5"
#define ATOI5 "5asdf"
#define ATOI6 "5     "
#define ATOI7 "    -5asdf   "
#define ATOI8 "   asdfasdf   asdfadsf   "

int main() {
    int numtoks, numpipes, numcmds;
    char **toks, *tok;
    int i, k;

    /* my_atoi */
    assert( my_atoi(ATOI1) == atoi(ATOI1) );
    assert( my_atoi(ATOI2) == atoi(ATOI2) );
    assert( my_atoi(ATOI3) == atoi(ATOI3) );
    assert( my_atoi(ATOI4) == atoi(ATOI4) );
    assert( my_atoi(ATOI5) == atoi(ATOI5) );
    assert( my_atoi(ATOI6) == atoi(ATOI6) );
    assert( my_atoi(ATOI7) == atoi(ATOI7) );
    assert( my_atoi(ATOI8) == atoi(ATOI8) );

    /* my_strcmp */
    assert( my_strcmp("cat", "cat") == 0 );
    assert( my_strcmp("cat", "catz") != 0 );
    assert( my_strcmp("catz", "cat") != 0 );
    assert( my_strcmp("odin", "odis") < 0 );
    assert( my_strcmp("obin", "odin") < 0 );
    assert( my_strcmp("odin", "nidhoggr") > 0 );
    assert( my_strcmp(NULL, "cat") == -1 );

    /* gettoks tests */
    char *t1 = "cat < /tmp/myfile";
    char **r1 = gettoks(t1, &numtoks);
    assert( numtoks == 3 );
    assert( strcmp("cat", r1[0]) == 0 );
    assert( strcmp("<", r1[1]) == 0 );
    assert( strcmp("/tmp/myfile", r1[2]) == 0 );

    /* getstdin tests */
    char *t2[] = { "cat", "<", "/tmp/myfile", (char *) NULL };
    char *r2 = getstdin(t2);
    assert( r2 != (char *) NULL );
    assert( strcmp("/tmp/myfile", r2) == 0 );

    char *t3[] = { "cat", "<", (char *) NULL };
    char *r3 = getstdin(t3);
    assert( r3 == (char *) NULL );

    toks = gettoks(STDIN1, &numtoks);
    assert( strcmp("test1", getstdin(toks)) == 0 );
    free(toks);

    toks = gettoks(STDIN2, &numtoks);
    assert( strcmp("test1", getstdin(toks)) == 0 );
    free(toks);

    toks = gettoks(STDIN3, &numtoks);
    assert( getstdin(toks) == (char *) NULL );
    free(toks);

    /* getstdout tests */
    char *t4[] = { "cat", ">", "/tmp/myfile", (char *) NULL };
    char *r4 = getstdout(t4);
    assert( r4 != (char *) NULL );
    assert( strcmp("/tmp/myfile", r4) == 0 );

    char *t5[] = { "cat", ">", (char *) NULL };
    char *r5 = getstdout(t5);
    assert( r5 == (char *) NULL );

    /* getpipes tests */
    toks = gettoks(PIPES1, &numtoks);
    int *pipes = getpipes(toks, &numpipes);
    assert( numpipes == 1 );
    free(toks);
    free(pipes);

    toks = gettoks(PIPES2, &numtoks);
    pipes = getpipes(toks, &numpipes);
    assert( numpipes == 2 );
    free(toks);
    free(pipes);

    /* getcmds tests */
    toks = gettoks(CMDS1, &numtoks);
    char ***cmds = getcmds(toks, numtoks, &numcmds);
    assert( numcmds == 1 );
    assert( strcmp(cmds[0][0], "cat") == 0 );
    free(cmds); // free other stuff?

    toks = gettoks(CMDS2, &numtoks);
    cmds = getcmds(toks, numtoks, &numcmds);
    assert( numcmds == 1 );
    assert( strcmp(cmds[0][0], "cat") == 0 );
    assert( strcmp(cmds[0][1], "test") == 0 );
    free(cmds);

    toks = gettoks(STDIN1, &numtoks);
    cmds = getcmds(toks, numtoks, &numcmds);
    assert( numcmds == 2 );
    assert( strcmp(cmds[0][0], "cat") == 0 );
    assert( strcmp(cmds[1][0], "head") == 0 );
    assert( strcmp(cmds[1][1], "test2") == 0 );
    free(cmds);

    toks = gettoks(CMDS4, &numtoks);
    cmds = getcmds(toks, numtoks, &numcmds);
    assert( numcmds == 3 );
    assert( strcmp(cmds[0][0], "cat") == 0 );
    assert( strcmp(cmds[1][0], "head") == 0 );
    assert( strcmp(cmds[1][1], "test2") == 0 );
    assert( strcmp(cmds[2][0], "tail") == 0 );
    assert( strcmp(cmds[2][1], "test3") == 0 );
    free(cmds);

    /* pipe splitting and redirect stripping */
    toks = gettoks(STDIN1, &numtoks);
    char ***split = pipesplit(toks, numtoks, &numcmds);
    assert( strcmp(split[0][0], "cat") == 0 );
    assert( strcmp(split[0][1], "<") == 0 );
    assert( strcmp(split[0][2], "test1") == 0 );
    assert( strcmp(split[1][0], "head") == 0 );
    assert( strcmp(split[1][1], "test2") == 0 );

    char **stripped = strip_redirects(split[0]);
    assert( strcmp(stripped[0], "cat") == 0 );
    assert( stripped[1] == (char *) NULL );

    stripped = strip_redirects(split[1]);
    assert( strcmp(stripped[0], "head") == 0 );
    assert( strcmp(stripped[1], "test2") == 0 );
    free(split);

    toks = gettoks(STDIN2, &numtoks);
    split = pipesplit(toks, numtoks, &numcmds);
    assert( strcmp(split[0][0], "head") == 0 );
    assert( strcmp(split[0][1], ">") == 0 );
    assert( strcmp(split[0][2], "test2") == 0 );
    assert( strcmp(split[1][0], "cat") == 0 );
    assert( strcmp(split[1][1], "<") == 0 );
    assert( strcmp(split[1][2], "test1") == 0 );
    /*k = 0;
    for(i=0;i<numcmds;i++) {
        printf("cmd %d\n", i);
        while( (tok = split[i][k]) != (char *) NULL ) {
            printf("split[%d][%d] = %s\n", i, k, tok);
            ++k;
        }
        k = 0;
    }*/
    free(split);

    /* parse tests */
    job *j = parse(STDIN1);
    assert( j->fg == 1 );
    assert( strcmp(j->fin[0], "test1") == 0 );
    assert( (j->numcmds) == 2 );
    assert( strcmp(j->cmds[0][0], "cat") == 0 );
    assert( strcmp(j->cmds[1][0], "head") == 0 );
    assert( strcmp(j->cmds[1][1], "test2") == 0 );
    free(j);

    j = parse(STDIN2);
    assert( j == (job *) NULL);
    free(j);

    j = parse(BG1);
    assert( j != (job *) NULL );
    assert( j->fg == 0 );
    assert( j->numcmds == 1 );
    assert( strcmp(j->cmds[0][0], "sleep") == 0 );
    assert( strcmp(j->cmds[0][1], "10") == 0 );

    printf("All tests pass!\n");
    return 0;
}
