#ifndef __SPLITTER_DOWNSTREAN_PIPE_INFO_H__
#define __SPLITTER_DOWNSTREAN_PIPE_INFO_H__

/*Information regarding a downstream pipe from splitter to sorter process*/
typedef struct DownstreamPipeInfo_str {
    /*Point in the file from where pipe will start sending bytes*/
    long filecursor_begin;
    /*Total bytes to be written eventually*/
    long bytes_total;
    /*Amount of bytes that have already been written*/
    long bytes_written;
}DownstreamPipeInfo;

/**
 * @brief Initialize Information necessary for each pipe.
 * 
 * @param PI [out]  Array of Pipe Information
 * @param filecursor_begin  Bytes corresponding to this splitter start from here
 * @param filecursor_end Bytes corresponding to this splitter end here
 * @param record_size Size of a Record
 * @param sorter_children_num Total number of sorter children
 * 
 * @details
 * There is a one to one correlation between a downstream pipe and a sorter process.
 * All pipes send downstream the same amount of bytes except for the last one which might send more.
 */
void DownstreamPipeInfo_array_initialize(DownstreamPipeInfo PI[], long filecursor_begin,long filecursor_end, long record_size, long sorter_children_num);

long DownstreamPipeInfo_getCursor(DownstreamPipeInfo* DPI);
long DownstreamPipeInfo_getRemainingBytes(DownstreamPipeInfo* DPI);


#endif