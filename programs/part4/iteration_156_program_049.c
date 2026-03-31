/* Test program to trigger modulo scheduling edge printing in GCC */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 256
#define ITERS 100

/* Global volatile sink to prevent optimization */
volatile int sink = 0;

/* Function 1: Loop with carried dependency and mixed operations */
void loop1_carried_dependency(volatile int* arr, int n, int scalar) {
    /* Loop with distance-1 carried dependency */
    for (int i = 1; i < n; i++) {
        /* Mixed integer operations with dependency chain */
        int temp = arr[i-1] * scalar;
        temp = temp + (temp >> 3);      /* Bitwise operation */
        arr[i] = temp + arr[i] * 7;     /* Another multiplication */
        
        /* Fake dependency to prevent optimization */
        asm volatile("" : "+r" (temp) : : "memory");
    }
}

/* Function 2: Reduction loop with floating point operations */
float loop2_reduction_mixed(float* farr, int* iarr, int n) {
    float sum = 0.0f;
    float prod = 1.0f;
    
    for (int i = 0; i < n; i++) {
        /* Floating point operations (higher latency) */
        float fval = farr[i] * 1.5f;
        sum += fval;
        
        /* Integer operations mixed in */
        int ival = iarr[i] & 0xFF;
        prod *= (fval + ival) * 0.5f;
        
        /* Cross-iteration dependency */
        if (i > 0) {
            farr[i] += farr[i-1] * 0.3f;
        }
    }
    
    /* Compiler barrier */
    asm volatile("" : "+r" (sum), "+r" (prod) : : "memory");
    return sum + prod;
}

/* Function 3: Pointer chasing with conditional inner logic */
int loop3_pointer_chasing(int* data, int n, int seed) {
    int* p = data;
    int sum = 0;
    int count = 0;
    
    /* Outer loop to encourage modulo scheduling */
    for (int outer = 0; outer < 3; outer++) {
        p = data + (seed % n);
        
        /* Inner loop with pointer chasing */
        for (int i = 0; i < ITERS && count < n*2; i++) {
            /* Load with potential cache miss */
            int val = *p;
            
            /* Conditional operation */
            if (val & 1) {
                val = val * 3 + 1;
            } else {
                val = val / 2;
            }
            
            /* Store with dependency */
            *p = val;
            sum += val;
            
            /* Pointer update - creates dependency chain */
            p = data + (val % n);
            count++;
            
            /* Memory barrier */
            asm volatile("" : : "r"(val) : "memory");
        }
    }
    
    return sum;
}

/* Function 4: Multiple independent statements with final dependent store */
void loop4_multiple_deps(double* darr, int* iarr, int n) {
    for (int i = 1; i < n; i++) {
        /* Independent computations */
        double a = darr[i] * 2.5;
        double b = darr[i-1] / 1.7;
        int c = iarr[i] | 0x0F;
        int d = iarr[i-1] & 0xF0;
        
        /* Dependent computation mixing all */
        double result = a + b + (c * d) / 100.0;
        
        /* Store with multiple dependencies */
        darr[i] = result + darr[i-1] * 0.1;
        
        /* Prevent optimization */
        asm volatile("" : "+r" (c), "+r" (d) : : "memory");
    }
}

/* Function 5: Nested loops with complex index calculations */
void loop5_nested_complex(short* sarr, int* iarr, int n) {
    int block = 16;
    
    for (int j = 0; j < n; j += block) {
        int limit = (j + block < n) ? j + block : n;
        
        /* Inner loop with modulo access pattern */
        for (int i = j; i < limit; i++) {
            /* Complex addressing with dependency */
            int idx = (i * 7) % n;
            int idx2 = (i * 13) % n;
            
            /* Mixed operations with carried dependency */
            short s1 = sarr[idx];
            short s2 = sarr[idx2];
            int val = (s1 * s2) + iarr[i];
            
            /* Conditional store */
            if (val > 0) {
                iarr[i] = val % 1000;
                sarr[idx] = (s1 + s2) & 0xFF;
            }
            
            /* Dependency across iterations */
            if (i > j) {
                sarr[idx] += sarr[(idx - 1) % n] / 2;
            }
        }
    }
}

int main() {
    /* Initialize arrays with non-zero values */
    volatile int arr1[SIZE];
    int arr2[SIZE];
    float farr[SIZE];
    double darr[SIZE];
    short sarr[SIZE];
    
    for (int i = 0; i < SIZE; i++) {
        arr1[i] = (i * 3 + 7) % 100;
        arr2[i] = (i * 5 + 11) % 255;
        farr[i] = (i * 0.7f) + 1.0f;
        darr[i] = (i * 1.3) + 0.5;
        sarr[i] = (i * 2) % 32767;
    }
    
    /* Call all loop functions to ensure they're compiled */
    loop1_carried_dependency(arr1, SIZE, 3);
    
    float fresult = loop2_reduction_mixed(farr, arr2, SIZE);
    sink += (int)fresult;
    
    int iresult = loop3_pointer_chasing(arr2, SIZE, 42);
    sink += iresult;
    
    loop4_multiple_deps(darr, arr2, SIZE);
    
    loop5_nested_complex(sarr, arr2, SIZE);
    
    /* Use results to prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += arr1[i] + arr2[i] + (int)farr[i] + (int)darr[i] + sarr[i];
    }
    
    sink += checksum;
    
    /* Print minimal output */
    printf("Result checksum: %d\n", checksum % 1000);
    
    return 0;
}
