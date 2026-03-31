/* tree-pretty-print-test.c - Test for uncovered OpenMP clause pretty-printing */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* For preventing optimization */
volatile int g_volatile_bound = 100;

/* Function to be used with declare target enter */
#pragma omp declare target enter(vec_add) to(p1:length(N)) to(p2:length(N)) to(result:length(N))
void vec_add(int *p1, int *p2, int *result, int N) {
    #pragma omp parallel for simd
    for (int i = 0; i < N; i++) {
        result[i] = p1[i] + p2[i];
    }
}

/* Another function with reduction that may generate _reductemp_ */
double complex_reduction(int *data, int n) {
    double sum = 0.0;
    double prefix_sum = 0.0;
    
    /* This combined construct may generate both _reductemp_ and _scantemp_ */
    #pragma omp parallel for simd reduction(+:sum) scan(inscan:prefix_sum)
    for (int i = 0; i < n; i++) {
        double val = data[i] * 0.5;
        
        #pragma omp scan inclusive(prefix_sum)
        prefix_sum += val;
        sum += val;
        
        /* Data-dependent operation to prevent optimization */
        if (prefix_sum > 1000.0) {
            prefix_sum *= 0.9;
        }
    }
    return sum + prefix_sum;
}

/* Function with nested loops that may generate _condtemp_ */
void nested_collapse(int *matrix, int rows, int cols) {
    volatile int bound = g_volatile_bound;
    int effective_rows = (rows < bound) ? rows : bound;
    
    /* Collapsed loop with volatile bound may generate condition temporaries */
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < effective_rows; i++) {
        for (int j = 0; j < cols; j++) {
            int idx = i * cols + j;
            /* Thread-dependent operation */
            if (omp_get_thread_num() % 2 == 0) {
                matrix[idx] *= 2;
            } else {
                matrix[idx] /= 2;
            }
        }
    }
}

/* Function using target with teams and reduction */
#pragma omp declare target
double target_reduction(double *array, int size) {
    double sum = 0.0;
    
    #pragma omp teams distribute parallel for reduction(+:sum)
    for (int i = 0; i < size; i++) {
        sum += array[i];
        /* Complex operation to prevent optimization */
        array[i] = sum / (i + 1);
    }
    return sum;
}
#pragma omp end declare target

int main(int argc, char **argv) {
    /* Use argc for pseudo-random but reproducible sizes */
    int seed = (argc > 1) ? atoi(argv[1]) : 42;
    srand(seed);
    
    /* Create runtime-dependent sizes to prevent optimization */
    int N = 1000 + (rand() % 100);
    int rows = 50 + (rand() % 50);
    int cols = 20 + (rand() % 30);
    
    /* Allocate arrays */
    int *data = (int*)malloc(N * sizeof(int));
    int *matrix = (int*)malloc(rows * cols * sizeof(int));
    double *darray = (double*)malloc(N * sizeof(double));
    int *p1 = (int*)malloc(N * sizeof(int));
    int *p2 = (int*)malloc(N * sizeof(int));
    int *result = (int*)malloc(N * sizeof(int));
    
    if (!data || !matrix || !darray || !p1 || !p2 || !result) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays with patterns */
    for (int i = 0; i < N; i++) {
        data[i] = i % 100;
        darray[i] = (i % 50) * 0.1;
        p1[i] = i;
        p2[i] = N - i;
    }
    
    for (int i = 0; i < rows * cols; i++) {
        matrix[i] = i % 255;
    }
    
    /* 1. Test reduction with scan - may generate _reductemp_ and _scantemp_ */
    double red_result = complex_reduction(data, N);
    printf("Reduction+scan result: %f\n", red_result);
    
    /* 2. Test nested collapse - may generate _condtemp_ */
    nested_collapse(matrix, rows, cols);
    
    /* 3. Test declare target enter with to clause */
    #pragma omp target enter data map(to: p1[0:N], p2[0:N]) map(alloc: result[0:N])
    
    /* Call the entered function on target */
    #pragma omp target teams distribute parallel for
    for (int i = 0; i < N; i += 100) {
        int end = (i + 100 < N) ? i + 100 : N;
        for (int j = i; j < end; j++) {
            result[j] = p1[j] + p2[j];
        }
    }
    
    #pragma omp target exit data map(from: result[0:N]) map(delete: p1[0:N], p2[0:N])
    
    /* 4. Test target region with reduction */
    #pragma omp target map(tofrom: darray[0:N])
    {
        double local_sum = target_reduction(darray, N);
        /* Use the result to prevent dead code elimination */
        darray[0] += local_sum;
    }
    
    /* Compute checksum to verify execution */
    long long checksum = 0;
    for (int i = 0; i < N; i += 10) {
        checksum += data[i] + result[i] + (int)(darray[i] * 100);
    }
    
    for (int i = 0; i < rows * cols; i += 20) {
        checksum += matrix[i];
    }
    
    printf("Final checksum: %lld\n", checksum);
    
    /* Cleanup */
    free(data);
    free(matrix);
    free(darray);
    free(p1);
    free(p2);
    free(result);
    
    return 0;
}

/* Dummy function to hint compiler about OpenMP clause types */
#ifdef DUMP_OMP
void __attribute__((used)) dump_omp_clause(int clause_type) {
    /* This function is never called, but its existence with OpenMP-related
       parameters may influence code generation */
    switch (clause_type) {
        case 0: printf("_reductemp_"); break;
        case 1: printf("_condtemp_"); break;
        case 2: printf("_scantemp_"); break;
        case 3: printf("enter"); break;
    }
}
#endif
