/* test-omp-array-section.c
 * 
 * This program is designed to trigger the pretty-printing logic for
 * OMP_ARRAY_SECTION nodes in GCC's tree-pretty-print.cc, specifically
 * lines 2736-2748. It uses OpenMP array sections with complex base
 * expressions and variable bounds to ensure the uncovered code block
 * is executed during compilation with appropriate dump flags.
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Helper function to prevent constant folding */
static int use_arg(int argc, char **argv) {
    return (argc > 1) ? atoi(argv[1]) : 5;
}

/* Function that uses array sections in multiple OpenMP contexts */
void process_sections(int n, int start, int len, int *arr1, int *arr2, int *ptr, int cond) {
    /* Complex base expression 1: conditional operator as base */
    #pragma omp target data map(to: (cond ? arr1 : arr2)[start:len])
    {
        #pragma omp target map(alloc: (cond ? arr1 : arr2)[0:n])
        for (int i = 0; i < len; i++) {
            (cond ? arr1 : arr2)[start + i] += i;
        }
    }

    /* Complex base expression 2: pointer arithmetic as base */
    #pragma omp target enter data map(to: (ptr + start)[0:len])
    
    #pragma omp target map(from: (ptr + 1)[0:n-1])
    for (int i = 0; i < len; i++) {
        (ptr + start)[i] *= 2;
    }
    
    #pragma omp target exit data map(from: (ptr + start)[0:len])

    /* Array section in depend clause for task */
    #pragma omp task depend(inout: arr1[start:len])
    {
        for (int i = 0; i < len; i++) {
            arr1[start + i] += 1;
        }
    }
    
    #pragma omp taskwait
}

/* Another function with different array section usage */
void mixed_sections(int m, volatile int *volatile_arr, int *base_arr) {
    /* Use volatile variable to prevent constant propagation */
    int offset = vol*atile_arr[0] % m;
    
    /* Array section with struct-like access simulation */
    struct {
        int *data;
        int size;
    } wrapper;
    wrapper.data = base_arr;
    wrapper.size = m;
    
    /* This may trigger type checking diagnostics */
    #pragma omp target data map(tofrom: wrapper.data[offset:m-offset])
    {
        #pragma omp target
        for (int i = 0; i < m - offset; i++) {
            wrapper.data[offset + i] = i * i;
        }
    }
}

int main(int argc, char **argv) {
    /* Use command-line arguments to prevent constant folding */
    int n = use_arg(argc, argv);
    int start = (argc > 2) ? atoi(argv[2]) : 0;
    int len = (argc > 3) ? atoi(argv[3]) : n/2;
    
    if (start + len > n) {
        fprintf(stderr, "Bounds exceed array size\n");
        return 1;
    }
    
    /* Allocate and initialize arrays */
    int *arr1 = (int *)malloc(n * sizeof(int));
    int *arr2 = (int *)malloc(n * sizeof(int));
    int *ptr_arr = (int *)malloc(n * sizeof(int));
    volatile int volatile_var = 10;
    
    for (int i = 0; i < n; i++) {
        arr1[i] = i;
        arr2[i] = n - i;
        ptr_arr[i] = i * 2;
    }
    
    /* Process with various array section patterns */
    int cond = (argc % 2); /* Dynamic condition */
    
    process_sections(n, start, len, arr1, arr2, ptr_arr, cond);
    mixed_sections(n, &volatile_var, arr1);
    
    /* Compute checksum to prevent dead code elimination */
    long long checksum = 0;
    for (int i = 0; i < n; i++) {
        checksum += arr1[i] + arr2[i] + ptr_arr[i];
    }
    
    printf("Checksum: %lld\n", checksum);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(ptr_arr);
    
    return 0;
}
