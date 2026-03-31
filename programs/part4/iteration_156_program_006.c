/* Test program to trigger modulo scheduling edge printing in GCC */
#include <stdlib.h>
#include <stdio.h>

#define SIZE 256
#define ITERS 100

/* Global volatile sink to prevent dead code elimination */
volatile int sink = 0;

/* Function 1: Loop with carried dependency and mixed operations */
void func1_carried_dep(volatile int* arr, int n) {
    if (n <= 1) return;
    
    /* Loop with carried dependency (distance 1) */
    for (int i = 1; i < n; i++) {
        /* Mix of integer operations with different latencies */
        int temp = arr[i-1] * 3;      /* Multiplication */
        temp = temp + (temp >> 2);    /* Shift and add */
        temp = temp ^ 0x55AA55AA;     /* Bitwise operation */
        arr[i] = arr[i] + temp;       /* Dependent store */
        
        /* Memory barrier to prevent over-optimization */
        asm volatile("" : : "r"(arr[i]) : "memory");
    }
}

/* Function 2: Reduction loop with floating-point operations */
double func2_fp_reduction(const double* data, int n) {
    double sum = 0.0;
    double prod = 1.0;
    
    /* Loop with floating-point operations (higher latency) */
    for (int i = 0; i < n; i++) {
        /* Multiple dependent FP operations */
        double val = data[i];
        sum = sum + val * 0.5;        /* FP multiply-add pattern */
        prod = prod * (val + 1.0);    /* Another FP dependency chain */
        
        /* Integer operations mixed in */
        int idx = (int)val & 0xFF;
        sum += idx * 0.01;
    }
    
    /* Create artificial dependency between sum and prod */
    volatile double vs = sum;
    volatile double vp = prod;
    asm volatile("" : : "r"(vs), "r"(vp) : "memory");
    
    return sum + prod;
}

/* Function 3: Pointer chasing with conditional inner logic */
void func3_pointer_chase(int* base, int n) {
    if (n < 2) return;
    
    int* p = base;
    int count = 0;
    
    /* Outer loop to encourage modulo scheduling */
    for (int outer = 0; outer < 10; outer++) {
        /* Inner loop with pointer chasing pattern */
        p = base;
        for (int i = 0; i < n - 1; i++) {
            /* Conditional inside loop */
            if (p[i] > 0) {
                p[i+1] = p[i] * 2 + p[i+1];
            } else {
                p[i+1] = p[i] - p[i+1];
            }
            
            /* Mix of operations */
            p[i] = (p[i] * 3) / 2;
            count += p[i] & 1;  /* Reduction */
        }
        
        /* Compiler barrier */
        asm volatile("" : : "r"(count) : "memory");
    }
    
    /* Use result */
    sink += count;
}

/* Function 4: Multiple independent statements with final dependent store */
void func4_multi_stmts(int* a, int* b, int* c, int n) {
    /* Loop with parallel chains converging */
    for (int i = 1; i < n; i++) {
        /* Independent computation chains */
        int chain1 = a[i-1] * 7 + 12345;
        int chain2 = b[i] ^ 0xDEADBEEF;
        int chain3 = c[i] + c[i-1] * 3;
        
        /* Some cross-chain operations */
        chain1 = chain1 + (chain2 & 0xFF);
        chain2 = chain2 | (chain3 << 8);
        
        /* Final convergence point - all chains used */
        a[i] = (chain1 + chain2 + chain3) & 0xFFFF;
        
        /* Prevent optimization */
        if (i % 16 == 0) {
            asm volatile("" : : "r"(a[i]) : "memory");
        }
    }
}

/* Function 5: Nested loops with complex index calculations */
void func5_nested_complex(int* mat, int rows, int cols) {
    if (rows < 2 || cols < 2) return;
    
    /* Outer loop */
    for (int r = 1; r < rows; r++) {
        /* Inner loop with stride access */
        for (int c = 1; c < cols; c++) {
            int idx = r * cols + c;
            int idx_up = (r-1) * cols + c;
            int idx_left = r * cols + (c-1);
            
            /* Complex dependency pattern */
            mat[idx] = mat[idx_up] * 2 + mat[idx_left] * 3 
                      - mat[idx] / 4;
            
            /* Additional operation with different latency */
            mat[idx] = (mat[idx] << 3) | (mat[idx] >> 29);
        }
        
        /* Barrier every few iterations */
        if (r % 4 == 0) {
            asm volatile("" : : "r"(mat[r * cols]) : "memory");
        }
    }
}

/* Main driver */
int main() {
    /* Initialize data arrays */
    volatile int arr1[SIZE];
    int arr2[SIZE];
    double fp_data[SIZE];
    int matrix[SIZE/2][SIZE/2];
    
    /* Initialize with non-zero values */
    for (int i = 0; i < SIZE; i++) {
        arr1[i] = (i * 3 + 7) & 0xFF;
        arr2[i] = (i * 5 + 11) & 0xFF;
        fp_data[i] = (i * 0.7 + 1.3);
    }
    
    for (int i = 0; i < SIZE/2; i++) {
        for (int j = 0; j < SIZE/2; j++) {
            matrix[i][j] = (i * j + i + j) & 0xFF;
        }
    }
    
    /* Execute all functions multiple times */
    for (int iter = 0; iter < ITERS; iter++) {
        func1_carried_dep(arr1, SIZE);
        
        double fp_result = func2_fp_reduction(fp_data, SIZE);
        sink += (int)fp_result;
        
        func3_pointer_chase(arr2, SIZE);
        
        func4_multi_stmts(arr2, (int*)arr1, arr2, SIZE);
        
        func5_nested_complex(&matrix[0][0], SIZE/4, SIZE/4);
        
        /* Mix up data to prevent pattern recognition */
        for (int i = 0; i < SIZE; i++) {
            arr1[i] ^= 0x11;
            arr2[i] += iter;
        }
    }
    
    /* Final checksum to use all results */
    int checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += arr1[i] + arr2[i];
    }
    
    printf("Result checksum: %d\n", checksum);
    return 0;
}
