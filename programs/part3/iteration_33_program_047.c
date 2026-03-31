/* tree-pretty-print-coverage.c
 * Targets uncovered lines in tree-pretty-print.cc (lines 512-523)
 * Compile with: gcc -O1 -fopenmp -fdump-tree-omplower -fdump-tree-ompexp -o test tree-pretty-print-coverage.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#ifdef DUMP_OMP
/* Dummy function to hint compiler about OpenMP clause types */
void dummy_omp_clause_hint(int clause_type) {
    volatile int hint = clause_type;
    (void)hint;
}
#endif

/* Function to be used with declare target enter */
#pragma omp declare target enter(add_vectors) to(array_a, array_b, array_c)
void add_vectors(double *a, double *b, double *c, int n) {
    #pragma omp parallel for simd
    for (int i = 0; i < n; i++) {
        c[i] = a[i] + b[i];
    }
}

int main(int argc, char *argv[]) {
    /* Use argc for reproducible but variable sizes */
    int seed = argc > 1 ? atoi(argv[1]) : 42;
    srand(seed);
    
    /* Prevent optimization with volatile and runtime values */
    volatile int base_size = 100 + (rand() % 100);
    int n = base_size;
    int m = 50 + (rand() % 50);
    
    /* Allocate arrays with dynamic sizes */
    double *array_a = (double *)malloc(n * sizeof(double));
    double *array_b = (double *)malloc(n * sizeof(double));
    double *array_c = (double *)malloc(n * sizeof(double));
    int *scan_array = (int *)malloc(n * sizeof(int));
    int *matrix = (int *)malloc(n * m * sizeof(int));
    
    if (!array_a || !array_b || !array_c || !scan_array || !matrix) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays */
    for (int i = 0; i < n; i++) {
        array_a[i] = i * 1.5;
        array_b[i] = i * 0.5;
        scan_array[i] = i % 10;
    }
    
    for (int i = 0; i < n * m; i++) {
        matrix[i] = i;
    }
    
    /* 1. REDUCTEMP and SCANTEMP: Use reduction + scan */
    int sum = 0;
    int prefix_sum = 0;
    
    #pragma omp parallel for simd reduction(+:sum) \
            scan(inscan:prefix_sum) if(n > 50)
    for (int i = 0; i < n; i++) {
        // Inscan phase
        {
            sum += scan_array[i];
            prefix_sum = sum;
        }
        #pragma omp scan inclusive(prefix_sum)
        // Use the scan result
        scan_array[i] = prefix_sum % 100;
    }
    
    /* 2. CONDTEMP: Nested loop with collapse and complex bounds */
    volatile int outer_bound = m;
    int total = 0;
    
    #pragma omp parallel for collapse(2) reduction(+:total) \
            private(seed) lastprivate(n)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < outer_bound; j++) {
            /* Data-dependent control flow */
            if ((i + j) % 3 == omp_get_thread_num() % 3) {
                matrix[i * m + j] *= 2;
            }
            total += matrix[i * m + j];
        }
    }
    
    /* 3. ENTER clause: Use declare target enter with to mapper */
    #pragma omp target enter data map(to: array_a[0:n], array_b[0:n]) \
            map(alloc: array_c[0:n]) if(n > 75)
    
    /* Call the function that was entered via declare target */
    #pragma omp target if(n > 75)
    {
        add_vectors(array_a, array_b, array_c, n);
    }
    
    #pragma omp target exit data map(from: array_c[0:n]) \
            map(delete: array_a[0:n], array_b[0:n])
    
    /* 4. Additional complex reduction for more reductemp chances */
    double complex_sum = 0.0;
    #pragma omp parallel for reduction(+:complex_sum) \
            schedule(dynamic, 4) if(n > 60)
    for (int i = 0; i < n; i++) {
        /* Nested reduction-like operation */
        double local_sum = 0.0;
        for (int k = 0; k < (i % 8) + 1; k++) {
            local_sum += array_c[i] * k;
        }
        complex_sum += local_sum;
        
        /* Thread-dependent conditional */
        if (omp_get_thread_num() % 2 == 0) {
            array_c[i] *= 1.1;
        }
    }
    
    /* Compute checksum to prevent dead code elimination */
    double checksum = 0.0;
    for (int i = 0; i < n && i < 20; i++) {
        checksum += array_c[i] + scan_array[i];
    }
    checksum += total + complex_sum;
    
    printf("Checksum: %f\n", checksum);
    
    #ifdef DUMP_OMP
    /* Hint compiler about clause types - may influence internal representation */
    dummy_omp_clause_hint(1 << 8);  /* _REDUCTEMP_ */
    dummy_omp_clause_hint(1 << 9);  /* _CONDTEMP_ */
    dummy_omp_clause_hint(1 << 10); /* _SCANTEMP_ */
    dummy_omp_clause_hint(1 << 11); /* ENTER */
    #endif
    
    /* Cleanup */
    free(array_a);
    free(array_b);
    free(array_c);
    free(scan_array);
    free(matrix);
    
    return 0;
}
