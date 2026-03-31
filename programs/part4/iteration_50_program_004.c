/* test_ddg_coverage.c
 * Designed to trigger DDG edge creation in GCC's instruction scheduler
 * Compile with: gcc -O2 -fmodulo-sched -funroll-loops -c test_ddg_coverage.c
 * Or: gcc -O3 -fmodulo-sched -fmodulo-sched-allow-regmoves -ftree-vectorize -c test_ddg_coverage.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Global variables to prevent optimization */
volatile int g_volatile = 0;
int g_array[1024];
int g_result = 0;
float g_float_array[1024];

/* Function to create true data dependencies (RAW) with loop-carried dependencies */
int test_raw_dep(int *a, int *b, int n) {
    int sum = 0;
    /* Loop with multiple RAW dependencies and varying distances */
    for (int i = 2; i < n; i++) {
        /* Distance 1 RAW dependency */
        a[i] = a[i-1] + b[i];
        
        /* Distance 2 RAW dependency */
        b[i] = b[i-2] * 3;
        
        /* Mixed integer/float operations */
        g_float_array[i] = g_float_array[i-1] * 1.5f + a[i];
        
        /* Complex expression to prevent simplification */
        sum += a[i] + b[i] + (int)g_float_array[i];
    }
    return sum + g_volatile;
}

/* Function to create anti (WAR) and output (WAW) dependencies */
int test_war_waw_dep(int *arr, int n) {
    int temp = 0;
    
    /* Loop with WAR and WAW dependencies */
    for (int i = 1; i < n; i++) {
        int x = arr[i];          /* Read arr[i] */
        arr[i] = temp + i;       /* Write arr[i] - WAR with above read */
        temp = x;                /* Use x */
        
        /* WAW dependency chain */
        g_array[i] = i * 2;      /* First write */
        g_array[i] = i * 3 + 1;  /* Second write - WAW with first */
        
        /* Another WAR example */
        float f1 = g_float_array[i];
        g_float_array[i] = f1 * 2.0f;
        float f2 = g_float_array[i];  /* WAR through g_float_array */
        
        temp += (int)f2;
    }
    
    return temp;
}

/* Function with memory aliasing through pointers */
int test_memory_aliasing(int *data, int n) {
    int *p = data;
    int *q = data + (n/2);
    int sum = 0;
    
    /* Loop with potential pointer aliasing */
    for (int i = 0; i < n/2; i++) {
        /* These may alias if data overlaps */
        *p = *q + i;
        *q = *p * 2;
        
        /* Additional memory operations with different offsets */
        data[i] = data[n-i-1] + 1;  /* May alias */
        
        /* Volatile access creates hard memory dependency */
        sum += g_volatile;
        
        p++;
        q--;
    }
    
    return sum;
}

/* Function with control dependencies and branching */
int test_control_dep(int *arr, int n) {
    int count = 0;
    
    /* Loop with internal control flow */
    for (int i = 0; i < n; i++) {
        /* Control-dependent operations */
        if (arr[i] > 0) {
            g_array[i] = arr[i] * 2;
            count++;
        } else {
            g_array[i] = -arr[i];
            if (i % 3 == 0) {
                /* Nested condition */
                g_float_array[i] = (float)arr[i] / 2.0f;
            }
        }
        
        /* Another condition with different dependency pattern */
        int val = (i % 2 == 0) ? arr[i] + 1 : arr[i] - 1;
        arr[i] = val + g_volatile;
    }
    
    return count;
}

/* Complex nested loop with mixed dependencies */
int test_nested_loops(int *a, int *b, int n) {
    int total = 0;
    
    /* Outer loop */
    for (int i = 1; i < n; i++) {
        int inner_sum = 0;
        
        /* Inner loop with its own dependencies */
        for (int j = 0; j < 8; j++) {
            /* RAW dependency across inner iterations */
            b[j] = b[j] + a[i];
            
            /* WAR in inner loop */
            int temp = a[i];
            a[i] = j * 2;
            inner_sum += temp;
            
            /* Memory operation */
            g_array[j] = g_array[j] + 1;
        }
        
        /* Loop-carried dependency in outer loop */
        a[i] = a[i-1] + inner_sum;
        total += a[i];
    }
    
    return total;
}

/* Function with function calls that act as memory barriers */
extern int external_func(int x);  /* Assume this is defined elsewhere */

int test_with_calls(int *arr, int n) {
    int result = 0;
    
    for (int i = 0; i < n; i++) {
        /* Function call creates memory clobbering dependency */
        arr[i] = external_func(arr[i]);
        
        /* Dependency through global memory */
        g_result = arr[i] + g_volatile;
        
        /* Another operation dependent on the function call result */
        result += arr[i] * 2;
        
        /* Volatile operations enforce ordering */
        g_volatile = i;
    }
    
    return result;
}

/* Main function that runs all tests */
int main(int argc, char **argv) {
    int n = 1000;
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n < 10) n = 1000;
    }
    
    /* Allocate and initialize arrays */
    int *data1 = (int*)malloc(n * sizeof(int));
    int *data2 = (int*)malloc(n * sizeof(int));
    
    for (int i = 0; i < n; i++) {
        data1[i] = (i * 3) % 97;
        data2[i] = (i * 7) % 113;
        g_array[i] = i;
        g_float_array[i] = (float)i * 0.5f;
    }
    
    /* Run all test functions to create various DDG edge types */
    int checksum = 0;
    
    checksum += test_raw_dep(data1, data2, n);
    checksum += test_war_waw_dep(data1, n);
    checksum += test_memory_aliasing(data2, n);
    checksum += test_control_dep(data1, n);
    checksum += test_nested_loops(data1, data2, n/2);
    
    /* Simulate external function */
    for (int i = 0; i < n; i++) {
        data2[i] = data2[i] % 100;
    }
    checksum += test_with_calls(data2, n/4);
    
    /* Final computation to use results */
    int final_result = 0;
    for (int i = 0; i < n; i++) {
        final_result += data1[i] + data2[i] + g_array[i];
    }
    final_result += checksum;
    
    printf("Result: %d\n", final_result % 1000000);
    
    free(data1);
    free(data2);
    
    return final_result > 0 ? 0 : 1;
}

/* Dummy external function definition */
int external_func(int x) {
    return (x * 13) % 79;
}
