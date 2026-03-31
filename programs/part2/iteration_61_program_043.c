/* test_modulo_sched.c
 * Designed to trigger GCC's modulo scheduler with distance-1 dependences
 * Compile with: gcc -O3 -fmodulo-sched -fdump-rtl-sms -c test_modulo_sched.c -mtune=itanium
 */

#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024

/* Prevent inlining to keep loop structure intact */
__attribute__((noinline))
double test1_fp_recurrence(double *arr, int n, double factor) {
    double sum = 0.0;
    /* Distance-1 recurrence: arr[i] depends on arr[i-1] */
    for (int i = 1; i < n; i++) {
        arr[i] = arr[i-1] * factor + (double)i * 0.5;
        sum += arr[i];
    }
    return sum;
}

__attribute__((noinline))
float test2_mixed_latency(float *fa, int *ia, int n, float fscale) {
    float total = 0.0f;
    /* Mixed operations with different latencies */
    for (int i = 1; i < n; i++) {
        /* Integer multiply (medium latency) */
        int imul = ia[i-1] * 3;
        /* Float multiply-add (higher latency) */
        fa[i] = fa[i-1] * fscale + (float)imul;
        /* Memory store then load (potential latency) */
        ia[i] = (int)fa[i] + i;
        total += fa[i];
    }
    return total;
}

__attribute__((noinline))
int test3_pointer_chase(int **ptr_arr, int *data, int n) {
    int sum = 0;
    /* Pointer-based recurrence with distance-1 */
    for (int i = 0; i < n-1; i++) {
        /* Load through pointer from previous iteration */
        int val = *ptr_arr[i];
        /* Store to next pointer location */
        data[i+1] = val + (i * 2);
        /* Update pointer for next iteration */
        ptr_arr[i+1] = &data[i+1];
        sum += val;
    }
    return sum;
}

__attribute__((noinline))
double test4_complex_chain(double *d1, double *d2, int n, double a, double b) {
    double acc = 0.0;
    /* Multiple operations with intra and inter-iteration dependences */
    for (int i = 1; i < n; i++) {
        /* Distance-1 dependence on d1 */
        double t1 = d1[i-1] * a + (double)i;
        /* Distance-1 dependence on d2 */
        double t2 = d2[i-1] * b - (double)i;
        /* Cross-dependence between t1 and t2 */
        d1[i] = t1 * t2 + d1[i];
        d2[i] = t1 - t2 * 0.5;
        /* Intra-iteration dependence */
        acc += d1[i] * d2[i];
    }
    return acc;
}

__attribute__((noinline))
int test5_multi_array(int *a, int *b, int *c, int n) {
    int sum = 0;
    /* Multiple arrays with loop-carried dependences */
    for (int i = 2; i < n; i++) {
        /* Distance-1 on array a */
        a[i] = a[i-1] + b[i-2];
        /* Distance-1 on array b */
        b[i] = b[i-1] * 2 - c[i-1];
        /* Distance-1 on array c */
        c[i] = a[i-1] + c[i-2] * 3;
        sum += a[i] + b[i] - c[i];
    }
    return sum;
}

int main(int argc, char *argv[]) {
    /* Use volatile to prevent constant propagation */
    volatile int size = (argc > 1) ? atoi(argv[1]) : SIZE;
    if (size < 10) size = SIZE;
    
    /* Allocate and initialize arrays */
    double *darr1 = (double*)malloc(size * sizeof(double));
    double *darr2 = (double*)malloc(size * sizeof(double));
    float *farr = (float*)malloc(size * sizeof(float));
    int *iarr1 = (int*)malloc(size * sizeof(int));
    int *iarr2 = (int*)malloc(size * sizeof(int));
    int *iarr3 = (int*)malloc(size * sizeof(int));
    int **ptr_arr = (int**)malloc(size * sizeof(int*));
    
    /* Initialize with non-zero values */
    for (int i = 0; i < size; i++) {
        darr1[i] = (double)(i % 100) * 0.1;
        darr2[i] = (double)(i % 50) * 0.2;
        farr[i] = (float)(i % 75) * 0.3f;
        iarr1[i] = i % 25;
        iarr2[i] = i % 30;
        iarr3[i] = i % 20;
        ptr_arr[i] = &iarr1[i];
    }
    
    double total = 0.0;
    
    /* Call test functions to trigger modulo scheduling */
    total += test1_fp_recurrence(darr1, size, 1.01);
    total += (double)test2_mixed_latency(farr, iarr1, size, 1.5f);
    total += (double)test3_pointer_chase(ptr_arr, iarr2, size);
    total += test4_complex_chain(darr1, darr2, size, 0.9, 1.1);
    total += (double)test5_multi_array(iarr1, iarr2, iarr3, size);
    
    printf("Total result: %f\n", total);
    
    /* Cleanup */
    free(darr1);
    free(darr2);
    free(farr);
    free(iarr1);
    free(iarr2);
    free(iarr3);
    free(ptr_arr);
    
    return 0;
}
