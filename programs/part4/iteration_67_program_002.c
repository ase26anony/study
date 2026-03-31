/* test-omp-array-section.c
 * 
 * This program is designed to trigger the pretty-printing logic for
 * OMP_ARRAY_SECTION nodes in GCC's tree-pretty-print.cc, specifically
 * lines 2736-2748. It uses OpenMP array sections with complex base
 * expressions and variable bounds in multiple OpenMP contexts.
 *
 * Compilation options for coverage:
 *   gcc -O1 -fopenmp -fdump-tree-original -fdump-tree-omplower -c test-omp-array-section.c
 *   gcc -O0 -fopenmp -Wall -Werror=openmp-mapping -c test-omp-array-section.c
 *   gcc -O2 -fopenmp -foffload=disable -fdump-tree-optimized -c test-omp-array-section.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Helper function to prevent constant folding */
static int use_arg(int argc, char **argv) {
    return (argc > 1) ? atoi(argv[1]) : 5;
}

/* Function that returns one of two arrays based on condition */
static int *select_array(int cond, int *a, int *b) {
    return cond ? a : b;
}

/* Structure with array member */
struct with_array {
    int member_arr[100];
    int offset;
};

int main(int argc, char **argv) {
    /* Use argc/argv to get dynamic values preventing constant propagation */
    volatile int base_size = use_arg(argc, argv);
    volatile int section_start = (argc > 2) ? atoi(argv[2]) : 2;
    volatile int section_len = (argc > 3) ? atoi(argv[3]) : 10;
    
    /* Initialize arrays */
    int arr1[100], arr2[100];
    int *ptr_arr = arr1;
    
    for (int i = 0; i < 100; i++) {
        arr1[i] = i;
        arr2[i] = i * 2;
    }
    
    /* Structure with array member */
    struct with_array s;
    for (int i = 0; i < 100; i++) {
        s.member_arr[i] = i * 3;
    }
    s.offset = 5;
    
    /* STRATEGY 1: Complex base expressions in OpenMP target data region */
    /* Base expression: conditional operator (lower precedence than array section) */
    #pragma omp target data map(tofrom: (select_array(base_size > 10, arr1, arr2))[section_start:section_len])
    {
        /* Inside data region: launch target kernel with array section */
        #pragma omp target map(tofrom: (select_array(base_size > 10, arr1, arr2))[section_start:section_len])
        {
            int *arr = select_array(base_size > 10, arr1, arr2);
            for (int i = section_start; i < section_start + section_len; i++) {
                arr[i] += 1;
            }
        }
    }
    
    /* STRATEGY 2: Pointer arithmetic as base expression */
    int offset = s.offset;
    #pragma omp target enter data map(to: (ptr_arr + offset)[0:base_size])
    #pragma omp target map(tofrom: (ptr_arr + offset)[0:base_size])
    {
        for (int i = 0; i < base_size; i++) {
            ptr_arr[offset + i] *= 2;
        }
    }
    #pragma omp target exit data map(from: (ptr_arr + offset)[0:base_size])
    
    /* STRATEGY 3: Structure member access as base */
    int m_start = section_start;
    int m_len = section_len;
    #pragma omp target data map(tofrom: s.member_arr[m_start:m_len])
    {
        #pragma omp target map(tofrom: s.member_arr[m_start:m_len])
        {
            for (int i = m_start; i < m_start + m_len; i++) {
                s.member_arr[i] -= 3;
            }
        }
    }
    
    /* STRATEGY 4: Array section in task depend clause */
    /* This creates additional OMP_ARRAY_SECTION nodes in different context */
    int *task_arr = arr2;
    int t_start = 20;
    int t_len = 15;
    
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp task depend(out: task_arr[t_start:t_len])
            {
                for (int i = t_start; i < t_start + t_len; i++) {
                    task_arr[i] = i * 4;
                }
            }
            
            #pragma omp task depend(in: task_arr[t_start:t_len])
            {
                int sum = 0;
                for (int i = t_start; i < t_start + t_len; i++) {
                    sum += task_arr[i];
                }
                /* Use sum to prevent dead code elimination */
                printf("Task sum: %d\n", sum);
            }
        }
    }
    
    /* STRATEGY 5: Potential type checking issue */
    /* Array section used in context that might trigger diagnostics */
    /* when compiled with -Werror=openmp-mapping */
    int *alias_ptr = arr1;
    #pragma omp target data map(tofrom: alias_ptr[0:5])
    {
        /* Different base: pointer dereference with offset */
        #pragma omp target map(tofrom: (*(&alias_ptr))[0:5])
        {
            for (int i = 0; i < 5; i++) {
                alias_ptr[i] += 10;
            }
        }
    }
    
    /* Compute checksum to verify execution and prevent optimization */
    int checksum = 0;
    for (int i = 0; i < 100; i++) {
        checksum += arr1[i] + arr2[i] + s.member_arr[i];
    }
    printf("Final checksum: %d\n", checksum);
    
    return 0;
}
