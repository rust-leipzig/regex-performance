#include <stdio.h>
#include <string.h>

#include "main.h"

#include <oniguruma.h>

int onig_find_all(char* pattern, char* subject, int subject_len, int repeat, struct result * result)
{
	regex_t* reg;
	OnigRegion *region;
	TIME_TYPE start, end;
	unsigned char *ptr;
	int res, len, found = 0;

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

    double * times = calloc(repeat, sizeof(double));
    int const times_len = repeat;

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
        times[repeat - 1] = TIME_DIFF_IN_MS(start, end);
	} while (--repeat > 0);

	result->matches = found;
    get_mean_and_derivation(times, times_len, result);

	onig_region_free(region, 1);
	onig_free(reg);
    free(times);

	return 0;
}
