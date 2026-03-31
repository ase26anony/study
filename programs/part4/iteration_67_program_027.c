/* test-omp-array-section.c
 * 
 * This program is designed to trigger the pretty-printing logic for
 * OMP_ARRAY_SECTION nodes in GCC's tree-pretty-print.cc, specifically
 * lines 2736-2748, which handle the formatting of OpenMP array sections.
 * 
 * The code uses complex base expressions (conditional operator, pointer
 * arithmetic, structure member access) with variable bounds to ensure
 * the op_prio() comparisons are exercised and parentheses may be needed.
 * 
 * Compilation suggestions for coverage:
 *   gcc -O1 -fopenmp -fdump-tree-original -fdump-tree-omplower -c test-omp-array-section.c
 *   gcc -O0 -fopenmp -Wall -Werror=openmp-mapping -c test-omp-array-section.c
 *   gcc -O2 -fopenmp -foffload=disable -fdump-tree-optimized -c test-omp-array-section.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define N 100

/* Structure with array member to test member access as base expression */
struct with_array {
    int data[N];
    int *ptr;
};

/* Function using array section in a potentially problematic context
 * (may trigger diagnostics about type mismatch) */
void process_section(int *section, int len) {
    for (int i = 0; i < len; i++) {
        section[i] += 1;
    }
}

/* Function to initialize arrays */
void init_array(int *arr, int n) {
    for (int i = 0; i < n; i++) {
        arr[i] = i;
    }
}

/* Function to compute checksum */
int checksum(int *arr, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += arr[i];
    }
    return sum;
}

int main(int argc, char *argv[]) {
    /* Use command-line arguments for dynamic bounds to prevent constant folding */
    volatile int start = (argc > 1) ? atoi(argv[1]) : 10;
    volatile int length = (argc > 2) ? atoi(argv[2]) : 20;
    volatile int offset = (argc > 3) ? atoi(argv[3]) : 5;
    
    /* Ensure bounds are within array limits */
    int lower_bound = (start % N);
    int section_len = (length % (N - lower_bound)) + 1;
    int ptr_offset = (offset % 10);
    
    /* Allocate and initialize arrays */
    int *arr1 = (int *)malloc(N * sizeof(int));
    int *arr2 = (int *)malloc(N * sizeof(int));
    int *dynamic_arr = (int *)malloc(2 * N * sizeof(int));
    
    init_array(arr1, N);
    init_array(arr2, N);
    init_array(dynamic_arr, 2 * N);
    
    /* Initialize structure */
    struct with_array s;
    for (int i = 0; i < N; i++) {
        s.data[i] = i * 2;
    }
    s.ptr = dynamic_arr;
    
    /* 1. Array section with conditional operator as base expression.
     * This tests op_prio() for the conditional operator (low precedence)
     * versus the array section. */
    int use_first = (argc % 2);
    
    #pragma omp target data map(tofrom: (use_first ? arr1 : arr2)[lower_bound:section_len])
    {
        #pragma omp target map(tofrom: (use_first ? arr1 : arr2)[lower_bound:section_len])
        {
            int *base = use_first ? arr1 : arr2;
            for (int i = 0; i < section_len; i++) {
                base[lower_bound + i] += 1;
            }
        }
    }
    
    /* 2. Array section with pointer arithmetic as base expression.
     * Tests op_prio() for addition versus array section. */
    #pragma omp target data map(to: (arr1 + ptr_offset)[0:section_len])
    {
        #pragma omp target map(from: (arr1 + ptr_offset)[0:section_len])
        {
            for (int i = 0; i < section_len; i++) {
                (arr1 + ptr_offset)[i] = i * 3;
            }
        }
    }
    
    /* 3. Array section with structure member access as base.
     * Tests op_prio() for member access versus array section. */
    #pragma omp target enter data map(to: s.data[lower_bound:section_len])
    
    #pragma omp target map(tofrom: s.data[lower_bound:section_len])
    {
        for (int i = 0; i < section_len; i++) {
            s.data[lower_bound + i] *= 2;
        }
    }
    
    #pragma omp target exit data map(from: s.data[lower_bound:section_len])
    
    /* 4. Deliberate type error to trigger diagnostic pretty-printing.
     * Passing an array section to a function expecting a pointer
     * (without proper OpenMP context) may generate a warning/error. */
    #ifdef TRIGGER_WARNING
    /* This would normally be invalid outside OpenMP directives */
    process_section(arr1[lower_bound:section_len], section_len); /* Likely syntax error */
    #endif
    
    /* 5. Use array section in task depend clause (OpenMP 4.5+).
     * Creates additional OMP_ARRAY_SECTION nodes. */
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp task depend(out: arr1[lower_bound:section_len])
            {
                for (int i = 0; i < section_len; i++) {
                    arr1[lower_bound + i] += 10;
                }
            }
            
            #pragma omp task depend(in: arr1[lower_bound:section_len])
            {
                int sum = 0;
                for (int i = 0; i < section_len; i++) {
                    sum += arr1[lower_bound + i];
                }
                printf("Task sum: %d\n", sum);
            }
        }
    }
    
    /* 6. Complex nested expression with multiple operators as base.
     * Tests parentheses insertion in pretty-printed output. */
    int *ptr = arr2;
    int cond = (argc > 4);
    
    #pragma omp target data map(tofrom: (cond ? ptr + 5 : ptr - 5)[0:section_len])
    {
        #pragma omp target map(tofrom: (cond ? ptr + 5 : ptr - 5)[0:section_len])
        {
            int *base = cond ? ptr + 5 : ptr - 5;
            for (int i = 0; i < section_len; i++) {
                base[i] = base[i] * 2 + 1;
            }
        }
    }
    
    /* Print checksums to prevent dead code elimination */
    printf("Checksum arr1: %d\n", checksum(arr1, N));
    printf("Checksum arr2: %d\n", checksum(arr2, N));
    printf("Checksum s.data: %d\n", checksum(s.data, N));
    printf("Checksum dynamic_arr: %d\n", checksum(dynamic_arr, 2 * N));
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(dynamic_arr);
    
    return 0;
}
