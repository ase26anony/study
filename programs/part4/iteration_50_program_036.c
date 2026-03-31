/* test_ddg_coverage.c
 * Program designed to trigger DDG edge creation in GCC's scheduler
 * Compile with: gcc -O2 -funroll-loops -fmodulo-sched -c test_ddg_coverage.c
 * Or: gcc -O3 -fmodulo-sched -fmodulo-sched-allow-regmoves -ftree-vectorize -c test_ddg_coverage.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Global variables to prevent optimization */
volatile int g_volatile = 0;
int g_array[1024];
int g_result = 0;

/* Function to create true data dependencies (RAW) */
int test_raw_dep(int *a, int *b, int n) {
    int sum = 0;
    /* Loop with flow dependencies across iterations */
    for (int i = 2; i < n; i++) {
        a[i] = a[i-1] + b[i];          /* RAW: a[i-1] read, then a[i] written */
        a[i] += a[i-2] + 1;            /* Additional RAW with distance 2 */
        sum += a[i];
    }
    return sum;
}

/* Function to create anti and output dependencies (WAR/WAW) */
float test_war_waw_dep(float *fa, float *fb, int n) {
    float temp = 0.0f;
    /* Mix of WAR and WAW dependencies */
    for (int i = 0; i < n; i++) {
        float x = fa[i] + fb[i];       /* Read fa[i], fb[i] */
        fa[i] = x * 2.0f;              /* WAR: fa[i] written after being read above */
        fa[i] = fa[i] + 1.0f;          /* WAW: fa[i] written again */
        temp += fa[i];
        
        /* Additional WAW with different data type */
        int *p = (int*)&fa[i];
        *p = *p + 1;                   /* WAW through pointer aliasing */
    }
    return temp;
}

/* Function with memory aliasing dependencies */
int test_memory_aliasing(int *arr1, int *arr2, int n) {
    int sum = 0;
    int *p = arr1;
    int *q = arr2;
    
    /* Loop with potential aliasing */
    for (int i = 0; i < n; i++) {
        *p = *q + g_volatile;          /* Memory dep: p and q may alias */
        sum += *p;
        p = &arr1[(i + 1) % n];        /* Change pointer */
        q = &arr2[(i + 2) % n];        /* Different stride for aliasing analysis */
        
        /* Additional memory operations */
        arr1[i % 8] = arr2[i % 8] + i; /* Fixed small indices may alias */
    }
    return sum;
}

/* Function with control dependencies */
double test_control_dep(double *da, double *db, int n) {
    double result = 0.0;
    
    /* Loop with internal branching */
    for (int i = 0; i < n; i++) {
        double val;
        if (i % 3 == 0) {
            val = da[i] * db[i];       /* Control-dependent computation */
        } else if (i % 3 == 1) {
            val = da[i] / (db[i] + 1.0);
        } else {
            val = da[i] - db[i];
        }
        
        /* Nested condition for more complex control flow */
        if (val > 0.0) {
            da[i] = val + 1.0;
        } else {
            da[i] = val - 1.0;
        }
        
        result += da[i];
        
        /* Volatile access creates hard dependency */
        da[i] += (double)g_volatile;
    }
    return result;
}

/* Complex loop with mixed dependencies */
int test_mixed_dependencies(int *a, int *b, int *c, int n) {
    int total = 0;
    
    /* Outer loop with carried dependencies */
    for (int i = 1; i < n; i++) {
        int acc = 0;
        
        /* Inner loop with various deps */
        for (int j = 0; j < 16; j++) {
            /* RAW with loop-carried dependency */
            a[j] = a[(j + 15) % 16] + b[j];
            
            /* WAR dependency */
            int tmp = b[j];
            b[j] = c[j] + i;
            acc += tmp;                /* Uses old b[j] value */
            
            /* WAW dependency */
            c[j] = i * j;
            c[j] = c[j] + acc;         /* Overwrite c[j] */
            
            /* Memory dependency through global */
            acc += g_array[j % 1024];
        }
        
        /* Cross-iteration dependency */
        a[0] = a[15] + i;
        total += acc;
    }
    
    return total;
}

/* Function with function calls that act as memory barriers */
int test_with_function_calls(int *arr, int n) {
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Function call creates memory clobber */
        sum += arr[i];
        
        /* Inline asm acts as optimization barrier */
        __asm__ volatile ("" : : "r"(arr[i]) : "memory");
        
        /* Complex expression with multiple dependencies */
        arr[i] = (arr[i] * 3 + sum) / 2;
        
        /* Dependency through volatile */
        arr[i] += g_volatile;
    }
    
    return sum;
}

/* Main function that runs all tests */
int main(int argc, char **argv) {
    int n = 1000;
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n < 100) n = 100;
        if (n > 10000) n = 10000;
    }
    
    /* Initialize data */
    int *a = (int*)malloc(n * sizeof(int));
    int *b = (int*)malloc(n * sizeof(int));
    int *c = (int*)malloc(n * sizeof(int));
    float *fa = (float*)malloc(n * sizeof(float));
    float *fb = (float*)malloc(n * sizeof(float));
    double *da = (double*)malloc(n * sizeof(double));
    double *db = (double*)malloc(n * sizeof(double));
    
    srand(time(NULL));
    for (int i = 0; i < n; i++) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
        c[i] = rand() % 100;
        fa[i] = (float)(rand() % 100) / 10.0f;
        fb[i] = (float)(rand() % 100) / 10.0f;
        da[i] = (double)(rand() % 100) / 10.0;
        db[i] = (double)(rand() % 100) / 10.0;
        if (i < 1024) g_array[i] = rand() % 100;
    }
    
    g_volatile = rand() % 100;
    
    /* Run all test functions to create various DDG edges */
    int result = 0;
    result += test_raw_dep(a, b, n);
    
    float fresult = test_war_waw_dep(fa, fb, n);
    result += (int)fresult;
    
    result += test_memory_aliasing(a, b, n);
    
    double dresult = test_control_dep(da, db, n);
    result += (int)dresult;
    
    result += test_mixed_dependencies(a, b, c, n / 4);
    
    result += test_with_function_calls(a, n);
    
    /* Use result to prevent dead code elimination */
    printf("Final checksum: %d\n", result);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(fa);
    free(fb);
    free(da);
    free(db);
    
    return result != 0 ? 0 : 1;
}
