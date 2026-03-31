/* test-omp-array-sections.c
 * This program is designed to trigger the OMP_ARRAY_SECTION pretty-printing
 * logic in GCC's tree-pretty-print.cc (lines 2736-2748).
 * Compile with: gcc -fopenmp -fdump-tree-original -O1 -c test-omp-array-sections.c
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
        for (int i = 0; i < N; i++)
            arr[i] = i;
    }
}

/* Function using array section with pointer dereference base */
void test_map_pointer_deref(void) {
    int (*ptr)[N] = malloc(sizeof(*ptr));
    if (!ptr) return;
    
    #pragma omp target data map(tofrom: (*ptr)[0:N])
    {
        #pragma omp target teams distribute parallel for map(tofrom: (*ptr)[0:N])
        for (int i = 0; i < N; i++)
            (*ptr)[i] = i * 2;
    }
    free(ptr);
}

/* Function using array section with structure member access */
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

/* Function using array section with function call base */
void test_map_func_call_base(void) {
    int n = SIZE / 2;
    #pragma omp target map(tofrom: get_array()[0:n])
    {
        int* arr = get_array();
        for (int i = 0; i < n; i++)
            arr[i] = i * 4;
    }
}

/* Function using array section with cast expression base */
void test_map_cast_base(void) {
    char* buffer = malloc(N * sizeof(int));
    int offset = 10;
    int count = N - 20;
    
    #pragma omp target map(tofrom: ((int*)buffer)[offset:count])
    {
        int* arr = (int*)buffer;
        for (int i = 0; i < count; i++)
            arr[offset + i] = i * 5;
    }
    free(buffer);
}

/* Function using array section in depend clause */
void test_task_depend(void) {
    int x[100], y[100];
    int start = 10, len = 80;
    
    #pragma omp task depend(in: x[0:1]) shared(x)
    { x[0] = 1; }
    
    #pragma omp task depend(inout: y[start:len]) shared(y)
    {
        for (int i = 0; i < len; i++)
            y[start + i] = i * 6;
    }
    
    #pragma omp task depend(in: x[0:1]) depend(out: y[1:SIZE-1]) shared(x, y)
    { y[1] = x[0]; }
    
    #pragma omp taskwait
}

/* Function using array section with complex bounds */
void test_complex_bounds(void) {
    int arr[N];
    int i = 5, j = 30;
    int start = 10, end = 90;
    int flag = 1;
    
    /* Variable expressions */
    #pragma omp target map(tofrom: arr[i:j])
    {
        for (int k = 0; k < j; k++)
            arr[i + k] = k * 7;
    }
    
    /* Arithmetic expressions */
    #pragma omp target map(tofrom: arr[start+1:end-start-1])
    {
        for (int k = 0; k < end-start-1; k++)
            arr[start + 1 + k] = k * 8;
    }
    
    /* Ternary expression in lower bound */
    #pragma omp target map(tofrom: arr[flag ? 0 : 10:SIZE])
    {
        for (int k = 0; k < SIZE; k++)
            arr[(flag ? 0 : 10) + k] = k * 9;
    }
}

/* Function using array section in linear clause */
void test_linear_clause(void) {
    int arr[N];
    int idx = 0;
    
    #pragma omp parallel for linear(arr[idx:1]:1)
    for (int i = 0; i < N; i++) {
        arr[idx] = i;
        idx++;  // Linear clause increments the implicit array section
    }
}

/* Custom reduction for array section */
#pragma omp declare reduction(arr_add : int [SIZE] : \
    for (int i = 0; i < SIZE; i++) \
        omp_out[i] += omp_in[i]) \
    initializer(omp_priv = omp_orig)

void test_reduction_array_section(void) {
    int arr[SIZE];
    for (int i = 0; i < SIZE; i++) arr[i] = i;
    
    /* Note: Array section reduction may require custom reduction */
    #pragma omp parallel for reduction(arr_add: arr[0:SIZE])
    for (int i = 0; i < SIZE; i++) {
        arr[i] += 1;
    }
}

/* Main driver that calls all test functions */
int main(void) {
    test_map_simple();
    test_map_pointer_deref();
    test_map_struct_member();
    test_map_func_call_base();
    test_map_cast_base();
    
    test_task_depend();
    test_complex_bounds();
    test_linear_clause();
    test_reduction_array_section();
    
    printf("All OpenMP array section tests completed (compile with -fopenmp -fdump-tree-original)\n");
    return 0;
}
