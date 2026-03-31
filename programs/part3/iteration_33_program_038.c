/* tree-pretty-print-coverage.c
 * Designed to trigger uncovered lines 512-523 in tree-pretty-print.cc
 * Compile with: gcc -O1 -fopenmp -fdump-tree-omplower -fdump-tree-ompexp tree-pretty-print-coverage.c -o coverage_test
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
volatile int vol_bound = 100;
volatile int vol_seed = 42;

/* Function to be used with declare target enter */
#pragma omp declare target enter(vec_add) to(array_a, array_b, array_c)
void vec_add(int n, double *a, double *b, double *c) {
    #pragma omp parallel for simd
    for (int i = 0; i < n; i++) {
        c[i] = a[i] + b[i];
    }
}

/* Another function with reduction and scan */
double compute_prefix_sum(int n, double *data, double *prefix) {
    double sum = 0.0;
    
    /* This should generate _reductemp_ and _scantemp_ clauses */
    #pragma omp parallel for simd reduction(+:sum) \
            simdlen(4) safelen(8) scan(inscan:sum)
    for (int i = 0; i < n; i++) {
        /* Inscan phase */
        {
            prefix[i] = sum;
            sum += data[i];
        }
    }
    return sum;
}

/* Function with nested collapse - may generate _condtemp_ */
void nested_collapse_loop(int m, int n, double *matrix) {
    int bound = vol_bound; /* Use volatile to prevent constant propagation */
    
    /* Complex loop bound to potentially generate condition temporaries */
    #pragma omp parallel for collapse(2) \
            if(bound > 50) num_threads(omp_get_max_threads())
    for (int i = 0; i < m && i < bound; i++) {
        for (int j = 0; j < n && j < bound; j++) {
            int idx = i * n + j;
            /* Data-dependent operation */
            if ((i + j) % 2 == 0) {
                matrix[idx] *= 2.0;
            } else {
                matrix[idx] /= 2.0;
            }
        }
    }
}

/* Function with reduction in teams - may generate more temporaries */
double teams_reduction(int n, double *data) {
    double total = 0.0;
    
    #pragma omp target teams distribute parallel for \
            reduction(+:total) map(tofrom: total) map(to: data[0:n])
    for (int i = 0; i < n; i++) {
        total += data[i] * (i % 10);
    }
    return total;
}

int main(int argc, char **argv) {
    /* Use argc for variable but reproducible sizes */
    int base_size = 1000;
    if (argc > 1) base_size = atoi(argv[1]);
    if (base_size < 100) base_size = 100;
    
    int n = base_size + (argc * 37) % 100; /* Make size data-dependent */
    int m = 50 + (argc * 23) % 30;
    
    printf("Running with n=%d, m=%d\n", n, m);
    
    /* Allocate arrays */
    double *data = (double *)malloc(n * sizeof(double));
    double *prefix = (double *)malloc(n * sizeof(double));
    double *matrix = (double *)malloc(m * n * sizeof(double));
    double *array_a = (double *)malloc(n * sizeof(double));
    double *array_b = (double *)malloc(n * sizeof(double));
    double *array_c = (double *)malloc(n * sizeof(double));
    
    if (!data || !prefix || !matrix || !array_a || !array_b || !array_c) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with simple patterns */
    for (int i = 0; i < n; i++) {
        data[i] = 1.0 + (i % 7) * 0.1;
        array_a[i] = i * 0.5;
        array_b[i] = i * 0.3;
    }
    
    for (int i = 0; i < m * n; i++) {
        matrix[i] = 1.0 + (i % 11) * 0.05;
    }
    
    /* 1. Test reduction with scan - targets _reductemp_ and _scantemp_ */
    printf("Testing reduction with scan...\n");
    double total_sum = compute_prefix_sum(n, data, prefix);
    printf("Total sum: %f\n", total_sum);
    
    /* 2. Test nested collapse - may generate _condtemp_ */
    printf("Testing nested collapse...\n");
    nested_collapse_loop(m, n, matrix);
    
    /* 3. Test declare target enter - targets ENTER clause */
    printf("Testing declare target enter...\n");
    
    /* Additional declare target with to specifier */
    #pragma omp declare target enter(vec_add) to(array_a, array_b, array_c)
    
    /* Actually use the target region */
    #pragma omp target map(to: array_a[0:n], array_b[0:n]) \
                       map(from: array_c[0:n]) \
                       device(0) if(n > 200)
    {
        vec_add(n, array_a, array_b, array_c);
    }
    
    /* 4. Test teams reduction */
    printf("Testing teams reduction...\n");
    double team_total = teams_reduction(n, data);
    printf("Teams reduction total: %f\n", team_total);
    
    /* 5. Additional complex OpenMP construct with multiple clauses */
    printf("Testing complex combined construct...\n");
    {
        double complex_sum = 0.0;
        double scan_temp = 0.0;
        
        /* Combined construct that might generate multiple temporaries */
        #pragma omp target teams distribute parallel for simd \
                reduction(+:complex_sum) \
                map(tofrom: complex_sum) \
                collapse(2) \
                simdlen(4) \
                private(scan_temp) \
                lastprivate(scan_temp)
        for (int i = 0; i < m; i++) {
            for (int j = 0; j < n; j++) {
                int idx = i * n + j;
                /* Complex expression that might need temporaries */
                double val = matrix[idx] * (i + 1) / (j + 1);
                complex_sum += val;
                scan_temp = val;
                
                /* Conditional that might generate condition code */
                if (omp_get_thread_num() % 2 == 0) {
                    matrix[idx] = val * 0.5;
                }
            }
        }
        printf("Complex sum: %f\n", complex_sum);
    }
    
    /* Compute checksum to verify execution and prevent optimization */
    double checksum = 0.0;
    for (int i = 0; i < n; i += 10) {
        checksum += prefix[i] + array_c[i];
    }
    for (int i = 0; i < m * n; i += 15) {
        checksum += matrix[i];
    }
    
    printf("Final checksum: %f\n", checksum);
    
    /* Cleanup */
    free(data);
    free(prefix);
    free(matrix);
    free(array_a);
    free(array_b);
    free(array_c);
    
    return 0;
}

/* Dummy function to hint compiler about OpenMP clause types
 * This won't directly trigger the pretty-printer but helps ensure
 * the compiler sees all the OpenMP constructs */
#ifdef DUMP_OMP
void __attribute__((used)) hint_omp_clauses() {
    /* References to trigger internal clause generation */
    #pragma omp parallel
    {
        #pragma omp for reduction(+:vol_seed)
        for (int i = 0; i < 10; i++) {
            vol_seed += i;
        }
        
        #pragma omp simd
        for (int i = 0; i < 10; i++) {
            /* Empty */
        }
    }
}
#endif
