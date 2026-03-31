/* Test program to trigger modulo scheduling edge printing in GCC */
#include <stdlib.h>
#include <stdio.h>

#define SIZE 256
#define ITERS 100

/* Global volatile to prevent optimization */
volatile int global_sink = 0;

/* Function 1: Loop with carried dependency (distance 1) */
void loop_carried_dependency(int *arr, int n, int scalar) {
    volatile int *varr = (volatile int *)arr;
    
    /* Loop with carried dependency: arr[i] depends on arr[i-1] */
    for (int i = 1; i < n; i++) {
        varr[i] = varr[i-1] * scalar + varr[i];
        
        /* Mix in some floating point operations */
        float ftemp = (float)varr[i] * 1.5f;
        varr[i] += (int)ftemp;
    }
    
    /* Compiler barrier to preserve dependencies */
    asm volatile("" : : "r"(arr) : "memory");
}

/* Function 2: Reduction loop with mixed operations */
int reduction_loop(int *arr, int n) {
    int sum = arr[0];
    volatile int vsum = sum;
    
    for (int i = 1; i < n; i++) {
        /* Multiple operations with different latencies */
        int temp = arr[i] * 3;
        temp = temp + (arr[i] >> 2);  /* Bitwise operation */
        vsum += temp;
        
        /* Floating point operation */
        double dtemp = (double)temp * 0.75;
        vsum += (int)dtemp;
    }
    
    /* Use result to prevent elimination */
    global_sink ^= vsum;
    return vsum;
}

/* Function 3: Pointer chasing pattern */
void pointer_chasing(int **ptr_arr, int n) {
    volatile int **vptr = ptr_arr;
    int *current = vptr[0];
    
    for (int i = 1; i < n && current != NULL; i++) {
        /* Pointer chasing with computation */
        int val = *current;
        val = val * 7 + i;
        
        /* Store and move to next */
        *current = val;
        current = vptr[i % n];
        
        /* Memory barrier */
        asm volatile("" : : "r"(current), "r"(val) : "memory");
    }
}

/* Function 4: Nested loops with conditional inner logic */
void nested_conditional_loops(int *arr, int n, int threshold) {
    volatile int *varr = (volatile int *)arr;
    
    /* Outer loop */
    for (int outer = 0; outer < 5; outer++) {
        /* Inner loop with condition */
        if (threshold > 0) {
            for (int i = 1; i < n; i++) {
                /* Complex dependency chain */
                int a = varr[i-1] * 2;
                int b = a + varr[i];
                int c = b ^ (b << 3);  /* Non-linear operation */
                
                /* Conditional store */
                if (c > threshold) {
                    varr[i] = c;
                } else {
                    varr[i] = b;
                }
                
                /* Floating point in conditional path */
                float f = (float)c * 0.25f;
                varr[i] += (int)f;
            }
        }
        
        /* Compiler barrier between outer iterations */
        asm volatile("" : : "r"(arr) : "memory");
    }
}

/* Function 5: Multiple independent statements with final dependency */
void multi_statement_loop(int *arr, int n) {
    volatile int *varr = (volatile int *)arr;
    
    for (int i = 1; i < n; i++) {
        /* Independent computations */
        int t1 = varr[i] * 11;
        int t2 = varr[i] + 17;
        int t3 = varr[i] ^ 0x55AA55AA;
        
        /* Floating point parallel path */
        float f1 = (float)t1 * 1.1f;
        float f2 = (float)t2 * 2.2f;
        
        /* Final dependent operation */
        int combined = (int)f1 + (int)f2 + t3;
        
        /* Carried dependency store */
        varr[i] = combined + varr[i-1];
    }
}

/* Main function that exercises all loops */
int main() {
    /* Initialize data arrays */
    int *arr1 = (int *)malloc(SIZE * sizeof(int));
    int *arr2 = (int *)malloc(SIZE * sizeof(int));
    int **ptr_arr = (int **)malloc(SIZE * sizeof(int *));
    
    if (!arr1 || !arr2 || !ptr_arr) {
        return 1;
    }
    
    /* Initialize with non-zero values */
    for (int i = 0; i < SIZE; i++) {
        arr1[i] = i + 1;
        arr2[i] = (i * 3) % 97;
        ptr_arr[i] = &arr2[i % SIZE];
    }
    
    /* Call all loop functions in sequence */
    loop_carried_dependency(arr1, SIZE, 3);
    
    int sum = reduction_loop(arr1, SIZE);
    
    pointer_chasing(ptr_arr, SIZE / 4);
    
    nested_conditional_loops(arr2, SIZE, 50);
    
    multi_statement_loop(arr2, SIZE);
    
    /* Use results to prevent dead code elimination */
    volatile int checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum ^= arr1[i];
        checksum ^= arr2[i];
    }
    checksum ^= sum;
    
    /* Print minimal output (optional) */
    printf("Checksum: %d\n", checksum);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(ptr_arr);
    
    return 0;
}
