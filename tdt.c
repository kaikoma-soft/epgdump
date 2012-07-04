#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tdt.h"

void dumpTDT(unsigned char *ptr)
{

	int		rc ;

	int len = 0;
	int loop_len = 0;
	int boff;
	int table_id,section_length,wk;
	char JST_time[6];

	boff = 0;
	table_id = getBit(ptr, &boff, 8);
	if (table_id != 0x70 && table_id != 0x73) return;
	wk = getBit(ptr, &boff, 4);
	section_length = getBit(ptr,&boff,12);
	memcpy(JST_time, ptr + (boff / 8),5);
        boff += 40;

#ifdef DEBUG
	printf("Table %x jst [%s] diff [%f]\n",table_id,strTime(getStartTime(JST_time),"%Y/%m/%d %H:%M:%S"),getTimeDiff(getStartTime(JST_time)));
#endif

	return;
}
