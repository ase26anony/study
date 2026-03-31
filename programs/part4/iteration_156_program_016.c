/* Test program to trigger modulo scheduling edge printing in GCC */
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
        /* Mix of operations with different latencies */
        int temp = varr[i-1] * scalar;      /* integer multiply */
        temp = temp + (temp >> 3);          /* shift and add */
        temp = temp ^ (temp * 3);           /* xor with multiplication */
        varr[i] = temp + varr[i];           /* dependent store */
        
        /* Memory barrier to preserve ordering */
        asm volatile("" : : "r"(temp) : "memory");
    }
}

/* Function 2: Reduction loop with floating point operations */
float func2_reduction(float *data, int n) {
    volatile float acc = 0.0f;
    volatile float *vdata = (volatile float *)data;
    
    /* Reduction with carried dependency */
    for (int i = 0; i < n; i++) {
        /* Mix of FP and integer operations */
        float val = vdata[i];
        val = val * 1.5f;                   /* FP multiply */
        int ival = (int)val;                /* type conversion */
        ival = ival & 0xFF;                 /* bitwise operation */
        val = (float)ival + val * 0.5f;     /* mixed operations */
        acc = acc + val;                    /* carried dependency */
    }
    
    return acc;
}

/* Function 3: Pointer chasing with conditional inner logic */
int func3_pointer_chase(int *base, int n) {
    volatile int *ptr = base;
    volatile int sum = 0;
    
    /* Outer loop to encourage modulo scheduling */
    for (int outer = 0; outer < 5; outer++) {
        /* Inner loop with pointer chasing */
        for (int i = 0; i < n; i++) {
            /* Conditional inside loop */
            if (ptr != 0) {
                int val = *ptr;
                val = val * 2 + i;          /* dependent on iteration */
                sum = sum ^ val;            /* reduction */
                
                /* Update pointer with dependency */
                ptr = base + (val % n);
                
                /* Compiler barrier */
                asm volatile("" : : "r"(val) : "memory");
            }
        }
    }
    
    return sum;
}

/* Function 4: Multiple independent statements with final dependent store */
void func4_multi_ops(int *a, int *b, int *c, int n) {
    volatile int *va = (volatile int *)a;
    volatile int *vb = (volatile int *)b;
    volatile int *vc = (volatile int *)c;
    
    for (int i = 1; i < n; i++) {
        /* Multiple independent operations */
        int x = va[i] * 3;
        int y = vb[i] + 7;
        int z = (x ^ y) << 2;
        
        /* Dependent operation mixing all */
        int result = (x + y) * z;
        
        /* Carried dependency through array */
        result = result + vc[i-1];
        
        /* Store with dependency */
        vc[i] = result;
        
        /* Prevent dead code elimination */
        asm volatile("" : : "r"(result) : "memory");
    }
}

/* Function 5: Nested loops with mixed operations */
double func5_nested(double *arr, int n) {
    volatile double total = 0.0;
    volatile double *varr = (volatile double *)arr;
    
    /* Outer loop */
    for (int j = 0; j < 3; j++) {
        double accum = 0.0;
        
        /* Inner loop with carried dependency */
        for (int i = 1; i < n; i++) {
            /* FP operations with dependency */
            double diff = varr[i] - varr[i-1];
            diff = diff * diff;             /* square */
            accum = accum + diff;           /* reduction */
            
            /* Integer operation mixed in */
            int idx = (int)diff % n;
            accum = accum + (double)idx * 0.1;
        }
        
        total = total + accum;
    }
    
    return total;
}

int main() {
    /* Initialize arrays with non-zero values */
    int *int_arr1 = (int *)malloc(SIZE * sizeof(int));
    int *int_arr2 = (int *)malloc(SIZE * sizeof(int));
    int *int_arr3 = (int *)malloc(SIZE * sizeof(int));
    float *float_arr = (float *)malloc(SIZE * sizeof(float));
    double *double_arr = (double *)malloc(SIZE * sizeof(double));
    
    for (int i = 0; i < SIZE; i++) {
        int_arr1[i] = (i * 3 + 7) % 100;
        int_arr2[i] = (i * 5 + 11) % 100;
        int_arr3[i] = (i * 7 + 13) % 100;
        float_arr[i] = (float)(i * 0.3 + 0.7);
        double_arr[i] = (double)(i * 0.1 + 0.5);
    }
    
    /* Call all functions to ensure they're compiled and executed */
    func1_carried_dep(int_arr1, ITERS, 3);
    
    float fresult = func2_reduction(float_arr, ITERS);
    global_sink += (int)fresult;
    
    int iresult = func3_pointer_chase(int_arr2, ITERS);
    global_sink += iresult;
    
    func4_multi_ops(int_arr1, int_arr2, int_arr3, ITERS);
    
    double dresult = func5_nested(double_arr, ITERS);
    global_sink += (int)dresult;
    
    /* Use results to prevent dead code elimination */
    volatile int checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += int_arr1[i] + int_arr2[i] + int_arr3[i];
    }
    
    /* Print minimal output */
    printf("Checksum: %d, Global: %d\n", checksum, global_sink);
    
    /* Cleanup */
    free(int_arr1);
    free(int_arr2);
    free(int_arr3);
    free(float_arr);
    free(double_arr);
    
    return 0;
}
