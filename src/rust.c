#include <stdio.h>

#include "main.h"

#include <rregex.h>

int rust_find_all(char * pattern, char * subject, int subject_len, int repeat, struct result * res)
{
    TIME_TYPE start, end;
    int found = 0;

    struct Regex const * regex_hdl = regex_new(pattern);
    if (regex_hdl == NULL) {
        fprintf(stderr, "ERROR: Unable to compile pattern \"%s\"\n", pattern);
        return -1;
    }

    double * times = calloc(repeat, sizeof(double));
    int const times_len = repeat;

    do {
        GET_TIME(start);
        found = regex_matches(regex_hdl, (uint8_t*) subject, subject_len);
        GET_TIME(end);

        times[repeat - 1] = TIME_DIFF_IN_MS(start, end);

    } while (--repeat > 0);

    res->matches = found;
    get_mean_and_derivation(times, times_len, res);

    regex_free(regex_hdl);
    free(times);

    return 0;
}
