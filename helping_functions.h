#ifndef __HELPING_FUNCTIONS_H__
#define __HELPING_FUNCTIONS_H__

#define WRITE   1
#define READ    0

#include <sys/types.h>
struct PipeInfo_str;
/*Create a double array of pipes*/
int** create_pipe_array(int arraysize);
void destroy_pipe_array(int** pipearray, int arraysize);

void* mymalloc(long total_bytes, const char*);
/*Create an array of pollfd's. Set them up for either reading or writting*/
struct pollfd* create_initialize_poll(int** pipearray, int arraysize, int end);
/*Create a double array of bytes. Each subarray corresponds to the results of a child*/
char** create_results(long arraysize, struct PipeInfo_str* PI);
void destroy_results(char** results, long arraysize);
/*Prepare execv arguments for splitter child process*/
char** execv_args_splitter(char* program, char* input,long filecursor_begin, long filecursor_end, int splitter, int splitters );
char** execv_args_sorter(char* program, int sorter,long bytes,pid_t root_pid);
/*Prepare execv arguments for sorter child process*/
void destroy_execv_args_splitter(char*** args);
void destroy_execv_args_sorter(char*** args);
/*perror and exit with EXIT_FAILURE*/
void exit_message(const char * message);
/*Close both ends of all pipes in the array, except for one*/
void close_pipes_except(int** pipes_array,int array_sz,int except);
#endif