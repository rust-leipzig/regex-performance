#include <stdio.h>
#include <string.h>

#include "main.h"

#include <re2/re2.h>
#include <re2/stringpiece.h>

static void * get_re2_object(char* pattern)
{
    RE2 * obj;

    obj = new RE2(pattern, RE2::Latin1);
    if (!obj->ok()) {
        printf("pattern error!\n");
        delete obj;
        obj = NULL;
    }
    return obj;
}

static void free_re2_object(void* obj)
{
    delete (RE2*)obj;
}

static int search_all_re2(void* obj, char* subject, int subject_len)
{
    re2::StringPiece input(subject, subject_len);
    //re2::StringPiece result;
    int found = 0;

    while (RE2::FindAndConsume(&input, *(RE2*)obj)) {
        // printf("match: %d %d @%d\n", result.data() - subject, result.size(), input.data() - subject);
        found++;
    }
    return found;
}

extern "C" int re2_find_all(char* pattern, char* subject, int subject_len, int repeat, struct result * res)
{
    void * obj = get_re2_object(pattern);
    TIME_TYPE start, end = 0;
    int found = 0;

    if (!obj) {
        printf("RE2 compilation failed\n");
        return -1;
    }

    double * times = (double*) std::calloc(repeat, sizeof(double));
    int const times_len = repeat;

    do {
        GET_TIME(start);
        found = search_all_re2(obj, subject, subject_len);
        GET_TIME(end);
        times[repeat - 1] = TIME_DIFF_IN_MS(start, end);

    } while (--repeat > 0);


    res->matches = found;
    get_mean_and_derivation(times, times_len, res);

    free_re2_object(obj);
    free(times);

    return 0;
}
