#include <stdio.h>

#include "main.h"

#include <tre/tre.h>

int tre_find_all(char* pattern, char* subject, int subject_len, int repeat)
{
	int err_val;
	regex_t regex;
	regmatch_t match[1];
	char *ptr;
	int len;
	TIME_TYPE start, end, resolution;
	int found, time, best_time = 0;

	GET_TIME_RESOLUTION(resolution);

	err_val = tre_regcomp(&regex, pattern, REG_EXTENDED | REG_NEWLINE);
	if (err_val != 0) {
		printf("TRE compilation failed with error %d\n", err_val);
		return -1;
	}

	do {
		found = 0;
		ptr = subject;
		len = subject_len;
		GET_TIME(start);
		while (1) {
			err_val = tre_regnexec(&regex, ptr, len, 1, match, 0);
			if (err_val != 0)
				break;

			// printf("match: %d %d\n", (ptr - subject) + match[0].rm_so, (ptr - subject) + match[0].rm_eo);
			found++;
			ptr += match[0].rm_eo;
			len -= match[0].rm_eo;
		}
		GET_TIME(end);
		time = TIME_DIFF_IN_MS(start, end, resolution);
		if (!best_time || time < best_time)
			best_time = time;
	} while (--repeat > 0);
	printResult("tre", best_time, found);

	tre_regfree(&regex);

    return best_time;
}
