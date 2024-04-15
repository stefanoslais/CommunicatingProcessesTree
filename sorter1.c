#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include "records.h"
#include "string.h"
#include "shell_sort.h"
#include "helping_functions.h"

int main(int argc, char* argv[]) {
if (argc != 4) {
    printf("Wrong number of arguments for sorter1\n");
    exit(EXIT_FAILURE);
}
int sorter_id = atoi(argv[1]);
/*Precise number of bytes the sorter process needs to read in total*/
long total_bytes = strtol(argv[2],NULL,10);
char* root_pid = strdup(argv[3]);
/*Precise number of records the sorter needs to read in total*/
long total_records = toRecordsNum(total_bytes);

/*Number of records the read() system call will try to read*/
long records_chunk;
if      ( total_records < 10)     records_chunk = 1;
else if (total_records <  100)    records_chunk = 10;
else if ( total_records < 1000)   records_chunk = 100;
else                            records_chunk = 1000;
/*Create a buffer to store read bytes*/
char* ReadBuffer = (char*) mymalloc(    \
                    toRecordBytes(records_chunk),   \
                    "Not enough memory for sorter read buffer");

/*Create an array to store Records*/
Record* Record_arr = (Record* )mymalloc(total_bytes,"Not enough memory to store Record array");

/*Number of records that have been read.*/
long records_read = 0;
while (records_read < total_records) {
    /*Temporary value representing the number of bytes read() will try to read in this iteration*/
    long max_number = 0;
    /*If there is a chunk to be read*/
    if ((total_records - records_read) / records_chunk > 0)
        max_number = toRecordBytes(records_chunk);
    else 
        max_number = toRecordBytes(total_records - records_read);

    /*Number of bytes that have been read*/
    long bytes_read = 0;
    /*Read the proper amount of bytes*/
    while (bytes_read < max_number) {
        long bytes_read_now = read(STDIN_FILENO,ReadBuffer + bytes_read,max_number - bytes_read);
        if (bytes_read_now < 0)
			exit_message("error in sorter reading bytes");
        bytes_read += bytes_read_now;
    }
    /*Store the read bytes to the Record's array*/
    memcpy(&(Record_arr[records_read]),ReadBuffer,max_number);
    records_read += records_chunk;
    
}
shell_sort(Record_arr,total_records);

free(ReadBuffer);

/*Write results to splitter proces*/
long bytes_written = 0;
while (bytes_written < total_bytes) {
    long remaining_bytes = total_bytes - bytes_written;
    int bytes_written_now = write(STDOUT_FILENO,Record_arr + bytes_written,remaining_bytes);
    if (bytes_written_now < 0)
		exit_message("error in sorter writting");
    bytes_written += bytes_written_now;
}




free(root_pid);
free(Record_arr);
exit(EXIT_SUCCESS);
}