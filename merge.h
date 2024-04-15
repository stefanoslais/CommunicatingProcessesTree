#ifndef __MERGE_H__
#define __MERGE_H__

struct Record_str;
struct PipeInfo_str;
void merge_sorted_arrays(struct Record_str* output, \
                        struct Record_str** recordsarrays,long recordsarrays_size,  \
                        struct PipeInfo_str* PI);







#endif