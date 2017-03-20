#include <stdint.h>

struct Regex;

extern struct Regex const * regex_new(char * const regex);
extern uint64_t regex_matches(struct Regex const * const exp, uint8_t * const str, uint64_t str_len);
extern void regex_free(struct Regex const * const exp);
