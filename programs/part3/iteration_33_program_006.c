/* test_omp_clauses.c - Test program for OpenMP internal clause pretty-printing */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Use volatile to prevent optimization of loop bounds */
volatile int g_volatile_bound = 100;

/* Function to be used with declare target enter */
#pragma omp declare target enter(add_vectors) to(map_to_from:result)
void add_vectors(int *a, int *b, int *result, int n) {
    #pragma omp parallel for simd
    for (int i = 0; i < n; i++) {
        result[i] = a[i] + b[i];
    }
}

/* Another function with reduction and scan */
double compute_prefix_sum(double *data, int n, double *prefix) {
    double sum = 0.0;
    
    /* This should generate _reductemp_ and _scantemp_ clauses */
    #pragma omp parallel for simd reduction(+:sum) \
            scan(inscan:prefix_sum:sum)
    for (int i = 0; i < n; i++) {
        double prefix_sum;
        
        #pragma omp scan inclusive(prefix_sum)
        prefix_sum = sum + data[i];
        
        prefix[i] = prefix_sum;
        sum = prefix_sum;
    }
    
    return sum;
}

/* Function with nested collapsed loops - may generate _condtemp_ */
void process_matrix(int **matrix, int rows, int cols) {
    int bound = g_volatile_bound;
    
    /* Complex loop bound may generate condition temporaries */
    #pragma omp parallel for collapse(2) \
            if(rows * cols > bound)
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            /* Data-dependent operation */
            if ((i + j) % 2 == 0) {
                matrix[i][j] *= 2;
            } else {
                matrix[i][j] /= 2;
            }
        }
    }
}

/* Function using target with teams and reduction */
#pragma omp declare target
double target_reduction(double *array, int n) {
    double sum = 0.0;
    
    #pragma omp teams distribute parallel for reduction(+:sum) \
            num_teams(4) thread_limit(64)
    for (int i = 0; i < n; i++) {
        sum += array[i] * (i % 10);
    }
    
    return sum;
}
#pragma omp end declare target

int main(int argc, char **argv) {
    /* Use argc for pseudo-random but reproducible sizes */
    int seed = (argc > 1) ? atoi(argv[1]) : 42;
    srand(seed);
    
    /* Variable sizes to prevent optimization */
    int n1 = 1000 + (rand() % 100);
    int n2 = 500 + (rand() % 50);
    int rows = 50 + (rand() % 20);
    int cols = 40 + (rand() % 20);
    
    printf("Testing OpenMP internal clause generation\n");
    printf("Array sizes: n1=%d, n2=%d, matrix=%dx%d\n", n1, n2, rows, cols);
    
    /* Allocate and initialize arrays */
    double *data1 = (double*)malloc(n1 * sizeof(double));
    double *prefix = (double*)malloc(n1 * sizeof(double));
    int *array1 = (int*)malloc(n2 * sizeof(int));
    int *array2 = (int*)malloc(n2 * sizeof(int));
    int *result = (int*)malloc(n2 * sizeof(int));
    
    /* Initialize with simple patterns */
    for (int i = 0; i < n1; i++) {
        data1[i] = (i % 7) * 1.5;
    }
    
    for (int i = 0; i < n2; i++) {
        array1[i] = i;
        array2[i] = n2 - i;
    }
    
    /* Allocate matrix */
    int **matrix = (int**)malloc(rows * sizeof(int*));
    for (int i = 0; i < rows; i++) {
        matrix[i] = (int*)malloc(cols * sizeof(int));
        for (int j = 0; j < cols; j++) {
            matrix[i][j] = i * cols + j;
        }
    }
    
    /* Test 1: Reduction and scan - should generate _reductemp_ and _scantemp_ */
    printf("Test 1: Computing prefix sum with reduction and scan\n");
    double total_sum = compute_prefix_sum(data1, n1, prefix);
    printf("  Total sum: %f\n", total_sum);
    
    /* Test 2: Nested collapsed loops - may generate _condtemp_ */
    printf("Test 2: Processing matrix with collapsed loops\n");
    process_matrix(matrix, rows, cols);
    
    /* Test 3: Declare target enter with to clause */
    printf("Test 3: Using declare target enter with to clause\n");
    
    /* The declare target directive above should generate ENTER clause */
    #pragma omp target map(to: array1[0:n2], array2[0:n2]) \
                      map(from: result[0:n2])
    {
        add_vectors(array1, array2, result, n2);
    }
    
    /* Test 4: Target region with teams and reduction */
    printf("Test 4: Target region with teams reduction\n");
    double *target_array = (double*)malloc(n1 * sizeof(double));
    for (int i = 0; i < n1; i++) {
        target_array[i] = data1[i];
    }
    
    double target_sum;
    #pragma omp target map(to: target_array[0:n1]) map(from: target_sum)
    {
        target_sum = target_reduction(target_array, n1);
    }
    printf("  Target reduction sum: %f\n", target_sum);
    
    /* Compute checksum to prevent dead code elimination */
    double checksum = 0.0;
    checksum += total_sum;
    checksum += target_sum;
    
    for (int i = 0; i < n2; i += 10) {
        checksum += result[i];
    }
    
    for (int i = 0; i < rows; i += 5) {
        for (int j = 0; j < cols; j += 5) {
            checksum += matrix[i][j];
        }
    }
    
    printf("Final checksum: %f\n", checksum);
    
    /* Cleanup */
    free(data1);
    free(prefix);
    free(array1);
    free(array2);
    free(result);
    free(target_array);
    
    for (int i = 0; i < rows; i++) {
        free(matrix[i]);
    }
    free(matrix);
    
    return 0;
}

/* Dummy function to hint compiler about OpenMP clause types
 * This won't directly trigger pretty-printing but helps keep
 * the structures in the IR */
#ifdef DUMP_OMP
void __attribute__((used)) hint_omp_clauses() {
    /* These don't need to be called, just referenced */
    #pragma omp parallel
    {
        int dummy = 0;
    }
    
    #pragma omp simd
    for (int i = 0; i < 10; i++) {
        /* empty */
    }
}
#endif
