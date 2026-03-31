#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define SIZE 1000

/* External function to prevent optimization */
extern void use_result(int, float);

/* Complex loop with mixed operations and dependencies */
void compute_kernel(int *arr1, int *arr2, float *farr1, float *farr2, 
                    int n, int threshold) {
    int acc = 0;                    /* Loop-carried dependency */
    float f_acc = 0.0f;             /* Floating-point accumulator */
    
    /* Outer loop to give scheduler repeated region */
    for (int outer = 0; outer < 3; outer++) {
        /* Main computational loop with complex dependencies */
        for (int i = 1; i < n; i++) {
            /* Loop-carried integer dependency */
            acc += arr1[i] * arr2[i-1];
            
            /* Mixed integer/float operations */
            float temp = (float)acc / (1.0f + fabsf(farr1[i]));
            farr2[i] = temp * 0.5f;
            
            /* Conditional control flow with data-dependent condition */
            if (farr1[i] > 0.5f && (arr1[i] & 0x3) == 0) {
                /* Complex floating-point operation */
                farr2[i] = sqrtf(farr1[i]) * cosf(temp);
                
                /* Integer operation with bitwise manipulation */
                arr2[i] = (arr1[i] & 0xFF) | (arr2[i-1] << 8);
            } else {
                /* Alternative computation path */
                arr2[i] = (arr1[i] ^ arr2[i-1]) + (int)(farr2[i] * 100.0f);
            }
            
            /* Cross-iteration floating-point dependency */
            f_acc = farr1[i] * 0.9f + f_acc * 0.1f;
            farr1[i] = f_acc;
            
            /* Additional memory access pattern */
            int idx = (i * 7) % n;
            arr1[idx] += arr2[i] % 256;
            
            /* Another conditional with mixed operations */
            if (acc > threshold) {
                farr2[i] *= 2.0f;
                arr1[i] >>= 1;
            }
        }
        
        /* Small perturbation between outer iterations */
        acc ^= 0x5555;
        f_acc += 0.1f;
    }
    
    /* Use results to prevent dead code elimination */
    use_result(acc, f_acc);
}

/* Wrapper function with volatile parameters */
void run_computation(volatile int iter_count) {
    /* Dynamically allocated arrays to force heap access */
    int *arr1 = malloc(SIZE * sizeof(int));
    int *arr2 = malloc(SIZE * sizeof(int));
    float *farr1 = malloc(SIZE * sizeof(float));
    float *farr2 = malloc(SIZE * sizeof(float));
    
    if (!arr1 || !arr2 || !farr1 || !farr2) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
    
    /* Initialize with pseudo-random data */
    srand(time(NULL));
    for (int i = 0; i < SIZE; i++) {
        arr1[i] = rand() % 1000;
        arr2[i] = rand() % 1000;
        farr1[i] = (float)rand() / RAND_MAX;
        farr2[i] = (float)rand() / RAND_MAX;
    }
    
    /* Volatile variable prevents constant propagation */
    volatile int threshold = rand() % 500 + 500;
    
    /* Call the computational kernel */
    compute_kernel(arr1, arr2, farr1, farr2, iter_count, threshold);
    
    /* Calculate checksum to ensure computation happens */
    int checksum = 0;
    float f_checksum = 0.0f;
    for (int i = 0; i < SIZE; i++) {
        checksum ^= arr1[i];
        checksum += arr2[i];
        f_checksum += farr1[i] + farr2[i];
    }
    
    printf("Checksum: int=%d, float=%f\n", checksum, f_checksum);
    
    free(arr1);
    free(arr2);
    free(farr1);
    free(farr2);
}

/* External function implementation */
void use_result(int val, float fval) {
    /* Do something non-trivial but compiler can't optimize away */
    static volatile int sink_int;
    static volatile float sink_float;
    sink_int = val;
    sink_float = fval;
}

int main(int argc, char *argv[]) {
    /* Use command-line argument for non-constant iteration count */
    volatile int iterations = SIZE - 50;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0 || iterations > SIZE) {
            iterations = SIZE - 50;
        }
    }
    
    /* Run multiple times to increase scheduling opportunities */
    for (int run = 0; run < 2; run++) {
        run_computation(iterations + run);
    }
    
    return 0;
}
