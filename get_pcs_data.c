/*
 * SPDX-FileCopyrightText: 2026 Andrea Mazzucchi <andrea.mazzucchi@tutamail.com>
 * SPDX-FileCopyrightText: 2026 Francesco Quaglia <francesco.quaglia@uniroma2.it>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE 1024

#ifndef RUN
#define RUN 5
#endif
#ifndef DURATION
#define DURATION 60
#endif
#ifndef THREADS
#define THREADS 2
#endif
#ifndef SPEC
#define SPEC (0.25)
#endif
#ifndef OBJECTS
#define OBJECTS 1024
#endif
#ifndef MIT
#define MIT (0.4)
#endif
#ifndef COMMITTED
#define COMMITTED 0
#endif

double get_t_value(int df) {
    double t_table[] = {12.706, 4.303, 3.182, 2.776, 2.571, 2.447, 2.365, 2.306, 2.262, 2.228,
                        2.201,  2.179, 2.160, 2.145, 2.131, 2.120, 2.110, 2.101, 2.093, 2.086,
                        2.080,  2.074, 2.069, 2.064, 2.060, 2.056, 2.052, 2.048, 2.045, 2.042};
    if (df >= 1 && df <= 30) {
        return t_table[df - 1];
    }
    return 1.96;
}

void mean_ci_95(double *samples, int n, double *mean, double *ci) {
    double sum = 0.0;
    for (int i = 0; i < n; i++) {
        sum += samples[i];
    }
    *mean = sum / n;

    double var = 0.0;
    for (int i = 0; i < n; i++) {
        double d = samples[i] - *mean;
        var += d * d;
    }
    double sd = sqrt(var / (n - 1));
    double sem = sd / sqrt(n);
    *ci = get_t_value(n - 1) * sem;
}

static void usage(const char *prog) {
    printf("Usage: %s [options]\n"
           "  -r <run>      Number of runs         (default: %d)\n"
           "  -d <duration> Run Duration           (default: %d)\n"
           "  -t <threads>  Number of threads      (default: %d)\n"
           "  -s <spec>     Speculation window     (default: %.2f)\n"
           "  -o <objects>  Number of objects      (default: %d)\n"
           "  -m <mit>      MIT value              (default: %.2f)\n"
           "  -c <0|1>      Use events(0) or committed(1) for throughput (default: %d)\n",
           prog, RUN, DURATION, THREADS, (double)SPEC, OBJECTS, MIT, COMMITTED);
}

int main(int argc, char *argv[]) {
    int run = RUN;
    int duration = DURATION;
    int threads = THREADS;
    double spec = SPEC;
    int objects = OBJECTS;
    double mit = MIT;
    int use_committed = COMMITTED;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-r") == 0 && i + 1 < argc) {
            run = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-d") == 0 && i + 1 < argc) {
            duration = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-t") == 0 && i + 1 < argc) {
            threads = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-s") == 0 && i + 1 < argc) {
            spec = atof(argv[++i]);
        } else if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
            objects = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-m") == 0 && i + 1 < argc) {
            mit = atof(argv[++i]);
        } else if (strcmp(argv[i], "-c") == 0 && i + 1 < argc) {
            use_committed = atoi(argv[++i]);
        } else {
            printf("Unknown or incomplete argument: %s\n", argv[i]);
            usage(argv[0]);
            return 1;
        }
    }

    printf("Get pcs data to plot with:\n"
           "- Run = %d;\n"
           "- Threads = %d;\n"
           "- Speculation Window = %lf;\n"
           "- Objects = %d;\n"
           "- MIT = %.2f;\n"
           "- Use Committed = %d.\n",
           run, threads, spec, objects, mit, use_committed);

    double *throughputs = malloc(run * sizeof(double));
    double *rb_spec_w = malloc(run * sizeof(double));
    if (!throughputs || !rb_spec_w) {
        printf("Memory allocation failed\n");
        return 1;
    }

    FILE *input_file = fopen("pcs.csv", "r");
    FILE *output_file = fopen("pcs_plot_data.csv", "a");
    if (!input_file || !output_file) {
        printf("Failed to open file(s)\n");
        return 1;
    }

    char *ckpt_types[4] = {"grid_ckpt", "chunk_ckpt", "full_ckpt", "mmap_mv"};
    char line[MAX_LINE];

    for (int i = 0; i < 4; i++) {
        int count = 0;
        fseek(input_file, 0, SEEK_SET);
        fgets(line, MAX_LINE, input_file); // Skip header
        while (fgets(line, MAX_LINE, input_file) && count < run) {
            char ckpt_type[64];
            int t, obj, epochs, rollbacks, events, committed, filtered;
            double mit_value, spec_window;

            sscanf(line, "%[^,],%d,%lf,%d,%lf,%d,%d,%d,%d,%d", ckpt_type, &t, &spec_window, &obj, &mit_value, &epochs,
                   &rollbacks, &events, &committed, &filtered);

            if (strcmp(ckpt_type, ckpt_types[i]) || t != threads || spec_window != spec || obj != objects ||
                mit_value != mit) {
                continue;
            }

            // Use committed or events based on the -c flag
            int count_value = use_committed ? committed : events;
            throughputs[count] = (double)count_value / (double)duration;
            rb_spec_w[count] = (epochs > 0) ? (double)rollbacks / (double)epochs : (double)rollbacks;
            count++;
        }

        if (count < run) {
            printf("Warning: only %d/%d runs found for %s\n", count, run, ckpt_types[i]);
        }

        double throughput_mean, throughput_ci, rb_spec_w_mean, rb_spec_w_ci;
        mean_ci_95(throughputs, count > 0 ? count : 1, &throughput_mean, &throughput_ci);
        mean_ci_95(rb_spec_w, count > 0 ? count : 1, &rb_spec_w_mean, &rb_spec_w_ci);

        fprintf(output_file, "%s,%d,%f,%d,%f,%f,%f,%f,%f\n", ckpt_types[i], threads, spec, objects, mit,
                throughput_mean, throughput_ci, rb_spec_w_mean, rb_spec_w_ci);
    }

    fclose(input_file);
    fclose(output_file);
    free(throughputs);
    free(rb_spec_w);
    return 0;
}