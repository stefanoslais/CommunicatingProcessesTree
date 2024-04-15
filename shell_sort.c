#include "shell_sort.h"
#include "records.h"
#include <string.h>

void shell_sort(Record* recarr, long arraysize) {
    /*Typical start for gap value is hald the array size*/
    for (long gap = arraysize/2; gap > 0; gap=gap/2) {
        for (long i = gap; i < arraysize; i++) {
            /*Pick Record to the right*/
            Record recR = recarr[i];
            long j;
            /*Pich Record to the left, make sure those 2 elemet have gap distance*/
            for (j = i; j >= gap && Record_isgreater(&(recarr[j-gap]),&recR); j -= gap)
            /*Move to the right is left elemet is greater that the right one*/
                    recarr[j] = recarr[j-gap];
            recarr[j] = recR;
        }
    }   
}
