#include <stdio.h>

#include "main.h"

#include <tre/tre.h>

int tre_find_all(char* pattern, char* subject, int subject_len, int repeat, struct result * res)
{
	int err_val;
	regex_t regex;
	regmatch_t match[1];
	char *ptr;
	int len;
	TIME_TYPE start, end;
	int found = 0;

	err_val = tre_regcomp(&regex, pattern, REG_EXTENDED | REG_NEWLINE);
	if (err_val != 0) {
		printf("TRE compilation failed with error %d\n", err_val);
		return -1;
	}

    double * times = calloc(repeat, sizeof(double));
    int const times_len = repeat;

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

        times[repeat - 1] = TIME_DIFF_IN_MS(start, end);
	} while (--repeat > 0);

    res->matches = found;
    get_mean_and_derivation(times, times_len, res);

	tre_regfree(&regex);
    free(times);

    return 0;
}
