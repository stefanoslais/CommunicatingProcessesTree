#include "helping_functions.h"
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <poll.h>
#include "pipeinfo.h"
#include <string.h>

int** create_pipe_array(int array_sz) {
    int** pipe_array = (int**) malloc(array_sz * sizeof(int*));
    if (pipe_array == NULL)
		exit_message("Not enough memory for pipe array");
	
    for (int i = 0; i < array_sz; i++) {
        pipe_array[i] = malloc(2*sizeof(int));
        if (pipe_array[i] == NULL)
			exit_message("Not enough memory for this pipe");
		
        if (pipe(pipe_array[i]) == -1)
			exit_message("error in creating pipe");
    }
    return pipe_array;
}

void destroy_pipe_array(int** pipe_array, int array_sz) {
    for (int i = 0; i < array_sz; i++)
        free(pipe_array[i]);
    free(pipe_array);

    return;
}

void* mymalloc(long total_bytes, const char* error_message) {
    void* buffer = malloc(total_bytes);
    if (buffer == NULL) {
        exit_message(error_message);
    }
    return buffer;
}

struct pollfd* create_initialize_poll(int** pipe_array, int array_sz, int end) {
    struct  pollfd* set = (struct  pollfd*) mymalloc( \
                                array_sz * sizeof(struct pollfd), \
                                "Not enough memory for poll");
    
    short event;
    switch(end) {
        case WRITE: event = POLLOUT;    break;
        case READ:  event = POLLIN; break;
        default:    event = 0;  break;    
    }
    for (int i = 0; i < array_sz; i++) {
        set[i].fd = pipe_array[i][end];
        set[i].events = event;
        set[i].revents = 0;
    }
    return set;
}


char** create_results(long array_sz, PipeInfo* PI) {
    char** results = (char**) mymalloc(array_sz*sizeof(char*),"Not enough memory for results");

    for (int child = 0; child < array_sz; child++)
        results[child] = (char*) mymalloc(PI[child].bytes_total,"Not enough memory for results");

    return results;
}

void destroy_results(char** results, long array_sz) {
    for (long i = 0; i < array_sz; i++)    free(results[i]);
    free(results);
}

char* stringify(long number) {
	/*Calculate how many bytes are needed for the parameter to be stringified.*/
	long length = snprintf(NULL,0,"%ld",number);
	char* number_str = (char*)mymalloc((length + 1) * sizeof(char),"Not enough space to stringify parametr");
	/*Copy contents of parametre to the string buffer*/
	snprintf(number_str, length + 1, "%ld", number);
	
	return number_str;
}

char** execv_args_splitter(char* program, char* input,long filecursor_begin, long filecursor_end, int splitter, int splitters ) {
	
	
	char* filecursor_begin_str = stringify(filecursor_begin);
	char* filecursor_end_str = stringify(filecursor_end);
	char* splitter_str = stringify((long)splitter);
	char* splitters_str = stringify((long)splitters);
	
	char** args = mymalloc(7 * sizeof(char*), "Not enough space for args array");
	args[0] = strdup(program);
	args[1] = strdup(input);
	args[2] = filecursor_begin_str;
	args[3] = filecursor_end_str;
	args[4] = splitter_str;
	args[5] = splitters_str;
	args[6] = (char*) NULL;
	
	return args;
}

void destroy_execv_args_splitter(char*** args){
	char** exec_args = *args;
	free(exec_args[0]);
	free(exec_args[1]);
	free(exec_args[2]);
	free(exec_args[3]);
	free(exec_args[4]);
	free(exec_args[5]);
	free(*args);
}
char** execv_args_sorter(char* program, int sorter,long bytes,pid_t root_pid) {
	char* program_str = strdup(program);
	char* sorter_str = stringify((long)sorter);
	char* bytes_str = stringify(bytes);
	char* pid_t_str = stringify((long)root_pid);
	char** args = mymalloc(5 * sizeof(char*),"Not enought space for args array");
	args[0] = program_str;
	args[1] = sorter_str;
	args[2] = bytes_str;
	args[3] = pid_t_str;
	args[4] = (char*) NULL;
	return args;
}

void destroy_execv_args_sorter(char*** args){
	char** exec_args = *args;
	free(exec_args[0]);
	free(exec_args[1]);
	free(exec_args[2]);
	free(exec_args[3]);
	free(*args);
}

void exit_message(const char * message) {
	perror(message);
	exit(EXIT_FAILURE);
}


void close_pipes_except(int** pipes_array,int array_sz,int except) {
	for (int i = 0; i < array_sz; i++) {
            if (i == except)  continue;
            close(pipes_array[i][READ]);
            close(pipes_array[i][WRITE]);
    }
}
	