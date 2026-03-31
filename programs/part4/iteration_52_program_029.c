/* test-omp-array-sections.c
 * 
 * This test is designed to exercise GCC's pretty-printer for OpenMP array sections.
 * Compile with: gcc -fopenmp -fdump-tree-original -O1 -c test-omp-array-sections.c
 * Additional dump flags: -fdump-tree-all, -fdump-tree-gimple
 */

#include <stdlib.h>
#include <stdio.h>

#define N 100
#define SIZE 50

/* Helper functions to create complex base expressions */
int* get_array(void) {
    static int arr[SIZE];
    return arr;
}

struct Container {
    int arr[N];
    int* ptr_arr;
};

/* Function using array section in map clause with simple base */
void test_map_simple(void) {
    int arr[N];
    
    #pragma omp target map(tofrom: arr[0:N])
    {
        for (int i = 0; i < N; i++) {
            arr[i] = i;
        }
    }
}

/* Function using array section with pointer dereference as base */
void test_map_pointer_deref(void) {
    int (*ptr)[N] = malloc(sizeof(int[N]));
    if (!ptr) return;
    
    #pragma omp target data map(tofrom: (*ptr)[0:N])
    {
        #pragma omp target teams distribute parallel for map(tofrom: (*ptr)[0:N])
        for (int i = 0; i < N; i++) {
            (*ptr)[i] = i * 2;
        }
    }
    
    free(ptr);
}

/* Function using array section with structure member access as base */
void test_map_struct_member(void) {
    struct Container s;
    s.ptr_arr = malloc(N * sizeof(int));
    
    #pragma omp target data map(tofrom: s.arr[0:N], s.ptr_arr[0:N])
    {
        #pragma omp target teams distribute parallel for \
            map(tofrom: s.arr[0:N]) map(tofrom: s.ptr_arr[0:N])
        for (int i = 0; i < N; i++) {
            s.arr[i] = i;
            s.ptr_arr[i] = i * 3;
        }
    }
    
    free(s.ptr_arr);
}

/* Function using array section with function call as base */
void test_map_func_call_base(void) {
    #pragma omp target data map(tofrom: get_array()[0:SIZE])
    {
        #pragma omp target teams distribute parallel for \
            map(tofrom: get_array()[0:SIZE])
        for (int i = 0; i < SIZE; i++) {
            get_array()[i] = i * 4;
        }
    }
}

/* Function using array section with cast expression as base */
void test_map_cast_base(void) {
    char* buffer = malloc(N * sizeof(int));
    
    #pragma omp target data map(tofrom: ((int*)buffer)[0:N])
    {
        #pragma omp target teams distribute parallel for \
            map(tofrom: ((int*)buffer)[0:N])
        for (int i = 0; i < N; i++) {
            ((int*)buffer)[i] = i * 5;
        }
    }
    
    free(buffer);
}

/* Function using array section in depend clause with variable bounds */
void test_task_depend(void) {
    int x[N], y[N];
    int start = 10, len = 20;
    
    #pragma omp task depend(inout: x[0:1]) \
                     depend(out: y[start:len])
    {
        for (int i = 0; i < len; i++) {
            y[start + i] = x[0] + i;
        }
    }
    
    #pragma omp task depend(in: y[1:N-1])
    {
        int sum = 0;
        for (int i = 1; i < N-1; i++) {
            sum += y[i];
        }
        printf("Sum: %d\n", sum);
    }
}

/* Function using array section with complex bounds expressions */
void test_complex_bounds(void) {
    int arr[N];
    int i = 5, j = 10;
    int flag = 1;
    
    /* Arithmetic expressions in bounds */
    #pragma omp target map(tofrom: arr[i+1:j-i-1])
    {
        for (int k = 0; k < j-i-1; k++) {
            arr[i+1 + k] = k * 2;
        }
    }
    
    /* Ternary operator in lower bound */
    #pragma omp target map(tofrom: arr[flag ? 0 : 10:N/2])
    {
        for (int k = 0; k < N/2; k++) {
            int idx = (flag ? 0 : 10) + k;
            if (idx < N) arr[idx] = k * 3;
        }
    }
}

/* Function with custom reduction using array section */
#pragma omp declare reduction(array_sum: int [N] : \
    for (int i = 0; i < N; i++) omp_out[i] += omp_in[i]) \
    initializer(omp_priv = omp_orig)

void test_reduction_array_section(void) {
    int arr[N];
    for (int i = 0; i < N; i++) arr[i] = i;
    
    /* Note: Array section reductions require custom reductions */
    #pragma omp parallel for reduction(array_sum: arr[0:N])
    for (int i = 0; i < N; i++) {
        arr[i] += 1;
    }
}

/* Function using linear clause with array section */
void test_linear_array_section(void) {
    int arr[N];
    int idx = 0;
    
    #pragma omp parallel for linear(arr[idx:1]:1)
    for (int i = 0; i < N; i++) {
        arr[idx] = i;
        idx++;  /* Linear clause ensures private idx increments */
    }
}

/* Function using array section in multiple clauses */
void test_multiple_clauses(void) {
    int a[N], b[N], c[N];
    
    #pragma omp target teams distribute parallel for \
        map(to: a[0:N], b[0:N]) map(from: c[0:N]) \
        reduction(+:c[0:N/2])
    for (int i = 0; i < N; i++) {
        c[i] = a[i] + b[i];
        if (i < N/2) c[i] += i;
    }
}

/* Main function to call all tests and prevent dead code elimination */
int main(void) {
    test_map_simple();
    test_map_pointer_deref();
    test_map_struct_member();
    test_map_func_call_base();
    test_map_cast_base();
    
    #pragma omp parallel
    {
        #pragma omp single
        {
            test_task_depend();
        }
    }
    
    test_complex_bounds();
    test_reduction_array_section();
    test_linear_array_section();
    test_multiple_clauses();
    
    return 0;
}
