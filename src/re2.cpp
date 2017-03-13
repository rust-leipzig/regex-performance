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

extern "C" int re2_find_all(char* pattern, char* subject, int subject_len, int repeat)
{
    void * obj = get_re2_object(pattern);
    TIME_TYPE start, end, resolution;
    int found, time, best_time = 0;

    GET_TIME_RESOLUTION(resolution);

    if (!obj) {
        printf("RE2 compilation failed\n");
        return -1;
    }

    do {
        GET_TIME(start);
        found = search_all_re2(obj, subject, subject_len);
        GET_TIME(end);
        time = TIME_DIFF_IN_MS(start, end, resolution);
        if (!best_time || time < best_time)
            best_time = time;
    } while (--repeat > 0);
    printResult((char *) "re2", best_time, found);

    free_re2_object(obj);

    return best_time;
}
