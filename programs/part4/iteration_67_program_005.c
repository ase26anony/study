/* This program is designed to trigger the pretty-printing logic for
   OMP_ARRAY_SECTION nodes in GCC's tree-pretty-print.cc, specifically
   lines 2736-2748. It uses complex base expressions with operator
   precedence issues, variable bounds, and multiple OpenMP contexts to
   ensure the uncovered code block is executed during compilation with
   appropriate dump or diagnostic flags. */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Helper to prevent constant folding */
static volatile int prevent_constant = 0;

/* Function that returns a pointer based on condition - used to create
   conditional expression base for array sections */
int* select_array(int* a, int* b, int cond) {
    return cond ? a : b;
}

/* Function expecting a pointer - used to create type mismatch warnings
   when array sections are passed */
void expects_pointer(int* p) {
    if (p) return;
}

/* Initialize array with values */
void init_array(int* arr, int n) {
    for (int i = 0; i < n; i++) {
        arr[i] = i + prevent_constant;
    }
}

/* Compute checksum to prevent dead code elimination */
int checksum(int* arr, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += arr[i];
    }
    return sum;
}

int main(int argc, char* argv[]) {
    /* Use command-line arguments for dynamic bounds to prevent constant folding */
    int size = (argc > 1) ? atoi(argv[1]) : 100;
    int start = (argc > 2) ? atoi(argv[2]) : 10;
    int length = (argc > 3) ? atoi(argv[3]) : 20;
    int offset = (argc > 4) ? atoi(argv[4]) : 5;
    
    if (size <= 0) size = 100;
    if (start < 0) start = 10;
    if (length <= 0) length = 20;
    if (offset < 0) offset = 5;
    
    /* Allocate and initialize arrays */
    int* arr1 = (int*)malloc(size * sizeof(int));
    int* arr2 = (int*)malloc(size * sizeof(int));
    int* arr3 = (int*)malloc(size * sizeof(int));
    
    init_array(arr1, size);
    init_array(arr2, size);
    init_array(arr3, size);
    
    /* STRUCTURE WITH ARRAY MEMBER - for member access base expression */
    struct {
        int* data;
        int len;
    } container;
    container.data = arr3;
    container.len = size;
    
    /* 1. TARGET DATA REGION with complex array section bases */
    /* This creates OMP_ARRAY_SECTION nodes in map clauses */
    #pragma omp target data \
        map(to: (arr1 + offset)[0:length]) \
        map(tofrom: select_array(arr1, arr2, start > 5)[start:length]) \
        map(alloc: container.data[offset:length])
    {
        /* 2. TARGET REGION inside data region - another context */
        #pragma omp target \
            map(always, tofrom: (arr1 + (offset * 2))[0:length/2]) \
            map(from: arr2[start:length])
        {
            /* Simple computation to ensure code isn't eliminated */
            for (int i = 0; i < length/2; i++) {
                (arr1 + (offset * 2))[i] += 1;
            }
            
            for (int i = 0; i < length; i++) {
                arr2[start + i] = i * 2;
            }
        }
        
        /* 3. Additional computation on host within data region */
        /* Using array section in pointer arithmetic context */
        int* section_ptr = &(arr1[offset]);
        for (int i = 0; i < length; i++) {
            section_ptr[i] *= 2;
        }
    }
    
    /* 4. TASK DEPEND clause with array section - different OpenMP context */
    /* This creates another OMP_ARRAY_SECTION node */
    #pragma omp task depend(out: arr1[0:size]) shared(arr1)
    {
        for (int i = 0; i < size; i++) {
            arr1[i] = arr1[i] > 100 ? 100 : arr1[i];
        }
    }
    
    #pragma omp task depend(in: arr1[0:size]) shared(arr1, arr2)
    {
        for (int i = 0; i < size; i++) {
            arr2[i] += arr1[i];
        }
    }
    
    /* 5. DELIBERATE TYPE CHECKING ISSUE - to potentially trigger diagnostics */
    /* Passing array section to function expecting pointer (without proper OpenMP context) */
    /* This may generate warnings/errors that invoke the pretty-printer */
    if (argc > 5) {
        /* Conditional to avoid compilation failure in normal case */
        expects_pointer(&arr1[start]);  /* This is OK */
        /* The next line might trigger OpenMP mapping warnings */
        /* expects_pointer(&arr1[start:length]); */ /* Uncomment for diagnostic testing */
    }
    
    /* 6. ENTER/EXIT DATA with array sections - more contexts */
    #pragma omp target enter data \
        map(to: arr3[(start + offset):length]) \
        map(alloc: (arr3 + size/2)[0:length/2])
    
    #pragma omp target \
        map(tofrom: arr3[(start + offset):length])
    {
        for (int i = 0; i < length; i++) {
            arr3[start + offset + i] += 3;
        }
    }
    
    #pragma omp target exit data \
        map(from: arr3[(start + offset):length]) \
        map(release: (arr3 + size/2)[0:length/2])
    
    /* Compute and print checksums to ensure execution */
    int sum1 = checksum(arr1, size);
    int sum2 = checksum(arr2, size);
    int sum3 = checksum(arr3, size);
    
    printf("Checksums: arr1=%d, arr2=%d, arr3=%d\n", sum1, sum2, sum3);
    printf("Array sections used with bases:\n");
    printf("  - Pointer arithmetic: (arr1 + offset)[0:length]\n");
    printf("  - Conditional operator: (cond ? arr1 : arr2)[start:length]\n");
    printf("  - Structure member: container.data[offset:length]\n");
    printf("  - Multiple clauses: to, tofrom, alloc, depend, enter/exit data\n");
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(arr3);
    
    return 0;
}
