/* This program is designed to trigger the pretty-printing logic for
   OMP_ARRAY_SECTION nodes in GCC's tree-pretty-print.cc, specifically
   lines 2736-2748, which handle the formatting of OpenMP array sections
   with proper parentheses for complex base expressions. */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Helper function to introduce volatility and prevent constant folding */
static int get_value(int x, int def) {
    volatile int v = x;
    return v > 0 ? v : def;
}

/* Function that uses array sections in multiple OpenMP contexts */
void process_sections(int n, int start, int len, int offset, int cond) {
    int arr1[100], arr2[100];
    int *ptr = arr1;
    
    /* Initialize arrays */
    for (int i = 0; i < 100; i++) {
        arr1[i] = i;
        arr2[i] = 100 - i;
    }
    
    /* STRATEGY 1: Complex base expressions with operator precedence */
    /* Base expression with conditional operator (lower precedence than []) */
    #pragma omp target data map(to: (cond ? arr1 : arr2)[start:len])
    {
        /* Inside target data region - launch a kernel */
        #pragma omp target map(tofrom: (cond ? arr1 : arr2)[start:len])
        {
            int *base = cond ? arr1 : arr2;
            for (int i = 0; i < len; i++) {
                base[start + i] += 1;
            }
        }
    }
    
    /* STRATEGY 2: Pointer arithmetic as base expression */
    #pragma omp target enter data map(to: (ptr + offset)[0:n])
    #pragma omp target map(tofrom: (ptr + offset)[0:n])
    {
        for (int i = 0; i < n; i++) {
            ptr[offset + i] *= 2;
        }
    }
    #pragma omp target exit data map(from: (ptr + offset)[0:n])
    
    /* STRATEGY 3: Multiple different OpenMP clauses */
    int arr3[200];
    for (int i = 0; i < 200; i++) arr3[i] = i;
    
    /* Use array section in 'to' and 'from' clauses separately */
    #pragma omp target data map(to: arr3[start:len]) map(from: arr3[start+50:len])
    {
        #pragma omp target
        {
            for (int i = 0; i < len; i++) {
                arr3[start + i] = arr3[start + 50 + i] + 1;
            }
        }
    }
    
    /* STRATEGY 4: Type mixing to potentially trigger diagnostics */
    /* This may cause warnings about array section usage */
    float farr[50];
    for (int i = 0; i < 50; i++) farr[i] = i * 1.5f;
    
    /* Array section with float type */
    #pragma omp target data map(tofrom: farr[10:20])
    {
        #pragma omp target
        {
            for (int i = 0; i < 20; i++) {
                farr[10 + i] += 0.5f;
            }
        }
    }
}

/* Function using array sections in task depend clauses */
void task_with_sections(int *arr, int n, int start1, int len1, int start2, int len2) {
    #pragma omp task depend(inout: arr[start1:len1])
    {
        for (int i = 0; i < len1; i++) {
            arr[start1 + i] += 1;
        }
    }
    
    #pragma omp task depend(in: arr[start1:len1]) depend(out: arr[start2:len2])
    {
        for (int i = 0; i < len2; i++) {
            arr[start2 + i] = arr[start1 + i % len1];
        }
    }
}

int main(int argc, char *argv[]) {
    /* Use command-line arguments to prevent constant propagation */
    int n = argc > 1 ? atoi(argv[1]) : 50;
    int start = argc > 2 ? atoi(argv[2]) : 10;
    int len = argc > 3 ? atoi(argv[3]) : 20;
    int offset = argc > 4 ? atoi(argv[4]) : 5;
    int cond = argc > 5 ? atoi(argv[5]) : 1;
    
    /* Get volatile values to prevent optimization */
    n = get_value(n, 50);
    start = get_value(start, 10);
    len = get_value(len, 20);
    offset = get_value(offset, 5);
    cond = get_value(cond, 1);
    
    /* Ensure bounds are valid */
    if (start < 0) start = 0;
    if (len < 0) len = 10;
    if (offset < 0) offset = 0;
    if (n < 0) n = 10;
    
    printf("Processing with n=%d, start=%d, len=%d, offset=%d, cond=%d\n",
           n, start, len, offset, cond);
    
    /* Process with complex array section expressions */
    process_sections(n, start, len, offset, cond);
    
    /* Additional test with task dependencies */
    int task_arr[100];
    for (int i = 0; i < 100; i++) task_arr[i] = i;
    
    #pragma omp parallel
    #pragma omp single
    {
        task_with_sections(task_arr, 100, start, len, start + 30, len);
        #pragma omp taskwait
    }
    
    /* Compute checksum to prevent dead code elimination */
    int sum = 0;
    for (int i = 0; i < 100; i++) {
        sum += task_arr[i];
    }
    printf("Checksum: %d\n", sum);
    
    return 0;
}
