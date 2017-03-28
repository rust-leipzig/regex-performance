#include <stdio.h>

#include "main.h"

#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2.h>

static int work_space[4096];

int pcre2_find_all(char* pattern, char* subject, int subject_len, int repeat, int mode, struct result * res)
{
    pcre2_code *re;
    pcre2_compile_context *comp_ctx;
    pcre2_match_data *match_data;
    pcre2_match_context *match_ctx;
    int err_code;
    PCRE2_SIZE err_offset;
    pcre2_jit_stack *stack = NULL;
    PCRE2_SIZE *ovector;
    char *ptr;
    int len;
    TIME_TYPE start = 0, end = 0;
    int found = 0;

    comp_ctx = pcre2_compile_context_create(NULL);
    if (!comp_ctx) {
        printf("PCRE2 cannot allocate compile context\n");
        return -1;
    }

    pcre2_set_newline(comp_ctx, PCRE2_NEWLINE_ANYCRLF);

    re = pcre2_compile(
        (PCRE2_SPTR8) pattern,    /* the pattern */
        PCRE2_ZERO_TERMINATED,    /* length */
        PCRE2_MULTILINE,    /* options */
        &err_code,        /* for error code */
        &err_offset,        /* for error offset */
        comp_ctx);        /* use default character tables */

    if (!re) {
        printf("PCRE2 compilation failed at offset %d: [%d]\n", (int)err_offset, err_code);
        return -1;
    }

    pcre2_compile_context_free(comp_ctx);

    match_ctx = pcre2_match_context_create(NULL);
    if (!match_ctx) {
        printf("PCRE JIT cannot allocate match context\n");
        return -1;
    }

    if (mode == 2) {
        if (pcre2_jit_compile(re, PCRE2_JIT_COMPLETE)) {
            printf("PCRE JIT compilation failed\n");
            return -1;
        }
        stack = pcre2_jit_stack_create(65536, 65536, NULL);
        if (!stack) {
            printf("PCRE JIT cannot allocate JIT stack\n");
            return -1;
        }
        pcre2_jit_stack_assign(match_ctx, NULL, stack);
    }

    if (mode == 1)
        match_data = pcre2_match_data_create(32, NULL);
    else
        match_data = pcre2_match_data_create_from_pattern(re, NULL);

    if (!match_data) {
        printf("PCRE2 cannot allocate match data\n");
        return -1;
    }

    ovector = pcre2_get_ovector_pointer(match_data);

    double * times = calloc(repeat, sizeof(double));
    int const times_len = repeat;

    do {
        found = 0;
        ptr = subject;
        len = subject_len;
        switch (mode) {
        case 0:
            GET_TIME(start);
            while (1) {
                err_code = pcre2_match(
                    re,            /* the compiled pattern */
                    (PCRE2_SPTR8) ptr,    /* the subject string */
                    len,            /* the length of the subject */
                    0,            /* start at offset 0 in the subject */
                    0,            /* default options */
                    match_data,        /* match data */
                    match_ctx);        /* match context */

                if (err_code <= 0) {
                    if (err_code == PCRE2_ERROR_NOMATCH)
                        break;
                    printf("PCRE pcre_exec failed with: %d\n", err_code);
                    break;
                }

                // printf("match: %d %d\n", (ptr - subject) + match[0], (ptr - subject) + match[1]);
                ptr += ovector[1];
                len -= ovector[1];
                found++;
            }
            GET_TIME(end);
            break;

        case 1:
            GET_TIME(start);
            while (1) {
                err_code = pcre2_dfa_match(
                    re,            /* the compiled pattern */
                    (PCRE2_SPTR8) ptr,    /* the subject string */
                    len,            /* the length of the subject */
                    0,            /* start at offset 0 in the subject */
                    0,            /* default options */
                    match_data,        /* match data */
                    match_ctx,        /* match context */
                    work_space,        /* work space */
                    4096);            /* number of elements (NOT size in bytes) */

                if (err_code <= 0) {
                    if (err_code == PCRE2_ERROR_NOMATCH)
                        break;
                    printf("PCRE pcre_exec failed with: %d\n", err_code);
                    break;
                }

                // printf("match: %d %d\n", (ptr - subject) + match[0], (ptr - subject) + match[1]);
                ptr += ovector[1];
                len -= ovector[1];
                found++;
            }
            GET_TIME(end);
            break;

        case 2:
            GET_TIME(start);
            while (1) {
                err_code = pcre2_jit_match(
                    re,            /* the compiled pattern */
                    (PCRE2_SPTR8) ptr,    /* the subject string */
                    len,            /* the length of the subject */
                    0,            /* start at offset 0 in the subject */
                    0,            /* default options */
                    match_data,        /* match data */
                    match_ctx);        /* match context */

                if (err_code <= 0) {
                    if (err_code == PCRE2_ERROR_NOMATCH)
                        break;
                    printf("PCRE pcre_exec failed with: %d\n", err_code);
                    break;
                }

                // printf("match: %d %d\n", (ptr - subject) + match[0], (ptr - subject) + match[1]);
                ptr += ovector[1];
                len -= ovector[1];
                found++;
            }
            GET_TIME(end);
            break;
        }

        times[repeat - 1] = TIME_DIFF_IN_MS(start, end);
    } while (--repeat > 0);

    res->matches = found;
    get_mean_and_derivation(times, times_len, res);

    if (stack)
        pcre2_jit_stack_free(stack);
    pcre2_match_context_free(match_ctx);
    pcre2_code_free(re);
    free(times);

    return 0;
}

int pcre2_std_find_all(char* pattern, char* subject, int subject_len, int repeat, struct result * res)
{
    return pcre2_find_all(pattern, subject, subject_len, repeat, 0, res);
}

int pcre2_dfa_find_all(char* pattern, char* subject, int subject_len, int repeat, struct result * res)
{
    return pcre2_find_all(pattern, subject, subject_len, repeat, 1, res);
}

int pcre2_jit_find_all(char* pattern, char* subject, int subject_len, int repeat, struct result * res)
{
    return pcre2_find_all(pattern, subject, subject_len, repeat, 2, res);
}
