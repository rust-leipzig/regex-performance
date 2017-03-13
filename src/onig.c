#include <stdio.h>
#include <string.h>

#include "main.h"

#include <oniguruma.h>

int onig_find_all(char* pattern, char* subject, int subject_len, int repeat)
{
	regex_t* reg;
	OnigRegion *region;
	TIME_TYPE start, end, resolution;
	unsigned char *ptr;
	int res, len, found, time, best_time = 0;

	GET_TIME_RESOLUTION(resolution);

	res = onig_new(&reg, (unsigned char *)pattern, (unsigned char *)pattern + strlen((char* )pattern),
		ONIG_OPTION_DEFAULT, ONIG_ENCODING_ASCII, ONIG_SYNTAX_DEFAULT, NULL);
	if (res != ONIG_NORMAL) {
		printf("Onig compilation failed\n");
		return -1;
	}
	region = onig_region_new();
	if (!region) {
		printf("Cannot allocate region\n");
		return -1;
	}

	do {
		found = 0;
		ptr = (unsigned char *)subject;
		len = subject_len;

		GET_TIME(start);
		while (1) {
			res = onig_search(reg, ptr, ptr + len, ptr, ptr + len, region, ONIG_OPTION_NONE);
			if (res < 0)
				break;
			// printf("match: %d %d\n", (ptr - (unsigned char *)subject) + region->beg[0], (ptr - (unsigned char *)subject) + region->end[0]);
			ptr += region->end[0];
			len -= region->end[0];
			found++;
		}
		GET_TIME(end);
		time = TIME_DIFF_IN_MS(start, end, resolution);
		if (!best_time || time < best_time)
			best_time = time;
	} while (--repeat > 0);
	printResult("onig", best_time, found);

	onig_region_free(region, 1);
	onig_free(reg);

	return best_time;
}
