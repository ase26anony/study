/* tree-pretty-print-coverage.c
 * Designed to trigger uncovered lines 512-523 in tree-pretty-print.cc
 * Compile with: gcc -O1 -fopenmp -fdump-tree-omplower -fdump-tree-ompexp -o coverage_test tree-pretty-print-coverage.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Function to be used with declare target enter */
#pragma omp declare target enter(vec_add) to(p1)
void vec_add(double *a, double *b, double *c, int n) {
    #pragma omp parallel for simd
    for (int i = 0; i < n; i++) {
        c[i] = a[i] + b[i];
    }
}

/* Another function with declare target and link */
#pragma omp declare target enter(mat_mult) link(mat_mult)
void mat_mult(double *A, double *B, double *C, int n) {
    #pragma omp target teams distribute parallel for collapse(2) map(to: A[0:n*n], B[0:n*n]) map(from: C[0:n*n])
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            double sum = 0.0;
            for (int k = 0; k < n; k++) {
                sum += A[i*n + k] * B[k*n + j];
            }
            C[i*n + j] = sum;
        }
    }
}

int main(int argc, char **argv) {
    /* Use argc for pseudo-random but reproducible sizes */
    int seed = (argc > 1) ? atoi(argv[1]) : 42;
    srand(seed);
    
    /* Prevent optimization with volatile and runtime values */
    volatile int v_size = 100 + (rand() % 100);
    int size = v_size;
    int scan_size = 50 + (rand() % 50);
    
    /* Allocate arrays with runtime sizes */
    double *array1 = (double *)malloc(size * sizeof(double));
    double *array2 = (double *)malloc(size * sizeof(double));
    double *result = (double *)malloc(size * sizeof(double));
    double *scan_array = (double *)malloc(scan_size * sizeof(double));
    int *cond_array = (int *)malloc(size * sizeof(int));
    
    if (!array1 || !array2 || !result || !scan_array || !cond_array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays with simple patterns */
    for (int i = 0; i < size; i++) {
        array1[i] = i * 1.5;
        array2[i] = i * 0.5;
        cond_array[i] = i % 10;
    }
    
    for (int i = 0; i < scan_size; i++) {
        scan_array[i] = (i % 3) + 0.5;
    }
    
    /* ====== SECTION 1: Generate _reductemp_ and _scantemp_ ====== */
    /* Use parallel for simd with reduction and scan */
    double total_sum = 0.0;
    double prefix_sum = 0.0;
    
    #pragma omp parallel for simd reduction(+:total_sum) \
            scan(inscan:prefix_sum) if(size > 50)
    for (int i = 0; i < scan_size; i++) {
        double val = scan_array[i];
        
        /* Data-dependent operation */
        #pragma omp scan inclusive(prefix_sum)
        prefix_sum += val;
        
        total_sum += val;
        
        /* Conditional operation based on thread/iteration */
        if (i % (omp_get_thread_num() + 1) == 0) {
            scan_array[i] = prefix_sum;
        }
    }
    
    /* ====== SECTION 2: Generate _condtemp_ ====== */
    /* Use collapse with non-trivial loop bounds */
    volatile int outer_bound = size / 10;
    volatile int inner_bound = 10;
    
    #pragma omp parallel for collapse(2) \
            private(result) shared(cond_array) \
            schedule(dynamic, 4)
    for (int i = 0; i < outer_bound; i++) {
        for (int j = 0; j < inner_bound; j++) {
            int idx = i * inner_bound + j;
            if (idx < size) {
                /* Complex condition that may generate condition temporaries */
                cond_array[idx] = (cond_array[idx] > 5) ? 
                                 (cond_array[idx] * 2) : 
                                 (cond_array[idx] + omp_get_thread_num());
            }
        }
    }
    
    /* ====== SECTION 3: Trigger ENTER clause with to() ====== */
    /* Use declare target enter with to mapper */
    #pragma omp declare target enter(vec_add) to(array1, array2, result)
    
    /* Target region using the entered function */
    #pragma omp target map(to: size) map(tofrom: array1[0:size], array2[0:size]) \
                     map(from: result[0:size]) if(size > 20)
    {
        vec_add(array1, array2, result, size);
    }
    
    /* Additional declare target with link */
    #pragma omp declare target enter(mat_mult) link(mat_mult)
    
    /* ====== SECTION 4: Complex combined construct ====== */
    /* Teams distribute parallel for with reduction */
    int teams_reduction = 0;
    
    #pragma omp target teams distribute parallel for \
            reduction(+:teams_reduction) map(to: cond_array[0:size]) \
            num_teams(4) thread_limit(64) if(size > 30)
    for (int i = 0; i < size; i++) {
        teams_reduction += cond_array[i] % 7;
    }
    
    /* ====== SECTION 5: Nested parallelism with conditionals ====== */
    double nested_sum = 0.0;
    
    #pragma omp parallel reduction(+:nested_sum)
    {
        int tid = omp_get_thread_num();
        #pragma omp for nowait
        for (int i = 0; i < size; i++) {
            /* Data-dependent conditional */
            if ((i + tid) % 3 == 0) {
                nested_sum += array1[i];
            } else {
                nested_sum += array2[i];
            }
        }
        
        /* Additional work in parallel region */
        #pragma omp barrier
        
        #pragma omp for simd reduction(+:nested_sum) schedule(static, 8)
        for (int i = 0; i < size/2; i++) {
            nested_sum += result[i] * 0.1;
        }
    }
    
    /* ====== SECTION 6: Target update with enter/exit ====== */
    #pragma omp target update from(result[0:size/2]) if(size > 25)
    
    /* ====== Compute checksum to prevent optimization ====== */
    double checksum = total_sum + prefix_sum + nested_sum + teams_reduction;
    
    for (int i = 0; i < size && i < 10; i++) {
        checksum += result[i] + cond_array[i];
    }
    
    printf("Checksum: %f\n", checksum);
    printf("Total sum: %f, Prefix sum: %f\n", total_sum, prefix_sum);
    printf("Teams reduction: %d, Nested sum: %f\n", teams_reduction, nested_sum);
    
    /* Cleanup */
    free(array1);
    free(array2);
    free(result);
    free(scan_array);
    free(cond_array);
    
    return 0;
}
