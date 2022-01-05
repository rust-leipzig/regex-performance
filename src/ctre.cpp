#include <stdio.h>
#include <string.h>
#include <unordered_map>
#include <string_view>
#include <functional>

#include "main.h"
#include <ctre.hpp>

using RegexFn = std::function<int(const std::string &)>;
using RegexMap = std::unordered_map<std::string, RegexFn>;

#define ENTRY(STR)                                  \
    {                                               \
        STR,                                        \
            [](const std::string_view &sv) -> int   \
        {                                           \
            int cnt = 0;                            \
            for (auto match : ctre::range<STR>(sv)) \
                cnt++;                              \
            return cnt;                             \
        }                                           \
    }

RegexMap remap = {
    ENTRY("(.*?,){13}z"),
    ENTRY("[a-z]shing"),
    ENTRY("Twain"),
    // ENTRY("(?i)Twain"),
    ENTRY("[a-z]shing"),
    ENTRY("Huck[a-zA-Z]+|Saw[a-zA-Z]+"),
    // ENTRY("\\b\\w+nn\\b"),
    ENTRY("[a-q][^u-z]{13}x"),
    ENTRY("Tom|Sawyer|Huckleberry|Finn"),
    // ENTRY("(?i)Tom|Sawyer|Huckleberry|Finn"),
    ENTRY(".{0,2}(Tom|Sawyer|Huckleberry|Finn)"),
    ENTRY(".{2,4}(Tom|Sawyer|Huckleberry|Finn)"),
    ENTRY("Tom.{10,25}river|river.{10,25}Tom"),
    ENTRY("[a-zA-Z]+ing"),
    ENTRY("\\s[a-zA-Z]{0,12}ing\\s"),
    ENTRY("([A-Za-z]awyer|[A-Za-z]inn)\\s"),
    ENTRY("[\"'][^\"']{0,30}[?!\\.][\"']"),
    ENTRY("\u221E|\u2713")
    // ENTRY("\\p{Sm}")
};

extern "C" int ctre_find_all(char *pattern, char *subject, int subject_len, int repeat, struct result *res)
{
    TIME_TYPE start, end = 0;
    int found = 0;

    std::string text(subject, subject_len);
    RegexMap::const_iterator it = remap.find(pattern);
    if (it != remap.end())
    {

        double *times = (double *)std::calloc(repeat, sizeof(double));
        int const times_len = repeat;

        do
        {
            GET_TIME(start);
            found = it->second(text);
            GET_TIME(end);
            times[repeat - 1] = TIME_DIFF_IN_MS(start, end);

        } while (--repeat > 0);

        res->matches = found;
        get_mean_and_derivation(times, times_len, res);

        free(times);
    }
    else
    {
        res->time = 999999;
        res->time_sd = 0;
        res->matches = 0;
    }

    return 0;
}
