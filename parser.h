#ifndef __PARSER_H__
#define __PARSER_H__

#include "jobs.h"

/**
* Implementation of the system call atoi, since we can't use it
*
* @param str    string to convert
* @return       integer
*/
int my_atoi(const char *str);

/**
* Compare two null-terminated strings.
*
* @param a  first string
* @param b  second string
* @return   0 - equal, non-zero - not equal
*/
int my_strcmp(const char *a, const char *b);

/**
* Copy over a string to a buffer.
*
* @param str    the string to copy over
* @param buf    the buffer
*/
void copy_to_buf(const char *a, char *buf);

/**
* Split up a string into tokens.
*
* @param buf        the string to tokenize
* @param numtoks    the int pointer to store the # of tokens
* @return           an array of (string) tokens, terminated by a null pointer
*/
char** gettoks(char *buf, int *numtoks);

/**
* Return the file where stdin should be redirected.
*
* @param toks   the array of strings to parse
* @return       filename string to redirect stdin to, or
*               NULL if there was more than one stdin redirect
*/
char* getstdin(char **toks);

/**
* Return the file where stdout should be redirected.
*
* @param toks   the array of strings to parse
* @return       filename string to redirect stdout to, or
*               NULL if there was more than one stdout redirect
*/
char* getstdout(char **toks);

/**
* Get all the pipes given a token list.
*
* @param toks       list of tokens
* @param numpipes   stores number of pipes after counting
* @return           location of pipes in token list
*/
int* getpipes(char **toks, int *numpipes);

/**
* Get commands within a job
*
* @param toks       list of tokens
* @param numtoks    number of tokens
* @param numcmds    to hold the number of commands after parsing
* @return           return commands
*/
char*** getcmds(char **toks, int numtoks, int *numcmds);

/** 
* Split up a list of string tokens into chunks using '|' as
* a delimiter of sorts.
*
* @param toks       cmd token list
* @param numtoks    length of toks
* @param numcmds    ptr to store number of commands
* @return           groups of tokens split based on '|'
*/
char*** pipesplit(char **toks, int numtoks, int *numcmds);

/**
* Strip off all the tokens after a redirection symbol.
*
* @param toks   list of tokens
* @return       cleaned list of tokens
*/
char** strip_redirects(char **toks);

/**
* Parse out a raw string command read in from the shell prompt.
*
* @param buf    the read in string
* @return       a job
*/
job* parse(char *buf);

#endif

