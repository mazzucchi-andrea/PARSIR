#include <math.h>
#include <stdio.h>
#include <string.h>

#define MAX_LINE 1024
#ifndef RUN
#define RUN 5
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
#ifndef M
#define M 1
#endif

double get_t_value(int df) {
    // Approximate t-values for 95% CI (two-tailed, alpha = 0.05)
    // For large samples (df > 30), approaches 1.96 (z-score)
    double t_table[] = {
        12.706, 4.303, 3.182, 2.776, 2.571, // df: 1-5
        2.447,  2.365, 2.306, 2.262, 2.228, // df: 6-10
        2.201,  2.179, 2.160, 2.145, 2.131, // df: 11-15
        2.120,  2.110, 2.101, 2.093, 2.086, // df: 16-20
        2.080,  2.074, 2.069, 2.064, 2.060, // df: 21-25
        2.056,  2.052, 2.048, 2.045, 2.042  // df: 26-30
    };

    if (df >= 1 && df <= 30) {
        return t_table[df - 1];
    } else if (df > 30) {
        return 1.96; // Use z-score for large samples
    }
    return 1.96; // Default
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

    double sd = sqrt(var / (n - 1)); // sample SD
    double sem = sd / sqrt(n);       // standard error

    *ci = get_t_value(n - 1) * sem;
}

int main(void) {
    FILE *input_file = fopen("phold_bench.csv", "r");
    FILE *output_file = fopen("phold_plot_data.csv", "a");
    char line[MAX_LINE];
    char *ckpt_types[3] = {"grid_ckpt", "chunk_ckpt", "full_ckpt"};

    for (int i = 0; i < 3; i++) {
        double throughputs[RUN];
        double rb_spec_w[RUN];
        int count = 0;

        fseek(input_file, 0, SEEK_SET);
        fgets(line, MAX_LINE, input_file); // Skip header
        while (fgets(line, MAX_LINE, input_file)) {
            char ckpt_type[64];
            int threads, objects, m, epochs, rollbacks, events, committed, filtered;
            double spec_window, throughput, committed_tput;

            sscanf(line, "%[^,],%d,%lf,%d,%d,%lf,%lf,%d,%d,%d,%d,%d", ckpt_type, &threads, &spec_window, &objects, &m,
                   &throughput, &committed_tput, &epochs, &rollbacks, &events, &committed, &filtered);
            if (strcmp(ckpt_type, ckpt_types[i]) || threads != THREADS || spec_window != SPEC || objects != OBJECTS ||
                m != M) {
                continue;
            }
            throughputs[count] = throughput;
            if (epochs > 0) {
                rb_spec_w[count] = (double)rollbacks / (double)epochs;
            } else {
                rb_spec_w[count] = (double)rollbacks / 1;
            }
            count++;
        }
        double throughput_mean, throughput_ci, rb_spec_w_mean, rb_spec_w_ci;
        mean_ci_95(throughputs, RUN, &throughput_mean, &throughput_ci);
        mean_ci_95(rb_spec_w, RUN, &rb_spec_w_mean, &rb_spec_w_ci);
        fprintf(output_file, "%s,%d,%f,%d,%d,%f,%f,%f,%f\n", ckpt_types[i], THREADS, SPEC, OBJECTS, M, throughput_mean,
                throughput_ci, rb_spec_w_mean, rb_spec_w_ci);
    }

    fclose(input_file);
    fclose(output_file);

    return 0;
}