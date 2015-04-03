#ifndef __JOB_HANDLER_H__
#define __JOB_HANDLER_H__

#include <unistd.h>
#include <sys/types.h>
#include "jobs.h"
#include "linked_list.h"

/**
* Generate a pgid for a new job by returning the smallest
* positive integer not taken by jobs in job_list
*
* @param job_list   the list of jobs to generate the id for
* @return           the next valid job id
*/
pid_t gen_job_id(LIST *job_list);

/**
* Set the process group id of all processes in the given 
* job to the given pgid.
*
* @param j      the job which needs to have its processes set
* @param pgid   the pgid to set it to
* @return       simply returns the pgid
*/
pid_t set_pgid(job *j, pid_t pgid);

/**
* Gets the current foreground job in the job list if any
*
* @param job_list   the job list to search
* @return           the foreground job
*/
job* get_fg(LIST *job_list);

/**
* Prints the list of jobs running in the background
*
* @param job_list   the list of jobs to traverse
*/
void print_bg(LIST *job_list);

/**
* Gets a job by its pgid
*
* @param job_list   the job list to search through
* @param pgid       the pgid to match against
* @return           the job with the matching pgid
*/
job* get_job_pgid(LIST *job_list, pid_t pgid);

/**
* Gets a job by its job id
*
* @param job_list   the job list to search through
* @param job_id     the job id to match against
* @return           the job with the matching job id
*/
job* get_job_jid(LIST *job_list, int job_id);

/**
* Gets the job that contains the pid
*
* @param job_list   the job list to search through
* @param pid        the process id to look for
* @return           the job that contains the process id
*/
job* get_job(LIST *job_list, pid_t pid);

/**
* Deletes a job from the list based on pgid
*
* @param job_list   the job list to delete from
* @param pgid       the pgid to delete
* @return           the job that was deleted
*/
job* del_job(LIST *job_list, pid_t pgid);

/**
* Sets a process within a job to complete as well as the entire job
*
* @param j      the job to loop through
* @param pid    the process to set complete
* @return       0 - success, -1 - error
*/
int set_complete(job *j, pid_t pid);

/**
* Frees the memory taken up by the job but not the commands within the job
*
* @params job   the job to free
*/
void free_job_cmd(job *job);

/**
* Frees the memory taken up by the job
*
* @param job    the job to free
*/
void free_job(job *job);

/**
* Send a job to the background
*
* @param job_list   the job list that contains the job
* @param pgid       the pgid of the job to send back
*/
void send_back(LIST *stack, pid_t pgid);

/**
* Sends a SIGCONT to specified job
*
* @param job_list   the job list that contains the job
* @param job_id     the job id of the job to bring forward
* @param fgcmd      whether the job should be brought forward or not
* @return           the job that was affected
*/
job* bring_forward(LIST *stack, int job_id, char fgcmd);

#endif

