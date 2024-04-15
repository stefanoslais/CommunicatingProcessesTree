#ifndef __PIPE_INFO_H__
#define __PIPE_INFO_H__

typedef struct PipeInfo_str {
    /*Total bytes to be read eventually*/
    long bytes_total;
    /*Amount of bytes that have already been read*/
    long bytes_read;
}PipeInfo;

/*Creates an array that usefull pipe information*/
PipeInfo* PipeInfo_createArray(int arraysize);
/*Initializes an element of a PipeInfo array*/
void Pipeinfo_initialize(PipeInfo* PI, long bytes_num);





#endif