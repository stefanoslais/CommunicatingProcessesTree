#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include "records.h"
#include <fcntl.h>
#include <sys/poll.h>
#include <errno.h>
#include <sys/wait.h>
#include <signal.h>
#include "splitter_downstream_pipeinfo.h"
#include "helping_functions.h"
#include "pipeinfo.h"
#include "merge.h"


int main(int argc, char* argv[]) {

/*Check and Parse parametres*/
if (argc != 6) exit_message("wrong number of arguments");
FILE* DataFile = fopen(argv[1],"rb");
long filecursor_begin = strtol(argv[2],NULL,10);
long filecursor_end = strtol(argv[3],NULL,10);
int splitter_id = atoi(argv[4]);
int splitters_total = atoi(argv[5]);
long bytes_total = filecursor_end - filecursor_begin;

int sorter_children_total = splitters_total-splitter_id;
/*Create an array of pipes for streaming information from splitter to sorter*/
int** downstreamSplitter = create_pipe_array(sorter_children_total);
/*Create an array of pipes for streaming information from sorter to splitter*/
int** upstreamSorter = create_pipe_array(sorter_children_total);
for (int pipe=0; pipe < sorter_children_total; pipe++) {
    /*Set the writting ends of the downstream pipes in non-blockin mode*/
    int nonBlocking_WriteEnd = fcntl(downstreamSplitter[pipe][WRITE],F_SETFL,O_NONBLOCK);
    /*Set the reading ends of the sorter upstream pipes in non-blockin mode*/
    int nonBlocking_ReadEnd = fcntl(upstreamSorter[pipe][READ],F_SETFL,O_NONBLOCK);
    if ( nonBlocking_ReadEnd < 0 || nonBlocking_WriteEnd < 0)
		exit_message("error in setting non blocking mode to pipes in splitter");
}

DownstreamPipeInfo* DownstreamInfo = (DownstreamPipeInfo*) mymalloc(  \
                                    sorter_children_total * sizeof(DownstreamPipeInfo),   \
                                    "Not enough memory for splitter downstream pipe infos");
DownstreamPipeInfo_array_initialize(DownstreamInfo, filecursor_begin,filecursor_end, sizeof(Record), sorter_children_total);

pid_t* sorterChildren = (pid_t*) mymalloc(    \
                        sorter_children_total * sizeof(pid_t),    \
                        "Not enough memory for sorter pid's");

for (int sorter=0; sorter < sorter_children_total; sorter++) {

	/*Parent prepares parametres for later child execv call*/ 
	char** args = execv_args_sorter("sorting1", sorter,DownstreamInfo[sorter].bytes_total,getppid());
    /*Spawn sorter child*/
    sorterChildren[sorter] = fork();
    /*About sorter child*/
    if (sorterChildren[sorter] == 0) {
        /*Child needs to close all other pipes except the one it will use*/
		close_pipes_except(downstreamSplitter,sorter_children_total,sorter);
		close_pipes_except(upstreamSorter,sorter_children_total,sorter);

        /*Child will only be reading from downstream pipe*/
        close(downstreamSplitter[sorter][WRITE]);
        dup2(downstreamSplitter[sorter][READ],STDIN_FILENO);
        close(downstreamSplitter[sorter][READ]);
        /*Child will only be writting to upstream pipe*/
        close(upstreamSorter[sorter][READ]);
        dup2(upstreamSorter[sorter][WRITE],STDOUT_FILENO);
        close(upstreamSorter[sorter][WRITE]);

        
        execv("sorting1",args);

    }
    else if (sorterChildren[sorter] < 0)
		exit_message("error in creating sorter child");
	
    /*About parent...*/
    /*Parent will only be writting to downstream pipe*/
    close(downstreamSplitter[sorter][READ]);
    /*Parent will only be reading from upstream pipe*/
    close(upstreamSorter[sorter][WRITE]);
	/*Parent does no longer needs the execv parametres*/
	destroy_execv_args_sorter(&args);
}

struct pollfd* writtingSet = create_initialize_poll(downstreamSplitter,sorter_children_total,WRITE);
struct pollfd* readingSet = create_initialize_poll(upstreamSorter,sorter_children_total,READ);
char* WriteBuffer = (char*) mymalloc(   \
                    DownstreamInfo[sorter_children_total - 1].bytes_total,   \
                    "Not enough memory for sorter reading buffer");

int remaining_write_pipes = sorter_children_total;

while(remaining_write_pipes > 0) {
    int sorter_ready_writting = poll(writtingSet,sorter_children_total,100);
    if (sorter_ready_writting == -1)
		exit_message("writting poll in splitter");
    if (sorter_ready_writting == 0)
        continue;
    
    /*Some write end(s) are ready*/
    for (int pipe = 0; pipe < sorter_children_total; pipe++) {
        if (!(writtingSet[pipe].revents)) continue;
        /*Prepare for writting...*/
        long move_cursor_to = DownstreamPipeInfo_getCursor(&(DownstreamInfo[pipe]));
        fseek(DataFile, move_cursor_to, SEEK_SET);
        long remaining_bytes = DownstreamPipeInfo_getRemainingBytes(&(DownstreamInfo[pipe]));
		/*Splitter write data from DataFile to pipe for Sorter to receive.*/
        fread(WriteBuffer, remaining_bytes, 1, DataFile);
        long bytes_writter_now = write(writtingSet[pipe].fd,WriteBuffer,remaining_bytes);
        if (bytes_writter_now == -1) {
            
            if (errno == EAGAIN || errno == EWOULDBLOCK) 
            // The pipe is full, skip this iteration and continue with the next pipe
                continue;
            else
				exit_message("Writting downstream splitter pipe");
        }
        DownstreamInfo[pipe].bytes_written += bytes_writter_now;
        if (DownstreamInfo[pipe].bytes_written == DownstreamInfo[pipe].bytes_total) {
            close(writtingSet[pipe].fd);
            writtingSet[pipe].fd = -1;
            writtingSet[pipe].events = 0;
            remaining_write_pipes--;
        }

    }

}

PipeInfo* UpstreamInfo = PipeInfo_createArray(sorter_children_total);
for (int pipe = 0; pipe < sorter_children_total; pipe++)
    Pipeinfo_initialize(&(UpstreamInfo[pipe]), DownstreamInfo[pipe].bytes_total);

/*Create a double array of the sorted results (by each sorter)*/
char** ReadResults = create_results(sorter_children_total,UpstreamInfo);

int remaining_read_pipes = sorter_children_total;

while(remaining_read_pipes > 0) {
    int sorter_ready_reading = poll(readingSet,sorter_children_total,100);
    if (sorter_ready_reading == -1) 
		exit_message("reading poll in splitter");
    if (sorter_ready_reading == 0) 
        continue;
	
    /*Some read end(s) are ready*/
    for (int pipe = 0; pipe < sorter_children_total; pipe++) {
        if (!(readingSet[pipe].revents)) continue;
        /*Prepare for reading...*/
        long remaining_bytes = UpstreamInfo[pipe].bytes_total - UpstreamInfo[pipe].bytes_read;

        long bytes_read_now = read(readingSet[pipe].fd,   \
                                    ReadResults[pipe] + UpstreamInfo[pipe].bytes_read,    \
                                    remaining_bytes);
        if (bytes_read_now == -1) {
            
            if (errno == EAGAIN || errno == EWOULDBLOCK) 
            // The pipe is empty, skip this iteration and continue with the next pipe
                continue;
            else
				exit_message("Reading upstream sorter pipe");
        }
		
        UpstreamInfo[pipe].bytes_read += bytes_read_now;
        if (UpstreamInfo[pipe].bytes_read == UpstreamInfo[pipe].bytes_total) {
            close(readingSet[pipe].fd);
            readingSet[pipe].fd = -1;
            readingSet[pipe].events = 0;
            remaining_read_pipes--;
        }

    }
	
}


Record* sortedRecords = (Record*) mymalloc(bytes_total,"Not enough memory for sorted array");
merge_sorted_arrays(sortedRecords, (Record**) ReadResults, sorter_children_total, UpstreamInfo);

/*Write to mysort*/
long bytes_written = 0;
while(bytes_written < bytes_total) {
    long bytes_written_now;
    bytes_written_now = write(STDOUT_FILENO,sortedRecords,bytes_total - bytes_written);
    if (bytes_written_now < 0)
		exit_message("error in splitter writting to mysort");
    bytes_written += bytes_written_now;
}


/*free() anything concerning the writting procedure*/
destroy_pipe_array(downstreamSplitter,sorter_children_total);

free(DownstreamInfo);
free(writtingSet);
free(WriteBuffer);

/*free() anything concerning the reading procedure*/
destroy_pipe_array(upstreamSorter,sorter_children_total);
free(readingSet);
for (int pipe = 0; pipe < sorter_children_total; pipe++)
    free(ReadResults[pipe]);
free(ReadResults);

free(UpstreamInfo);
free(sortedRecords);


for (int child=0; child < sorter_children_total; child++) {
    int status;
    pid_t  child_pid = wait(&status);
    if (!WIFEXITED(status)) 
        fprintf(stderr,"Child %d did NOT exit successfully\n",child_pid);
}

free(sorterChildren);

fclose(DataFile);

if (kill(getppid(),SIGUSR1) == -1) {
    perror("kill in splitter");
    exit(EXIT_FAILURE);
}


exit(EXIT_SUCCESS);

return 0;
}
