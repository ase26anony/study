/* test_omp_clauses.c - Program to trigger OpenMP internal clause pretty-printing */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Use volatile to prevent optimization of loop bounds */
volatile int g_volatile_bound = 100;

/* Function to be used with declare target enter */
#pragma omp declare target enter(add_vectors) to(p1:length(N)) to(p2:length(N)) to(p3:length(N))
void add_vectors(double *a, double *b, double *c, int n) {
    #pragma omp parallel for simd
    for (int i = 0; i < n; i++) {
        c[i] = a[i] + b[i];
    }
}

/* Another function with reduction and scan */
double compute_prefix_sum(double *arr, int n, double *prefix) {
    double sum = 0.0;
    
    /* This should generate _reductemp_ and _scantemp_ clauses */
    #pragma omp parallel for simd reduction(+:sum) scan(inscan:sum)
    for (int i = 0; i < n; i++) {
        double val = arr[i];
        
        #pragma omp scan inclusive(sum)
        sum += val;
        prefix[i] = sum;
    }
    
    return sum;
}

/* Function with nested loops and collapse - may generate _condtemp_ */
void process_matrix(int rows, int cols, double *matrix) {
    int bound = g_volatile_bound; /* Volatile prevents constant propagation */
    
    /* Complex loop bound may require condition temporaries */
    #pragma omp parallel for collapse(2) if(rows * cols > 1000)
    for (int i = 0; i < rows && i < bound; i++) {
        for (int j = 0; j < cols && j < bound; j++) {
            int idx = i * cols + j;
            /* Data-dependent operation */
            if ((i + j) % 2 == 0) {
                matrix[idx] *= 2.0;
            } else {
                matrix[idx] /= 2.0;
            }
        }
    }
}

/* Function using target with teams and reduction */
#pragma omp declare target
double target_reduction(double *data, int n) {
    double sum = 0.0;
    
    #pragma omp teams distribute parallel for reduction(+:sum) \
                num_teams(4) thread_limit(64)
    for (int i = 0; i < n; i++) {
        sum += data[i] * (i % 10);
    }
    
    return sum;
}
#pragma omp end declare target

int main(int argc, char **argv) {
    /* Use argc for pseudo-random but reproducible sizes */
    int seed = (argc > 1) ? atoi(argv[1]) : 42;
    srand(seed);
    
    /* Variable sizes prevent compile-time optimization */
    int N = 500 + (rand() % 100);
    int rows = 50 + (rand() % 20);
    int cols = 50 + (rand() % 20);
    
    printf("Testing with N=%d, rows=%d, cols=%d\n", N, rows, cols);
    
    /* Allocate arrays */
    double *arr1 = (double*)malloc(N * sizeof(double));
    double *arr2 = (double*)malloc(N * sizeof(double));
    double *arr3 = (double*)malloc(N * sizeof(double));
    double *prefix = (double*)malloc(N * sizeof(double));
    double *matrix = (double*)malloc(rows * cols * sizeof(double));
    
    if (!arr1 || !arr2 || !arr3 || !prefix || !matrix) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays with simple patterns */
    for (int i = 0; i < N; i++) {
        arr1[i] = i * 0.5;
        arr2[i] = i * 0.3;
    }
    
    for (int i = 0; i < rows * cols; i++) {
        matrix[i] = (double)(i % 100);
    }
    
    /* Test 1: Reduction and scan (targets _reductemp_ and _scantemp_) */
    double total_sum = compute_prefix_sum(arr1, N, prefix);
    printf("Prefix sum total: %f\n", total_sum);
    
    /* Test 2: Nested loops with collapse (may generate _condtemp_) */
    process_matrix(rows, cols, matrix);
    
    /* Test 3: declare target enter with to clause (targets ENTER clause) */
    #pragma omp target enter data map(to: arr1[:N], arr2[:N]) map(alloc: arr3[:N])
    
    /* Call the entered function on target */
    #pragma omp target teams distribute parallel for
    for (int i = 0; i < N; i += 100) {
        int end = (i + 100 < N) ? i + 100 : N;
        add_vectors(&arr1[i], &arr2[i], &arr3[i], end - i);
    }
    
    #pragma omp target exit data map(from: arr3[:N])
    
    /* Test 4: Target region with reduction */
    double target_sum = 0.0;
    #pragma omp target map(tofrom: target_sum) map(to: arr1[:N])
    {
        target_sum = target_reduction(arr1, N);
    }
    printf("Target reduction sum: %f\n", target_sum);
    
    /* Compute checksum to prevent dead code elimination */
    double checksum = 0.0;
    for (int i = 0; i < N; i += 10) {
        checksum += arr3[i] + prefix[i];
    }
    for (int i = 0; i < rows * cols; i += 20) {
        checksum += matrix[i];
    }
    
    printf("Final checksum: %f\n", checksum);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(arr3);
    free(prefix);
    free(matrix);
    
    return 0;
}

/* Dummy function to hint compiler about OpenMP clause types */
#ifdef DUMP_OMP
void __attribute__((used)) hint_omp_clauses() {
    /* These declarations might help keep OpenMP structures */
    asm volatile("" : : "r"(&omp_default_mem_alloc));
    asm volatile("" : : "r"(&omp_high_bw_mem_alloc));
    asm volatile("" : : "r"(&omp_const_mem_alloc));
}
#endif
