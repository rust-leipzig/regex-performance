#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>

#include "main.h"
#include "version.h"

static char* data = NULL;
static int data_len = 0;

struct regex_imp {
    char * name;
    int score;
    int time;
    int inc_time;
    int (*find_all)(char* pattern, char* subject, int subject_len, int repeat);
};

static struct regex_imp regex [] = {
    {.name = "pcre",        .score = 0, .time = -1, .inc_time = 0, .find_all = pcre2_std_find_all},
    {.name = "pcre-dfa",    .score = 0, .time = -1, .inc_time = 0, .find_all = pcre2_dfa_find_all},
    {.name = "pcre-jit",    .score = 0, .time = -1, .inc_time = 0, .find_all = pcre2_jit_find_all},
    {.name = "re2",         .score = 0, .time = -1, .inc_time = 0, .find_all = re2_find_all},
    {.name = "onig",        .score = 0, .time = -1, .inc_time = 0, .find_all = onig_find_all},
    {.name = "tre",         .score = 0, .time = -1, .inc_time = 0, .find_all = tre_find_all},
    {.name = "hscan",       .score = 0, .time = -1, .inc_time = 0, .find_all = hs_find_all},
    {.name = "rust_regex",  .score = 0, .time = -1, .inc_time = 0, .find_all = rust_find_all},
};

void load(char const * file_name)
{
    int i;

    FILE* f;
    f = fopen(file_name, "rb");
    if (!f) {
        fprintf(stderr, "Cannot open '%s'!\n", file_name);
        return;
    }

    fseek(f, 0, SEEK_END);
    data_len = ftell(f);
    fseek(f, 0, SEEK_SET);

    data = (char*)malloc(data_len + 1);
    if (!data) {
        fprintf(stderr, "Cannot allocate memory!\n");
        fclose(f);
        return;
    }
    data[data_len] = '\0';
    fread(data, data_len, 1, f);
    fclose(f);

    for (i = 0; i < data_len; ++i)
        if (data[i] == '\r')
            data[i] = '\n';
    fprintf(stdout, "'%s' loaded. (Length: %d bytes)\n", file_name, data_len);
}

void find_all(char* pattern, char* subject, int subject_len, int repeat)
{
    int iter, top;
    fprintf(stdout, "-----------------\nRegex: '%s'\n", pattern);
    for (iter = 0; iter < sizeof(regex)/sizeof(regex[0]); iter++) {
        int ret = regex[iter].find_all(pattern, subject, subject_len, repeat);
        if (ret != -1) {
            regex[iter].time = ret;
            regex[iter].inc_time += ret;
        }
    }

    int score_points = 5;
    for (top = 0; top < score_points; top++) {
        int best = -1;

        for (iter = 0; iter < sizeof(regex)/sizeof(regex[0]); iter++) {
            if (regex[iter].time != -1 && (best == -1 || best > regex[iter].time)) {
                best = regex[iter].time;
            }
        }

        for (iter = 0; iter < sizeof(regex)/sizeof(regex[0]); iter++) {
            if (regex[iter].time != -1 && best == regex[iter].time) {
                regex[iter].score += score_points;
                regex[iter].time = -1;
            }
        }

        score_points--;
    }
}

void printResult(char * name, int time, int found)
{
    fprintf(stdout, "[%10s] time: %5d ms (%d matches)\n", name, (int)time, found);
    fflush(stdout);
}

int main(int argc, char **argv)
{
    char const * file = NULL;
    int repeat = 5;
    int c = 0;

    while ((c = getopt(argc, argv, "nhvf:")) != -1) {
        switch (c) {
            case 'f':
                file = optarg;
                break;
            case 'n':
                repeat = atoi(optarg);
                break;
            case 'v':
                printf("%s\n", VERSION_STRING);
                exit(EXIT_SUCCESS);
            case 'h':
                printf("Usage: %s [option] -f <file> \n\n", argv[0]);
                printf("Options:\n");
                printf("  -f\tInput file.\n");
                printf("  -n\tSet number of repetitions. Default: 5\n");
                printf("  -v\tGet the application version and build date.\n");
                printf("  -h\tPrint this help message\n\n");
                exit(EXIT_SUCCESS);
        }
    }

    if (file == NULL) {
        fprintf(stderr, "No input file given.\n");
        exit(EXIT_FAILURE);
    }

    load(file);
    if (data_len == 0) {
        exit(EXIT_FAILURE);
    }

    find_all("Twain", data, data_len, repeat);
    find_all("(?i)Twain", data, data_len, repeat);
    find_all("[a-z]shing", data, data_len, repeat);
    find_all("Huck[a-zA-Z]+|Saw[a-zA-Z]+", data, data_len, repeat);
    find_all("\\b\\w+nn\\b", data, data_len, repeat);
    find_all("[a-q][^u-z]{13}x", data, data_len, repeat);
    find_all("Tom|Sawyer|Huckleberry|Finn", data, data_len, repeat);
    find_all("(?i)Tom|Sawyer|Huckleberry|Finn", data, data_len, repeat);
    find_all(".{0,2}(Tom|Sawyer|Huckleberry|Finn)", data, data_len, repeat);
    find_all(".{2,4}(Tom|Sawyer|Huckleberry|Finn)", data, data_len, repeat);
    find_all("Tom.{10,25}river|river.{10,25}Tom", data, data_len, repeat);
    find_all("[a-zA-Z]+ing", data, data_len, repeat);
    find_all("\\s[a-zA-Z]{0,12}ing\\s", data, data_len, repeat);
    find_all("([A-Za-z]awyer|[A-Za-z]inn)\\s", data, data_len, repeat);
    find_all("[\"'][^\"']{0,30}[?!\\.][\"']", data, data_len, repeat);
    find_all("\u221E|\u2713", data, data_len, repeat);
    find_all("\\p{Sm}", data, data_len, repeat);

    int iter;
    fprintf(stdout, "-----------------\nScores:\n");
    for (iter = 0; iter < sizeof(regex)/sizeof(regex[0]); iter++) {
        fprintf(stdout, "%10s: %6u points, %6u ms\n", regex[iter].name, regex[iter].score, regex[iter].inc_time);
    }

    free(data);

    exit(EXIT_SUCCESS);
}
