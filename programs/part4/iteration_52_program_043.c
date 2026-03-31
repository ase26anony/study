/* test-omp-array-sections.c
 * 
 * This test program is designed to exercise the OMP_ARRAY_SECTION
 * pretty-printing logic in GCC's tree-pretty-print.cc.
 * It contains various OpenMP constructs with array sections that
 * have complex base expressions, varied bounds, and appear in
 * different OpenMP clauses.
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
    static int arr[N];
    return arr;
}

int compute_lower(void) { return 0; }
int compute_length(void) { return N/2; }

struct Container {
    int arr[N];
    int *ptr;
};

/* Function using array section with pointer dereference as base */
void test_pointer_deref_base(void) {
    int (*ptr)[N];
    int buffer[N];
    ptr = &buffer;
    
    /* Array section with pointer dereference as base - may need parentheses */
    #pragma omp target map((*ptr)[0:N])
    {
        for (int i = 0; i < N; i++)
            (*ptr)[i] = i;
    }
}

/* Function using structure member access as base */
void test_struct_member_base(void) {
    struct Container s;
    struct Container *p = &s;
    
    /* Array section with direct member access */
    #pragma omp target data map(tofrom: s.arr[0:N])
    {
        #pragma omp target teams distribute parallel for map(to: s.arr[0:N])
        for (int i = 0; i < N; i++)
            s.arr[i] = i * 2;
    }
    
    /* Array section with pointer member access */
    #pragma omp task depend(inout: p->arr[10:N-10])
    {
        for (int i = 10; i < N-10; i++)
            p->arr[i] += 1;
    }
}

/* Function using function call as base */
void test_function_call_base(void) {
    /* Array section with function call as base */
    #pragma omp target map(get_array()[0:N])
    {
        int *arr = get_array();
        for (int i = 0; i < N; i++)
            arr[i] = i * 3;
    }
}

/* Function using cast expression as base */
void test_cast_base(void) {
    char buffer[N * sizeof(int)];
    
    /* Array section with cast expression as base */
    #pragma omp target map(((int *)buffer)[0:N])
    {
        int *arr = (int *)buffer;
        for (int i = 0; i < N; i++)
            arr[i] = i * 4;
    }
}

/* Function with varied lower bound and length expressions */
void test_varied_bounds(void) {
    int arr[N];
    int start = 10, end = 90;
    int i = 5, j = 20;
    int flag = 1;
    
    /* Simple constants */
    #pragma omp target data map(tofrom: arr[0:100])
    {}
    
    /* Variable expressions */
    #pragma omp task depend(in: arr[i:j])
    {}
    
    /* Arithmetic expressions */
    #pragma omp target teams distribute parallel for \
        map(to: arr[start+1:end-start-1])
    for (int k = start+1; k < end; k++)
        arr[k] = k;
    
    /* Function calls in bounds */
    #pragma omp target map(arr[compute_lower():compute_length()])
    {
        for (int k = compute_lower(); k < compute_length(); k++)
            arr[k] = k * 2;
    }
    
    /* Conditional (ternary) expression in lower bound */
    #pragma omp task depend(out: arr[flag ? 0 : 10: N/2])
    {
        int lower = flag ? 0 : 10;
        for (int k = lower; k < lower + N/2; k++)
            arr[k] = k * 3;
    }
}

/* Function with array section in reduction clause (requires custom reduction) */
#pragma omp declare reduction(array_plus : int [SIZE] : \
    for (int i = 0; i < SIZE; i++) \
        omp_out[i] += omp_in[i]) \
    initializer(omp_priv = omp_orig)

void test_reduction_with_section(void) {
    int arr[SIZE];
    for (int i = 0; i < SIZE; i++)
        arr[i] = i;
    
    /* Array section in reduction clause */
    #pragma omp parallel for reduction(array_plus: arr[0:SIZE])
    for (int i = 0; i < SIZE; i++)
        arr[i] += 1;
}

/* Function with array section in linear clause */
void test_linear_clause(void) {
    int arr[N];
    int idx = 0;
    
    /* Array section in linear clause */
    #pragma omp parallel for linear(arr[idx:1]:1)
    for (int i = 0; i < N; i++) {
        arr[idx] = i;
        idx++;
    }
}

/* Function with multiple array sections in complex expressions */
void test_complex_combined(void) {
    struct Container containers[5];
    int *ptr_array[5];
    
    for (int i = 0; i < 5; i++) {
        ptr_array[i] = containers[i].arr;
    }
    
    /* Complex base: pointer array dereference */
    #pragma omp target teams distribute parallel for \
        map(to: ptr_array[2][0:N], containers[3].arr[10:N-10]) \
        map(from: containers[4].arr[0:N])
    for (int i = 0; i < N; i++) {
        if (i < N) ptr_array[2][i] = i;
        if (i >= 10 && i < N-10) containers[3].arr[i] = i * 2;
        containers[4].arr[i] = i * 3;
    }
    
    /* Nested complex base expression */
    int (**pptr)[N];
    int (*array_ptr)[N];
    int buffer2[N];
    array_ptr = &buffer2;
    pptr = &array_ptr;
    
    #pragma omp task depend(inout: (**pptr)[5:20])
    {
        for (int i = 5; i < 25; i++)
            (**pptr)[i] += 1;
    }
}

/* Main driver function */
int main(void) {
    /* Call all test functions to ensure code generation */
    test_pointer_deref_base();
    test_struct_member_base();
    test_function_call_base();
    test_cast_base();
    test_varied_bounds();
    test_reduction_with_section();
    test_linear_clause();
    test_complex_combined();
    
    printf("All OpenMP array section tests completed (compile with -fopenmp -fdump-tree-* for coverage)\n");
    return 0;
}
