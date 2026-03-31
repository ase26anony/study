/* tree-pretty-print-test.c */
/* Compile with: gcc -O1 -fopenmp -fdump-tree-omplower -fdump-tree-all -foffload=disable tree-pretty-print-test.c -o test */
/* Also try: gcc -O0 -fopenmp -foffload-abi=lp64 -fdump-tree-original tree-pretty-print-test.c -o test */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Use volatile to prevent optimization of loop bounds */
volatile int g_volatile_bound = 100;

/* Function to be used with declare target enter */
#pragma omp declare target enter(vec_add) to(map_to_from:array)
void vec_add(double *a, double *b, double *c, int n) {
    #pragma omp parallel for simd
    for (int i = 0; i < n; i++) {
        c[i] = a[i] + b[i];
    }
}

/* Another function with reduction that may generate _reductemp_ */
double complex_reduction(double *data, int n) {
    double total = 0.0;
    
    /* Combined construct with reduction and scan - may generate both _reductemp_ and _scantemp_ */
    #pragma omp parallel for simd reduction(+:total) \
            simdlen(4) if(omp_get_max_threads() > 1)
    for (int i = 0; i < n; i++) {
        total += data[i] * data[i];
    }
    
    return total;
}

/* Function with scan directive - explicitly generates _scantemp_ */
void inclusive_scan(double *input, double *output, int n) {
    double prefix_sum = 0.0;
    
    /* This should generate _scantemp_ clauses */
    #pragma omp parallel for simd reduction(inscan, +:prefix_sum)
    for (int i = 0; i < n; i++) {
        double val = input[i];
        
        #pragma omp scan inclusive(prefix_sum)
        prefix_sum += val;
        output[i] = prefix_sum;
    }
}

/* Function with collapse clause - may generate _condtemp_ */
void nested_collapse(int *matrix, int rows, int cols) {
    /* Use volatile variable for loop bound to prevent optimization */
    int bound = g_volatile_bound;
    
    /* Collapsed loop with non-trivial bound - may generate _condtemp_ */
    #pragma omp parallel for collapse(2) \
            if(bound > 50)  /* Condition to potentially generate condition temporaries */
    for (int i = 0; i < rows && i < bound; i++) {
        for (int j = 0; j < cols && j < bound; j++) {
            int idx = i * cols + j;
            /* Data-dependent operation */
            if ((i + j) % 2 == 0) {
                matrix[idx] *= 2;
            } else {
                matrix[idx] /= 2;
            }
        }
    }
}

/* Target region using the entered function */
void target_computation(double *a, double *b, double *c, int n) {
    #pragma omp target map(to: a[0:n], b[0:n]) map(from: c[0:n]) \
            if(n > 100)  /* Conditional to potentially generate more temporaries */
    {
        vec_add(a, b, c, n);
    }
}

int main(int argc, char **argv) {
    /* Use argc for pseudo-random but reproducible sizes */
    int seed = (argc > 1) ? atoi(argv[1]) : 42;
    srand(seed);
    
    /* Variable sizes to prevent compile-time optimization */
    int size1 = 100 + (rand() % 100);
    int size2 = 50 + (rand() % 50);
    int rows = 30 + (rand() % 20);
    int cols = 40 + (rand() % 20);
    
    printf("Running with sizes: %d, %d, matrix %dx%d\n", 
           size1, size2, rows, cols);
    
    /* Allocate arrays */
    double *array1 = (double *)malloc(size1 * sizeof(double));
    double *array2 = (double *)malloc(size1 * sizeof(double));
    double *array3 = (double *)malloc(size1 * sizeof(double));
    double *scan_input = (double *)malloc(size2 * sizeof(double));
    double *scan_output = (double *)malloc(size2 * sizeof(double));
    int *matrix = (int *)malloc(rows * cols * sizeof(int));
    
    if (!array1 || !array2 || !array3 || !scan_input || !scan_output || !matrix) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays with patterns */
    for (int i = 0; i < size1; i++) {
        array1[i] = i * 0.5;
        array2[i] = i * 0.3;
    }
    
    for (int i = 0; i < size2; i++) {
        scan_input[i] = (i % 10) * 0.1;
    }
    
    for (int i = 0; i < rows * cols; i++) {
        matrix[i] = i + 1;
    }
    
    /* 1. Test reduction (potential _reductemp_) */
    double reduction_result = complex_reduction(array1, size1);
    printf("Reduction result: %f\n", reduction_result);
    
    /* 2. Test scan (explicit _scantemp_) */
    inclusive_scan(scan_input, scan_output, size2);
    
    /* 3. Test nested collapse (potential _condtemp_) */
    nested_collapse(matrix, rows, cols);
    
    /* 4. Test target with entered function (ENTER clause) */
    target_computation(array1, array2, array3, size1);
    
    /* 5. Additional combined construct with multiple features */
    double final_sum = 0.0;
    #pragma omp target teams distribute parallel for \
            reduction(+:final_sum) map(tofrom: final_sum) \
            if(size1 > 150)  /* Conditional */
    for (int i = 0; i < size1; i++) {
        /* Thread-dependent operation */
        if (omp_get_thread_num() % 3 == 0) {
            final_sum += array3[i];
        } else {
            final_sum += array1[i];
        }
    }
    
    /* Compute checksum to prevent dead code elimination */
    double checksum = reduction_result + final_sum;
    for (int i = 0; i < size2 && i < 10; i++) {
        checksum += scan_output[i];
    }
    for (int i = 0; i < rows * cols && i < 20; i += rows + 1) {
        checksum += matrix[i];
    }
    
    printf("Final checksum: %f\n", checksum);
    
    /* Cleanup */
    free(array1);
    free(array2);
    free(array3);
    free(scan_input);
    free(scan_output);
    free(matrix);
    
    return 0;
}

/* Dummy function to potentially trigger pretty-printing if compiled with DUMP_OMP */
#ifdef DUMP_OMP
void __attribute__((used)) dump_hint() {
    /* This function doesn't do anything but may help keep symbols */
    volatile int hint = 0;
    (void)hint;
}
#endif
