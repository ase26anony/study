/* test_ddg_coverage.c
 * Designed to trigger DDG edge creation in GCC's instruction scheduler
 * Compile with: gcc -O2 -fmodulo-sched -funroll-loops -c test_ddg_coverage.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Global variables to prevent optimization */
volatile int g_volatile = 0;
int g_global_array[1024];
int g_result = 0;

/* Function to create true dependencies (RAW) */
int test_raw_dep(int *a, int *b, int n) {
    int sum = 0;
    /* Loop-carried true dependency with distance 1 */
    for (int i = 1; i < n; i++) {
        a[i] = a[i-1] + b[i];  /* RAW: a[i-1] read, then a[i] written */
        sum += a[i];
    }
    
    /* Additional RAW dependency with different distance */
    for (int i = 2; i < n; i++) {
        b[i] = a[i-2] * 2;  /* Distance 2 dependency */
        sum += b[i];
    }
    
    /* Mixed integer/float operations for different data types */
    float *fa = (float*)a;
    float *fb = (float*)b;
    for (int i = 1; i < n/2; i++) {
        fa[i] = fa[i-1] + fb[i] * 0.5f;  /* Floating-point RAW */
        sum += (int)fa[i];
    }
    
    return sum + g_volatile;  /* Prevent dead code elimination */
}

/* Function to create anti and output dependencies (WAR/WAW) */
int test_war_waw_dep(int *a, int *b, int n) {
    int sum = 0;
    
    /* WAR (anti-dependency) pattern */
    for (int i = 0; i < n; i++) {
        int temp = a[i];      /* Read a[i] */
        a[i] = b[i] + 1;      /* Write a[i] - creates WAR with previous read */
        b[i] = temp * 2;      /* Write b[i] */
        sum += a[i] + b[i];
    }
    
    /* WAW (output-dependency) pattern */
    for (int i = 0; i < n; i++) {
        a[i] = i * 3;         /* First write to a[i] */
        a[i] = a[i] + b[i];   /* Second write to a[i] - WAW dependency */
        sum += a[i];
    }
    
    /* Complex WAR/WAW mixture */
    for (int i = 1; i < n-1; i++) {
        int x = a[i];         /* Read a[i] */
        a[i] = x + i;         /* Write a[i] - WAR */
        int y = a[i+1];       /* Read a[i+1] */
        a[i] = y * 2;         /* Write a[i] again - WAW with previous write */
        sum += a[i];
    }
    
    return sum + g_volatile;
}

/* Function to create memory aliasing dependencies */
int test_memory_aliasing(int *arr, int n) {
    int sum = 0;
    
    /* Pointer aliasing with unknown relationship */
    int *p = arr;
    int *q = arr + n/2;
    
    for (int i = 0; i < n/2; i++) {
        *p = *q + i;          /* May alias with q */
        p++;
        *q = *p * 2;          /* May alias with p */
        q--;
        sum += *p + *q;
    }
    
    /* Array indexing with potential overlap */
    for (int i = 0; i < n-10; i++) {
        arr[i] = arr[i+5] + arr[i+10];  /* Potential memory dependencies */
        sum += arr[i];
    }
    
    /* Volatile memory access creates hard dependencies */
    volatile int *volatile_ptr = &g_global_array[0];
    for (int i = 0; i < n && i < 100; i++) {
        *volatile_ptr = i;
        volatile_ptr++;
        sum += g_global_array[i];
    }
    
    return sum + g_volatile;
}

/* Function to create control dependencies */
int test_control_dep(int *a, int *b, int n) {
    int sum = 0;
    
    /* Loop with internal branching */
    for (int i = 0; i < n; i++) {
        if (a[i] > 0) {           /* Control dependency */
            b[i] = a[i] * 2;
            sum += b[i];
        } else {
            b[i] = -a[i];
            sum -= b[i];
        }
        
        /* Nested control flow */
        switch (i % 4) {
            case 0: a[i] = sum; break;
            case 1: a[i] = sum * 2; break;
            case 2: a[i] = sum / 2; break;
            default: a[i] = 0; break;
        }
    }
    
    /* Loop with function call (acts as memory clobber) */
    for (int i = 0; i < n; i++) {
        /* External function call creates memory barrier */
        sum += a[i] + b[i];
        /* Simulate function call effect */
        g_global_array[i % 100] = sum;
    }
    
    return sum + g_volatile;
}

/* Complex nested loop structure */
int test_nested_loops(int *a, int *b, int n, int m) {
    int sum = 0;
    
    /* Outer loop with carried dependency */
    for (int i = 1; i < n; i++) {
        /* Inner loop with multiple dependency types */
        for (int j = 1; j < m; j++) {
            /* RAW with loop-carried dependency in inner loop */
            a[j] = a[j-1] + b[j];
            
            /* WAR in inner loop */
            int temp = b[j];
            b[j] = a[j] * 2;
            a[j] = temp + i;
            
            /* Memory dependency with outer loop index */
            g_global_array[j] = a[j] + i;
            
            sum += a[j] + b[j] + g_global_array[j];
        }
        
        /* Control dependency in outer loop */
        if (i % 2 == 0) {
            for (int j = 0; j < m; j++) {
                b[j] = a[j] + g_global_array[j];
                sum += b[j];
            }
        }
    }
    
    return sum + g_volatile;
}

/* Main function that exercises all patterns */
int main(int argc, char **argv) {
    int n = 1000;
    int m = 100;
    
    /* Dynamically allocate to prevent compile-time optimization */
    int *array1 = (int*)malloc(n * sizeof(int));
    int *array2 = (int*)malloc(n * sizeof(int));
    
    if (!array1 || !array2) {
        return 1;
    }
    
    /* Initialize with non-constant values */
    for (int i = 0; i < n; i++) {
        array1[i] = (i * 3) % 97;
        array2[i] = (i * 7) % 113;
        g_global_array[i % 1024] = i;
    }
    
    /* Force runtime value */
    g_volatile = argc;
    
    /* Execute all test patterns */
    int result = 0;
    
    result += test_raw_dep(array1, array2, n);
    result += test_war_waw_dep(array1, array2, n);
    result += test_memory_aliasing(array1, n);
    result += test_control_dep(array1, array2, n);
    result += test_nested_loops(array1, array2, n/10, m);
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    free(array1);
    free(array2);
    
    return result != 0 ? 0 : 1;
}
