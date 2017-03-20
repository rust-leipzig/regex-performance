#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>

#include "main.h"
#include "version.h"

static char* data = NULL;
static int data_len = 0;

struct result {
    int score;
    int time;
};

struct engines {
    char * name;
    int (*find_all)(char* pattern, char* subject, int subject_len, int repeat);
};

static struct engines engines [] = {
    {.name = "pcre",        .find_all = pcre2_std_find_all},
    {.name = "pcre-dfa",    .find_all = pcre2_dfa_find_all},
    {.name = "pcre-jit",    .find_all = pcre2_jit_find_all},
    {.name = "re2",         .find_all = re2_find_all},
    {.name = "onig",        .find_all = onig_find_all},
    {.name = "tre",         .find_all = tre_find_all},
    {.name = "hscan",       .find_all = hs_find_all},
    {.name = "rust_regex",  .find_all = rust_find_all},
};

static char * regex [] = {
    "Twain",
    "(?i)Twain",
    "[a-z]shing",
    "Huck[a-zA-Z]+|Saw[a-zA-Z]+",
    "\\b\\w+nn\\b",
    "[a-q][^u-z]{13}x",
    "Tom|Sawyer|Huckleberry|Finn",
    "(?i)Tom|Sawyer|Huckleberry|Finn",
    ".{0,2}(Tom|Sawyer|Huckleberry|Finn)",
    ".{2,4}(Tom|Sawyer|Huckleberry|Finn)",
    "Tom.{10,25}river|river.{10,25}Tom",
    "[a-zA-Z]+ing",
    "\\s[a-zA-Z]{0,12}ing\\s",
    "([A-Za-z]awyer|[A-Za-z]inn)\\s",
    "[\"'][^\"']{0,30}[?!\\.][\"']",
    "\u221E|\u2713",
    "\\p{Sm}"                               // any mathematical symbol
};

void load(char const * file_name)
{
    int i;

    FILE * f;
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

    int size = fread(data, data_len, 1, f);
    if (size == 0) {
        fprintf(stderr, "Reading file failed!\n");
    }
    fclose(f);

    for (i = 0; i < data_len; ++i) {
        if (data[i] == '\r') {
            data[i] = '\n';
        }
    }
    fprintf(stdout, "'%s' loaded. (Length: %d bytes)\n", file_name, data_len);
}

void find_all(char* pattern, char* subject, int subject_len, int repeat, struct result * engine_results)
{
    int iter;

    fprintf(stdout, "-----------------\nRegex: '%s'\n", pattern);

    for (iter = 0; iter < sizeof(engines)/sizeof(engines[0]); iter++) {
        int ret = engines[iter].find_all(pattern, subject, subject_len, repeat);
        if (ret != -1) {
            engine_results[iter].time = ret;
        } else {
            engine_results[iter].time = 0;
        }
    }

    int score_points = 5;
    for (int top = 0; top < score_points; top++) {
        int best = 0;

        for (iter = 0; iter < sizeof(engines)/sizeof(engines[0]); iter++) {
            if (engine_results[iter].time > 0 &&
                engine_results[iter].score == 0 &&
                (best == 0 || best > engine_results[iter].time)) {
                best = engine_results[iter].time;
            }
        }

        for (iter = 0; iter < sizeof(engines)/sizeof(engines[0]); iter++) {
            if (engine_results[iter].time > 0 && best == engine_results[iter].time) {
                engine_results[iter].score = score_points;
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
    char * out_file = NULL;
    int repeat = 5;
    int c = 0;

    while ((c = getopt(argc, argv, "n:hvf:o:")) != -1) {
        switch (c) {
            case 'f':
                file = optarg;
                break;
            case 'n':
                repeat = atoi(optarg);
                break;
            case 'o':
                out_file = optarg;
                break;
            case 'v':
                printf("%s\n", VERSION_STRING);
                exit(EXIT_SUCCESS);
            case 'h':
                printf("Usage: %s [option] -f <file> \n\n", argv[0]);
                printf("Options:\n");
                printf("  -f\tInput file.\n");
                printf("  -n\tSet number of repetitions. Default: 5\n");
                printf("  -o\tWrite measured data into CSV file.\n");
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

    struct result results[sizeof(regex)/sizeof(regex[0])][sizeof(engines)/sizeof(engines[0])] = {0};
    struct result engine_results[sizeof(engines)/sizeof(engines[0])] = {0};

    for (int iter = 0; iter < sizeof(regex)/sizeof(regex[0]); iter++) {
        find_all(regex[iter], data, data_len, repeat, results[iter]);

        for (int iiter = 0; iiter < sizeof(engines)/sizeof(engines[0]); iiter++) {
            engine_results[iiter].time += results[iter][iiter].time;
            engine_results[iiter].score += results[iter][iiter].score;
        }
    }

    fprintf(stdout, "-----------------\nTotal Results:\n");
    for (int iter = 0; iter < sizeof(engines)/sizeof(engines[0]); iter++) {
        fprintf(stdout, "[%10s] time:  %6u ms, score: %6u points,\n", engines[iter].name, engine_results[iter].time, engine_results[iter].score);
    }

    if (out_file != NULL) {
        int iter, iiter;

        FILE * f;
        f = fopen(out_file, "w");
        if (!f) {
            fprintf(stderr, "Cannot open '%s'!\n", out_file);
            exit(EXIT_FAILURE);
        }

        /* write table header*/
        fprintf(f, "regex;");
        for (iter = 0; iter < sizeof(engines)/sizeof(engines[0]); iter++) {
            fprintf(f, "%s [ms];", engines[iter].name);
        }
        for (iter = 0; iter < sizeof(engines)/sizeof(engines[0]); iter++) {
            fprintf(f, "%s [sp];", engines[iter].name);
        }
        fprintf(f, "\n");

        /* write data */
        for (iter = 0; iter < sizeof(regex)/sizeof(regex[0]); iter++) {
            fprintf(f, "%s;", regex[iter]);

            for (iiter = 0; iiter < sizeof(engines)/sizeof(engines[0]); iiter++) {
                fprintf(f, "%d;", results[iter][iiter].time);
            }
            for (iiter = 0; iiter < sizeof(engines)/sizeof(engines[0]); iiter++) {
                fprintf(f, "%d;", results[iter][iiter].score);
            }
            fprintf(f, "\n");
        }

        fclose(f);
    }

    free(data);

    exit(EXIT_SUCCESS);
}
