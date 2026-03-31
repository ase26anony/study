/* test-omp-array-section.c
 * 
 * This program is designed to trigger the pretty-printing logic for
 * OMP_ARRAY_SECTION nodes in GCC's tree-pretty-print.cc, specifically
 * lines 2736-2748, which handle the formatting of OpenMP array sections
 * with parentheses for complex base expressions.
 *
 * Compilation flags to generate tree dumps and diagnostics:
 *   gcc -O1 -fopenmp -fdump-tree-original -fdump-tree-omplower -c test-omp-array-section.c
 *   gcc -O0 -fopenmp -Wall -Werror=openmp-mapping -c test-omp-array-section.c
 *   gcc -O2 -fopenmp -foffload=disable -fdump-tree-optimized -c test-omp-array-section.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Helper function to introduce variability and prevent constant folding */
static int get_bound(int base, int offset) {
    volatile int v = base; /* volatile to prevent optimization */
    return v + offset;
}

/* Function that uses array sections with complex base expressions */
void process_sections(int *arr1, int *arr2, int n, int start, int len, int cond) {
    /* Complex base expression 1: conditional operator (low precedence) */
    /* This should trigger op_prio check for parentheses */
    #pragma omp target data map(tofrom: (cond ? arr1 : arr2)[start:len])
    {
        #pragma omp target map(tofrom: (cond ? arr1 : arr2)[start:len])
        {
            for (int i = 0; i < len; i++) {
                (cond ? arr1 : arr2)[start + i] += i;
            }
        }
    }

    /* Complex base expression 2: pointer arithmetic */
    int *ptr = arr1 + n/2;
    int offset = 2;
    #pragma omp target data map(tofrom: (ptr + offset)[0:len/2])
    {
        #pragma omp target map(tofrom: (ptr + offset)[0:len/2])
        {
            for (int i = 0; i < len/2; i++) {
                (ptr + offset)[i] *= 2;
            }
        }
    }
}

/* Structure with array member to test member access as base */
struct WithArray {
    int header;
    int data[100];
};

void process_struct_section(struct WithArray *s, int start, int len) {
    /* Complex base expression 3: structure member access */
    #pragma omp target data map(tofrom: s->data[start:len])
    {
        #pragma omp target map(tofrom: s->data[start:len])
        {
            for (int i = 0; i < len; i++) {
                s->data[start + i] = start + i;
            }
        }
    }
}

/* Function that deliberately creates type warning/error context */
void problematic_usage(int *arr, int n) {
    /* This may trigger diagnostics about array section in non-OpenMP context */
    int *section = &arr[0:n];  /* Array section used in non-OpenMP context */
    (void)section; /* Suppress unused warning */
    
    /* Array section in is_device_ptr clause with pointer arithmetic */
    int *dev_ptr = NULL;
    #pragma omp target data map(to: arr[0:n]) is_device_ptr(dev_ptr)
    {
        dev_ptr = arr;
        #pragma omp target is_device_ptr(dev_ptr) map(from: arr[n/2:5])
        {
            for (int i = 0; i < 5; i++) {
                arr[n/2 + i] = dev_ptr[i];
            }
        }
    }
}

/* Task dependency with array sections */
void task_with_depend(int *arr, int n, int chunk) {
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp task depend(out: arr[0:chunk])
            {
                for (int i = 0; i < chunk; i++) arr[i] = 1;
            }
            
            #pragma omp task depend(in: arr[0:chunk]) depend(out: arr[chunk:chunk])
            {
                for (int i = chunk; i < 2*chunk; i++) arr[i] = arr[i-chunk] + 1;
            }
            
            #pragma omp task depend(in: arr[chunk:chunk])
            {
                int sum = 0;
                for (int i = 0; i < 2*chunk; i++) sum += arr[i];
                printf("Task sum: %d\n", sum);
            }
        }
    }
}

int main(int argc, char *argv[]) {
    /* Use command-line arguments to prevent constant propagation */
    int size = argc > 1 ? atoi(argv[1]) : 100;
    int start = argc > 2 ? atoi(argv[2]) : 10;
    int length = argc > 3 ? atoi(argv[3]) : 20;
    int cond = argc > 4 ? atoi(argv[4]) : 1;
    
    /* Dynamic allocation prevents compile-time knowledge */
    int *array1 = (int *)malloc(size * sizeof(int));
    int *array2 = (int *)malloc(size * sizeof(int));
    struct WithArray mystruct;
    
    if (!array1 || !array2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays */
    for (int i = 0; i < size; i++) {
        array1[i] = i;
        array2[i] = size - i;
        if (i < 100) mystruct.data[i] = 0;
    }
    
    /* Get bounds using volatile/function to prevent constant folding */
    int dyn_start = get_bound(start, 0);
    int dyn_len = get_bound(length, 0);
    
    printf("Processing with start=%d, len=%d, cond=%d\n", dyn_start, dyn_len, cond);
    
    /* Test 1: Complex base expressions with conditional operator */
    process_sections(array1, array2, size, dyn_start, dyn_len, cond);
    
    /* Test 2: Structure member access */
    process_struct_section(&mystruct, dyn_start/2, dyn_len/2);
    
    /* Test 3: Problematic usage for diagnostics */
    problematic_usage(array1, size);
    
    /* Test 4: Task dependencies with array sections */
    task_with_depend(array1, size, dyn_len);
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < size; i++) {
        checksum += array1[i] + array2[i];
        if (i < 100) checksum += mystruct.data[i];
    }
    printf("Final checksum: %d\n", checksum);
    
    free(array1);
    free(array2);
    
    return 0;
}
