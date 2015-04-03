CFLAGS=-g -Wall
CC=gcc
LIBS=tokenizer.o parser.o linked_list.o job_handler.o

pmsh : pmsh.c parser.o linked_list.o tokenizer.o job_handler.o
	$(CC) $(CFLAGS) $(LIBS) -o pmsh pmsh.c

parser : parser.c tokenizer.o
	$(CC) $(CFLAGS) $(LIBS) -c parser.c

linked_list : linked_list.c
	$(CC) $(CFLAGS) $(LIBS) -c linked_list.c

job_handler : job_handler.c
	$(CC) $(CFLAGS) $(LIBS) -c job_handler.c

tokenizer : tokenizer.c
	$(CC) $(CFLAGS) $(LIBS) -c tokenizer.c

parsertst : parser.o tokenizer.o
	$(CC) $(LIBS) -Wall -o ./tst/parsertst ./tst/parsertst.c
	./tst/parsertst

linked_list_tst : linked_list.o
	$(CC) $(LIBS) -Wall -o ./tst/linked_list_tst ./tst/linked_list_tst.c
	./tst/linked_list_tst

job_handler_tst : job_handler.o linked_list.o
	$(CC) $(LIBS) -Wall -o ./tst/job_handler_tst ./tst/job_handler_tst.c
	./tst/job_handler_tst

freetst : parser.o tokenizer.o linked_list.o
	$(CC) $(LIBS) -Wall -o ./tst/freetst ./tst/freetst.c

clean :
	rm -f *.o pmsh ./tst/parsertst ./tst/linked_list_tst ./tst/job_handler_tst ./tst/freetst
