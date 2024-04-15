#include "pipeinfo.h"
#include "helping_functions.h"

PipeInfo* PipeInfo_createArray(int arraysize) {
    PipeInfo* PI_arr = (PipeInfo*) mymalloc(    \
                        arraysize * sizeof(PipeInfo), \
                        "Not enough memory for PipeInfo");
    
    return PI_arr;
}

void Pipeinfo_initialize(PipeInfo* PI, long bytes_num) {
    PI->bytes_total = bytes_num;
    PI->bytes_read = 0;
    return;
}

