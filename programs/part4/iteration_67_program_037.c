/* This program demonstrates OpenMP array sections with complex base expressions
   to trigger the OMP_ARRAY_SECTION pretty-printing logic in GCC's tree-pretty-print.cc.
   Compile with: gcc -std=c99 -fopenmp -O1 -fdump-tree-original -fdump-tree-omplower -Wall -c tree-pretty-print-test.c
   Additional flags for diagnostics: -Werror=openmp-mapping -fdump-tree-optimized
*/

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Helper function to introduce variability and prevent constant folding */
static int get_value(int base, int multiplier) {
    volatile int v = base; /* volatile to prevent optimization */
    return v * multiplier;
}

/* Function that uses array sections in a way that may cause type warnings */
void process_section(float* section, int len) {
    for (int i = 0; i < len; i++) {
        section[i] += 1.0f;
    }
}

int main(int argc, char* argv[]) {
    /* Use command-line arguments to prevent constant propagation */
    int size = (argc > 1) ? atoi(argv[1]) : 100;
    int start = (argc > 2) ? atoi(argv[2]) : 10;
    int length = (argc > 3) ? atoi(argv[3]) : 20;
    
    if (size < start + length) size = start + length;
    
    /* Allocate and initialize arrays */
    float* arr1 = (float*)malloc(size * sizeof(float));
    float* arr2 = (float*)malloc(size * sizeof(float));
    float* arr3 = (float*)malloc(size * sizeof(float));
    
    for (int i = 0; i < size; i++) {
        arr1[i] = (float)i;
        arr2[i] = (float)(i * 2);
        arr3[i] = (float)(i * 3);
    }
    
    /* Complex base expression 1: Conditional operator as base */
    /* This should trigger op_prio checks for parentheses */
    int use_arr1 = (argc > 4);
    float* base_ptr = use_arr1 ? arr1 : arr2;
    
    /* Complex base expression 2: Pointer arithmetic */
    int offset = get_value(5, 2); /* volatile-based value */
    
    /* OpenMP target data region with array sections having complex bases */
    #pragma omp target data map(tofrom: (use_arr1 ? arr1 : arr2)[start:length]) \
                            map(to: (arr3 + offset)[0:length/2])
    {
        /* Nested target region with different array section usage */
        #pragma omp target map(alloc: (base_ptr + offset/2)[0:length/3])
        {
            int local_start = start;
            int local_len = length;
            /* Simple computation to ensure code isn't eliminated */
            for (int i = 0; i < local_len/3; i++) {
                base_ptr[offset/2 + i] += 0.5f;
            }
        }
        
        /* Another target region with pointer arithmetic in base */
        #pragma omp target map(tofrom: (arr1 + start/2)[0:length/4])
        {
            for (int i = 0; i < length/4; i++) {
                arr1[start/2 + i] *= 2.0f;
            }
        }
    }
    
    /* Use array sections in task depend clauses (OpenMP 4.5+) */
    #pragma omp parallel
    #pragma omp single
    {
        #pragma omp task depend(out: arr1[start:length/2])
        {
            for (int i = 0; i < length/2; i++) {
                arr1[start + i] += 3.0f;
            }
        }
        
        #pragma omp task depend(in: arr1[start:length/2])
        {
            float sum = 0.0f;
            for (int i = 0; i < length/2; i++) {
                sum += arr1[start + i];
            }
            printf("Task sum: %f\n", sum);
        }
    }
    
    /* Deliberate type-checking issue: passing array section to function
       This may generate diagnostics that invoke the pretty-printer */
    if (argc > 5) {
        /* This line may cause an OpenMP mapping warning when compiled with
           -Werror=openmp-mapping, triggering diagnostic output */
        process_section(arr1 + start, length);
    }
    
    /* Compute checksum to prevent dead code elimination */
    double checksum = 0.0;
    for (int i = 0; i < size; i++) {
        checksum += arr1[i] + arr2[i] + arr3[i];
    }
    printf("Checksum: %f\n", checksum);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(arr3);
    
    return 0;
}
