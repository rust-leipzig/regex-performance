#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define TIME_TYPE                   clock_t
#define GET_TIME(res)               { res = clock(); }
#define TIME_DIFF_IN_MS(begin, end) (((double) (end - start)) * 1000 / CLOCKS_PER_SEC)
#define UNUSED __attribute__((unused))

struct result {
    int score;
    double time;
    double time_sd;
    int matches;
};

void get_mean_and_derivation(double * times, uint32_t times_len, struct result * res);
void printResult(char * name, struct result * res);

#ifdef INCLUDE_PCRE2
int pcre2_std_find_all(char* pattern, char* subject, int subject_len, int repeat, struct result * res);
int pcre2_dfa_find_all(char* pattern, char* subject, int subject_len, int repeat, struct result * res);
int pcre2_jit_find_all(char* pattern, char* subject, int subject_len, int repeat, struct result * res);
#endif
#ifdef INCLUDE_RE2
int re2_find_all(char* pattern, char* subject, int subject_len, int repeat, struct result * res);
#endif
#ifdef INCLUDE_TRE
int tre_find_all(char* pattern, char* subject, int subject_len, int repeat, struct result * res);
#endif
#ifdef INCLUDE_ONIGURUMA
int onig_find_all(char* pattern, char* subject, int subject_len, int repeat, struct result * res);
#endif
#ifdef INCLUDE_HYPERSCAN
int hs_find_all(char * pattern, char * subject, int subject_len, int repeat, struct result * res);
#endif
int rust_find_all(char * pattern, char * subject, int subject_len, int repeat, struct result * res);
int regress_find_all(char * pattern, char * subject, int subject_len, int repeat, struct result * res);

#ifdef __cplusplus
}
#endif

