#ifndef __HISTORY_H__
#define __HISTORY_H__

/**
* Add a command to shell history
*
* @param str    command
* @return       integer
*/
int add_history(const char *cmd);

/**
* Get a command from past history
*
* @param idx    index into stack-based ordering of history
* @return       the command at history[idx]
*/
const char* get_history(char *buf, int size, int idx);

#endif
