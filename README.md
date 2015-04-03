# PMShell

PMShell is a project to open up and explore the internals of a standard job-control shell--a "blackbox" for most programmers. By starting with barebones functionality, we hope to trim the legacy fat and build a lean (and fun) shell for the 21st century.

Currently, PMShell functions in almost the same way as a full featured shell. It supports redirection of standard in and standard out as well as multi-state pipelines. It allows for jobs to run either in the foreground or in the background. Job control is also supported to allow the user to move jobs between the foreground and the background.

## Compilation Instructions

    make
    ./pmsh

## Code Description

The code follows this general order of execution:

1. User is prompted for input.

2. After receiving input, the string is cleaned and parsed.

3. Cleaning involves taking care of more input than the buffer size. Parsing tokenizes the string by gathering the commands, arguments attached to the commands, redirections, and pipes and then storing everything as a job.

4. For each command in the job, fork() is called to create a child process, the standard in and standard out are set for any redirects, and the process is executed.

5. For each job, the process group id is set so that each job can be grouped together.

6. Meanwhile, as jobs are being run in the background and foreground, terminal control is set according to which job currently has control of the foreground.

7. Because there are multiple jobs running simultaneously and out of order either in the back or in the front, signal handling must be asynchronous. In order to do this, a signal handler is attached to SIGCHLD that handles the multiple incoming signals. The handler loops to capture simultaneous SIGCHLDs in case more than one SIGCHLD is received at the same time.

8. Each foreground job must also be waited on. In this case, the job waits for a signal to be delivered to the job. Once the signal is delivered, it checks to see if the job is complete. If completed, the job is deleted from the job list and freed.

There are other files included in the submission as well. Jobs.h is a header file that contains the structs that define a job and a process. job_handler deals with managing the jobs in the job list. parser is a wrapper on top of the given tokenizer that nicely packages the input string into a job. linked_list is an implementation of a doubly linked list. tokenizer is the original parser provided to us.

## Stuff to try

1. Run your favorite text editor.

2. Compile your code.

3. Manage your git repository.

4. Launch a web browser.

## Examples

```sh
maxmguo@spec29:~/pmsh> ./pmsh
pmsh $ git clone https://github.com/SuperChocomocha/Ring-Buffer.git 
Cloning into Ring-Buffer...
remote: Counting objects: 4, done.
remote: Compressing objects: 100% (3/3), done.
remote: Total 4 (delta 1), reused 4 (delta 1)
Unpacking objects: 100% (4/4), done.
pmsh $ ls
job_handler.c  job_handler.o  linked_list.c  linked_list.o  parser.c  parser.o  pmsh.c     Ring-Buffer  tokenizer.h  tst
job_handler.h  jobs.h         linked_list.h  Makefile       parser.h  pmsh  README.md  tokenizer.c  tokenizer.o
pmsh $ vim ./Ring-Buffer/ring_buf.h
pmsh $ git status                   
# On branch master
# Changes not staged for commit:
#   (use "git add <file>..." to update what will be committed)
#   (use "git checkout -- <file>..." to discard changes in working directory)
#
#   modified:   README.md
#
# Untracked files:
#   (use "git add <file>..." to include in what will be committed)
#
#   Ring-Buffer/
#   job_handler.o
#   linked_list.o
#   parser.o
#   pmsh
#   tokenizer.o
no changes added to commit (use "git add" and/or "git commit -a")
pmsh $ rm -rf Ring-Buffer
pmsh $ ls
job_handler.c  job_handler.o  linked_list.c  linked_list.o  parser.c  parser.o  pmsh.c     tokenizer.c  tokenizer.o
job_handler.h  jobs.h         linked_list.h  Makefile       parser.h  pmsh  README.md  tokenizer.h  tst
pmsh $ git add README.md
pmsh $ git commit -m "showing-off-pmsh"
[master 321dfcd] "showing-off-pmsh"
 1 files changed, 2 insertions(+), 2 deletions(-)
pmsh $ git push origin master
Username: 
Password: 
Counting objects: 5, done.
Delta compression using up to 2 threads.
Compressing objects: 100% (3/3), done.
Writing objects: 100% (3/3), 366 bytes, done.
Total 3 (delta 2), reused 0 (delta 0)
To https://github.com/pdsouza/pmsh.git
   358cd6e..321dfcd  master -> master
pmsh $ 
pmsh $ 
pmsh $ git status
# On branch master
# Untracked files:
#   (use "git add <file>..." to include in what will be committed)
#
#   job_handler.o
#   linked_list.o
#   parser.o
#   pmsh
#   tokenizer.o
nothing added to commit but untracked files present (use "git add" to track)
pmsh $ quit



maxmguo@spec29:~/pmsh> ./pmsh 
pmsh $ sleep 5 | sleep 5 | sleep 5 &
Running: sleep 5 | sleep 5 | sleep 5 &
pmsh $ jobs
[1] sleep 5 | sleep 5 | sleep 5 & (Running)
pmsh $ sleep 500&
Running: sleep 500&
Finished: sleep 5 | sleep 5 | sleep 5 &
pmsh $ jobs
[1] sleep 500& (Running)
pmsh $ sleep 600
^Z
Stopped: sleep 600
pmsh $ jobs
[1] sleep 500& (Running)
[2] sleep 600 (Stopped)
pmsh $ sleep 700&
Running: sleep 700&
pmsh $ jobs
[1] sleep 500& (Running)
[2] sleep 600 (Stopped)
[3] sleep 700& (Running)
pmsh $ fg
Restarting: sleep 700&
^Z
Stopped: sleep 700&
pmsh $ jobs
[1] sleep 500& (Running)
[2] sleep 600 (Stopped)
[3] sleep 700& (Stopped)
pmsh $ bg
Running: sleep 700&
pmsh $ jobs
[1] sleep 500& (Running)
[2] sleep 600 (Stopped)
[3] sleep 700& (Running)
pmsh $ bg
Running: sleep 600
pmsh $ jobs
[1] sleep 500& (Running)
[2] sleep 600 (Running)
[3] sleep 700& (Running)
pmsh $ fg 1
Restarting: sleep 500&
^Z
Stopped: sleep 500&
pmsh $ jobs
[1] sleep 500& (Stopped)
[2] sleep 600 (Running)
[3] sleep 700& (Running)
pmsh $ sleep 800 
^Z
Stopped: sleep 800
pmsh $ jobs
[1] sleep 500& (Stopped)
[2] sleep 600 (Running)
[3] sleep 700& (Running)
[4] sleep 800 (Stopped)
pmsh $ bg 1
Running: sleep 500&
pmsh $ jobs
[1] sleep 500& (Running)
[2] sleep 600 (Running)
[3] sleep 700& (Running)
[4] sleep 800 (Stopped)
pmsh $ quit



maxmguo@spec29:~/pmsh> ./pmsh 
pmsh $ cat < /proc/cpuinfo | head | tail | wc > test_out
pmsh $ cat test_out
     10      43     210
pmsh $ cat < /proc/cpuinfo | head | tail | grep model > test_out2
pmsh $ cat test_out2
model      : 107
model name : AMD Athlon(tm) 64 X2 Dual Core Processor 5600+
pmsh $ quit
```

## Caveats

1. When running a process in the foreground that takes some time to run (example: sleep 10), a normal terminal would buffer any new lines that the user inputs as the process is running. After the process finishes, the new lines translate into multiple prompts printed on multiple lines. However, our shell doesn't behave in this fashion. The multiple prompts would print, but would stay on one line.

2. Consecutive pipes can sometimes cause issues. Example, if you run '||', the program segfaults.

## TODOs

1. Add support for arrow keys, tab complete.

2. Make it an executable and stick in /bin, startup as default shell so we can use it to test and iron out bugs.

3. Add support for switching directories (cd command).

## Licensing

PMShell is licensed under the BSD 3-Clause License, see [LICENSE](https://github.com/pdsouza/pmsh/blob/master/LICENSE) for all the legal stuff.

## Authors

Preetam D'Souza (github: pdsouza)

Max Guo (github: SuperChocomocha)

https://github.com/pdsouza/pmsh
