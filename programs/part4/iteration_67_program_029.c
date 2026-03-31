/* test_omp_array_section.c
 * 
 * This program demonstrates OpenMP array sections with complex base expressions
 * to trigger the OMP_ARRAY_SECTION pretty-printing logic in GCC's tree-pretty-print.cc.
 * 
 * Compilation options for coverage:
 *   1. Debug dumps: gcc -O1 -fopenmp -fdump-tree-original -fdump-tree-omplower test_omp_array_section.c
 *   2. Diagnostics: gcc -O0 -fopenmp -Wall -Werror=openmp-mapping -c test_omp_array_section.c
 *   3. Optimization: gcc -O2 -fopenmp -foffload=disable -fdump-tree-optimized test_omp_array_section.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Helper function to prevent constant folding */
static int use_arg(int argc, char **argv) {
    return (argc > 1) ? atoi(argv[1]) : 5;
}

/* Function that takes a pointer to trigger type checking */
void process_ptr(float *p, int n) {
    for (int i = 0; i < n; i++) p[i] += 1.0f;
}

int main(int argc, char **argv) {
    /* Use command-line arguments to prevent constant propagation */
    volatile int base_size = use_arg(argc, argv);
    volatile int offset = (argc > 2) ? atoi(argv[2]) : 2;
    volatile int section_len = (argc > 3) ? atoi(argv[3]) : 3;
    
    int N = base_size + 10; /* Ensure enough space */
    float *arr1 = (float *)malloc(N * sizeof(float));
    float *arr2 = (float *)malloc(N * sizeof(float));
    float *ptr = arr1;
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        arr1[i] = (float)i;
        arr2[i] = (float)(i * 2);
    }
    
    /* STRATEGY 1: Complex base expressions in OpenMP data mapping */
    
    /* Case A: Pointer arithmetic as base - (ptr + offset)[0:len] */
    #pragma omp target data map(tofrom: (ptr + offset)[0:section_len])
    {
        #pragma omp target map(tofrom: (ptr + offset)[0:section_len])
        {
            for (int i = 0; i < section_len; i++) {
                (ptr + offset)[i] *= 2.0f;
            }
        }
    }
    
    /* Case B: Conditional expression as base - (cond ? arr1 : arr2)[start:len] */
    volatile int cond = (argc > 4);
    #pragma omp target enter data map(to: (cond ? arr1 : arr2)[offset:section_len])
    
    #pragma omp target map(tofrom: (cond ? arr1 : arr2)[offset:section_len])
    {
        for (int i = 0; i < section_len; i++) {
            (cond ? arr1 : arr2)[offset + i] += 3.0f;
        }
    }
    
    #pragma omp target exit data map(from: (cond ? arr1 : arr2)[offset:section_len])
    
    /* STRATEGY 2: Different OpenMP clauses with array sections */
    
    /* 'to' and 'from' clauses */
    float *dynamic_arr = (float *)malloc(N * sizeof(float));
    for (int i = 0; i < N; i++) dynamic_arr[i] = (float)i;
    
    #pragma omp target data map(to: dynamic_arr[1:4]) map(from: dynamic_arr[5:4])
    {
        #pragma omp target
        {
            for (int i = 1; i < 5; i++) dynamic_arr[i] = 100.0f;
            for (int i = 5; i < 9; i++) dynamic_arr[i] = 200.0f;
        }
    }
    
    /* STRATEGY 3: Type checking edge cases */
    /* This may trigger diagnostics when -Werror=openmp-mapping is used */
    float *another_ptr = arr2;
    
    /* Array section in is_device_ptr clause (with pointer arithmetic) */
    #pragma omp target data map(to: arr2[0:N]) \
                            is_device_ptr((another_ptr + 1)[0:2])
    {
        /* Empty but still creates the array section nodes */
    }
    
    /* STRATEGY 4: Task depend with array sections */
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp task depend(out: arr1[0:2])
            {
                arr1[0] = 1.0f; arr1[1] = 2.0f;
            }
            
            #pragma omp task depend(in: arr1[0:2])
            {
                arr1[0] += 10.0f;
            }
        }
    }
    
    /* STRATEGY 5: Nested complexity for optimization dumps */
    {
        int m = section_len;
        int n = offset;
        #pragma omp target teams distribute parallel for \
                    map(tofrom: arr1[n:m])
        for (int i = n; i < n + m; i++) {
            arr1[i] = arr1[i] / 2.0f + (float)i;
        }
    }
    
    /* Compute checksum to prevent dead code elimination */
    double checksum = 0.0;
    for (int i = 0; i < N; i++) {
        checksum += arr1[i] + arr2[i];
        if (i < N) checksum += dynamic_arr[i]; /* Conditional to avoid simple loops */
    }
    
    printf("Checksum: %f\n", checksum);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(dynamic_arr);
    
    return 0;
}
