#include "merge.h"
#include "records.h"
#include "helping_functions.h"
#include "pipeinfo.h"
#include "string.h"
#include <stdlib.h>
void merge_sorted_arrays(Record* output, Record** recordsarrays,long recordsarrays_size,PipeInfo* PI) {

    long* inner_index = (long*) mymalloc(    \
                            recordsarrays_size * sizeof(long), \
                            "Not enough memory for Result inner_index");
    for (int i = 0; i < recordsarrays_size; i++)   inner_index[i] = 0;

    long output_index = 0;
    while(1) {

    /*Find minimum*/
    long index_min;
    /*Find the first record to set as minimum*/
    for(index_min = 0; index_min < recordsarrays_size; index_min++)
        if (inner_index[index_min] != -1)   break;
    /*If all inner_index have been exhausted, the merging has been completed*/
    if (index_min == recordsarrays_size)    break;
    
    /*Find the actual minimum*/
    for(long i = 0; i < recordsarrays_size; i++) {
        if (inner_index[i] != -1 &&     \
            Record_isless(recordsarrays[i] + inner_index[i], recordsarrays[index_min] + inner_index[index_min])) {
            
            index_min = i;
        }
    }
    memcpy(&(output[output_index]),recordsarrays[index_min] + inner_index[index_min],sizeof(Record));
    output_index++;
    inner_index[index_min]++;
    if (inner_index[index_min] == toRecordsNum(PI[index_min].bytes_total))
        inner_index[index_min] = -1;

    }

    free(inner_index);
    return;

}