/* test-omp-array-sections.c
 * 
 * This test program is designed to exercise GCC's pretty-printer for
 * OpenMP array sections (OMP_ARRAY_SECTION tree nodes). It contains
 * various OpenMP constructs with array sections having complex base
 * expressions, varied lower bounds, and lengths to ensure coverage
 * of the target lines in tree-pretty-print.cc.
 *
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

int compute_lower(void) { return 5; }
int compute_length(void) { return 20; }

/* Structure with array member */
struct with_array {
    int arr[N];
    int* ptr_arr;
};

/* Function using array section in map clause with complex base */
void test_map_clause(void) {
    int local_arr[N];
    int* dyn_arr = malloc(N * sizeof(int));
    struct with_array s = {0};
    int* ptr = &local_arr[0];
    
    /* Base: array variable */
    #pragma omp target map(tofrom: local_arr[0:N])
    {
        for (int i = 0; i < N; i++)
            local_arr[i] = i;
    }
    
    /* Base: pointer dereference */
    #pragma omp target data map(tofrom: (*ptr)[0:N/2])
    {
        #pragma omp target map(tofrom: (*ptr)[0:N/2])
        for (int i = 0; i < N/2; i++)
            local_arr[i] *= 2;
    }
    
    /* Base: structure member access */
    #pragma omp target map(tofrom: s.arr[10:30])
    {
        for (int i = 0; i < 30; i++)
            s.arr[10 + i] = i * 3;
    }
    
    /* Base: function call returning pointer */
    #pragma omp target map(to: get_array()[0:SIZE])
    {
        int* arr = get_array();
        for (int i = 0; i < SIZE; i++)
            arr[i] = i * 4;
    }
    
    free(dyn_arr);
}

/* Function using array sections in depend clauses */
void test_depend_clause(void) {
    int arr[N];
    int start = 10, len = 20;
    
    /* Base: array variable with variable bounds */
    #pragma omp task depend(inout: arr[start:len])
    {
        for (int i = 0; i < len; i++)
            arr[start + i] += 1;
    }
    
    /* Base: array with arithmetic expressions */
    #pragma omp task depend(in: arr[0:1]) depend(out: arr[N/2:N/4])
    {
        arr[0] = 100;
        for (int i = 0; i < N/4; i++)
            arr[N/2 + i] = 200;
    }
    
    /* Base: array with conditional lower bound */
    int flag = 1;
    #pragma omp task depend(inout: arr[flag ? 0 : 10: len])
    {
        for (int i = 0; i < len; i++)
            arr[i] *= 2;
    }
    
    #pragma omp taskwait
}

/* Function using array section in reduction-like pattern */
void test_reduction_pattern(void) {
    int arr[N];
    int sum = 0;
    
    /* Simulate reduction on array section */
    #pragma omp parallel for reduction(+:sum)
    for (int i = 0; i < N; i++) {
        arr[i] = i;
        sum += arr[i];
    }
    
    /* Linear clause with array section */
    int idx = 0;
    #pragma omp parallel for linear(arr[idx:1]:1)
    for (int i = 0; i < N; i++) {
        arr[idx] += i;
        idx++;
    }
}

/* Function with cast expression as base */
void test_cast_base(void) {
    char buffer[N * sizeof(int)];
    int offset = 10, count = 20;
    
    /* Base: cast expression */
    #pragma omp target map(tofrom: ((int *)buffer)[offset:count])
    {
        int* int_buf = (int*)buffer;
        for (int i = 0; i < count; i++)
            int_buf[offset + i] = i * 10;
    }
}

/* Complex nested array sections */
void test_nested_complex(void) {
    struct with_array struct_arr[5];
    int* ptr_arr[N];
    
    /* Initialize pointer array */
    for (int i = 0; i < N; i++) {
        ptr_arr[i] = malloc(SIZE * sizeof(int));
    }
    
    /* Base: pointer array element with function call bounds */
    #pragma omp target teams distribute parallel for \
                map(to: ptr_arr[0][compute_lower():compute_length()])
    for (int i = 0; i < compute_length(); i++) {
        ptr_arr[0][compute_lower() + i] = i * i;
    }
    
    /* Base: structure array element */
    #pragma omp target map(tofrom: struct_arr[2].arr[5:15])
    {
        for (int i = 0; i < 15; i++)
            struct_arr[2].arr[5 + i] = i * 3;
    }
    
    /* Cleanup */
    for (int i = 0; i < N; i++) {
        free(ptr_arr[i]);
    }
}

/* Main driver function */
int main(void) {
    printf("Testing OpenMP array section pretty-printing coverage\n");
    
    test_map_clause();
    test_depend_clause();
    test_reduction_pattern();
    test_cast_base();
    test_nested_complex();
    
    printf("All OpenMP array section tests completed (compile-time coverage)\n");
    return 0;
}
