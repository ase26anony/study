/* test_ddg_coverage.c
 * Designed to trigger DDG edge creation in GCC's scheduler
 * Compile with: gcc -O2 -funroll-loops -fmodulo-sched -c test_ddg_coverage.c
 * Or: gcc -O3 -fmodulo-sched -fmodulo-sched-allow-regmoves -ftree-vectorize -c test_ddg_coverage.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Global variables to prevent optimization */
volatile int g_volatile = 0;
int g_array[1024];
int g_result = 0;

/* Function to prevent dead code elimination */
static int use_value(int x) {
    g_volatile = x;
    return x;
}

/* 1. Loop with true data dependencies (RAW) and loop-carried dependencies */
int test_raw_dep(int *a, int *b, int n) {
    int sum = 0;
    /* Multiple RAW dependencies with different distances */
    for (int i = 2; i < n; i++) {
        a[i] = a[i-1] + b[i];          /* Distance 1 RAW */
        a[i] += a[i-2] * 2;            /* Distance 2 RAW */
        sum += a[i];
    }
    /* Additional RAW with floating point */
    float *fa = (float*)a;
    for (int i = 3; i < n; i++) {
        fa[i] = fa[i-1] + fa[i-3] * 1.5f;  /* Mixed distances, float type */
    }
    return sum + (int)fa[n-1];
}

/* 2. Loop with anti (WAR) and output (WAW) dependencies */
int test_war_waw_dep(int *a, int *b, int *c, int n) {
    int temp = 0;
    for (int i = 0; i < n; i++) {
        int t1 = a[i] + b[i];      /* Read a[i] */
        a[i] = t1 * 2;             /* Write a[i] - WAR with above read */
        
        c[i] = temp + i;           /* Write c[i] */
        c[i] = c[i] * 3;           /* Write c[i] again - WAW */
        
        temp = a[i] - c[i];        /* Use both modified values */
    }
    return temp;
}

/* 3. Loop with memory aliasing via pointers */
int test_memory_aliasing(int *arr, int n) {
    int *p = arr;
    int *q = arr + (n/2);
    int sum = 0;
    
    /* Create ambiguous aliasing - compiler can't tell if p and q alias */
    for (int i = 0; i < n/2; i++) {
        *p = *q + i;        /* Read from q, write to p */
        *q = *p * 2;        /* Read from p (may alias), write to q */
        sum += *p + *q;
        p++;
        q++;
    }
    
    /* Additional aliasing with different offsets */
    int *r = arr;
    int *s = arr + 1;
    for (int i = 0; i < n-1; i++) {
        *r = *s + g_volatile;  /* Volatile adds memory barrier */
        *s = *r - i;
        r++;
        s++;
    }
    return sum;
}

/* 4. Loop with control dependencies and function calls */
int test_control_dep(int *a, int *b, int n) {
    int count = 0;
    for (int i = 0; i < n; i++) {
        /* Control-dependent operations */
        if (a[i] > 0) {
            b[i] = a[i] * 2;
            count++;
        } else if (a[i] < -10) {
            b[i] = a[i] / 2;
            count--;
        } else {
            b[i] = 0;
        }
        
        /* Function call acts as memory clobber */
        if (i % 3 == 0) {
            b[i] += use_value(i);
        }
    }
    return count;
}

/* 5. Complex nested loop with mixed dependencies */
int test_nested_mixed(int *a, int *b, int n, int m) {
    int total = 0;
    for (int i = 1; i < n; i++) {
        int acc = a[i-1];  /* Loop-carried RAW */
        for (int j = 0; j < m; j++) {
            /* Multiple dependency types */
            b[j] = acc + j;            /* RAW from acc */
            acc = b[j] * 2;            /* WAR on b[j], WAW on acc */
            a[i] += acc;               /* RAW from acc */
            
            /* Memory dependency with potential aliasing */
            if (j % 2 == 0) {
                *(a + i) = *(b + j) + 1;  /* May alias with a[i] */
            }
        }
        total += acc;
    }
    return total;
}

/* 6. Loop with volatile accesses creating hard dependencies */
int test_volatile_dep(volatile int *v, int *arr, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        int val = *v;           /* Volatile read - creates memory dependency */
        arr[i] = val + i;
        *v = arr[i];            /* Volatile write - creates output dependency */
        sum += arr[i];
    }
    return sum;
}

/* Main function that runs all tests */
int main(int argc, char **argv) {
    int n = 1000;
    int m = 100;
    
    /* Initialize data */
    int *a = (int*)malloc(n * sizeof(int));
    int *b = (int*)malloc(n * sizeof(int));
    int *c = (int*)malloc(n * sizeof(int));
    volatile int *v = (volatile int*)malloc(sizeof(int));
    
    for (int i = 0; i < n; i++) {
        a[i] = (i % 50) - 25;  /* Mix of positive and negative values */
        b[i] = i * 2;
        c[i] = i * 3;
    }
    *v = 42;
    
    /* Run all test functions to create various DDG edges */
    int result = 0;
    
    result += test_raw_dep(a, b, n);
    result += test_war_waw_dep(a, b, c, n);
    result += test_memory_aliasing(a, n);
    result += test_control_dep(a, b, n);
    result += test_nested_mixed(a, b, n/10, m);
    result += test_volatile_dep(v, c, n);
    
    /* Use results to prevent dead code elimination */
    g_result = result;
    printf("Result: %d\n", result);
    
    /* Store to global array to create memory dependencies */
    for (int i = 0; i < n; i++) {
        g_array[i % 1024] = a[i] + b[i] + c[i];
    }
    
    free(a);
    free(b);
    free(c);
    free((void*)v);
    
    return g_result != 0 ? 0 : 1;
}
