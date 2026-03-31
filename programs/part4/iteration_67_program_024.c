/* test_omp_array_section.c
 * 
 * This program demonstrates OpenMP array section usage with complex base
 * expressions to trigger the OMP_ARRAY_SECTION pretty-printing logic
 * in GCC's tree-pretty-print.cc.
 *
 * Compile with: gcc -O1 -fopenmp -fdump-tree-original -fdump-tree-omplower -c test_omp_array_section.c
 * Or for diagnostics: gcc -O0 -fopenmp -Wall -Werror=openmp-mapping -c test_omp_array_section.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define N 100

/* Helper function to introduce variability and prevent constant folding */
static int get_bound(int base, int argc, char **argv) {
    volatile int v = base;
    if (argc > 1) v += atoi(argv[1]);
    return v;
}

/* Function that uses array sections in multiple contexts */
void process_sections(int *arr1, int *arr2, int start, int length, int cond, int argc, char **argv) {
    int i;
    
    /* Use volatile variables to prevent constant propagation */
    volatile int vstart = start;
    volatile int vlen = length;
    
    /* 1. Array section with complex base: conditional operator */
    #pragma omp target data map(tofrom: (cond ? arr1 : arr2)[vstart:vlen])
    {
        /* Inside target data region - launch a kernel */
        #pragma omp target map(tofrom: (cond ? arr1 : arr2)[vstart:vlen])
        {
            for (i = 0; i < vlen; i++) {
                (cond ? arr1 : arr2)[vstart + i] += i;
            }
        }
    }
    
    /* 2. Array section with pointer arithmetic base */
    int offset = get_bound(5, argc, argv);
    #pragma omp target enter data map(to: (arr1 + offset)[0:vlen])
    
    /* 3. Use array section in a task depend clause */
    #pragma omp task depend(inout: arr2[vstart:vlen])
    {
        for (i = 0; i < vlen; i++) {
            arr2[vstart + i] *= 2;
        }
    }
    
    #pragma omp taskwait
    
    #pragma omp target exit data map(from: (arr1 + offset)[0:vlen])
}

/* Another function with different array section usage */
void mixed_sections(struct Data {
    int *ptr;
    int array[N];
} *data, int idx, int count, int argc, char **argv) {
    
    /* 4. Array section with structure member access */
    int lower = get_bound(idx, argc, argv);
    int upper = get_bound(count, argc, argv);
    
    /* Deliberate type-ish issue: using array section without proper OpenMP context
     * in a way that might trigger diagnostics when compiled with -Werror=openmp-mapping */
    #pragma omp target data map(tofrom: data->array[lower:upper])
    {
        #pragma omp target map(tofrom: data->array[lower:upper])
        {
            for (int i = 0; i < upper; i++) {
                data->array[lower + i] = i * i;
            }
        }
    }
    
    /* 5. Array section with computed bounds */
    int *dynamic_arr = malloc(N * sizeof(int));
    if (dynamic_arr) {
        int start = argc > 2 ? atoi(argv[2]) : 10;
        int len = argc > 3 ? atoi(argv[3]) : 20;
        
        #pragma omp target data map(tofrom: dynamic_arr[start:len])
        {
            #pragma omp target map(tofrom: dynamic_arr[start:len])
            {
                for (int i = 0; i < len; i++) {
                    dynamic_arr[start + i] = start + i;
                }
            }
        }
        
        free(dynamic_arr);
    }
}

int main(int argc, char **argv) {
    int arr1[N], arr2[N];
    int i, sum = 0;
    
    /* Initialize arrays */
    for (i = 0; i < N; i++) {
        arr1[i] = i;
        arr2[i] = N - i;
    }
    
    /* Get bounds from command line to prevent constant folding */
    int start = get_bound(10, argc, argv);
    int length = get_bound(20, argc, argv);
    int cond = (argc > 0) ? 1 : 0;  /* Always true but compiler doesn't know */
    
    /* Process with array sections having complex base expressions */
    process_sections(arr1, arr2, start, length, cond, argc, argv);
    
    /* Use structure with array member */
    struct Data my_data;
    my_data.ptr = arr1;
    for (i = 0; i < N; i++) my_data.array[i] = 0;
    
    mixed_sections(&my_data, 5, 15, argc, argv);
    
    /* Compute checksum to prevent dead code elimination */
    for (i = 0; i < N; i++) {
        sum += arr1[i] + arr2[i] + my_data.array[i];
    }
    
    printf("Checksum: %d\n", sum);
    
    /* Additional array section in is_device_ptr clause with pointer arithmetic */
    int *ptr = arr1 + start;
    #pragma omp target data map(tofrom: arr1[0:N]) use_device_ptr(ptr)
    {
        /* This creates OMP_ARRAY_SECTION nodes for both map and use_device_ptr */
        #pragma omp target is_device_ptr(ptr) map(tofrom: arr2[start:length])
        {
            for (i = 0; i < length; i++) {
                ptr[i] += arr2[start + i];
            }
        }
    }
    
    return 0;
}
