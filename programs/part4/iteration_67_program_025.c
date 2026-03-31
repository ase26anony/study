/* test-omp-array-section.c
 * 
 * This program demonstrates OpenMP array sections with complex base expressions
 * to trigger the OMP_ARRAY_SECTION pretty-printing logic in GCC's tree-pretty-print.cc.
 * 
 * Compilation options for coverage:
 *   gcc -O1 -fopenmp -fdump-tree-original -fdump-tree-omplower test-omp-array-section.c -o test
 *   gcc -O0 -fopenmp -Wall -Werror=openmp-mapping -c test-omp-array-section.c
 *   gcc -O2 -fopenmp -foffload=disable -fdump-tree-optimized test-omp-array-section.c -o test
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Helper function to prevent constant folding */
static int use_arg(int argc, char **argv) {
    return (argc > 1) ? atoi(argv[1]) : 5;
}

/* Function that returns different arrays based on condition */
int* select_array(int cond, int *a, int *b) {
    return cond ? a : b;
}

/* Function expecting a pointer (not an array section) - may cause type warnings */
void process_pointer(int *p, int n) {
    for (int i = 0; i < n; i++) p[i] += 1;
}

int main(int argc, char **argv) {
    /* Use volatile and argc to prevent constant propagation */
    volatile int base_size = use_arg(argc, argv);
    int offset = (argc > 2) ? atoi(argv[2]) : 2;
    int section_len = (argc > 3) ? atoi(argv[3]) : 3;
    
    const int N = 100;
    int arr1[N], arr2[N];
    int *ptr1 = arr1;
    int *ptr2 = arr2;
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        arr1[i] = i;
        arr2[i] = i * 2;
    }
    
    /* STRATEGY 1: Complex base expressions in OpenMP map clauses */
    
    /* Case 1: Conditional operator as base (triggers op_prio check) */
    #pragma omp target data map(tofrom: (argc > 1 ? arr1 : arr2)[offset:section_len])
    {
        #pragma omp target map(tofrom: (argc > 2 ? arr1 : arr2)[offset:section_len])
        {
            for (int i = 0; i < section_len; i++) {
                int *base = (argc > 1 ? arr1 : arr2);
                base[offset + i] += 1;
            }
        }
    }
    
    /* Case 2: Pointer arithmetic as base */
    #pragma omp target data map(tofrom: (ptr1 + offset)[0:section_len])
    {
        #pragma omp target map(tofrom: (ptr1 + offset)[0:section_len])
        {
            for (int i = 0; i < section_len; i++) {
                ptr1[offset + i] *= 2;
            }
        }
    }
    
    /* Case 3: Function call returning array pointer */
    #pragma omp target enter data map(to: select_array(1, arr1, arr2)[5:10])
    #pragma omp target exit data map(from: select_array(1, arr1, arr2)[5:10])
    
    /* STRATEGY 2: Array sections in different OpenMP clauses */
    
    /* In 'to' and 'from' clauses */
    #pragma omp target data map(to: arr1[0:base_size]) map(from: arr2[base_size:base_size])
    {
        #pragma omp target 
        {
            for (int i = 0; i < base_size; i++) {
                arr2[base_size + i] = arr1[i] + 100;
            }
        }
    }
    
    /* In 'alloc' clause */
    int *dynamic_arr = (int*)malloc(N * sizeof(int));
    #pragma omp target data map(alloc: dynamic_arr[offset:section_len])
    {
        #pragma omp target map(alloc: dynamic_arr[offset:section_len])
        {
            for (int i = 0; i < section_len; i++) {
                dynamic_arr[offset + i] = i * 3;
            }
        }
    }
    
    /* STRATEGY 3: Potential type checking issues */
    /* This may trigger diagnostics when compiled with -Werror=openmp-mapping */
    /* Array section in non-OpenMP context (function argument) */
    if (argc > 4) {
        /* Deliberate type mismatch: passing array section where pointer expected */
        /* process_pointer(arr1[0:10], 10); */ /* Would cause warning/error */
    }
    
    /* STRATEGY 4: Array sections in task depend clauses */
    #pragma omp parallel
    #pragma omp single
    {
        #pragma omp task depend(out: arr1[0:base_size])
        {
            for (int i = 0; i < base_size; i++) arr1[i] = i * 10;
        }
        
        #pragma omp task depend(in: arr1[0:base_size])
        {
            for (int i = 0; i < base_size; i++) arr2[i] = arr1[i] + 5;
        }
    }
    
    /* STRATEGY 5: Nested complex expressions */
    struct Container {
        int data[N];
        int *ptr;
    } container;
    
    container.ptr = arr1;
    
    /* Member access with array section */
    #pragma omp target map(tofrom: container.data[offset:section_len])
    {
        for (int i = 0; i < section_len; i++) {
            container.data[offset + i] += 1000;
        }
    }
    
    /* Complex nested expression */
    #pragma omp target data map(tofrom: ((argc % 2) ? container.ptr : arr2)[offset:section_len])
    {
        #pragma omp target map(tofrom: ((argc % 2) ? container.ptr : arr2)[offset:section_len])
        {
            for (int i = 0; i < section_len; i++) {
                int *base = ((argc % 2) ? container.ptr : arr2);
                base[offset + i] -= 500;
            }
        }
    }
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum += arr1[i] + arr2[i];
    }
    
    if (dynamic_arr) {
        for (int i = 0; i < section_len; i++) {
            checksum += dynamic_arr[offset + i];
        }
        free(dynamic_arr);
    }
    
    printf("Checksum: %d\n", checksum);
    printf("Array sections used with bounds: offset=%d, len=%d\n", offset, section_len);
    
    return 0;
}
