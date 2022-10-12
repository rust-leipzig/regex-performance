#include <stdio.h>
#include <stdlib.h>
#include <yara.h>

#include "main.h"


static int capture_matches(
    YR_SCAN_CONTEXT* context,
    int message,
    void* message_data,
    void* user_data)
{
  if (message == CALLBACK_MSG_RULE_MATCHING)
  {
    YR_RULE* rule = (YR_RULE*) message_data;
    YR_STRING* string;

    yr_rule_strings_foreach(rule, string)
    {
      YR_MATCH* match;

      yr_string_matches_foreach(context, string, match)
          (*(int*) user_data)++;
    }
  }

  return 0;
}

char * return_rule_file(char* pattern)
{
  if (strcmp(pattern, "Twain") == 0)
    return "../src/yara/ruleset/twain.yar";
  else if (strcmp(pattern, "(?i)Twain") == 0)
    return "../src/yara/ruleset/twain_i.yar";
  else if (strcmp(pattern, "[a-z]shing") == 0)
    return "../src/yara/ruleset/shing.yar";
  else if (strcmp(pattern, "Huck[a-zA-Z]+|Saw[a-zA-Z]+") == 0)
    return "../src/yara/ruleset/huck.yar";
  else if (strcmp(pattern, "\\b\\w+nn\\b") == 0)
    return "../src/yara/ruleset/word.yar";
  else if (strcmp(pattern, "[a-q][^u-z]{13}x") == 0)
    return "../src/yara/ruleset/aax.yar";
  else if (strcmp(pattern, "Tom|Sawyer|Huckleberry|Finn") == 0)
    return "../src/yara/ruleset/tom.yar";
  else if (strcmp(pattern, "(?i)Tom|Sawyer|Huckleberry|Finn") == 0)
    return "../src/yara/ruleset/tom_i.yar";
  else if (strcmp(pattern, ".{0,2}(Tom|Sawyer|Huckleberry|Finn)") == 0)
    return "../src/yara/ruleset/tom2.yar";
  else if (strcmp(pattern, ".{2,4}(Tom|Sawyer|Huckleberry|Finn)") == 0)
    return "../src/yara/ruleset/tom4.yar";
  else if (strcmp(pattern, "Tom.{10,25}river|river.{10,25}Tom") == 0)
    return "../src/yara/ruleset/tom10.yar";
  else if (strcmp(pattern, "[a-zA-Z]+ing") == 0)
    return "../src/yara/ruleset/ing+.yar";
  else if (strcmp(pattern, "\\s[a-zA-Z]{0,12}ing\\s") == 0)
    return "../src/yara/ruleset/ing.yar";
  else if (strcmp(pattern, "([A-Za-z]awyer|[A-Za-z]inn)\\s") == 0)
    return "../src/yara/ruleset/inn.yar";
  else if (strcmp(pattern, "[\"'][^\"']{0,30}[?!\\.][\"']") == 0)
    return "../src/yara/ruleset/mix.yar";
  else if (strcmp(pattern, "\u221E|\u2713") == 0)
    return "../src/yara/ruleset/inf.yar";
  else
    return "";
}

int yara_find_all(char* pattern, char* subject, int subject_len, int repeat, struct result* res)
{
  TIME_TYPE start, end = 0;
  YR_COMPILER* compiler = NULL;
  YR_RULES* rules = NULL;
  int counter = 0;

  char * rule_file = return_rule_file(pattern);

  if (strlen(rule_file) == 0)
  {
    res->time = 999999;
    res->time_sd = 0;
    res->matches = 0;
  }
  else
  {
    FILE* fh = fopen(rule_file, "r");

    yr_initialize();
    yr_compiler_create(&compiler);
    yr_compiler_add_file(compiler, fh, NULL, NULL);
    yr_compiler_get_rules(compiler, &rules);

    double * times = calloc(repeat, sizeof(double));
    int const times_len = repeat;
    do
    {
        counter = 0;
        GET_TIME(start);
        yr_rules_scan_mem(rules, (const uint8_t*) subject, subject_len, 0, capture_matches, &counter, 0);
        GET_TIME(end);
        times[repeat - 1] = TIME_DIFF_IN_MS(start, end);

    } while (--repeat > 0);

    yr_rules_destroy(rules);
    yr_compiler_destroy(compiler);
    fclose(fh);

    res->matches = counter;
    get_mean_and_derivation(times, times_len, res);

    free(times);
    yr_finalize();
  }

  return 0;
}
