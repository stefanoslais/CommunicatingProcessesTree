SRCS_MYSORT = records.c helping_functions.c pipeinfo.c merge.c  mysort.c 
OBJS_MYSORT = $(SRCS_MYSORT:.c=.o)

SRCS_SPLITTER = records.c helping_functions.c pipeinfo.c splitter_downstream_pipeinfo.c merge.c splitter.c 
OBJS_SPLITTER = $(SRCS_SPLITTER:.c=.o)

SRCS_SORTER1 = records.c helping_functions.c shell_sort.c sorter1.c 
OBJS_SORTER1 = $(SRCS_SORTER1:.c=.o)

all:  sorter1 splitter mysort  

mysort: $(OBJS_MYSORT) splitter
	gcc -g -o mysort $(OBJS_MYSORT)

splitter: $(OBJS_SPLITTER) sorter1
	gcc -g -o splitter $(OBJS_SPLITTER)

sorter1: $(OBJS_SORTER1)
	gcc -g -o sorter1  $(OBJS_SORTER1)

records.o: records.c records.h
	gcc -g -c records.c -o records.o

helping_functions.o: helping_functions.h helping_functions.c pipeinfo.h
	gcc -g -c helping_functions.c -o helping_functions.o

pipeinfo.o: pipeinfo.c pipeinfo.h helping_functions.h
	gcc -g -c pipeinfo.c -o pipeinfo.o

shell_sort.o: shell_sort.c shell_sort.h records.h
	gcc -g -c shell_sort.c -o shell_sort.o

sorter1.o: sorter1.c records.h shell_sort.h helping_functions.h
	gcc -g -c sorter1.c -o sorter1.o

splitter_downstream_pipeinfo.o: splitter_downstream_pipeinfo.c splitter_downstream_pipeinfo.h
	gcc -g -c splitter_downstream_pipeinfo.c -o splitter_downstream_pipeinfo.o

merge.o: merge.c merge.h records.h helping_functions.h splitter_downstream_pipeinfo.h
	gcc -g -c merge.c -o merge.o

splitter.o: splitter.c records.h helping_functions.h splitter_downstream_pipeinfo.h merge.h
	gcc -g -c splitter.c -o splitter.o

mysort.o: mysort.c records.h helping_functions.h pipeinfo.h merge.h 
	gcc -g -c mysort.c -o mysort.o

clean:
	rm -f $(OBJS_MYSORT) $(OBJS_SPLITTER) $(OBJS_SORTER1) mysort splitter sorter1