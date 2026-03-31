/* tree-pretty-print-omp-array-section.c
 * 
 * This program demonstrates OpenMP array sections with complex base expressions
 * to trigger the uncovered pretty-printing logic for OMP_ARRAY_SECTION nodes.
 * Compile with: gcc -std=c99 -fopenmp -O1 -fdump-tree-original -fdump-tree-omplower
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Helper function to prevent constant folding */
static int get_value(int base, int offset) {
    volatile int temp = base;
    return temp + offset;
}

/* Function using array section in map clause */
void process_section(int *arr, int start, int length, int n) {
    /* Complex base expression 1: conditional operator */
    int *ptr = (n > 0) ? arr : (arr + 10);
    
    #pragma omp target data map(tofrom: (n > 0 ? arr : (arr + 10))[start:length])
    {
        /* Complex base expression 2: pointer arithmetic */
        #pragma omp target map(tofrom: (ptr + start)[0:length])
        {
            for (int i = 0; i < length; i++) {
                (ptr + start)[i] += i;
            }
        }
    }
}

/* Structure with array member for member access testing */
struct Data {
    int header;
    int values[100];
    int footer;
};

/* Function demonstrating member array sections */
void process_struct_section(struct Data *d, int idx, int len) {
    /* Complex base expression 3: structure member access */
    #pragma omp target data map(tofrom: d->values[idx:len])
    {
        #pragma omp target map(tofrom: d[0].values[idx:len])
        {
            for (int i = 0; i < len; i++) {
                d->values[idx + i] *= 2;
            }
        }
    }
}

/* Function with deliberate type issues to trigger diagnostics */
void problematic_usage(int *arr, int n) {
    /* This may trigger warnings about array section usage */
    int *section = &arr[0:n];  /* Array section in non-OpenMP context */
    (void)section; /* Suppress unused warning */
    
    /* Array section with incompatible bounds */
    int volatile v = 5;
    #pragma omp target data map(to: arr[v:n])  /* v is volatile, prevents const folding */
    {
        /* Empty - just for tree generation */
    }
}

/* Task with array section in depend clause */
void process_with_tasks(int *arr1, int *arr2, int size) {
    #pragma omp task depend(inout: arr1[0:size])
    {
        for (int i = 0; i < size; i++) {
            arr1[i] = i;
        }
    }
    
    #pragma omp task depend(in: arr1[0:size]) depend(out: arr2[0:size])
    {
        for (int i = 0; i < size; i++) {
            arr2[i] = arr1[i] * 2;
        }
    }
}

int main(int argc, char *argv[]) {
    /* Use argc to prevent constant propagation */
    int size = (argc > 1) ? atoi(argv[1]) : 100;
    if (size <= 0) size = 100;
    
    /* Dynamic allocation prevents static analysis */
    int *array1 = (int *)malloc(size * sizeof(int));
    int *array2 = (int *)malloc(size * sizeof(int));
    
    /* Initialize arrays */
    for (int i = 0; i < size; i++) {
        array1[i] = i;
        array2[i] = 0;
    }
    
    /* Get bounds from command line to prevent constant folding */
    int start = (argc > 2) ? atoi(argv[2]) % size : 10;
    int length = (argc > 3) ? atoi(argv[3]) % (size - start) : 20;
    
    /* Process with complex base expressions */
    process_section(array1, start, length, argc);
    
    /* Create struct for member access test */
    struct Data data;
    for (int i = 0; i < 100; i++) {
        data.values[i] = i;
    }
    
    /* Process struct with array section */
    int idx = get_value(5, 3);  /* Volatile prevents const folding */
    int len = get_value(10, 2);
    process_struct_section(&data, idx, len);
    
    /* Try problematic usage to trigger diagnostics */
    problematic_usage(array2, size/2);
    
    /* Process with tasks */
    #pragma omp parallel
    #pragma omp single
    {
        process_with_tasks(array1, array2, size/4);
    }
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < size; i++) {
        checksum += array1[i] + array2[i] + data.values[i % 100];
    }
    
    printf("Checksum: %d\n", checksum);
    
    /* Cleanup */
    free(array1);
    free(array2);
    
    return 0;
}
