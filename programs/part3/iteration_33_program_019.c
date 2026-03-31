/* tree-pretty-print-coverage.c
 * Designed to trigger uncovered lines 512-523 in tree-pretty-print.cc
 * Compile with: gcc -O1 -fopenmp -fdump-tree-omplower -fdump-tree-ompexp tree-pretty-print-coverage.c -o test
 * Or with: gcc -O2 -fopenmp -fopenmp-simd -fdump-tree-all -foffload=disable tree-pretty-print-coverage.c -o test
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Use volatile to prevent optimization of loop bounds */
volatile int volatile_bound = 100;

/* Function to be used with declare target enter */
#pragma omp declare target enter(add_vectors) to(map_to_from:result)
void add_vectors(double *a, double *b, double *result, int n) {
    #pragma omp parallel for simd
    for (int i = 0; i < n; i++) {
        result[i] = a[i] + b[i];
    }
}

/* Another function for declare target */
#pragma omp declare target enter(process_data)
void process_data(int *data, int n, int *sum) {
    #pragma omp parallel for reduction(+:*sum)
    for (int i = 0; i < n; i++) {
        *sum += data[i];
    }
}

int main(int argc, char **argv) {
    /* Use argc for pseudo-random but reproducible sizes */
    int base_size = 100 + (argc % 50);
    int n = base_size;
    int m = base_size / 2;
    
    /* Allocate arrays with dynamic sizes */
    double *array1 = (double *)malloc(n * sizeof(double));
    double *array2 = (double *)malloc(n * sizeof(double));
    double *result = (double *)malloc(n * sizeof(double));
    int *int_data = (int *)malloc(m * sizeof(int));
    
    /* Initialize arrays with patterns */
    for (int i = 0; i < n; i++) {
        array1[i] = i * 1.5;
        array2[i] = i * 0.5;
    }
    
    for (int i = 0; i < m; i++) {
        int_data[i] = i % 10;
    }
    
    double sum = 0.0;
    double prefix_sum = 0.0;
    
    /* TARGET 1: Generate _reductemp_ and _scantemp_ clauses */
    /* Use parallel for simd with reduction and scan */
    printf("Starting reduction and scan...\n");
    
    #pragma omp parallel for simd reduction(+:sum) \
            scan(inscan:prefix_sum) if(n > 50)
    for (int i = 0; i < n; i++) {
        double val = array1[i] + array2[i];
        
        #pragma omp scan inclusive(prefix_sum)
        prefix_sum += val;
        sum += val;
        
        /* Data-dependent operation to create complex control flow */
        if (i % (omp_get_thread_num() + 2) == 0) {
            result[i] = val * 2.0;
        } else {
            result[i] = val;
        }
    }
    
    printf("Sum after reduction: %f\n", sum);
    printf("Prefix sum: %f\n", prefix_sum);
    
    /* TARGET 2: Generate _condtemp_ clause */
    /* Use collapse with non-trivial loop bounds */
    printf("\nStarting collapsed loops...\n");
    
    int total = 0;
    int outer_bound = volatile_bound;  /* volatile prevents optimization */
    
    #pragma omp parallel for collapse(2) reduction(+:total) \
            schedule(dynamic, 4)
    for (int i = 0; i < outer_bound; i++) {
        /* Use runtime value for inner bound to create condition temporary */
        int inner_bound = (i % 10) + 5;
        for (int j = 0; j < inner_bound; j++) {
            /* Complex condition that may require temporary */
            if ((i * j) % 7 == (omp_get_thread_num() % 3)) {
                total += i + j;
            }
        }
    }
    
    printf("Total from collapsed loops: %d\n", total);
    
    /* TARGET 3: Trigger ENTER clause with to() */
    /* Use declare target enter with to mapper */
    printf("\nUsing declare target enter...\n");
    
    int target_sum = 0;
    
    /* The declare target directives above should generate ENTER clauses */
    #pragma omp target map(to:array1[0:n], array2[0:n]) \
                       map(from:result[0:n]) if(n > 30)
    {
        add_vectors(array1, array2, result, n);
    }
    
    /* Another target region */
    #pragma omp target map(to:int_data[0:m]) map(tofrom:target_sum) \
                       device(0) if(m > 20)
    {
        process_data(int_data, m, &target_sum);
    }
    
    printf("Target sum: %d\n", target_sum);
    
    /* TARGET 4: Additional scan usage for _scantemp_ */
    /* Exclusive scan in parallel region */
    printf("\nUsing exclusive scan...\n");
    
    int scan_array[100];
    int exclusive_prefix = 0;
    
    for (int i = 0; i < 100; i++) {
        scan_array[i] = i % 7;
    }
    
    #pragma omp parallel for simd reduction(+:total) \
            scan(exclusive:exclusive_prefix)
    for (int i = 0; i < 100; i++) {
        int val = scan_array[i];
        
        #pragma omp scan exclusive(exclusive_prefix)
        scan_array[i] = exclusive_prefix;
        exclusive_prefix += val;
        total += val;
    }
    
    /* TARGET 5: Complex reduction that may generate multiple _reductemp_ */
    /* Nested parallelism with reduction */
    printf("\nNested parallelism with reduction...\n");
    
    double nested_sum = 0.0;
    #pragma omp parallel reduction(+:nested_sum)
    {
        #pragma omp for nowait
        for (int i = 0; i < n/2; i++) {
            double local_sum = 0.0;
            #pragma omp simd reduction(+:local_sum)
            for (int j = 0; j < 10; j++) {
                local_sum += array1[i] * j + array2[i];
            }
            nested_sum += local_sum;
        }
    }
    
    printf("Nested sum: %f\n", nested_sum);
    
    /* Compute final checksum to prevent dead code elimination */
    double checksum = sum + total + target_sum + nested_sum;
    printf("\nFinal checksum: %f\n", checksum);
    
    /* Cleanup */
    free(array1);
    free(array2);
    free(result);
    free(int_data);
    
    return 0;
}
