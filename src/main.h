#ifdef __cplusplus
extern "C" {
#endif

#include <time.h>

#define TIME_TYPE clock_t
#define GET_TIME(res) do { res = clock(); } while(0)
#define GET_TIME_RESOLUTION(res) do { res = CLOCKS_PER_SEC; } while(0)
#define TIME_DIFF_IN_MS(begin, end, resolution) ((int)((double)((end) - (begin)) * 1000 / (resolution)))
#define UNUSED __attribute__((unused))

// Common result print
void printResult(char* name, int time, int found);

int pcre2_std_find_all(char* pattern, char* subject, int subject_len, int repeat);
int pcre2_dfa_find_all(char* pattern, char* subject, int subject_len, int repeat);
int pcre2_jit_find_all(char* pattern, char* subject, int subject_len, int repeat);
int re2_find_all(char* pattern, char* subject, int subject_len, int repeat);
int tre_find_all(char* pattern, char* subject, int subject_len, int repeat);
int onig_find_all(char* pattern, char* subject, int subject_len, int repeat);
int hs_find_all(char * pattern, char * subject, int subject_len, int repeat);
int rust_find_all(char * pattern, char * subject, int subject_len, int repeat);

#ifdef __cplusplus
}
#endif

