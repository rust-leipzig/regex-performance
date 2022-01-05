#include <stdio.h>
#include <string.h>

#include "main.h"
#include <boost/regex.hpp>


static int search_all( boost::regex& rx, const std::string& text )
{
    auto words_begin = boost::sregex_iterator( text.begin(), text.end(), rx );
    auto words_end = boost::sregex_iterator();
    return std::distance(words_begin, words_end);    
}

extern "C" int boost_find_all(char* pattern, char* subject, int subject_len, int repeat, struct result * res)
{
    TIME_TYPE start, end = 0;
    int found = 0;

    try {
        boost::regex rx(pattern,boost::regex::optimize|boost::regex::extended);
        std::string text( subject, subject_len );

        double * times = (double*) std::calloc(repeat, sizeof(double));
        int const times_len = repeat;

        do {
            GET_TIME(start);
            found = search_all( rx, text );
            GET_TIME(end);
            times[repeat - 1] = TIME_DIFF_IN_MS(start, end);

        } while (--repeat > 0);


        res->matches = found;
        get_mean_and_derivation(times, times_len, res);

        free(times);
    } catch ( ... ) {
        res->time = 999999;
        res->time_sd = 0;
        res->matches = 0;
    }

    return 0;
}
