/* test_modulo_sched.c - Program to trigger GCC's modulo scheduler edge printing */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 256
#define ITERS 100

/* Global volatile to prevent optimization */
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
        
        /* Fake dependency barrier */
        asm volatile("" : : "r"(temp) : "memory");
    }
}

/* Function 2: Reduction loop with floating-point operations */
float func2_reduction(float *arr, int n) {
    volatile float sum = 0.0f;
    volatile float *varr = (volatile float *)arr;
    
    /* Reduction with carried dependency */
    for (int i = 0; i < n; i++) {
        /* Mix of float and int operations */
        float val = varr[i];
        val = val * 1.5f + (float)i;    /* FP operation */
        int ival = (int)val;
        ival = ival * 3 + (ival & 0xFF); /* Integer operation */
        sum = sum + (float)ival;
        
        /* Prevent dead code elimination */
        asm volatile("" : : "r"(val), "r"(ival) : "memory");
    }
    return sum;
}

/* Function 3: Pointer chasing pattern */
void func3_pointer_chase(int **ptr_arr, int n) {
    volatile int **vptr = (volatile int **)ptr_arr;
    volatile int sum = 0;
    
    /* Pointer chasing creates unpredictable dependencies */
    for (int i = 1; i < n; i++) {
        int *current = vptr[i-1];
        if (current != NULL) {
            sum += *current;
            vptr[i] = current + 1;  /* Create address dependency */
        }
        
        /* Mix with arithmetic */
        sum = sum * 1103515245 + 12345;
    }
    
    global_sink += sum;
}

/* Function 4: Multiple independent statements with final dependent store */
void func4_multi_ops(int *arr1, int *arr2, int *arr3, int n) {
    volatile int *v1 = (volatile int *)arr1;
    volatile int *v2 = (volatile int *)arr2;
    volatile int *v3 = (volatile int *)arr3;
    
    for (int i = 1; i < n; i++) {
        /* Independent computations */
        int a = v1[i] * 3;
        int b = v2[i] + 7;
        int c = v3[i] ^ 0xFF;
        
        /* Dependent operation mixing all three */
        int combined = (a * b) + (c << 2);
        
        /* Store with carried dependency */
        v1[i] = v1[i-1] + combined;
        v2[i] = v2[i-1] ^ combined;
        
        /* Memory barrier */
        asm volatile("" : : "r"(a), "r"(b), "r"(c), "r"(combined) : "memory");
    }
}

/* Function 5: Nested loops with conditional inner logic */
void func5_nested_conditional(int *arr, int n, int threshold) {
    volatile int *varr = (volatile int *)arr;
    volatile int outer_acc = 0;
    
    /* Outer loop */
    for (int j = 0; j < 5; j++) {
        /* Inner loop with condition */
        if (threshold > 0) {
            for (int i = 1; i < n; i++) {
                /* Complex dependency chain */
                int val = varr[i-1];
                val = val * (j + 1);
                
                /* Conditional operation */
                if (val > threshold) {
                    val = val >> 2;
                } else {
                    val = val * 3;
                }
                
                varr[i] = val + i;
                outer_acc += val;
                
                /* Prevent optimization */
                asm volatile("" : : "r"(val) : "memory");
            }
        }
    }
    
    global_sink += outer_acc;
}

/* Main function that exercises all loops */
int main() {
    /* Initialize arrays with non-zero values */
    int arr_int[SIZE];
    float arr_float[SIZE];
    int *ptr_arr[SIZE];
    int arr1[SIZE], arr2[SIZE], arr3[SIZE];
    
    for (int i = 0; i < SIZE; i++) {
        arr_int[i] = i + 1;
        arr_float[i] = (float)(i * 1.1);
        arr1[i] = i * 2;
        arr2[i] = i * 3;
        arr3[i] = i * 5;
        
        /* Allocate some pointers for chasing */
        if (i % 8 == 0) {
            ptr_arr[i] = &arr_int[i];
        } else {
            ptr_arr[i] = NULL;
        }
    }
    
    volatile int checksum = 0;
    
    /* Call all functions multiple times to ensure they're compiled */
    for (int iter = 0; iter < ITERS; iter++) {
        func1_carried_dep(arr_int, SIZE, 3);
        checksum += arr_int[SIZE-1];
        
        float fsum = func2_reduction(arr_float, SIZE);
        checksum += (int)fsum;
        
        func3_pointer_chase(ptr_arr, SIZE/2);
        func4_multi_ops(arr1, arr2, arr3, SIZE);
        func5_nested_conditional(arr_int, SIZE, 100);
    }
    
    /* Use results to prevent dead code elimination */
    printf("Checksum: %d\n", checksum);
    printf("Global sink: %d\n", global_sink);
    
    return 0;
}
