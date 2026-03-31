/* tree-pretty-print-omp-array-section.c
 * Designed to trigger OMP_ARRAY_SECTION pretty-printing in GCC's tree-pretty-print.cc
 * Compile with: gcc -O1 -fopenmp -fdump-tree-original -fdump-tree-omplower -Wall -c tree-pretty-print-omp-array-section.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Helper function to prevent constant folding */
static volatile int global_offset = 1;

/* Function using array sections with complex base expressions */
void process_sections(int *arr1, int *arr2, int start, int length, int cond, int argc) {
    /* Complex base expression 1: conditional operator (low precedence) */
    #pragma omp target data map(tofrom: (cond ? arr1 : arr2)[start:length])
    {
        #pragma omp target map(tofrom: (cond ? arr1 : arr2)[start:length])
        {
            for (int i = 0; i < length; i++) {
                (cond ? arr1 : arr2)[start + i] += i;
            }
        }
    }
    
    /* Complex base expression 2: pointer arithmetic */
    int *ptr = arr1 + global_offset;
    #pragma omp target enter data map(to: (ptr + argc)[0:10])
    #pragma omp target map(tofrom: (ptr + argc)[0:10])
    {
        for (int i = 0; i < 10; i++) {
            (ptr + argc)[i] *= 2;
        }
    }
    #pragma omp target exit data map(from: (ptr + argc)[0:10])
    
    /* Array section in depend clause for task */
    #pragma omp task depend(inout: arr2[start:length])
    {
        for (int i = 0; i < length; i++) {
            arr2[start + i] = arr1[start + i];
        }
    }
}

/* Another function with different array section usage */
void mixed_sections(struct Data {
    int *main_arr;
    int *alt_arr;
} *data, int idx, int len) {
    /* Complex base: structure member access */
    #pragma omp target data map(tofrom: data->main_arr[idx:len], data->alt_arr[0:len])
    {
        #pragma omp target map(tofrom: data->main_arr[idx:len])
        {
            for (int i = 0; i < len; i++) {
                data->main_arr[idx + i] = data->alt_arr[i] + idx;
            }
        }
    }
    
    /* Potential type warning: array section passed as pointer */
    /* This may trigger diagnostics that pretty-print the array section */
    int *section_ptr = &data->main_arr[idx]; /* Not the array section itself, but close */
    (void)section_ptr; /* Suppress unused warning */
}

/* Function designed to potentially trigger diagnostics */
void problematic_usage(int *arr, int n) {
    /* Array section with variable bounds from volatile */
    volatile int start = 0;
    volatile int count = n / 2;
    
    /* Using array section in multiple clauses */
    #pragma omp target data map(to: arr[start:count]) map(from: arr[count:n-count])
    {
        #pragma omp target 
        {
            for (int i = 0; i < count; i++) {
                arr[start + i] = arr[count + i] * 3;
            }
        }
    }
    
    /* Nested array section usage in complex expression */
    int *mid_ptr = arr + n/3;
    #pragma omp target map(tofrom: (mid_ptr + global_offset)[0:n/4])
    {
        for (int i = 0; i < n/4; i++) {
            (mid_ptr + global_offset)[i] = i * i;
        }
    }
}

int main(int argc, char *argv[]) {
    /* Use argc for dynamic bounds to prevent constant folding */
    int size = 100;
    if (argc > 1) size = atoi(argv[1]);
    if (size < 20) size = 20;
    
    /* Allocate and initialize arrays */
    int *arr1 = (int *)malloc(size * sizeof(int));
    int *arr2 = (int *)malloc(size * sizeof(int));
    struct Data data = {arr1, arr2};
    
    for (int i = 0; i < size; i++) {
        arr1[i] = i;
        arr2[i] = size - i;
    }
    
    /* Dynamic bounds from command line */
    int start = (argc > 2) ? atoi(argv[2]) % (size/2) : 5;
    int length = (argc > 3) ? atoi(argv[3]) % (size/2) : 10;
    int cond = (argc > 4) ? 1 : 0;
    
    /* Call functions with array sections */
    process_sections(arr1, arr2, start, length, cond, argc);
    mixed_sections(&data, start, length);
    problematic_usage(arr1, size);
    
    /* Verify results to prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < size; i++) {
        checksum += arr1[i] + arr2[i];
    }
    printf("Checksum: %d\n", checksum);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    
    return 0;
}
