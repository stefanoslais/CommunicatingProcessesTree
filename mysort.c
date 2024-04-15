#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <poll.h>
#include <fcntl.h>
#include <errno.h>
#include "records.h"
#include "helping_functions.h"
#include "pipeinfo.h"
#include "merge.h"
#include <signal.h>
#include <string.h>
/*About Signal handling...*/

static int USR1_timesCatched = 0;
void catchUSR1(int signo) {
    USR1_timesCatched++;
}

int main(int argc, char* argv[]){
	
/*User input must be in format: ./mysort -i DataFile -k NumofChildren -e1 sorter1 */
/* Check arguments*/
if (argc != 7 )	exit_message("wrong number of arguments");
if ( strcmp(argv[6],"sorter1") != 0 ) exit_message("First sorting algorithm must be 'sorter1'");
char* DataFile_Name = argv[2];
int splitters_total = atoi(argv[4]);
pid_t* splitterChildren = (pid_t*) mymalloc(  \
                        splitters_total * sizeof(pid_t),  \
                        "Not enough memory to store splitter pid's");

/*Open data file*/
FILE* DataFile = fopen(DataFile_Name,"rb");
if(DataFile == NULL)	exit_message("error opening binary file"); 
/*Setting up sigactions*/
/*Set a mask with USR1*/
sigset_t mask;
sigemptyset(&mask);
sigaddset(&mask,SIGUSR1);
sigaddset(&mask,SIGUSR2);
/*Handle SIGUSR1*/
struct sigaction handleUSR1;
handleUSR1.sa_handler = catchUSR1;
handleUSR1.sa_mask = mask;
handleUSR1.sa_flags = 0;
if (sigaction(SIGUSR1,&handleUSR1,NULL)  == -1) exit_message("Sigaction for SIGUSR1 failed");
/*Find the total number of bytes and records*/
fseek(DataFile , 0 , SEEK_END);
long filecursor_end = ftell(DataFile);
long total_bytes = filecursor_end;
int records_total = toRecordsNum(filecursor_end);

/*Number of records given to each splitter child.
Last splitter child might be given more records*/
int splitted_records = records_total/splitters_total;

long filecursor_begin=0;
filecursor_end=0;
rewind(DataFile);

/*An array of pipes between sorter and splitters. The direction of information is upwards*/
int** UpstreamSplitter = create_pipe_array(splitters_total);
PipeInfo* UpstreamInfo = PipeInfo_createArray(splitters_total);
struct pollfd* readingSet = create_initialize_poll(UpstreamSplitter,splitters_total,READ);
for (int pipe = 0; pipe < splitters_total; pipe++) {
    /*Set the reading ends of the upstream pipes in non-blockin mode*/
    if(fcntl(UpstreamSplitter[pipe][READ],F_SETFL,O_NONBLOCK))
		exit_message("error in setting non blocking mode to pipes in splitter");
}

/*Distribute to each splitter child their corresponding beginning and ending  file cursors*/
for (int splitter = 0; splitter < splitters_total; splitter++) {
    filecursor_begin = ftell(DataFile);
    /*About the last splitter child*/
    if (splitter == splitters_total -1) {
        fseek(DataFile,0,SEEK_END);
    }
    /*About the rest of the children*/
    else {
        fseek(DataFile,splitted_records * sizeof(Record), SEEK_CUR);
    }
    filecursor_end = ftell(DataFile);
    Pipeinfo_initialize(&(UpstreamInfo[splitter]),filecursor_end - filecursor_begin);
	
    /*Parent prepares parametres for later child execv call*/ 
	char** args = execv_args_splitter("splitter",DataFile_Name,filecursor_begin,filecursor_end,splitter,splitters_total);

	/*Spawn splitter child*/
    splitterChildren[splitter] = fork();
    if (splitterChildren[splitter] ==  0) {
        /*Child needs to close all other pipes except the one it will use*/
        close_pipes_except(UpstreamSplitter,splitters_total,splitter);
		
        /*Child will only be writting to upstream pipe*/
        close(UpstreamSplitter[splitter][READ]);
        dup2(UpstreamSplitter[splitter][WRITE],STDOUT_FILENO);
        close(UpstreamSplitter[splitter][WRITE]);
        /*Ignore SIGUSR1 signals by setting a new sigaction*/
        struct sigaction doNothing;
        doNothing.sa_handler = SIG_IGN;
        sigemptyset(&doNothing.sa_mask);
        doNothing.sa_flags=0;
        sigaction(SIGUSR1,&doNothing,NULL);
        
        /*Ready to exec*/
        execv("splitter", args);
    }
    else if( splitterChildren[splitter] < 0) exit_message("error in creating splitter child");

    /*About parent*/
    /*Parent will only be reading from this pipe*/
    close(UpstreamSplitter[splitter][WRITE]);
	/*Parent does no longer needs the execv parametres*/
	destroy_execv_args_splitter(&args);
}

/*File cursors have now been distributed, sorter no longer needs the file*/
fclose(DataFile);

/*Create a double array of the sorted results (by each splitter)*/
char** ReadResults = create_results(splitters_total,UpstreamInfo);

int remaining_read_pipes = splitters_total;
/*End while loop when all information has been read from the pipes*/
while(remaining_read_pipes > 0) {
    int mysort_ready_reading = poll(readingSet,splitters_total,1000);
    if (mysort_ready_reading == -1) {
        if (errno == EINTR) continue;
        else exit_message("reading poll in mysort"); 
    }
    if (mysort_ready_reading == 0) {
        continue;
    }
    /*Some read end(s) are ready*/
    for (int pipe = 0; pipe < splitters_total; pipe++) {
        if (!(readingSet[pipe].revents)) continue;
        /*Prepare for reading...*/
        long remaining_bytes = UpstreamInfo[pipe].bytes_total - UpstreamInfo[pipe].bytes_read;

        long bytes_read_now = read(readingSet[pipe].fd,   \
                                    ReadResults[pipe] + UpstreamInfo[pipe].bytes_read,    \
                                    remaining_bytes);
									
        if (bytes_read_now == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) 
            /*The pipe is empty or a signal occured, continue with the next pipe*/
                continue;
            else
				exit_message("Reading upstream splitter pipe");
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

Record* sortedRecords = (Record*) mymalloc(total_bytes,"Not enough memory for sorted array");
merge_sorted_arrays(sortedRecords, (Record**) ReadResults, splitters_total, UpstreamInfo);

for (int record = 0; record < records_total; record++) {
    record_print(&(sortedRecords[record]));
}

printf("-->SIGUSR1 was called %d times\n",USR1_timesCatched);
free(sortedRecords);

for (int child=0; child < splitters_total; child++) {
    int status;
    pid_t  child_pid = wait(&status);
    if (!WIFEXITED(status)) 
        fprintf(stderr, "Child %d did NOT exit successfully\n",child_pid);
}

destroy_results(ReadResults,splitters_total);
destroy_pipe_array(UpstreamSplitter,splitters_total);
free(UpstreamInfo);
free(readingSet);
free(splitterChildren);

return 0;
}