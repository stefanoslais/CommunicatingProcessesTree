#include "splitter_downstream_pipeinfo.h"

void DownstreamPipeInfo_array_initialize(DownstreamPipeInfo PI[], long filecursor_begin,long filecursor_end, long record_size, long sorter_children_num) {
    /*Number of records that correspond to this splitter proccess.*/
    long records_num = (filecursor_end - filecursor_begin)  / record_size;
    /*Number of records that correspond to each sorter process,
    except probably for the last one.*/
    long sorter_records_num = records_num / sorter_children_num;

    /*Initialize file cursors and bytes written*/
    for (int this_sorter=0; this_sorter < sorter_children_num; this_sorter++) {
        PI[this_sorter].filecursor_begin = filecursor_begin + this_sorter*sorter_records_num*record_size;
        PI[this_sorter].bytes_written = 0;
    }

    /*Initialize total amount of bytes for all pipes except for the last one*/
    for (int this_sorter = 0; this_sorter < sorter_children_num - 1; this_sorter++)
        PI[this_sorter].bytes_total = PI[this_sorter + 1].filecursor_begin - PI[this_sorter].filecursor_begin;
    
    /*Initialize total amount of bytes by the last pipe*/
    PI[sorter_children_num - 1].bytes_total = filecursor_end - PI[sorter_children_num - 1].filecursor_begin;

    
    return;
}

long DownstreamPipeInfo_getCursor(DownstreamPipeInfo* DPI) {
    long cursor = DPI->filecursor_begin + DPI->bytes_written;
    return cursor;
}

long DownstreamPipeInfo_getRemainingBytes(DownstreamPipeInfo* DPI) {
    long remaining_bytes = DPI->bytes_total - DPI->bytes_written;
    return remaining_bytes;
}