#include "tokenizer.h"
#include "jobs.h"
#include "job_handler.h"
#include "linked_list.h"
#include "parser.h"

/**
* Implementation of the system call atoi, since we can't use it
*
* @param str    string to convert
* @return       integer
*/
int my_atoi(const char *str) {
    int i = 0;
    int neg = 1;
    int number = 0;

    if (str == NULL) {
        return 0;
    }

    while (str[i] == ' ') {
        i++;
    }

    if (str[i] == '-') {
        neg = -1;
        i++;
    }

    while (str[i] != '\0' && str[i] >= '0' && str[i] <= '9') {
        number = number * 10 + (str[i] - '0');
        i++;
    }

    number *= neg;
    return number;
}

/**
* Compare two null-terminated strings.
*
* @param a  first string
* @param b  second string
* @return   0 - equal, non-zero - not equal
*/
int my_strcmp(const char *a, const char *b) {
    int i = 0;
    if(a == NULL || b == NULL) return -1;
    while(a[i] != '\0') {
        if(a[i] != b[i])
            return a[i] - b[i];
        ++i;
    }
    return a[i] - b[i];
}

/**
* Copy over a string to a buffer.
*
* @param str    the string to copy over
* @param buf    the buffer
*/
void copy_to_buf(const char *str, char *buf) {
    while(*str != '\0') {
        *buf = *str;
        ++str;
        ++buf;
    }
    *buf = '\0';
}

/**
* Split up a string into tokens.
*
* @param buf        the string to tokenize
* @param numtoks    the int pointer to store the # of tokens
* @return           an array of (string) tokens, terminated by a null pointer
*/
char** gettoks(char *buf, int *numtoks) {

    TOKENIZER *tokenizer;
    char *tok;
    int i;

    /* Count up args */
    *numtoks = 0;
    tokenizer = init_tokenizer(buf);
    while( (tok = get_next_token(tokenizer)) != NULL ) {
        free(tok);
        ++*numtoks;
    }
    free_tokenizer(tokenizer);

    char **toks = malloc(sizeof(char *) * (*numtoks+1));

    /* Copy 'em over */
    i = 0;
    tokenizer = init_tokenizer(buf);
    while( (tok = get_next_token(tokenizer)) != NULL ) 
        toks[i++] = tok;
    free_tokenizer(tokenizer);

    toks[*numtoks] = (char *) NULL; // null terminated argv

    return toks;
}

/**
* Return a count of sym in toks.
*
* @param toks   the array of strings
* @return       the cnt of sym in toks
*/
int getcnt(char **toks, char sym) {
    int i=0, cnt=0;
    char *tok;
    while( (tok = toks[i++]) != (char *) NULL ) {
        if(tok[0] == sym) {
            ++cnt;
        }
    }

    return cnt;
}

/**
* Return the file where stdin should be redirected.
*
* @param toks   the array of strings to parse
* @return       filename string to redirect stdin to, or
*               NULL if there was more than one stdin redirect
*/
char* getstdin(char **toks) {
    int i=0, cnt=0;
    char *tok, *fin;
    while( (tok = toks[i++]) != (char *) NULL ) {
        if(tok[0] == '<') {
            fin = toks[i];
            ++cnt;
        }
    }

    return cnt == 1 ? fin : (char *) NULL;
}

/**
* Return the file where stdout should be redirected.
*
* @param toks   the array of strings to parse
* @return       filename string to redirect stdout to, or
*               NULL if there was more than one stdout redirect
*/
char* getstdout(char **toks) {
    int i=0, cnt=0;
    char *tok, *fout;
    while( (tok = toks[i++]) != (char *) NULL ) {
        if(tok[0] == '>') {
            fout = toks[i];
            ++cnt;
        }
    }

    return cnt == 1 ? fout : (char *) NULL;
}

/**
* Get all the pipes given a token list.
*
* @param toks       list of tokens
* @param numpipes   stores number of pipes after counting
* @return           location of pipes in token list
*/
int* getpipes(char **toks, int *numpipes) {
    int i = 0;
    *numpipes = 0;
    char *tok;
    while( (tok = toks[i++]) != (char *) NULL )
        if(tok[0] == '|') 
            ++(*numpipes);

    int *pipes = (int*) malloc(sizeof(int *) * (*numpipes));

    i = 0; *numpipes = 0;
    while( (tok = toks[i]) != (char *) NULL ) {
        if(tok[0] == '|')
            pipes[(*numpipes)++] = i; 
        ++i;
    }

    return pipes;
 }        

/**
* Get the index of the next symbol in the token list.
*
* @param toks   list of tokens
* @param from   index to start looking from
* @return       index of next symbol
*/
int nextsym(char **toks, int from) {
    int i = from;
    int idx = -1;
    char *tok;
    while( (tok = toks[i]) != (char *) NULL ) {
        if(tok[0] == '<' || tok[0] == '>' || tok[0] == '|') {
            idx = i;
            break;
        }
        i++;
    }
    return idx;
}


/** 
* Copy over cmd tokens starting from start in the given token list.
*
* @param argvptr    ptr to argv list to copy tokens over to
* @param toks       cmd token list
* @param start      start index to copy from
* @param numtoks    length of toks
* @return           start index
*/
int tokcpy(char ***argvptr, char **toks, int start, int numtoks) {
    int sidx = nextsym(toks, start);
    int end = sidx == -1 ? numtoks : sidx; // if no symbols after start, go to end
    int cnt = end - start;
    int i;    

    *argvptr = (char **) malloc(sizeof(char *) * (cnt + 1));
    for(i=0;i<cnt;i++)
        (*argvptr)[i] = toks[start++];
    (*argvptr)[i] = (char *) NULL; // null terminated argv

    //if(end < numtoks) free(toks[start]);

    return start;
}

/**
* Get commands within a job
*
* @param toks       list of tokens
* @param numtoks    number of tokens
* @param numcmds    to hold the number of commands after parsing
* @return           return commands
*/
char*** getcmds(char **toks, int numtoks, int *numcmds) {
    int cidx=0;
    int start=0;
    int i;
    int numpipes;

    int *pipes = getpipes(toks, &numpipes);
    char ***cmds = (char ***) malloc(sizeof(char **) * (numpipes+1)); // n pipes implies n+1 cmds

    if(toks[0][0] == '>' || toks[0][0] == '<' || toks[0][0] == '|')
        return (char ***) NULL;

    /* Copy first command (there's gotta be at least one) */
    start = tokcpy(&cmds[0], toks, start, numtoks);
    cidx = 1;

    /* Copy over next commands (gotta be after a pipe) */
    for(i=0;i<numpipes;i++) {
        start = pipes[i] + 1; // cmd starts right after pipe
        start = tokcpy(&cmds[cidx++], toks, start, numtoks);
    }

    *numcmds = cidx;
    return cmds;
}

/**
* Get the index of the next pipe in the token list.
*
* @param toks   list of tokens
* @param from   index to start looking from
* @return       index of next symbol
*/
int nextpipe(char **toks, int from) {
    int i = from;
    int idx = -1;
    char *tok;
    while( (tok = toks[i]) != (char *) NULL ) {
        if(tok[0] == '|') {
            idx = i;
            break;
        }
        i++;
    }
    return idx;
}

/** 
* Copy over cmd tokens starting from start in the given token list
* up to a pipe. 
*
* @param argvptr    ptr to argv list to copy tokens over to
* @param toks       cmd token list
* @param start      start index to copy from
* @param numtoks    length of toks
* @return           start index
*/
int pipecpy(char ***argvptr, char **toks, int start, int numtoks) {
    int sidx = nextpipe(toks, start);
    int end = sidx == -1 ? numtoks : sidx; // if no symbols after start, go to end
    int cnt = end - start;
    int i;    

    *argvptr = (char **) malloc(sizeof(char *) * (cnt + 1));
    for(i=0;i<cnt;i++)
        (*argvptr)[i] = toks[start++];
    (*argvptr)[i] = (char *) NULL; // null terminated argv

    return start;
}

/** 
* Split up a list of string tokens into chunks using '|' as
* a delimiter of sorts.
*
* @param toks       cmd token list
* @param numtoks    length of toks
* @param numcmds    ptr to store number of commands
* @return           groups of tokens split based on '|'
*/
char*** pipesplit(char **toks, int numtoks, int *numcmds) {
    int i = 0, cur = 0;
    int numpipes = 0;
    char *tok;

    while( (tok = toks[i++]) != (char *) NULL )
        if(tok[0] == '|') 
            ++numpipes;

    *numcmds = numpipes + 1;
    char ***cmds = (char ***) malloc(sizeof(char **) * (*numcmds));

    cur = pipecpy(&cmds[0], toks, cur, numtoks) + 1;

    for(i=0;i<numpipes;i++)
        cur = pipecpy(&cmds[i+1], toks, cur, numtoks) + 1;
        
    return cmds;
}

/**
* Strip off all the tokens after a redirection symbol.
*
* @param toks   list of tokens
* @return       cleaned list of tokens
*/
char** strip_redirects(char **toks) {
    int numtoks = 0;
    char **stripped;

    while( toks[numtoks++] != (char *) NULL ); // count up numtoks

    tokcpy(&stripped, toks, 0, numtoks);

    return stripped;
}

/**
* Check for reditection errors in a job.
*
* @param j          the job to check
* @param rawcmds    array of raw cmd strings
* @return           0 if no errors, -1 otherwise
*/
int check_redirects(job *j, char ***rawcmds) {
    int i = 0;

    if(j->numcmds > 1 && j->fout[0] != (char *) NULL) { // ls > a | wc case
        return -1;
    } 

    // ls > a > b
    if(getcnt(rawcmds[0], '<') > 1)
        return -1;
    if(getcnt(rawcmds[0], '>') > 1)
        return -1;

    for(i=1;i<j->numcmds;i++) { // if any cmd nested in pipe redirects stdout, e.g. ls | wc > a | head
        if(j->fin[i] != (char *) NULL || (i != j->numcmds-1 && j->fout[i] != (char *) NULL)) {
            return -1;
        }

        // ls | wc > a > b
        if(getcnt(rawcmds[i], '<') > 1)
            return -1;
        if(getcnt(rawcmds[i], '>') > 1)
            return -1;

    }

    return 0;
    
}


/**
* Parse out a raw string command read in from the shell prompt.
*
* @param buf    the read in string
* @return       a job
*/
job * parse(char *buf) {
    int i, numtoks, numcmds, err;
    job *j = (job *) malloc(sizeof(job));

    i = 0;
    while(buf[i] != '\0') {
        j->rawcmd[i] = buf[i];
        i++;
    }
    j->rawcmd[i] = '\0';

    char **toks = gettoks(buf, &numtoks);
    if(toks[numtoks-1][0] == '&') { 
        j->fg = 0;
        free(toks[numtoks-1]);
        toks[numtoks-1] = (char *) NULL;
        numtoks--; // drop '&'
    }
    else j->fg = 1;

    char ***rawcmds = pipesplit(toks, numtoks, &(j->numcmds));
    j->cmds = (char ***) malloc(sizeof(char **) * (j->numcmds));
    j->fin = (char **) malloc(sizeof(char *) * (j->numcmds));
    j->fout = (char **) malloc(sizeof(char *) * (j->numcmds));

    for(i=0;i<(j->numcmds);i++) {
        j->fin[i] = getstdin(rawcmds[i]); 
        j->fout[i] = getstdout(rawcmds[i]);
        j->cmds[i] = strip_redirects(rawcmds[i]);
    }

    j->pid_list = init_list();

    numcmds = j->numcmds; // copy to free rawcmds later
    err = check_redirects(j, rawcmds);

    if(err == -1) {
        for(i=0;i<numtoks;i++) {
            free(toks[i]);
        }
        free_job_cmd(j);
        j = (job *) NULL;
    } else {
        // free up the special symbols in toks
        for(i=0;i<numtoks;i++) {
            //printf("toks[%d]: %s\n", i, toks[i]);
            if(toks[i][0] == '>' || toks[i][0] == '<' || toks[i][0] == '|' || toks[i][0] == '&')
                free(toks[i]);
        }
    }

    for(i=0;i<numcmds;i++)
        free(rawcmds[i]);
    free(rawcmds);
    free(toks);

    return j;
}

