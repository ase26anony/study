/* test-omp-array-section.c
 * 
 * This program is designed to trigger the pretty-printing logic for
 * OMP_ARRAY_SECTION nodes in GCC's tree-pretty-print.cc, specifically
 * lines 2736-2748, which handle the formatting of OpenMP array sections.
 *
 * The code uses complex base expressions (conditional operator, pointer
 * arithmetic, structure member access) with variable bounds to ensure
 * the op_prio() checks and non-constant bound nodes are exercised.
 * Array sections are placed in multiple OpenMP contexts (map, to, from,
 * depend) to increase the likelihood of pretty-printer invocation
 * during compilation dumps or diagnostics.
 *
 * Compilation suggestions for coverage:
 *   gcc -O1 -fopenmp -fdump-tree-original -fdump-tree-omplower -c test-omp-array-section.c
 *   gcc -O0 -fopenmp -Wall -Werror=openmp-mapping -c test-omp-array-section.c
 *   gcc -O2 -fopenmp -foffload=disable -fdump-tree-optimized -c test-omp-array-section.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Use volatile to prevent constant propagation of bounds */
static volatile int global_start = 5;
static volatile int global_len = 20;

/* Structure with array member to test member access as base */
struct with_array {
    int data[100];
    int offset;
};

/* Function that returns a pointer to use in pointer arithmetic */
int* get_ptr(int *base, int idx) {
    return base + idx;
}

/* Function that uses array section in a potentially problematic way
 * (to possibly trigger diagnostics) */
void problematic_use(int *section) {
    /* Empty: just to create a context where array section might be
     * interpreted outside of OpenMP clause */
}

int main(int argc, char *argv[]) {
    /* Use argc to make bounds variable and non-constant */
    int start = (argc > 1) ? atoi(argv[1]) : global_start;
    int length = (argc > 2) ? atoi(argv[2]) : global_len;
    
    /* Ensure bounds are positive and within reasonable range */
    if (start < 0) start = 0;
    if (length < 0) length = 10;
    if (start + length > 100) length = 100 - start;
    
    int arr1[100], arr2[100];
    struct with_array s = { .offset = 10 };
    int *ptr = arr1;
    
    /* Initialize arrays */
    for (int i = 0; i < 100; i++) {
        arr1[i] = i;
        arr2[i] = i * 2;
        s.data[i] = i * 3;
    }
    
    /* STRATEGY 1: Complex base expressions in OpenMP target data region
     * 
     * Base expressions with lower precedence than array section:
     * 1. Conditional operator: (cond ? arr1 : arr2)[start:length]
     * 2. Pointer arithmetic: (ptr + s.offset)[0:length]
     * 3. Structure member access: s.data[start:length]
     */
    
    printf("Starting OpenMP target data region with array sections...\n");
    
    #pragma omp target data map(to: (argc > 3 ? arr1 : arr2)[start:length], \
                                     (ptr + s.offset)[0:length], \
                                     s.data[start:length])
    {
        /* Inside data region: run a target kernel with more array sections */
        int cond = (argc > 1);
        
        #pragma omp target map(tofrom: (cond ? arr1 : arr2)[start:length]) \
                           map(from: (get_ptr(arr1, 5))[0:length])
        {
            /* Simple computation to ensure the kernel runs */
            for (int i = 0; i < length; i++) {
                if (cond) {
                    arr1[start + i] += 1;
                } else {
                    arr2[start + i] += 1;
                }
            }
        }
        
        /* STRATEGY 2: Use array sections in different OpenMP clauses
         * to create more OMP_ARRAY_SECTION nodes */
        #pragma omp target enter data map(alloc: arr1[start:length])
        #pragma omp target exit data map(from: arr1[start:length])
        
    } /* end target data region */
    
    /* STRATEGY 3: Use array section in task depend clause
     * (another context where array sections appear) */
    #pragma omp parallel
    #pragma omp single
    {
        #pragma omp task depend(out: arr1[start:length])
        {
            for (int i = 0; i < length; i++) {
                arr1[start + i] *= 2;
            }
        }
        
        #pragma omp task depend(in: arr1[start:length])
        {
            int sum = 0;
            for (int i = 0; i < length; i++) {
                sum += arr1[start + i];
            }
            printf("Task computed sum: %d\n", sum);
        }
    }
    
    /* STRATEGY 4: Deliberate type-ish issue to potentially trigger diagnostics
     * (passing array section to function expecting pointer) */
    #ifdef TRIGGER_WARNING
    problematic_use(arr1[start:length]);  /* This may warn without OpenMP context */
    #endif
    
    /* Final checksum to prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < 100; i++) {
        checksum += arr1[i] + arr2[i] + s.data[i];
    }
    printf("Final checksum: %d\n", checksum);
    
    return 0;
}
