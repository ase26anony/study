/* tree-pretty-print-coverage.c
 * Designed to trigger uncovered lines 512-523 in tree-pretty-print.cc
 * Compile with: gcc -O1 -fopenmp -fdump-tree-omplower -fdump-tree-ompexp -o test_omp test_omp.c
 * Additional flags: -fopenmp-simd -fdump-tree-all -foffload=disable
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Function to be used with declare target enter */
#pragma omp declare target enter(vec_add) to(array1, array2, result)
void vec_add(int n, double *a, double *b, double *c) {
    #pragma omp parallel for simd
    for (int i = 0; i < n; i++) {
        c[i] = a[i] + b[i];
    }
}

/* Another function with reduction and scan */
double compute_prefix_sum(int n, double *data, double *prefix) {
    double sum = 0.0;
    volatile int bound = n; /* Prevent optimization */
    
    /* This should generate _reductemp_ and _scantemp_ */
    #pragma omp parallel for simd reduction(+:sum) \
            scan(inscan:prefix_sum:sum) \
            collapse(1) if(bound > 1000)
    for (int i = 0; i < bound; i++) {
        double val = data[i];
        
        /* Inscan phase */
        #pragma omp scan inclusive(prefix_sum)
        {
            sum += val;
            prefix[i] = sum;
        }
        
        /* Conditional operation to potentially generate _condtemp_ */
        if (i % (omp_get_thread_num() + 1) == 0) {
            prefix[i] *= 1.01;
        }
    }
    return sum;
}

/* Function with nested collapse for _condtemp_ */
void nested_collapse_operation(int m, int n, double *matrix) {
    volatile int rows = m;
    volatile int cols = n;
    
    /* Complex collapse with volatile bounds may generate _condtemp_ */
    #pragma omp parallel for collapse(2) \
            private(j) shared(matrix) \
            if(rows * cols > 1000)
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            int idx = i * cols + j;
            /* Data-dependent operation */
            matrix[idx] = (i * 1.5 + j * 2.3) / (idx + 1);
            
            /* Thread-dependent condition */
            if (omp_get_thread_num() % 2 == 0) {
                matrix[idx] += 0.5;
            }
        }
    }
}

/* Function with reduction in teams */
void teams_reduction(int n, double *arr, double *result) {
    double total = 0.0;
    
    #pragma omp target teams distribute parallel for \
            map(to: arr[0:n]) map(from: total) \
            reduction(+:total) if(n > 500)
    for (int i = 0; i < n; i++) {
        total += arr[i] * (i % 10);
    }
    
    *result = total;
}

int main(int argc, char **argv) {
    /* Use argc for pseudo-random but reproducible sizes */
    int seed = (argc > 1) ? atoi(argv[1]) : 42;
    srand(seed);
    
    /* Variable sizes to prevent optimization */
    int size1 = 500 + (rand() % 500);
    int size2 = 300 + (rand() % 300);
    int rows = 50 + (rand() % 50);
    int cols = 40 + (rand() % 40);
    
    printf("Running with sizes: %d, %d, %dx%d\n", 
           size1, size2, rows, cols);
    
    /* Allocate arrays */
    double *array1 = (double *)malloc(size1 * sizeof(double));
    double *array2 = (double *)malloc(size1 * sizeof(double));
    double *result = (double *)malloc(size1 * sizeof(double));
    double *prefix = (double *)malloc(size1 * sizeof(double));
    double *matrix = (double *)malloc(rows * cols * sizeof(double));
    double *data = (double *)malloc(size2 * sizeof(double));
    
    if (!array1 || !array2 || !result || !prefix || !matrix || !data) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays with patterns */
    for (int i = 0; i < size1; i++) {
        array1[i] = i * 1.1;
        array2[i] = i * 0.9;
    }
    
    for (int i = 0; i < size2; i++) {
        data[i] = (i % 20) * 0.5;
    }
    
    /* 1. Test reduction with scan (for _reductemp_ and _scantemp_) */
    printf("Computing prefix sum...\n");
    double total_sum = compute_prefix_sum(size2, data, prefix);
    printf("Total sum: %f\n", total_sum);
    
    /* 2. Test nested collapse (for _condtemp_) */
    printf("Running nested collapse operation...\n");
    nested_collapse_operation(rows, cols, matrix);
    
    /* 3. Use declare target enter function */
    printf("Using declare target enter function...\n");
    
    /* Update arrays to target device */
    #pragma omp target update to(array1[0:size1], array2[0:size1])
    
    /* Call the entered function on target */
    #pragma omp target teams distribute parallel for \
            map(to: size1) map(tofrom: result[0:size1])
    for (int i = 0; i < size1; i += 256) {
        int chunk = (i + 256 < size1) ? 256 : (size1 - i);
        vec_add(chunk, &array1[i], &array2[i], &result[i]);
    }
    
    /* 4. Test teams reduction */
    printf("Running teams reduction...\n");
    double reduction_result;
    teams_reduction(size2, data, &reduction_result);
    printf("Reduction result: %f\n", reduction_result);
    
    /* Compute checksum to prevent dead code elimination */
    double checksum = 0.0;
    for (int i = 0; i < size1 && i < 100; i++) {
        checksum += result[i] + prefix[i % size2];
    }
    for (int i = 0; i < rows * cols && i < 100; i++) {
        checksum += matrix[i];
    }
    
    printf("Final checksum: %f\n", checksum);
    
    /* Cleanup */
    free(array1);
    free(array2);
    free(result);
    free(prefix);
    free(matrix);
    free(data);
    
    return 0;
}

/* Dummy function to hint compiler about OpenMP clauses
 * This won't directly trigger pretty-printing but helps keep
 * the structures in the intermediate representation */
#ifdef DUMP_OMP
void __attribute__((used)) 
dummy_omp_hint(int clause_type) {
    switch (clause_type) {
        case 0: /* _REDUCTEMP_ */ break;
        case 1: /* _CONDTEMP_ */ break;
        case 2: /* _SCANTEMP_ */ break;
        case 3: /* ENTER */ break;
    }
}
#endif
