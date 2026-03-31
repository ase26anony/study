/* omp_coverage_test.c
 * Designed to trigger uncovered lines 512-523 in tree-pretty-print.cc
 * Compile with: gcc -O1 -fopenmp -fdump-tree-omplower -fdump-tree-ompexp omp_coverage_test.c -o omp_coverage_test
 * Additional flags for more coverage: -O2 -fopenmp-simd -fdump-tree-all -foffload=disable
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Function to be used with declare target enter clause */
#pragma omp declare target enter(add_vectors) to(a, b, c)
void add_vectors(double *a, double *b, double *c, int n) {
    #pragma omp parallel for simd
    for (int i = 0; i < n; i++) {
        c[i] = a[i] + b[i];
    }
}

/* Another function with reduction and scan */
double compute_prefix_sum(double *arr, int n, volatile int use_scan) {
    double prefix_sum = 0.0;
    double total = 0.0;
    
    /* This should generate _reductemp_ and _scantemp_ clauses */
    #pragma omp parallel for simd reduction(+:total) \
            scan(inscan:prefix_sum) if(n > 1000)
    for (int i = 0; i < n; i++) {
        double val = arr[i];
        
        /* Inscan phase */
        #pragma omp scan inclusive(prefix_sum)
        prefix_sum += val;
        
        /* Use thread-dependent computation to prevent optimization */
        if (omp_get_thread_num() % 2 == 0) {
            val *= 1.001;
        }
        
        total += val;
    }
    
    return total + prefix_sum / n;
}

/* Function with nested loops and collapse - may generate _condtemp_ */
void process_matrix(double **matrix, int rows, int cols, volatile int bound) {
    int effective_rows = rows % bound + 1;  /* Volatile prevents constant folding */
    
    /* Collapsed loop with non-trivial bound may generate condition temporaries */
    #pragma omp parallel for collapse(2) \
            if(effective_rows * cols > 1000)
    for (int i = 0; i < effective_rows; i++) {
        for (int j = 0; j < cols; j++) {
            /* Data-dependent computation */
            if ((i + j) % 3 == 0) {
                matrix[i][j] *= 1.5;
            } else if ((i * j) % 5 == 0) {
                matrix[i][j] /= 1.3;
            }
            
            /* Use thread ID to prevent optimization */
            matrix[i][j] += omp_get_thread_num() * 0.001;
        }
    }
}

/* Target region using the entered function */
void target_computation(double *a, double *b, double *c, int n) {
    #pragma omp target map(tofrom: a[0:n], b[0:n]) map(from: c[0:n]) \
            if(n > 500)
    {
        add_vectors(a, b, c, n);
        
        /* Additional computation in target region */
        #pragma omp parallel for reduction(+:c[0:n])
        for (int i = 0; i < n; i++) {
            c[i] += i * 0.01;
        }
    }
}

int main(int argc, char **argv) {
    /* Use argc for pseudo-random but reproducible sizes */
    int seed = (argc > 1) ? atoi(argv[1]) : 42;
    srand(seed);
    
    /* Volatile variables to prevent optimization */
    volatile int v_size_factor = rand() % 100 + 50;
    volatile int use_scan = 1;
    
    /* Array sizes based on volatile to prevent constant propagation */
    int n1 = 1000 + v_size_factor;
    int n2 = 500 + (seed % 200);
    int rows = 100 + (seed % 50);
    int cols = 100 + (seed % 50);
    
    printf("Testing OpenMP coverage with sizes: n1=%d, n2=%d, rows=%d, cols=%d\n",
           n1, n2, rows, cols);
    
    /* Allocate and initialize arrays */
    double *arr1 = (double *)malloc(n1 * sizeof(double));
    double *arr2 = (double *)malloc(n2 * sizeof(double));
    double *arr3 = (double *)malloc(n2 * sizeof(double));
    
    double **matrix = (double **)malloc(rows * sizeof(double *));
    for (int i = 0; i < rows; i++) {
        matrix[i] = (double *)malloc(cols * sizeof(double));
        for (int j = 0; j < cols; j++) {
            matrix[i][j] = (i * cols + j) * 0.1;
        }
    }
    
    for (int i = 0; i < n1; i++) {
        arr1[i] = i * 0.01;
    }
    
    for (int i = 0; i < n2; i++) {
        arr2[i] = (i % 10) * 0.1;
        arr3[i] = 0.0;
    }
    
    /* 1. Test reduction and scan - should generate _reductemp_ and _scantemp_ */
    double result1 = compute_prefix_sum(arr1, n1, use_scan);
    printf("Prefix sum result: %f\n", result1);
    
    /* 2. Test nested collapsed loops - may generate _condtemp_ */
    process_matrix(matrix, rows, cols, v_size_factor);
    
    /* 3. Test declare target enter with to clause */
    target_computation(arr2, arr3, arr2, n2);
    
    /* 4. Additional combined construct with multiple clauses */
    double final_sum = 0.0;
    #pragma omp target teams distribute parallel for \
            reduction(+:final_sum) map(tofrom: final_sum) \
            if(n1 > 800)
    for (int i = 0; i < n1; i++) {
        final_sum += arr1[i];
        
        /* Nested loop inside teams region */
        #pragma omp simd
        for (int j = 0; j < 4; j++) {
            arr1[i] += j * 0.001;
        }
    }
    
    /* 5. Test scan in SIMD context */
    double scan_buffer[100];
    for (int i = 0; i < 100; i++) scan_buffer[i] = i * 0.1;
    
    double scan_sum = 0.0;
    #pragma omp simd reduction(+:scan_sum) scan(inscan:scan_buffer[0:100])
    for (int i = 0; i < 100; i++) {
        #pragma omp scan inclusive(scan_buffer[i])
        scan_buffer[i] += scan_sum;
        scan_sum += i * 0.1;
    }
    
    /* Compute checksum to prevent dead code elimination */
    double checksum = result1 + final_sum + scan_sum;
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            checksum += matrix[i][j];
        }
    }
    
    printf("Final checksum: %f\n", checksum / (n1 + n2 + rows * cols));
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(arr3);
    for (int i = 0; i < rows; i++) {
        free(matrix[i]);
    }
    free(matrix);
    
    return 0;
}

/* Dummy function to hint compiler about OpenMP clause types
 * This might help keep clause information in the AST
 */
#ifdef DUMP_OMP
void __attribute__((used)) hint_omp_clauses() {
    /* References to trigger thinking about these clauses */
    asm volatile("" : : "r"((long)OMP_CLAUSE__REDUCTEMP_));
    asm volatile("" : : "r"((long)OMP_CLAUSE__CONDTEMP_));
    asm volatile("" : : "r"((long)OMP_CLAUSE__SCANTEMP_));
    asm volatile("" : : "r"((long)OMP_CLAUSE_ENTER));
}
#endif
