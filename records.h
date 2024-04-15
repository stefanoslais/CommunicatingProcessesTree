#ifndef __RECORDS_H__
#define __RECORDS_H__

#include <stdbool.h>

typedef struct Record_str{
	int  	custid;
	char 	LastName[20];
	char 	FirstName[20];
	char	postcode[6];
}Record;

void record_print(Record* rec);
/*Translate a number of records to its corresponding amount of bytes*/
long toRecordBytes(long record_num);
/*Translate a number of bytes to its corresponding amount of Records*/
long toRecordsNum(long bytes);
/**
 * @brief Perform rec1 > rec2
 * @param rec1 Pointer to rec1
 * @param rec2 Pointer to rec2
 * @return True if rec1 > rec2 , otherwise return false
 */

/*Perform rec1 > rec2*/
bool Record_isgreater(Record* rec1, Record* rec2);
/*Perform rec1 < rec2*/
bool Record_isless(Record* rec1, Record* rec2);



#endif