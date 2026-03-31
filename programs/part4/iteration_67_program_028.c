/* test-omp-array-section.c
 * 
 * This program is designed to trigger the pretty-printing logic for
 * OMP_ARRAY_SECTION nodes in GCC's tree-pretty-print.cc, specifically
 * lines 2736-2748. It uses OpenMP array sections with complex base
 * expressions (to test op_prio comparisons) and variable bounds in
 * multiple OpenMP contexts.
 *
 * Compile with: gcc -O1 -fopenmp -fdump-tree-original -fdump-tree-omplower \
 *                  -Wall -Werror=openmp-mapping -c test-omp-array-section.c
 *
 * The -fdump-tree-* flags will cause GCC to dump its internal representation,
 * invoking the pretty-printer on all tree nodes, including OMP_ARRAY_SECTION.
 * The -Werror=openmp-mapping flag may trigger diagnostics that also require
 * pretty-printing of array sections.
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Helper function to use in conditional base expressions */
int select_array(int cond, int *a, int *b) {
    return cond ? *a : *b;
}

/* Function that uses array sections with complex base expressions */
void process_sections(int *arr1, int *arr2, int n, int offset, int len, int flag) {
    volatile int start = offset;  /* Prevent constant folding */
    volatile int length = len;
    
    /* 1. Array section with pointer arithmetic base: (ptr + offset)[0:len] */
    #pragma omp target data map(tofrom: (arr1 + start)[0:length])
    {
        #pragma omp target map(alloc: (arr1 + start)[0:length])
        {
            for (int i = 0; i < length; i++) {
                (arr1 + start)[i] += 1;
            }
        }
    }
    
    /* 2. Array section with conditional operator base: (flag ? arr1 : arr2)[start:len] */
    #pragma omp target enter data map(to: (flag ? arr1 : arr2)[start:length])
    #pragma omp target map(alloc: (flag ? arr1 : arr2)[start:length])
    {
        for (int i = 0; i < length; i++) {
            (flag ? arr1 : arr2)[start + i] *= 2;
        }
    }
    #pragma omp target exit data map(from: (flag ? arr1 : arr2)[start:length])
    
    /* 3. Array section with function call in base expression */
    int *base_ptr = (select_array(flag, arr1, arr2) == *arr1) ? arr1 : arr2;
    #pragma omp target data map(tofrom: base_ptr[start:length])
    {
        #pragma omp target
        {
            for (int i = 0; i < length; i++) {
                base_ptr[start + i] += 3;
            }
        }
    }
}

/* Structure with array member to test member access base */
struct WithArray {
    int header;
    int data[100];
};

void process_struct_section(struct WithArray *s, int n, int m) {
    /* 4. Array section with structure member access base: s->data[n:m] */
    #pragma omp target map(tofrom: s->data[n:m])
    {
        for (int i = 0; i < m; i++) {
            s->data[n + i] = i;
        }
    }
    
    /* 5. Complex base with multiple operators: (s->data + n)[0:m] */
    #pragma omp target data map(alloc: (s->data + n)[0:m])
    {
        #pragma omp target
        {
            for (int i = 0; i < m; i++) {
                (s->data + n)[i] += s->header;
            }
        }
    }
}

/* Task dependency with array section */
void task_with_dep(int *arr, int n, int chunk) {
    #pragma omp task depend(out: arr[0:chunk])
    {
        for (int i = 0; i < chunk; i++) {
            arr[i] = 1;
        }
    }
    
    #pragma omp task depend(in: arr[0:chunk]) depend(out: arr[chunk:n-chunk])
    {
        for (int i = chunk; i < n; i++) {
            arr[i] = arr[i-chunk] + 1;
        }
    }
}

int main(int argc, char *argv[]) {
    /* Use command-line arguments to prevent constant propagation */
    int size = (argc > 1) ? atoi(argv[1]) : 100;
    int offset = (argc > 2) ? atoi(argv[2]) : 10;
    int length = (argc > 3) ? atoi(argv[3]) : 20;
    int flag = (argc > 4) ? atoi(argv[4]) : 1;
    
    if (size < offset + length) {
        fprintf(stderr, "Error: size must be >= offset + length\n");
        return 1;
    }
    
    /* Allocate and initialize arrays */
    int *arr1 = (int *)malloc(size * sizeof(int));
    int *arr2 = (int *)malloc(size * sizeof(int));
    struct WithArray s = {0};
    
    for (int i = 0; i < size; i++) {
        arr1[i] = i;
        arr2[i] = size - i;
        if (i < 100) s.data[i] = 0;
    }
    
    /* Process with various array section patterns */
    process_sections(arr1, arr2, size, offset, length, flag);
    
    process_struct_section(&s, offset, length);
    
    /* Task-based array section usage */
    #pragma omp parallel
    #pragma omp single
    task_with_dep(arr1, size, length);
    
    /* Compute checksum to prevent dead code elimination */
    long long sum = 0;
    for (int i = 0; i < size; i++) {
        sum += arr1[i] + arr2[i];
        if (i < 100) sum += s.data[i];
    }
    printf("Checksum: %lld\n", sum);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    
    return 0;
}
