/* test-omp-array-sections.c
 * 
 * This test program is designed to trigger the OMP_ARRAY_SECTION
 * pretty-printing logic in GCC's tree-pretty-print.cc.
 * 
 * Compile with: gcc -fopenmp -fdump-tree-original -O1 -c test-omp-array-sections.c
 * Additional dump flags: -fdump-tree-all, -fdump-tree-gimple
 */

#include <stdlib.h>
#include <stdio.h>

#define N 100
#define SIZE 50

/* Helper functions to create complex expressions */
int *get_array(void) {
    static int arr[SIZE];
    return arr;
}

int compute_lower(void) { return 5; }
int compute_length(void) { return 20; }

struct Container {
    int arr[N];
    int *ptr;
};

/* Function using array section in map clause */
void target_map_sections(int *arr, int n) {
    #pragma omp target map(tofrom: arr[0:n])
    {
        for (int i = 0; i < n; i++) {
            arr[i] *= 2;
        }
    }
}

/* Function using array section in depend clause */
void task_depend_sections(int *x, int *y, int size) {
    #pragma omp task depend(in: x[0:1]) depend(out: y[1:size-1])
    {
        for (int i = 1; i < size-1; i++) {
            y[i] = x[0] + i;
        }
    }
}

/* Function with complex base expressions */
void complex_base_expressions(struct Container *p, int **ptr2d, int *buffer, 
                              int offset, int count, int flag, int len) {
    /* Structure member access as base */
    #pragma omp target map(tofrom: p->arr[2:8])
    {
        for (int i = 2; i < 10; i++) {
            p->arr[i] += i;
        }
    }
    
    /* Pointer dereference as base */
    #pragma omp task depend(inout: (*ptr2d)[offset:count])
    {
        for (int i = offset; i < offset + count; i++) {
            (*ptr2d)[i] = i;
        }
    }
    
    /* Cast expression as base */
    #pragma omp target data map(tofrom: ((int *)buffer)[offset:count])
    {
        #pragma omp target map(tofrom: ((int *)buffer)[offset:count])
        {
            for (int i = offset; i < offset + count; i++) {
                ((int *)buffer)[i] *= 2;
            }
        }
    }
    
    /* Function call returning pointer as base */
    #pragma omp target map(to: get_array()[0:10])
    {
        int *arr = get_array();
        for (int i = 0; i < 10; i++) {
            arr[i] = i;
        }
    }
    
    /* Ternary operator in lower bound */
    #pragma omp task depend(in: p->arr[flag ? 0 : 10: len])
    {
        for (int i = (flag ? 0 : 10); i < (flag ? 0 : 10) + len; i++) {
            p->arr[i] += 1;
        }
    }
}

/* Function with arithmetic expressions in bounds */
void arithmetic_bounds(int *arr, int start, int end) {
    /* Arithmetic in both lower bound and length */
    #pragma omp target teams distribute parallel for \
                map(to: arr[start+1:end-start-1])
    for (int i = start+1; i < end; i++) {
        arr[i] = arr[i] * arr[i];
    }
}

/* Function with function calls in bounds */
void function_call_bounds(int *arr) {
    /* Function calls as lower bound and length */
    #pragma omp target data map(tofrom: arr[compute_lower():compute_length()])
    {
        #pragma omp target map(tofrom: arr[compute_lower():compute_length()])
        {
            int lower = compute_lower();
            int length = compute_length();
            for (int i = lower; i < lower + length; i++) {
                arr[i] += 3;
            }
        }
    }
}

/* Custom reduction for array section */
#pragma omp declare reduction(array_section_add : int [SIZE] : \
    for (int i = 0; i < SIZE; i++) \
        omp_out[i] += omp_in[i]) \
    initializer(omp_priv = omp_orig)

void reduction_with_section(int *arr, int size) {
    /* Note: Array section reductions typically require custom reductions */
    #pragma omp parallel for reduction(array_section_add: arr[0:size])
    for (int i = 0; i < size; i++) {
        arr[i] += i;
    }
}

/* Linear clause with array section */
void linear_with_section(int *arr, int n) {
    int idx = 0;
    #pragma omp parallel for linear(arr[idx:1]:1) linear(idx:1)
    for (int i = 0; i < n; i++) {
        arr[idx] = i;
        idx++;
    }
}

/* Multiple array sections in same directive */
void multiple_sections(int *a, int *b, int *c, int n) {
    #pragma omp target teams distribute parallel for \
                map(to: a[0:n], b[0:n]) map(from: c[0:n])
    for (int i = 0; i < n; i++) {
        c[i] = a[i] + b[i];
    }
}

int main(void) {
    /* Allocate and initialize arrays */
    int *arr1 = (int *)malloc(N * sizeof(int));
    int *arr2 = (int *)malloc(N * sizeof(int));
    int *arr3 = (int *)malloc(N * sizeof(int));
    int *buffer = (int *)malloc(N * sizeof(int));
    
    int *ptr2d = (int *)malloc(N * sizeof(int));
    struct Container s;
    struct Container *p = &s;
    
    /* Initialize data */
    for (int i = 0; i < N; i++) {
        arr1[i] = i;
        arr2[i] = i * 2;
        arr3[i] = 0;
        buffer[i] = i * 3;
        ptr2d[i] = i * 4;
        s.arr[i] = i * 5;
    }
    
    /* Call functions with various OpenMP array section patterns */
    
    /* Simple map clause with array section */
    target_map_sections(arr1, N);
    
    /* Task with depend clauses */
    #pragma omp parallel
    #pragma omp single
    {
        task_depend_sections(arr1, arr2, N);
    }
    
    /* Complex base expressions */
    complex_base_expressions(p, &ptr2d, buffer, 10, 20, 1, 15);
    
    /* Arithmetic expressions in bounds */
    arithmetic_bounds(arr3, 5, 45);
    
    /* Function calls in bounds */
    function_call_bounds(arr1);
    
    /* Multiple sections */
    multiple_sections(arr1, arr2, arr3, N/2);
    
    /* Linear clause with array section */
    linear_with_section(arr1, N);
    
    /* Wait for all tasks */
    #pragma omp taskwait
    
    /* Print results to prevent dead code elimination */
    printf("Results: %d %d %d\n", arr1[0], arr2[1], arr3[2]);
    printf("Container: %d\n", s.arr[5]);
    printf("Buffer: %d\n", buffer[15]);
    printf("Ptr2d: %d\n", ptr2d[10]);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(arr3);
    free(buffer);
    free(ptr2d);
    
    return 0;
}
