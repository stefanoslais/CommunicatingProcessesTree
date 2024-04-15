#include "records.h"
#include <stdio.h>
#include <string.h>

void record_print(Record* rec) {
    printf("%s %s %d  %s\n", rec->LastName, rec->FirstName, rec->custid, rec->postcode);
}

long toRecordBytes(long record_num) {
    return record_num * sizeof(Record);
}

long toRecordsNum(long bytes) {
    return bytes / sizeof(Record);
}

bool Record_isgreater(Record* rec1, Record* rec2) {
    /*Compare last names*/
    int result_LastName = strcmp(rec1->LastName, rec2->LastName);
    if (result_LastName > 0)    return true;
    else if (result_LastName < 0) return false;
    /*Compare first names if last names are the same*/
    int result_FirstName = strcmp(rec1->FirstName, rec2->FirstName);
    if (result_FirstName > 0)    return true;
    else if (result_FirstName < 0) return false;
    /*Compare customer id's if last and first names are the same*/
    if (rec1->custid > rec2->custid)    return true;
    else    return false;

    return false;

}
bool Record_isless(Record* rec1, Record* rec2) {
    /*Compare last names*/
    int result_LastName = strcmp(rec1->LastName, rec2->LastName);
    if (result_LastName < 0)    return true;
    else if (result_LastName > 0) return false;
    /*Compare first names if last names are the same*/
    int result_FirstName = strcmp(rec1->FirstName, rec2->FirstName);
    if (result_FirstName < 0)    return true;
    else if (result_FirstName > 0) return false;
    /*Compare customer id's if last and first names are the same*/
    if (rec1->custid < rec2->custid)    return true;
    else    return false;

    return false;

}
