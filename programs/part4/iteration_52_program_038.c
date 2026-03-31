/* test-omp-array-sections.c
 * 
 * This program is designed to trigger the OMP_ARRAY_SECTION pretty-printing
 * logic in GCC's tree-pretty-print.cc (lines 2736-2748).
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

struct Container {
    int arr[N];
    int* ptr_arr;
};

/* Function using array section with complex base expression */
void test_complex_base(struct Container *s, struct Container *p, int **ptr) {
    int flag = 1;
    int offset = 10;
    int count = 30;
    
    /* 1. Pointer dereference as base: (*ptr)[i:10] */
    #pragma omp target map((*ptr)[0:10])
    {
        for (int i = 0; i < 10; i++) 
            (*ptr)[i] = i;
    }
    
    /* 2. Structure member access: s.arr[1:5] */
    #pragma omp target data map(tofrom: s->arr[1:5])
    {
        #pragma omp target teams distribute parallel for map(to: s->arr[1:5])
        for (int i = 1; i < 6; i++)
            s->arr[i] *= 2;
    }
    
    /* 3. Function call returning pointer: get_array()[0:n] */
    #pragma omp task depend(inout: get_array()[0:15])
    {
        for (int i = 0; i < 15; i++)
            get_array()[i] = i * 2;
    }
    
    /* 4. Cast expression as base: (int *)buffer)[offset:count] */
    char buffer[1000];
    #pragma omp target map((int *)buffer)[offset:count]
    {
        int *int_buf = (int *)buffer;
        for (int i = offset; i < offset + count; i++)
            int_buf[i] = i - offset;
    }
    
    /* 5. Complex lower bound and length expressions */
    int start = 10, end = 90, len = 40;
    #pragma omp target teams distribute parallel for \
                map(to: s->arr[start+1:end-start-1])
    for (int i = start + 1; i < end; i++)
        s->arr[i] = i * 3;
    
    /* 6. Function calls in bounds: arr[compute_lower():compute_length()] */
    #pragma omp task depend(in: get_array()[compute_lower():compute_length()])
    {
        for (int i = compute_lower(); i < compute_lower() + compute_length(); i++)
            get_array()[i] += 5;
    }
    
    /* 7. Conditional (ternary) expression in lower bound */
    #pragma omp target data map(tofrom: p->arr[flag ? 0 : 10: len])
    {
        #pragma omp target teams distribute parallel for \
                    map(to: p->arr[flag ? 0 : 10: len])
        for (int i = (flag ? 0 : 10); i < (flag ? 0 : 10) + len; i++)
            p->arr[i] = i * 4;
    }
}

/* Function using array sections in reduction-like context */
void test_reduction_context(void) {
    int arr[N];
    int sum = 0;
    
    /* Initialize array */
    for (int i = 0; i < N; i++)
        arr[i] = i + 1;
    
    /* Array section in linear clause */
    #pragma omp parallel for linear(arr[0:1]:1)
    for (int i = 0; i < N; i++) {
        arr[i] += i;
    }
    
    /* Multiple array sections in map clauses */
    int a[N], b[N], c[N];
    #pragma omp target teams distribute parallel for \
                map(to: a[0:N], b[0:N]) map(from: c[0:N])
    for (int i = 0; i < N; i++) {
        c[i] = a[i] + b[i];
    }
}

/* Function with multiple depend clauses using array sections */
void test_task_dependencies(void) {
    int x[100], y[100];
    int size = 100;
    
    #pragma omp task depend(in: x[0:1]) depend(out: y[1:size-1])
    {
        y[1] = x[0] * 2;
        for (int i = 2; i < size; i++)
            y[i] = y[i-1] + 1;
    }
    
    #pragma omp task depend(inout: x[10:20])
    {
        for (int i = 10; i < 30; i++)
            x[i] = i * 3;
    }
    
    #pragma omp taskwait
}

/* Main driver function */
int main(void) {
    struct Container s1, s2;
    struct Container *p1 = &s1, *p2 = &s2;
    int *dynamic_arr = (int *)malloc(N * sizeof(int));
    int *ptr_arr = dynamic_arr;
    
    /* Initialize data */
    for (int i = 0; i < N; i++) {
        s1.arr[i] = i;
        s2.arr[i] = i * 2;
        dynamic_arr[i] = i * 3;
    }
    
    s1.ptr_arr = dynamic_arr;
    s2.ptr_arr = dynamic_arr;
    
    /* Call functions with various OpenMP array section patterns */
    test_complex_base(p1, p2, &ptr_arr);
    test_reduction_context();
    test_task_dependencies();
    
    /* Ensure arrays are used to prevent dead code elimination */
    int sum = 0;
    for (int i = 0; i < N; i++) {
        sum += s1.arr[i] + s2.arr[i] + dynamic_arr[i];
    }
    
    printf("Result check: %d\n", sum);
    
    free(dynamic_arr);
    return 0;
}
