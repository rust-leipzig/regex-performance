#include <stdio.h>

#include "main.h"

#include <rregex.h>

int rust_find_all(char * pattern, char * subject, int subject_len, int repeat)
{
    TIME_TYPE start, end, resolution;
    int time, best_time = 0;
    int found = 0;

    struct Regex const * regex_hdl = regex_new(pattern);
    if (regex_hdl == NULL) {
        fprintf(stderr, "ERROR: Unable to compile pattern \"%s\"\n", pattern);
        return -1;
    }

    GET_TIME_RESOLUTION(resolution);

    do {
        GET_TIME(start);
        found = regex_matches(regex_hdl, (uint8_t*) subject, subject_len);
        GET_TIME(end);

        time = TIME_DIFF_IN_MS(start, end, resolution);
        if (!best_time || time < best_time) {
            best_time = time;
        }
    } while (--repeat > 0);

    printResult("rust_regex", best_time, found);

    regex_free(regex_hdl);

    return best_time;
}
