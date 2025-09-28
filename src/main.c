#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <errno.h>
#include <sys/stat.h>
#include <float.h>
#include <math.h>

#include "main.h"
#include "version.h"

static char* data = NULL;
static int data_len = 0;

struct engines {
    char * name;
    int (*find_all)(char* pattern, char* subject, int subject_len, int repeat, struct result * result);
};

static struct engines engines [] = {
#ifdef INCLUDE_CTRE
    {.name = "ctre",        .find_all = ctre_find_all},
#endif
#ifdef INCLUDE_BOOST
    {.name = "boost",        .find_all = boost_find_all},
#endif
#ifdef INCLUDE_CPPSTD
    {.name = "cppstd",        .find_all = cppstd_find_all},
#endif
#ifdef INCLUDE_PCRE2
    {.name = "pcre",        .find_all = pcre2_std_find_all},
    {.name = "pcre-dfa",    .find_all = pcre2_dfa_find_all},
    {.name = "pcre-jit",    .find_all = pcre2_jit_find_all},
#endif
#ifdef INCLUDE_RE2
    {.name = "re2",         .find_all = re2_find_all},
#endif
#ifdef INCLUDE_ONIGURUMA
    {.name = "onig",        .find_all = onig_find_all},
#endif
#ifdef INCLUDE_TRE
    {.name = "tre",         .find_all = tre_find_all},
#endif
#ifdef INCLUDE_HYPERSCAN
    {.name = "hscan",       .find_all = hs_find_all},
#endif
#ifdef INCLUDE_YARA
    {.name = "yara",        .find_all = yara_find_all},
#endif
    {.name = "rust_regex",  .find_all = rust_find_all},
    {.name = "rust_regrs",  .find_all = regress_find_all},
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
    "\\p{Sm}",                               // any mathematical symbol
    "(.*?,){13}z"
};

void cleanup_data(void)
{
    if (data) {
        free(data);
        data = NULL;
        data_len = 0;
    }
}

int load(char const * file_name)
{
    struct stat st;
    size_t bytes_read = 0;

    // Validate file exists and get size safely
    if (stat(file_name, &st) != 0) {
        fprintf(stderr, "Cannot stat file '%s': %s\n", file_name, strerror(errno));
        return -1;
    }

    // Check for reasonable file size (prevent huge allocations)
    if (st.st_size <= 0 || st.st_size > 1024 * 1024 * 1024) { // 1GB limit
        fprintf(stderr, "File '%s' has invalid size: %ld bytes\n", file_name, st.st_size);
        return -1;
    }

    FILE * f = fopen(file_name, "rb");
    if (!f) {
        fprintf(stderr, "Cannot open '%s': %s\n", file_name, strerror(errno));
        return -1;
    }

    data_len = st.st_size;
    data = (char*)malloc(data_len + 1);
    if (!data) {
        fprintf(stderr, "Cannot allocate %d bytes for file '%s': %s\n",
                data_len + 1, file_name, strerror(errno));
        fclose(f);
        return -1;
    }

    // Read file in chunks with proper bounds checking
    while (bytes_read < data_len) {
        size_t to_read = data_len - bytes_read;
        size_t chunk_size = fread(data + bytes_read, 1, to_read, f);

        if (chunk_size == 0) {
            if (feof(f)) {
                break; // End of file reached
            } else if (ferror(f)) {
                fprintf(stderr, "Error reading file '%s': %s\n", file_name, strerror(errno));
                cleanup_data();
                fclose(f);
                return -1;
            }
        }
        bytes_read += chunk_size;
    }

    fclose(f);

    // Verify we read the expected amount
    if (bytes_read != data_len) {
        fprintf(stderr, "Warning: Expected %d bytes, read %zu bytes from '%s'\n",
                data_len, bytes_read, file_name);
        data_len = bytes_read; // Adjust to actual size
    }

    data[data_len] = '\0'; // Null terminate

    // Convert line endings
    for (int i = 0; i < data_len; ++i) {
        if (data[i] == '\r') {
            data[i] = '\n';
        }
    }

    fprintf(stdout, "'%s' loaded. (Length: %d bytes)\n", file_name, data_len);
    return 0;
}

void find_all(char* pattern, char* subject, int subject_len, int repeat, struct result * engine_results)
{
    int iter;

    fprintf(stdout, "-----------------\nRegex: '%s'\n", pattern);

    for (iter = 0; iter < sizeof(engines)/sizeof(engines[0]); iter++) {
        // Warmup for JIT engines
        if (needs_warmup(engines[iter].name)) {
            struct result warmup_result = {0};
            int warmup_cycles = 3;
            engines[iter].find_all(pattern, subject, subject_len, warmup_cycles, &warmup_result);
            // Warmup result is discarded
        }

        // Record baseline memory before engine execution
        record_memory_baseline(&(engine_results[iter]));

        int ret = engines[iter].find_all(pattern, subject, subject_len, repeat, &(engine_results[iter]));

        // Record peak memory after engine execution
        record_memory_peak(&(engine_results[iter]));

        if (ret == -1) {
            engine_results[iter].time = 0;
            engine_results[iter].time_sd = 0;
            engine_results[iter].confidence_95_lower = 0;
            engine_results[iter].confidence_95_upper = 0;
            engine_results[iter].matches = 0;
            engine_results[iter].score = 0;
            engine_results[iter].samples_used = 0;
            // Memory fields are already set by record_memory_peak
        } else {
            printResult(engines[iter].name, &(engine_results[iter]));
        }
    }

    // Note: Validation is now done in main() after all patterns are tested

    int score_points = 5;
    for (int top = 0; top < score_points; top++) {
        double best = 0;

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

int compare_doubles(const void *a, const void *b) {
    double da = *(const double*)a;
    double db = *(const double*)b;
    return (da > db) - (da < db);
}

void get_mean_and_derivation(double * times, uint32_t times_len, struct result * res)
{
    double mean, sd, var, sum = 0.0, sdev = 0.0;
    int32_t iter;
    uint32_t valid_count = times_len;

    if (times == NULL || res == NULL || times_len == 0) {
        return;
    }

    if (times_len == 1) {
        res->time = times[0];
        res->time_sd = 0;
        return;
    }

    // Create a copy for outlier detection
    double *sorted_times = malloc(times_len * sizeof(double));
    if (!sorted_times) {
        // Fallback to original algorithm if malloc fails
        for (iter = 0; iter < times_len; iter++) {
            sum += times[iter];
        }
        mean = sum / times_len;

        for (iter = 0; iter < times_len; iter++) {
            sdev += (times[iter] - mean) * (times[iter] - mean);
        }
        var = sdev / (times_len - 1);
        sd = sqrt(var);

        res->time = mean;
        res->time_sd = sd;
        return;
    }

    memcpy(sorted_times, times, times_len * sizeof(double));
    qsort(sorted_times, times_len, sizeof(double), compare_doubles);

    // Remove extreme outliers (beyond 2.5 std devs) only if we have enough samples
    if (times_len >= 10) {
        // Calculate initial stats to identify outliers
        for (iter = 0; iter < times_len; iter++) {
            sum += sorted_times[iter];
        }
        mean = sum / times_len;

        sdev = 0.0;
        for (iter = 0; iter < times_len; iter++) {
            sdev += (sorted_times[iter] - mean) * (sorted_times[iter] - mean);
        }
        sd = sqrt(sdev / (times_len - 1));

        // Remove outliers beyond 2.5 standard deviations
        double threshold = 2.5 * sd;
        valid_count = 0;
        sum = 0.0;

        for (iter = 0; iter < times_len; iter++) {
            if (fabs(sorted_times[iter] - mean) <= threshold) {
                sum += sorted_times[iter];
                valid_count++;
            }
        }

        if (valid_count < times_len / 2) {
            // Too many outliers, use all data
            valid_count = times_len;
            sum = 0.0;
            for (iter = 0; iter < times_len; iter++) {
                sum += sorted_times[iter];
            }
        }
    } else {
        // Use all data for small sample sizes
        for (iter = 0; iter < times_len; iter++) {
            sum += sorted_times[iter];
        }
    }

    mean = sum / valid_count;

    // Recalculate variance with outliers removed
    sdev = 0.0;
    uint32_t count = 0;
    double threshold = (times_len >= 10) ? 2.5 * sd : DBL_MAX;

    for (iter = 0; iter < times_len; iter++) {
        if (times_len < 10 || fabs(sorted_times[iter] - mean) <= threshold) {
            sdev += (sorted_times[iter] - mean) * (sorted_times[iter] - mean);
            count++;
        }
    }

    if (count > 1) {
        var = sdev / (count - 1);
        sd = sqrt(var);
    } else {
        sd = 0.0;
    }

    res->time = mean;
    res->time_sd = sd;
    res->samples_used = count;

    // Calculate 95% confidence interval
    if (count > 1) {
        // t-distribution critical value for 95% confidence (approximation for large samples)
        double t_critical = (count >= 30) ? 1.96 : 2.776; // Conservative estimate for small samples
        if (count >= 5 && count < 30) {
            // Rough approximation for t-distribution
            t_critical = 2.5 - 0.5 * (count - 5) / 25.0;
        }

        double margin_of_error = t_critical * (sd / sqrt(count));
        res->confidence_95_lower = mean - margin_of_error;
        res->confidence_95_upper = mean + margin_of_error;

        // Ensure confidence bounds are non-negative
        if (res->confidence_95_lower < 0) {
            res->confidence_95_lower = 0;
        }
    } else {
        res->confidence_95_lower = mean;
        res->confidence_95_upper = mean;
    }

    free(sorted_times);
}

void printResult(char * name, struct result * res)
{
    double cv = (res->time > 0) ? (res->time_sd / res->time) * 100 : 0;
    const char* stability = "";

    // Measurement stability indicators
    if (cv > 30) {
        stability = " [UNSTABLE]";
    } else if (cv > 15) {
        stability = " [NOISY]";
    } else if (cv > 5) {
        stability = " [VARIABLE]";
    }

    // Format memory information
    char memory_info[64] = "";
    if (res->memory_rss_peak_kb > 0) {
        if (res->memory_delta_kb > 0) {
            snprintf(memory_info, sizeof(memory_info), " mem: %ld kB (+%ld)",
                     res->memory_rss_peak_kb, res->memory_delta_kb);
        } else {
            snprintf(memory_info, sizeof(memory_info), " mem: %ld kB",
                     res->memory_rss_peak_kb);
        }
    }

    fprintf(stdout, "[%10s] time: %7.1f ms [%6.1f-%6.1f] (±%4.1f%%)%s matches: %8d (n=%d)%s\n",
            name, res->time,
            res->confidence_95_lower, res->confidence_95_upper,
            cv, memory_info, res->matches, res->samples_used, stability);
    fflush(stdout);
}

int validate_results(char* pattern, struct result* results, char** engine_names, int num_engines)
{
    int reference_matches = -1;
    char* reference_engine = NULL;
    int validation_errors = 0;
    int valid_engines = 0;

    // Find a reference match count from the first successful engine
    for (int i = 0; i < num_engines; i++) {
        if (results[i].time > 0) { // Engine succeeded
            if (reference_matches == -1) {
                reference_matches = results[i].matches;
                reference_engine = engine_names[i];
            }
            valid_engines++;
        }
    }

    // If no engines succeeded, skip validation
    if (valid_engines == 0) {
        fprintf(stderr, "WARNING: No engines succeeded for pattern '%s'\n", pattern);
        return 0;
    }

    // If only one engine succeeded, can't validate
    if (valid_engines == 1) {
        fprintf(stdout, "INFO: Only one engine (%s) succeeded for pattern '%s', skipping validation\n",
                reference_engine, pattern);
        return 0;
    }

    // Compare all successful engines against reference
    for (int i = 0; i < num_engines; i++) {
        if (results[i].time > 0) { // Engine succeeded
            if (results[i].matches != reference_matches) {
                fprintf(stderr, "VALIDATION ERROR: Pattern '%s' - %s found %d matches, %s found %d matches\n",
                        pattern, engine_names[i], results[i].matches,
                        reference_engine, reference_matches);
                validation_errors++;
            }
        }
    }

    if (validation_errors == 0 && valid_engines > 1) {
        // Only print success message for complex patterns to reduce noise
        if (reference_matches > 1000 || strstr(pattern, "\\p{") || strstr(pattern, "(?i)")) {
            fprintf(stdout, "VALIDATION OK: All %d engines agree on %d matches for pattern '%s'\n",
                    valid_engines, reference_matches, pattern);
        }
    }

    return validation_errors;
}

int needs_warmup(const char* engine_name)
{
    // JIT engines benefit from warmup cycles
    return (strstr(engine_name, "jit") != NULL ||
            strstr(engine_name, "re2") != NULL ||
            strstr(engine_name, "rust") != NULL);
}

long get_memory_usage_kb(const char* metric)
{
    FILE* status_file = fopen("/proc/self/status", "r");
    if (!status_file) {
        return -1;
    }

    char line[256];
    long value = -1;

    while (fgets(line, sizeof(line), status_file)) {
        if (strncmp(line, metric, strlen(metric)) == 0) {
            // Parse the value (format: "MetricName: 1234 kB")
            char* colon = strchr(line, ':');
            if (colon) {
                value = atol(colon + 1);
                break;
            }
        }
    }

    fclose(status_file);
    return value;
}

void record_memory_baseline(struct result* res)
{
    res->memory_start_kb = get_memory_usage_kb("VmRSS");
    if (res->memory_start_kb < 0) {
        res->memory_start_kb = 0;
    }
}

void record_memory_peak(struct result* res)
{
    long current_peak = get_memory_usage_kb("VmHWM");
    long current_rss = get_memory_usage_kb("VmRSS");

    if (current_peak > 0) {
        res->memory_rss_peak_kb = current_peak;
        res->memory_peak_kb = get_memory_usage_kb("VmPeak");

        if (res->memory_start_kb > 0 && current_rss > res->memory_start_kb) {
            res->memory_delta_kb = current_rss - res->memory_start_kb;
        } else {
            res->memory_delta_kb = 0;
        }
    } else {
        res->memory_rss_peak_kb = 0;
        res->memory_peak_kb = 0;
        res->memory_delta_kb = 0;
    }
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

    if (load(file) != 0) {
        fprintf(stderr, "Failed to load file '%s'\n", file);
        exit(EXIT_FAILURE);
    }

    // Register cleanup function for proper memory management
    atexit(cleanup_data);

    struct result results[sizeof(regex)/sizeof(regex[0])][sizeof(engines)/sizeof(engines[0])] = {0};
    struct result engine_results[sizeof(engines)/sizeof(engines[0])] = {0};
    int total_validation_errors = 0;

    for (int iter = 0; iter < sizeof(regex)/sizeof(regex[0]); iter++) {
        find_all(regex[iter], data, data_len, repeat, results[iter]);

        // Validate results for this pattern
        char* engine_names[sizeof(engines)/sizeof(engines[0])];
        for (int iiter = 0; iiter < sizeof(engines)/sizeof(engines[0]); iiter++) {
            engine_names[iiter] = engines[iiter].name;
        }
        int errors = validate_results(regex[iter], results[iter], engine_names, sizeof(engines)/sizeof(engines[0]));
        total_validation_errors += errors;

        for (int iiter = 0; iiter < sizeof(engines)/sizeof(engines[0]); iiter++) {
            engine_results[iiter].time += results[iter][iiter].time;
            engine_results[iiter].score += results[iter][iiter].score;
        }
    }

    fprintf(stdout, "-----------------\nTotal Results:\n");
    for (int iter = 0; iter < sizeof(engines)/sizeof(engines[0]); iter++) {
        char memory_summary[32] = "";
        if (engine_results[iter].memory_rss_peak_kb > 0) {
            snprintf(memory_summary, sizeof(memory_summary), ", mem: %ld kB",
                     engine_results[iter].memory_rss_peak_kb);
        }
        fprintf(stdout, "[%10s] time:  %7.1f ms, score: %6u points%s\n",
                engines[iter].name, engine_results[iter].time, engine_results[iter].score, memory_summary);
    }

    // Print validation and stability summary
    fprintf(stdout, "-----------------\nValidation Summary:\n");
    if (total_validation_errors == 0) {
        fprintf(stdout, "✓ ALL ENGINES PASSED VALIDATION - No discrepancies found\n");
    } else {
        fprintf(stderr, "✗ VALIDATION FAILURES: %d discrepancies found across patterns\n", total_validation_errors);
        fprintf(stderr, "WARNING: Results may not be reliable due to engine inconsistencies\n");
    }

    fprintf(stdout, "-----------------\nMeasurement Quality:\n");
    int unstable_engines = 0, noisy_engines = 0, variable_engines = 0, stable_engines = 0;

    for (int iter = 0; iter < sizeof(engines)/sizeof(engines[0]); iter++) {
        if (engine_results[iter].time > 0) {
            double cv = (engine_results[iter].time_sd / engine_results[iter].time) * 100;
            if (cv > 30) unstable_engines++;
            else if (cv > 15) noisy_engines++;
            else if (cv > 5) variable_engines++;
            else stable_engines++;
        }
    }

    fprintf(stdout, "Stable engines: %d, Variable: %d, Noisy: %d, Unstable: %d\n",
            stable_engines, variable_engines, noisy_engines, unstable_engines);

    if (unstable_engines > 0) {
        fprintf(stderr, "WARNING: %d engines show unstable measurements (>30%% CV)\n", unstable_engines);
    }

    // Memory vs Speed Analysis
    fprintf(stdout, "\n-----------------\n");
    fprintf(stdout, "Memory vs Speed Analysis:\n");

    // Calculate average memory usage and speed across all regex patterns
    double avg_memory[sizeof(engines)/sizeof(engines[0])] = {0};
    double avg_time[sizeof(engines)/sizeof(engines[0])] = {0};
    int valid_results[sizeof(engines)/sizeof(engines[0])] = {0};

    for (int e = 0; e < sizeof(engines)/sizeof(engines[0]); e++) {
        for (int r = 0; r < sizeof(regex)/sizeof(regex[0]); r++) {
            if (results[r][e].time > 0 && results[r][e].memory_peak_kb > 0) {
                avg_memory[e] += results[r][e].memory_delta_kb;
                avg_time[e] += results[r][e].time;
                valid_results[e]++;
            }
        }
        if (valid_results[e] > 0) {
            avg_memory[e] /= valid_results[e];
            avg_time[e] /= valid_results[e];
        }
    }

    // Find most memory efficient and fastest engines
    int most_efficient = -1, fastest = -1;
    double min_memory = 1e9, min_time = 1e9;

    for (int e = 0; e < sizeof(engines)/sizeof(engines[0]); e++) {
        if (valid_results[e] > 0) {
            if (avg_memory[e] < min_memory) {
                min_memory = avg_memory[e];
                most_efficient = e;
            }
            if (avg_time[e] < min_time) {
                min_time = avg_time[e];
                fastest = e;
            }
        }
    }

    // Report memory efficiency rankings
    fprintf(stdout, "Memory Efficiency Rankings (avg delta KB):\n");
    for (int e = 0; e < sizeof(engines)/sizeof(engines[0]); e++) {
        if (valid_results[e] > 0) {
            fprintf(stdout, "  %s: %.1f kB avg\n", engines[e].name, avg_memory[e]);
        }
    }

    fprintf(stdout, "Speed Rankings (avg ms):\n");
    for (int e = 0; e < sizeof(engines)/sizeof(engines[0]); e++) {
        if (valid_results[e] > 0) {
            fprintf(stdout, "  %s: %.1f ms avg\n", engines[e].name, avg_time[e]);
        }
    }

    if (most_efficient >= 0 && fastest >= 0) {
        fprintf(stdout, "\nSummary:\n");
        fprintf(stdout, "  Most memory efficient: %s (%.1f kB avg)\n",
                engines[most_efficient].name, avg_memory[most_efficient]);
        fprintf(stdout, "  Fastest: %s (%.1f ms avg)\n",
                engines[fastest].name, avg_time[fastest]);

        if (most_efficient == fastest) {
            fprintf(stdout, "  %s wins both speed and memory efficiency!\n",
                    engines[fastest].name);
        } else {
            // Calculate memory/speed ratio for trade-off analysis
            fprintf(stdout, "  Trade-off: %s uses %.1fx more memory for %.1fx speed improvement\n",
                    engines[fastest].name,
                    avg_memory[fastest] / avg_memory[most_efficient],
                    avg_time[most_efficient] / avg_time[fastest]);
        }
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
        for (iter = 0; iter < sizeof(engines)/sizeof(engines[0]); iter++) {
            fprintf(f, "%s [matches];", engines[iter].name);
        }
        for (iter = 0; iter < sizeof(engines)/sizeof(engines[0]); iter++) {
            fprintf(f, "%s [mem_kb];", engines[iter].name);
        }
        fprintf(f, "\n");

        /* write data */
        for (iter = 0; iter < sizeof(regex)/sizeof(regex[0]); iter++) {
            fprintf(f, "%s;", regex[iter]);

            for (iiter = 0; iiter < sizeof(engines)/sizeof(engines[0]); iiter++) {
                fprintf(f, "%7.1f;", results[iter][iiter].time);
            }
            for (iiter = 0; iiter < sizeof(engines)/sizeof(engines[0]); iiter++) {
                fprintf(f, "%d;", results[iter][iiter].score);
            }
            for (iiter = 0; iiter < sizeof(engines)/sizeof(engines[0]); iiter++) {
                fprintf(f, "%d;", results[iter][iiter].matches);
            }
            for (iiter = 0; iiter < sizeof(engines)/sizeof(engines[0]); iiter++) {
                fprintf(f, "%ld;", results[iter][iiter].memory_delta_kb);
            }
            fprintf(f, "\n");
        }

        fclose(f);
    }

    free(data);

    exit(EXIT_SUCCESS);
}
