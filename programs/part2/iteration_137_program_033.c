/* Program to trigger GCC modulo scheduler debug logging */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Prevent vectorization and unrolling for this function */
__attribute__((optimize("no-tree-vectorize", "no-unroll-loops")))
int modulo_sched_test(int *a, int *b, int n_outer) {
    volatile int sum = 1;  /* Use volatile to prevent optimization */
    int i, j;
    
    /* Outer loop to provide multiple scheduling contexts */
    for (j = 0; j < n_outer; j++) {
        /* Inner loop with carried dependency - critical for modulo scheduling */
        /* Fixed small iteration count suitable for modulo scheduling */
        for (i = 0; i < 32; i++) {
            /* Complex operation with true data dependency across iterations */
            /* sum depends on previous iteration's sum value */
            sum = (sum * a[i] + b[i]) >> 1;
            
            /* Additional operations to create more scheduling opportunities */
            a[i] = a[i] + (sum & 0x1F);  /* Use low bits to prevent overflow */
            b[i] = b[i] - (sum & 0x0F);
        }
        
        /* Modify input slightly for outer loop variation */
        if (j % 2 == 0) {
            b[0] += sum;
        } else {
            a[0] -= sum;
        }
    }
    
    return sum;
}

/* Another test function with different pattern */
__attribute__((optimize("no-tree-vectorize", "no-unroll-loops")))
int modulo_sched_test2(int *arr, int n) {
    int sum = arr[0];
    int i;
    
    /* Loop with multiple carried dependencies */
    for (i = 1; i < 64; i++) {
        /* Multiple operations with dependencies */
        int temp = sum * 3;
        sum = temp + arr[i];
        arr[i] = (arr[i-1] + sum) & 0xFF;  /* Dependency on previous array element */
        sum = (sum >> 2) | (sum << 30);    /* Rotate operation */
    }
    
    return sum;
}

/* Test with floating point operations */
__attribute__((optimize("no-tree-vectorize", "no-unroll-loops")))
float modulo_sched_float(float *fa, float *fb, int n_outer) {
    volatile float fsum = 1.0f;
    int i, j;
    
    for (j = 0; j < n_outer; j++) {
        for (i = 0; i < 40; i++) {
            /* Floating point operations with dependencies */
            fsum = fsum * fa[i] + fb[i];
            fa[i] = fa[i] + fsum * 0.5f;
            fb[i] = fb[i] - fsum * 0.25f;
        }
        fa[0] += fsum;
    }
    
    return fsum;
}

int main() {
    int a[128], b[128];
    float fa[128], fb[128];
    int i, result1, result2;
    float fresult;
    
    /* Seed random number generator */
    srand(time(NULL));
    
    /* Initialize arrays with pseudo-random values */
    /* Using rand() prevents constant propagation */
    for (i = 0; i < 128; i++) {
        a[i] = rand() % 256;
        b[i] = rand() % 256;
        fa[i] = (float)(rand() % 100) / 10.0f;
        fb[i] = (float)(rand() % 100) / 10.0f;
    }
    
    printf("Starting modulo scheduling tests...\n");
    
    /* Test 1: Integer operations with carried dependency */
    result1 = modulo_sched_test(a, b, 10);
    printf("Test 1 result: %d\n", result1);
    
    /* Re-initialize for second test */
    for (i = 0; i < 128; i++) {
        a[i] = rand() % 256;
    }
    
    /* Test 2: Different integer pattern */
    result2 = modulo_sched_test2(a, 10);
    printf("Test 2 result: %d\n", result2);
    
    /* Test 3: Floating point operations */
    fresult = modulo_sched_float(fa, fb, 8);
    printf("Test 3 result: %f\n", fresult);
    
    /* Use results to prevent dead code elimination */
    return (result1 + result2 + (int)fresult) & 0xFF;
}
