#include <stdio.h>

#include "main.h"

#include <hs/hs.h>

static int found = 0;

static int eventHandler(UNUSED unsigned int  id,
                        UNUSED unsigned long long from,
                        UNUSED unsigned long long to,
                        UNUSED unsigned int flags,
                        UNUSED void * ctx) {
    found++;
    return 0;
}

int hs_find_all(char * pattern, char * subject, int subject_len, int repeat, struct result * res)
{
    TIME_TYPE start, end;

    hs_database_t * database;
    hs_compile_error_t * compile_err;
    if (hs_compile(pattern, HS_FLAG_DOTALL | HS_FLAG_MULTILINE | HS_FLAG_SOM_LEFTMOST, HS_MODE_BLOCK, NULL, &database, &compile_err) != HS_SUCCESS) {
        fprintf(stderr, "ERROR: Unable to compile pattern \"%s\": %s\n",
                pattern, compile_err->message);
        hs_free_compile_error(compile_err);
        return -1;
    }

    hs_scratch_t * scratch = NULL;
    if (hs_alloc_scratch(database, &scratch) != HS_SUCCESS) {
        fprintf(stderr, "ERROR: Unable to allocate scratch space. Exiting.\n");
        hs_free_database(database);
        return -1;
    }

    double * times = calloc(repeat, sizeof(double));
    int const times_len = repeat;

    do {
        found = 0;
        GET_TIME(start);
        if (hs_scan(database, subject, subject_len, 0, scratch, eventHandler, pattern) != HS_SUCCESS) {
            fprintf(stderr, "ERROR: Unable to scan input buffer. Exiting.\n");
            hs_free_scratch(scratch);
            hs_free_database(database);
            free(times);
            return -1;
        }
        GET_TIME(end);

        times[repeat - 1] = TIME_DIFF_IN_MS(start, end);
    } while (--repeat > 0);

    res->matches = found;
    get_mean_and_derivation(times, times_len, res);

    hs_free_scratch(scratch);
    hs_free_database(database);
    free(times);

    return 0;
}
