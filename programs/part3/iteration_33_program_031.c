/* omp_temp_clauses.c - Generate OpenMP constructs that create _reductemp_, _condtemp_, _scantemp_, and ENTER clauses */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Function to be used with declare target enter */
#pragma omp declare target enter(add_vectors) to(res)
void add_vectors(int n, double *a, double *b, double *res) {
    #pragma omp parallel for simd
    for (int i = 0; i < n; i++) {
        res[i] = a[i] + b[i];
    }
}

/* Another function for nested collapse */
void process_matrix(int rows, int cols, double *matrix) {
    volatile int use_rows = rows; /* Prevent optimization */
    volatile int use_cols = cols;
    
    /* This may generate _condtemp_ for collapse bounds */
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < use_rows; i++) {
        for (int j = 0; j < use_cols; j++) {
            int idx = i * cols + j;
            if ((i + j) % 2 == 0) {
                matrix[idx] *= 2.0;
            } else {
                matrix[idx] /= 2.0;
            }
        }
    }
}

int main(int argc, char **argv) {
    /* Use argc for pseudo-random but reproducible sizes */
    int base_size = 100 + (argc * 17) % 50;
    int n = base_size;
    int rows = 20 + (argc * 13) % 10;
    int cols = 15 + (argc * 7) % 10;
    
    /* Allocate arrays with runtime sizes */
    double *array1 = (double *)malloc(n * sizeof(double));
    double *array2 = (double *)malloc(n * sizeof(double));
    double *result = (double *)malloc(n * sizeof(double));
    double *matrix = (double *)malloc(rows * cols * sizeof(double));
    
    if (!array1 || !array2 || !result || !matrix) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays with simple patterns */
    for (int i = 0; i < n; i++) {
        array1[i] = i * 0.5;
        array2[i] = i * 0.25;
    }
    
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            matrix[i * cols + j] = (i + j) * 0.1;
        }
    }
    
    double sum = 0.0;
    double prefix_sum = 0.0;
    
    /* SECTION 1: Generate _reductemp_ and _scantemp_ clauses */
    /* Use reduction with scan to generate both temporary types */
    printf("Starting reduction with scan...\n");
    
    #pragma omp parallel for simd reduction(+:sum) \
            scan(inscan:prefix_sum) if(n > 50)
    for (int i = 0; i < n; i++) {
        double val = array1[i] + array2[i];
        
        #pragma omp scan inclusive(prefix_sum)
        prefix_sum += val;
        
        /* Conditional operation based on thread/iteration */
        if (i % (omp_get_thread_num() + 2) == 0) {
            val *= 1.1;
        }
        
        sum += val;
        result[i] = prefix_sum;
    }
    
    printf("Sum after reduction: %f\n", sum);
    printf("Final prefix_sum: %f\n", prefix_sum);
    
    /* SECTION 2: Generate _condtemp_ clause with collapse */
    printf("Processing matrix with collapse...\n");
    process_matrix(rows, cols, matrix);
    
    /* SECTION 3: Use declare target with ENTER clause */
    /* This should generate ENTER clause with to() specifier */
    printf("Using declare target enter...\n");
    
    /* Reset result array */
    for (int i = 0; i < n; i++) {
        result[i] = 0.0;
    }
    
    /* The declare target directive above should generate ENTER clause */
    /* Now use target region */
    #pragma omp target map(to: array1[0:n], array2[0:n]) \
                       map(from: result[0:n]) if(n > 30)
    {
        add_vectors(n, array1, array2, result);
    }
    
    /* SECTION 4: Complex nested OpenMP with multiple temporaries */
    /* Combined parallel for simd with reduction in nested loops */
    {
        double nested_sum = 0.0;
        volatile int outer_bound = rows / 2;
        
        #pragma omp parallel for reduction(+:nested_sum) \
                private(j) collapse(2) if(rows > 10)
        for (int i = 0; i < outer_bound; i++) {
            for (int j = 0; j < cols; j++) {
                int idx = i * cols + j;
                
                /* Complex condition that might need temporaries */
                if (matrix[idx] > 0.5 || (i * j) % 3 == 0) {
                    nested_sum += matrix[idx] * result[j % n];
                }
            }
        }
        
        printf("Nested sum: %f\n", nested_sum);
    }
    
    /* Compute final checksum to prevent optimization */
    double checksum = 0.0;
    for (int i = 0; i < n; i += 4) {
        checksum += result[i];
    }
    for (int i = 0; i < rows * cols; i += 5) {
        checksum += matrix[i];
    }
    
    printf("Final checksum: %f\n", checksum);
    
    /* Cleanup */
    free(array1);
    free(array2);
    free(result);
    free(matrix);
    
    return 0;
}

/* Dummy function to potentially trigger pretty-printing if compiled with specific flags */
#ifdef DUMP_OMP
void __attribute__((used)) trigger_pretty_print() {
    /* This function doesn't need to do anything - it's just to ensure
       the compiler includes OpenMP constructs in the AST for dumping */
    volatile int x = 0;
    #pragma omp parallel
    {
        x += omp_get_thread_num();
    }
}
#endif
