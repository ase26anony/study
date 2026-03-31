/* This program demonstrates OpenMP array sections with complex base expressions
   to trigger the OMP_ARRAY_SECTION pretty-printing logic in GCC's tree-pretty-print.cc.
   Compile with: gcc -std=c99 -fopenmp -O1 -fdump-tree-original -fdump-tree-omplower -Wall -c tree-pretty-print-omp-section.c
   Additional flags for diagnostics: -Werror=openmp-mapping -fdump-tree-all
*/

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Helper function to introduce variability and prevent constant folding */
static int get_value(int base, int offset) {
    volatile int v = base; /* volatile to prevent optimization */
    return v + offset;
}

/* Function that uses array section in a complex base expression context */
void process_section(int *arr, int start, int length, int use_alt) {
    int *alt_arr = arr + 100; /* Different memory region */
    
    /* Complex base expression: conditional operator with lower precedence than [] */
    #pragma omp target data map(tofrom: (use_alt ? alt_arr : arr)[start:length])
    {
        #pragma omp target map(tofrom: (use_alt ? alt_arr : arr)[start:length])
        {
            for (int i = 0; i < length; i++) {
                /* Access through the conditional base */
                (use_alt ? alt_arr : arr)[start + i] += i;
            }
        }
    }
}

/* Another function using pointer arithmetic in base expression */
void process_with_offset(int *base_ptr, int offset, int start, int len) {
    /* Base expression: pointer arithmetic (lower precedence than []) */
    #pragma omp target enter data map(to: (base_ptr + offset)[start:len])
    
    #pragma omp target map(tofrom: (base_ptr + offset)[start:len])
    {
        for (int i = 0; i < len; i++) {
            (base_ptr + offset)[start + i] *= 2;
        }
    }
    
    #pragma omp target exit data map(from: (base_ptr + offset)[start:len])
}

/* Function that might trigger diagnostics due to type/size mismatch */
void problematic_section(int *arr, int n) {
    /* Using array section in a context that might provoke warnings */
    int (*ptr_array)[10] = (int (*)[10])arr;
    
    /* This array section has different element type than the base pointer */
    #pragma omp target data map(to: arr[0:n])
    {
        /* Mixing array sections with different dimensions */
        #pragma omp target map(tofrom: arr[0:n])
        {
            /* Some computation */
            for (int i = 0; i < n; i++) {
                arr[i] = i;
            }
        }
    }
}

/* Structure with array member for member access base expression */
struct DataContainer {
    int header;
    int values[100];
    int footer;
};

void process_struct_section(struct DataContainer *container, int idx, int len) {
    /* Base expression: structure member access */
    #pragma omp target data map(tofrom: container->values[idx:len])
    {
        #pragma omp target map(tofrom: container->values[idx:len])
        {
            for (int i = 0; i < len; i++) {
                container->values[idx + i] += container->header;
            }
        }
    }
}

/* Task dependency with array section */
void process_with_tasks(int *arr, int n) {
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp task depend(out: arr[0:n/2])
            {
                #pragma omp target map(tofrom: arr[0:n/2])
                {
                    for (int i = 0; i < n/2; i++) {
                        arr[i] = 1;
                    }
                }
            }
            
            #pragma omp task depend(in: arr[0:n/2]) depend(out: arr[n/2:n/2])
            {
                #pragma omp target map(tofrom: arr[n/2:n/2])
                {
                    for (int i = n/2; i < n; i++) {
                        arr[i] = arr[i - n/2] * 2;
                    }
                }
            }
            
            #pragma omp task depend(in: arr[n/2:n/2])
            {
                #pragma omp target map(from: arr[0:n])
                {
                    /* Just copy back */
                }
            }
        }
    }
}

int main(int argc, char *argv[]) {
    /* Use command line arguments to prevent constant propagation */
    int size = argc > 1 ? atoi(argv[1]) : 1000;
    int section_start = argc > 2 ? atoi(argv[2]) : 100;
    int section_len = argc > 3 ? atoi(argv[3]) : 200;
    int offset = argc > 4 ? atoi(argv[4]) : 50;
    
    /* Dynamically allocate arrays to avoid static size assumptions */
    int *array1 = (int *)malloc(size * sizeof(int));
    int *array2 = (int *)malloc(size * sizeof(int));
    struct DataContainer *container = (struct DataContainer *)malloc(sizeof(struct DataContainer));
    
    if (!array1 || !array2 || !container) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize data */
    for (int i = 0; i < size; i++) {
        array1[i] = i;
        array2[i] = size - i;
    }
    container->header = 42;
    for (int i = 0; i < 100; i++) {
        container->values[i] = i * 2;
    }
    container->footer = -1;
    
    /* Test 1: Conditional operator as base expression */
    process_section(array1, section_start, section_len, 0);
    process_section(array2, section_start, section_len, 1);
    
    /* Test 2: Pointer arithmetic as base expression */
    process_with_offset(array1, offset, section_start, section_len);
    
    /* Test 3: Structure member access as base expression */
    process_struct_section(container, 10, 20);
    
    /* Test 4: Task dependencies with array sections */
    process_with_tasks(array1, size);
    
    /* Test 5: Potentially problematic usage for diagnostics */
    problematic_section(array2, size/2);
    
    /* Compute checksum to prevent dead code elimination */
    long long checksum = 0;
    for (int i = 0; i < size; i++) {
        checksum += array1[i] + array2[i];
    }
    for (int i = 0; i < 100; i++) {
        checksum += container->values[i];
    }
    
    printf("Checksum: %lld\n", checksum);
    printf("Container header: %d, footer: %d\n", container->header, container->footer);
    
    /* Cleanup */
    free(array1);
    free(array2);
    free(container);
    
    return 0;
}
