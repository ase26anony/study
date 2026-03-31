/* test_modulo_sched.c - Program to trigger GCC's modulo scheduler edge printing */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 100
#define ITERS 1000

/* Global volatile sink to prevent optimization */
volatile int global_sink = 0;

/* Function 1: Loop with carried dependency and mixed operations */
void func1_carried_dep(int *arr, int n, int scalar) {
    volatile int *varr = (volatile int *)arr;
    
    /* Loop with carried dependency (distance 1) */
    for (int i = 1; i < n; i++) {
        /* Mixed integer operations with dependency chain */
        int temp = varr[i-1] * scalar;
        temp = temp + (temp >> 3);      /* Bitwise operation */
        temp = temp ^ (temp << 2);      /* More bitwise ops */
        varr[i] = temp + varr[i];
        
        /* Fake dependency with asm */
        asm volatile("" : "+r" (temp) : : "memory");
    }
}

/* Function 2: Reduction loop with floating point and integer mix */
double func2_reduction(const double *data, int n) {
    double sum = 0.0;
    volatile double vsum = sum;
    int int_sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Floating point operation */
        vsum += data[i] * 1.5;
        
        /* Integer operation in same loop */
        int_sum += (int)data[i] & 0xFF;
        
        /* Small carried dependency */
        if (i > 0) {
            int_sum ^= int_sum << 1;
        }
        
        /* Compiler barrier */
        asm volatile("" : "+r" (int_sum) : : "memory");
    }
    
    return vsum + int_sum;
}

/* Function 3: Pointer chasing with conditional inner logic */
int func3_pointer_chase(int *base, int n) {
    volatile int *current = base;
    int result = 0;
    
    /* Outer loop to encourage modulo scheduling */
    for (int outer = 0; outer < 10; outer++) {
        /* Inner loop with pointer chasing */
        int *p = base;
        for (int i = 0; i < n; i++) {
            /* Conditional logic inside loop */
            if (p != NULL) {
                result ^= *p;
                p = base + (*p % n);  /* Pointer chasing */
                
                /* Mixed operations */
                result = result * 3 + 1;
            }
            
            /* Memory barrier */
            asm volatile("" : "+r" (result) : : "memory");
        }
        
        /* Update volatile to prevent optimization */
        *current = result;
        current = base + (outer % 5);
    }
    
    return result;
}

/* Function 4: Multiple independent statements with dependent store */
void func4_multiple_deps(float *fa, int *ia, int n) {
    volatile float *vfa = (volatile float *)fa;
    volatile int *via = (volatile int *)ia;
    
    for (int i = 1; i < n; i++) {
        /* Independent computations */
        float f1 = vfa[i-1] * 2.5f;
        float f2 = vfa[i] * 1.8f;
        int i1 = via[i-1] * 3;
        int i2 = via[i] + 7;
        
        /* Dependent store with mixed types */
        vfa[i] = f1 + f2 + (float)(i1 ^ i2);
        
        /* Another dependent operation */
        via[i] = (int)f1 * i2 - (i1 >> 2);
        
        /* Prevent reordering */
        asm volatile("" : : "r" (f1), "r" (i2) : "memory");
    }
}

/* Function 5: Nested loops with complex index calculations */
void func5_nested_loops(short *arr, int rows, int cols) {
    volatile short *varr = (volatile short *)arr;
    
    for (int r = 1; r < rows; r++) {
        /* Inner loop with carried dependency across columns */
        for (int c = 1; c < cols; c++) {
            int idx = r * cols + c;
            int prev_idx = (r-1) * cols + c;
            int left_idx = r * cols + (c-1);
            
            /* Complex dependency pattern */
            short val = varr[prev_idx] + varr[left_idx];
            val = (val * 3) >> 1;
            val ^= val << 4;
            
            varr[idx] = val + (short)(idx % 256);
            
            /* Memory barrier every few iterations */
            if (c % 8 == 0) {
                asm volatile("" : : "r" (val) : "memory");
            }
        }
    }
}

int main() {
    /* Initialize data arrays */
    int int_arr[SIZE];
    double double_arr[SIZE];
    float float_arr[SIZE];
    short short_arr[SIZE * 10];  /* For nested loops */
    
    /* Fill with non-zero, non-uniform data */
    for (int i = 0; i < SIZE; i++) {
        int_arr[i] = (i * 37 + 123) % 1000;
        double_arr[i] = (i * 1.7 + 2.3);
        float_arr[i] = (float)(i * 0.7 + 1.2);
    }
    
    for (int i = 0; i < SIZE * 10; i++) {
        short_arr[i] = (short)((i * 19 + 7) % 32767);
    }
    
    /* Call all functions to ensure they're compiled */
    func1_carried_dep(int_arr, SIZE, 3);
    
    double sum = func2_reduction(double_arr, SIZE);
    global_sink += (int)sum;
    
    int chase_result = func3_pointer_chase(int_arr, SIZE);
    global_sink ^= chase_result;
    
    func4_multiple_deps(float_arr, int_arr, SIZE);
    
    func5_nested_loops(short_arr, 10, SIZE);
    
    /* Use results to prevent dead code elimination */
    for (int i = 0; i < SIZE; i++) {
        global_sink += int_arr[i] + (int)float_arr[i];
    }
    
    /* Print minimal output */
    printf("Result checksum: %d\n", global_sink);
    
    return 0;
}
