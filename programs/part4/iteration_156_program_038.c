/* test_modulo_sched.c - Test program for GCC modulo scheduler coverage */

#include <stdio.h>
#include <stdlib.h>

#define SIZE 256
#define ITERS 100

/* Global volatile sink to prevent dead code elimination */
volatile int sink = 0;

/* Function 1: Loop with carried dependency and mixed operations */
void loop1_carried_dep(int *arr, int n, int scalar) {
    volatile int local_sink = 0;
    
    /* Loop with carried dependency (distance 1) */
    for (int i = 1; i < n; i++) {
        /* Mix of integer operations with different latencies */
        int temp = arr[i-1] * scalar;      /* Multiplication */
        temp = temp + (arr[i] & 0xFF);     /* Bitwise AND + addition */
        temp = temp ^ (i << 2);            /* Shift + XOR */
        arr[i] = temp + (temp >> 4);       /* Shift + addition */
        
        /* Memory barrier to prevent over-optimization */
        asm volatile("" : : "r"(arr[i]) : "memory");
    }
    
    /* Use result to prevent elimination */
    local_sink = arr[n-1];
    sink += local_sink;
}

/* Function 2: Reduction loop with floating-point operations */
float loop2_fp_reduction(float *arr, int n) {
    volatile float acc = 1.0f;
    
    /* Loop with floating-point operations (higher latency) */
    for (int i = 0; i < n; i++) {
        /* Mix of FP operations */
        float val = arr[i];
        val = val * 1.5f;                  /* FP multiplication */
        val = val + (float)i * 0.1f;       /* FP addition with conversion */
        
        /* Integer operations mixed in */
        int idx = i & 0xF;
        val = val + (float)(idx * idx);    /* Integer arithmetic + conversion */
        
        acc = acc * val;                   /* FP multiplication with carried dependency */
        
        /* Compiler barrier */
        asm volatile("" : : "r"(val) : "memory");
    }
    
    sink += (int)acc;
    return acc;
}

/* Function 3: Pointer chasing pattern with conditional */
void loop3_pointer_chase(int **ptr_arr, int n) {
    volatile int sum = 0;
    int *current = ptr_arr[0];
    
    /* Pointer chasing with conditional inner logic */
    for (int i = 1; i < n; i++) {
        if (current != NULL) {
            /* Load-use dependency chain */
            int val = *current;
            val = val * 3 + i;             /* Multiplication + addition */
            val = val | 0x1;               /* Bitwise OR */
            
            /* Store with dependency */
            *current = val;
            sum += val;
            
            /* Move to next pointer with branch */
            current = (val & 1) ? ptr_arr[i] : current + 1;
            
            asm volatile("" : : "r"(val) : "memory");
        }
    }
    
    sink += sum;
}

/* Function 4: Multiple independent statements with final dependent store */
void loop4_multi_ops(int *a, int *b, int *c, int n) {
    volatile int checksum = 0;
    
    /* Loop with parallel operations then convergence */
    for (int i = 1; i < n; i++) {
        /* Independent computations */
        int x = a[i] * 7;
        int y = b[i] + 11;
        int z = c[i] ^ 0xAA;
        
        /* Converging dependency */
        int combined = (x & y) | z;
        
        /* Carried dependency */
        a[i] = combined + a[i-1];
        b[i] = a[i] * 2;
        
        checksum += a[i] + b[i];
        
        asm volatile("" : : "r"(combined) : "memory");
    }
    
    sink += checksum;
}

/* Function 5: Nested loops with inner modulo-scheduled candidate */
void loop5_nested(int *arr, int n, int m) {
    volatile int total = 0;
    
    /* Outer loop */
    for (int j = 0; j < m; j++) {
        int offset = j * 16;
        
        /* Inner loop - target for modulo scheduling */
        for (int i = 1; i < n; i++) {
            /* Complex carried dependency */
            int val = arr[i-1] * 3;
            val = val + (arr[i] >> 2);
            val = val ^ (offset + i);
            
            /* Conditional store */
            if (val > 0) {
                arr[i] = val & 0xFFF;
            } else {
                arr[i] = -val;
            }
            
            total += arr[i];
            
            /* Prevent optimization */
            if (i % 8 == 0) {
                asm volatile("" : : "r"(val) : "memory");
            }
        }
    }
    
    sink += total;
}

/* Main function that drives all test loops */
int main() {
    /* Initialize data arrays */
    int int_arr[SIZE];
    float float_arr[SIZE];
    int *ptr_arr[SIZE];
    int arr_b[SIZE], arr_c[SIZE];
    
    /* Seed for reproducibility */
    srand(42);
    
    /* Initialize arrays with non-zero values */
    for (int i = 0; i < SIZE; i++) {
        int_arr[i] = rand() % 100 + 1;
        float_arr[i] = (float)(rand() % 100) / 10.0f + 0.1f;
        ptr_arr[i] = &int_arr[i % SIZE];
        arr_b[i] = rand() % 50;
        arr_c[i] = rand() % 80;
    }
    
    /* Call each loop function multiple times with different parameters */
    for (int iter = 0; iter < ITERS; iter++) {
        int scalar = (iter % 7) + 2;  /* Vary scalar across iterations */
        
        /* Function 1 - Basic carried dependency */
        loop1_carried_dep(int_arr, SIZE - iter % 32, scalar);
        
        /* Function 2 - Floating point reduction */
        float fp_result = loop2_fp_reduction(float_arr, SIZE - iter % 16);
        sink += (int)(fp_result * 100);
        
        /* Function 3 - Pointer chasing */
        loop3_pointer_chase(ptr_arr, SIZE / 2);
        
        /* Function 4 - Multiple operations */
        loop4_multi_ops(int_arr, arr_b, arr_c, SIZE - iter % 64);
        
        /* Function 5 - Nested loops */
        loop5_nested(int_arr, 128, 3 + (iter % 4));
        
        /* Shuffle data occasionally */
        if (iter % 10 == 0) {
            for (int i = 0; i < SIZE; i++) {
                int_arr[i] ^= 0x55;
            }
        }
    }
    
    /* Final use of sink to prevent elimination */
    printf("Final checksum: %d\n", sink);
    
    return 0;
}
