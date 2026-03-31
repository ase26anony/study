/* tree-pretty-print-coverage.c
 * Designed to trigger uncovered OpenMP clause pretty-printing code
 * Compile with: gcc -O1 -fopenmp -fdump-tree-omplower -fdump-tree-ompexp -o omp_coverage tree-pretty-print-coverage.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#ifdef DUMP_OMP
/* Dummy function to hint compiler about OpenMP clause types */
void dummy_omp_clause_hint(int clause_type) {
    /* This function doesn't do anything meaningful, but its existence
     * might influence code generation paths in the compiler */
    volatile int hint = clause_type;
    (void)hint;
}
#endif

/* Function to be used with declare target enter */
#pragma omp declare target enter(vec_add) to(array1, array2, result)
void vec_add(int n, double *a, double *b, double *c) {
    #pragma omp parallel for simd
    for (int i = 0; i < n; i++) {
        c[i] = a[i] + b[i];
    }
}

int main(int argc, char *argv[]) {
    /* Use argc to create variable but reproducible sizes */
    int seed = argc > 1 ? atoi(argv[1]) : 42;
    srand(seed);
    
    /* Create runtime-dependent sizes to prevent optimization */
    volatile int base_size = 1000;
    int n1 = base_size + (rand() % 100);
    int n2 = base_size + (rand() % 100);
    int rows = 50 + (rand() % 50);
    int cols = 50 + (rand() % 50);
    
    /* Allocate arrays with dynamic sizes */
    double *array1 = (double*)malloc(n1 * sizeof(double));
    double *array2 = (double*)malloc(n1 * sizeof(double));
    double *result = (double*)malloc(n1 * sizeof(double));
    int *matrix = (int*)malloc(rows * cols * sizeof(int));
    double *scan_array = (double*)malloc(n2 * sizeof(double));
    
    if (!array1 || !array2 || !result || !matrix || !scan_array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays with simple patterns */
    for (int i = 0; i < n1; i++) {
        array1[i] = i * 1.5;
        array2[i] = i * 0.5;
    }
    
    for (int i = 0; i < n2; i++) {
        scan_array[i] = (i % 10) * 0.1;
    }
    
    for (int i = 0; i < rows * cols; i++) {
        matrix[i] = i;
    }
    
    /* 1. OpenMP parallel for simd with reduction and scan 
     * Targets: _reductemp_ and _scantemp_ clauses */
    double sum = 0.0;
    double prefix_sum = 0.0;
    
    #pragma omp parallel for simd reduction(+:sum) \
            scan(inscan:prefix_sum) \
            schedule(static) \
            num_threads(omp_get_max_threads())
    for (int i = 0; i < n2; i++) {
        double val = scan_array[i];
        
        #pragma omp scan inclusive(prefix_sum)
        prefix_sum += val;
        
        /* Conditional operation based on thread and index */
        if (omp_get_thread_num() % 2 == 0) {
            val *= 1.1;
        }
        
        sum += val;
        scan_array[i] = prefix_sum;
    }
    
    /* 2. Nested OpenMP loop with collapse and volatile bound
     * Targets: _condtemp_ clause */
    volatile int bound_adjust = 5;
    int total = 0;
    
    #pragma omp parallel for collapse(2) reduction(+:total) \
            private(bound_adjust) \
            num_threads(omp_get_max_threads())
    for (int i = 0; i < rows - bound_adjust; i++) {
        for (int j = 0; j < cols - (bound_adjust % 3); j++) {
            int idx = i * cols + j;
            /* Data-dependent conditional */
            if (matrix[idx] % (omp_get_thread_num() + 2) == 0) {
                total += matrix[idx];
            } else {
                total -= matrix[idx] % 10;
            }
        }
    }
    
    /* 3. Use declare target enter function in a target region
     * Targets: ENTER clause with to() */
    #pragma omp target map(to: array1[0:n1], array2[0:n1]) \
                     map(from: result[0:n1]) \
                     device(0) \
                     if(n1 > 500)
    {
        vec_add(n1, array1, array2, result);
        
        /* Additional computation in target region */
        #pragma omp parallel for reduction(+:sum)
        for (int i = 0; i < n1; i++) {
            result[i] += (i % 100) * 0.01;
            sum += result[i];
        }
    }
    
    /* 4. Teams distribute parallel for with reduction
     * Additional complexity for clause generation */
    double team_sum = 0.0;
    #pragma omp target teams distribute parallel for \
            reduction(+:team_sum) \
            map(tofrom: team_sum) \
            num_teams(omp_get_max_threads() / 2) \
            thread_limit(64)
    for (int i = 0; i < n1; i += 2) {
        team_sum += array1[i] * array2[i];
        if (i % 3 == 0) {
            team_sum -= result[i % n1];
        }
    }
    
    /* 5. Combined parallel for simd with multiple clauses */
    double final_sum = 0.0;
    double final_prefix = 0.0;
    
    #pragma omp parallel for simd reduction(+:final_sum) \
            scan(inscan:final_prefix) \
            linear(i:1) \
            aligned(result:64) \
            safelen(16)
    for (int i = 0; i < n1 && i < 500; i++) {
        double val = result[i];
        
        #pragma omp scan inclusive(final_prefix)
        final_prefix += val;
        
        /* Complex conditional that might generate condition temporaries */
        int thread_id = omp_get_thread_num();
        int simd_lane = i % 8;  /* Simulate SIMD lane */
        
        if ((thread_id + simd_lane) % 4 == 0) {
            val *= 1.05;
        } else if ((thread_id * simd_lane) % 7 == 0) {
            val /= 1.02;
        }
        
        final_sum += val;
    }
    
    /* Compute checksum to prevent dead code elimination */
    double checksum = sum + total + team_sum + final_sum;
    
    /* Use results to prevent optimization */
    printf("Checksum: %f\n", checksum);
    printf("Array1[0]=%f, Result[100]=%f\n", array1[0], result[100 % n1]);
    printf("Scan array[50]=%f, Matrix total=%d\n", scan_array[50 % n2], total);
    
    #ifdef DUMP_OMP
    /* Hint compiler about OpenMP clause types */
    dummy_omp_clause_hint(0);  /* OMP_CLAUSE__REDUCTEMP_ */
    dummy_omp_clause_hint(1);  /* OMP_CLAUSE__CONDTEMP_ */
    dummy_omp_clause_hint(2);  /* OMP_CLAUSE__SCANTEMP_ */
    dummy_omp_clause_hint(3);  /* OMP_CLAUSE_ENTER */
    #endif
    
    /* Cleanup */
    free(array1);
    free(array2);
    free(result);
    free(matrix);
    free(scan_array);
    
    return 0;
}
