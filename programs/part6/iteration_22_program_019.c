/* Test program to trigger scheduler context save/restore cleanup */
/* Compile with: gcc -O3 -fmodulo-sched -fschedule-insns -funroll-loops -fdump-rtl-sched1 -fdump-rtl-sched2 test_scheduler_coverage.c -o test_scheduler_coverage */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1024
#define ITERATIONS 100000

/* Function 1: Compute-intensive with tight floating-point loop */
/* This creates pressure for software pipelining */
void compute_intensive(float *a, float *b, float *c, int n) {
    int i, j;
    for (i = 0; i < n; i++) {
        /* Complex floating-point operations with dependencies */
        float acc = 0.0f;
        for (j = 0; j < 8; j++) {
            acc += a[i] * b[i] * (j + 1);
            a[i] = acc * 0.5f;
            b[i] = acc * 0.25f;
        }
        c[i] = acc;
        
        /* Conditional creates branch scheduling complexity */
        if (c[i] > 100.0f) {
            c[i] = 100.0f;
        } else if (c[i] < -100.0f) {
            c[i] = -100.0f;
        }
    }
}

/* Function 2: Integer processing with mixed operations and memory access */
/* Creates varied instruction mix for scheduler */
void integer_processing(int *arr, int *mask, int n) {
    int i, j;
    volatile int sum = 0; /* volatile prevents optimization */
    
    /* Nested loops with data dependencies */
    for (i = 0; i < n; i++) {
        int val = arr[i];
        
        /* Mixed arithmetic operations */
        for (j = 0; j < 4; j++) {
            val = (val * 3 + 7) >> 1;
            val ^= mask[j % 4];
            val = (val << 2) | (val >> 30); /* rotate */
        }
        
        /* Conditional store with memory barrier effect */
        if (val > 0) {
            arr[i] = val;
            sum += val;
        } else {
            arr[i] = -val;
            sum -= val;
        }
    }
    
    /* Use sum to prevent dead code elimination */
    if (sum < 0) {
        printf(""); /* Side effect */
    }
}

/* Function 3: Complex control flow with function calls */
/* Creates scheduling barriers */
int complex_control_flow(int *data, int n) {
    int i, result = 0;
    int *temp = malloc(n * sizeof(int));
    
    if (!temp) return -1;
    
    /* Loop with early exit condition - creates CFG complexity */
    for (i = 0; i < n; i++) {
        if (data[i] == 0) {
            /* Function call creates scheduling barrier */
            temp[i] = rand() % 100;
        } else if (data[i] > 0) {
            /* Nested loop with dependency */
            int j;
            for (j = 0; j < data[i] && j < 10; j++) {
                temp[i] += data[i] * j;
            }
        } else {
            /* Another path with different operations */
            temp[i] = data[i] * data[i] - data[i];
        }
        
        result += temp[i];
        
        /* Conditional break adds more control flow */
        if (result > 1000000) {
            break;
        }
    }
    
    free(temp);
    return result;
}

/* Function 4: Matrix operations - creates large basic blocks */
void matrix_operations(float mat1[SIZE][SIZE], float mat2[SIZE][SIZE], 
                       float result[SIZE][SIZE], int size) {
    int i, j, k;
    
    /* Triple nested loop - good for modulo scheduling */
    for (i = 0; i < size; i++) {
        for (j = 0; j < size; j++) {
            float sum = 0.0f;
            for (k = 0; k < size; k++) {
                /* Complex expression with multiple dependencies */
                sum += mat1[i][k] * mat2[k][j] 
                     + mat1[i][k] * 0.5f 
                     + mat2[k][j] * 0.25f;
            }
            result[i][j] = sum;
            
            /* Conditional with side effect */
            if (result[i][j] != sum) {
                /* Should never happen, but creates control flow */
                result[i][j] = 0.0f;
            }
        }
    }
}

/* Function 5: Mixed data types and operations */
void mixed_operations(short *sdata, int *idata, float *fdata, int n) {
    int i;
    
    for (i = 0; i < n; i++) {
        /* Type conversions and mixed operations */
        float fval = (float)idata[i] * 1.5f;
        int ival = (int)(fval * 100.0f);
        short sval = (short)(ival % 32767);
        
        /* Store with different strides */
        sdata[i] = sval;
        idata[i] = ival;
        fdata[i] = fval;
        
        /* Loop with variable bound */
        int j;
        for (j = 0; j < (i % 8); j++) {
            fdata[i] += 0.1f * j;
        }
    }
}

int main(int argc, char *argv[]) {
    int i;
    clock_t start, end;
    
    /* Use argc to prevent constant folding */
    int data_size = (argc > 1) ? atoi(argv[1]) : 1000;
    if (data_size <= 0) data_size = 1000;
    
    /* Allocate test data */
    float *fa = malloc(data_size * sizeof(float));
    float *fb = malloc(data_size * sizeof(float));
    float *fc = malloc(data_size * sizeof(float));
    
    int *idata = malloc(data_size * sizeof(int));
    int *mask = malloc(4 * sizeof(int));
    short *sdata = malloc(data_size * sizeof(short));
    
    float mat1[SIZE][SIZE];
    float mat2[SIZE][SIZE];
    float result[SIZE][SIZE];
    
    /* Initialize data */
    srand(time(NULL));
    
    for (i = 0; i < data_size; i++) {
        fa[i] = (float)rand() / RAND_MAX * 100.0f;
        fb[i] = (float)rand() / RAND_MAX * 100.0f;
        idata[i] = rand() % 1000;
        sdata[i] = (short)(rand() % 1000);
    }
    
    for (i = 0; i < 4; i++) {
        mask[i] = rand();
    }
    
    /* Initialize matrices */
    for (i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            mat1[i][j] = (float)rand() / RAND_MAX;
            mat2[i][j] = (float)rand() / RAND_MAX;
        }
    }
    
    start = clock();
    
    /* Call all functions to ensure they're compiled */
    compute_intensive(fa, fb, fc, data_size);
    integer_processing(idata, mask, data_size);
    int cf_result = complex_control_flow(idata, data_size);
    matrix_operations(mat1, mat2, result, 64); /* Use smaller size for speed */
    mixed_operations(sdata, idata, fc, data_size);
    
    /* Do some computation with results to prevent optimization */
    float total = 0.0f;
    for (i = 0; i < data_size; i++) {
        total += fc[i] + idata[i];
    }
    
    end = clock();
    
    printf("Total: %f\n", total);
    printf("Control flow result: %d\n", cf_result);
    printf("Time: %f seconds\n", (double)(end - start) / CLOCKS_PER_SEC);
    
    /* Cleanup */
    free(fa);
    free(fb);
    free(fc);
    free(idata);
    free(mask);
    free(sdata);
    
    return 0;
}
