/* Complex scheduling test for GCC HAIFA scheduler state save/restore */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

#define ARRAY_SIZE 1024
#define CHASE_SIZE 256
#define UNROLL_FACTOR 8

/* Volatile variables to create scheduling barriers */
static volatile int vol_barrier = 0;
static volatile double vol_double = 0.0;

/* Packed struct to force misaligned accesses */
struct __attribute__((packed)) misaligned_data {
    char c;
    int i;
    double d;
    short s;
};

/* Function pointer type for computed jumps */
typedef double (*compute_func_t)(double, double, int);

/* Various computation kernels */
static double kernel_add(double a, double b, int i) {
    volatile double result = a + b + (i * 0.1);
    asm volatile("" ::: "memory");
    return result * 1.1;
}

static double kernel_mul(double a, double b, int i) {
    volatile double result = a * b * (1.0 + (i % 5) * 0.01);
    asm volatile("" ::: "memory");
    return result / 1.01;
}

static double kernel_mixed(double a, double b, int i) {
    double t1 = a + b;
    double t2 = a - b;
    double t3 = t1 * t2;
    double t4 = t3 / (fabs(b) + 1.0);
    volatile double result = t4 + sin(i * 0.01);
    asm volatile("" ::: "memory");
    return result;
}

/* Non-inlineable function to create scheduling boundary */
__attribute__((noinline)) 
static double complex_calculation(double base, int iterations) {
    double acc = base;
    for (int j = 0; j < iterations; j++) {
        acc = acc * 1.01 + sin(acc * 0.001);
        if (j & 1) {
            acc += cos(acc * 0.0001);
        } else {
            acc -= tan(acc * 0.00001);
        }
    }
    vol_barrier = iterations;
    return acc;
}

/* Pointer chasing simulation */
static double pointer_chase(double *array, int size, int start) {
    double sum = 0.0;
    int idx = start;
    
    for (int i = 0; i < size; i++) {
        /* Create loop-carried dependency */
        sum += array[idx];
        idx = (int)(array[idx] * 1000) % size;
        if (idx < 0) idx = -idx;
        
        /* Memory barrier */
        asm volatile("" ::: "memory");
    }
    return sum;
}

/* Main computation with complex control flow */
static uint64_t compute(int iterations) {
    /* Allocate arrays with different types and alignments */
    double *darray = (double*)aligned_alloc(64, ARRAY_SIZE * sizeof(double));
    float *farray = (float*)malloc(ARRAY_SIZE * sizeof(float));
    int *iarray = (int*)malloc(ARRAY_SIZE * sizeof(int));
    struct misaligned_data *mdata = (struct misaligned_data*)
        malloc(ARRAY_SIZE * sizeof(struct misaligned_data));
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        darray[i] = (i * 1103515245LL) * 0.000001;
        farray[i] = (i * 1103515245LL) * 0.01f;
        iarray[i] = i * 1103515245;
        mdata[i].c = i & 0xFF;
        mdata[i].i = i * 3;
        mdata[i].d = i * 0.5;
        mdata[i].s = i * 2;
    }
    
    /* Array of function pointers for computed jumps */
    compute_func_t funcs[] = {
        kernel_add,
        kernel_mul,
        kernel_mixed,
        kernel_add,
        kernel_mul,
        kernel_mixed,
        kernel_add,
        kernel_mul,
        kernel_mixed,
        kernel_add
    };
    
    double accumulator = 0.0;
    float faccum = 0.0f;
    int iaccum = 0;
    
    /* Main computation loop */
    for (int i = 0; i < iterations; i++) {
        /* Deeply nested conditional chain */
        if (i & 1) {
            if (i & 2) {
                if (i & 4) {
                    if (i & 8) {
                        /* Branch 1: Pointer chasing */
                        accumulator += pointer_chase(darray, CHASE_SIZE, i % ARRAY_SIZE);
                    } else {
                        /* Branch 2: Chain of dependent operations */
                        double a = darray[i % ARRAY_SIZE];
                        double b = farray[i % ARRAY_SIZE] * 2.0;
                        double c = a * b + iaccum;
                        double d = c / (fabs(a) + 1.0);
                        double e = d * d - b * b;
                        accumulator += e * 0.5;
                    }
                } else {
                    /* Branch 3: Call non-inlineable function */
                    accumulator += complex_calculation(darray[i % ARRAY_SIZE], 5);
                }
            } else {
                /* Branch 4: Mixed type operations */
                faccum += farray[i % ARRAY_SIZE] * 1.1f;
                iaccum += iarray[(i * 7) % ARRAY_SIZE];
                accumulator += faccum + iaccum;
            }
        } else {
            /* Branch 5: Large sequential block with independent operations */
            /* This should fill the instruction queue */
            int base = i % (ARRAY_SIZE - UNROLL_FACTOR);
            for (int j = 0; j < UNROLL_FACTOR; j++) {
                darray[base + j] = darray[base + j] * 1.01 + j * 0.1;
                farray[base + j] = farray[base + j] * 1.1f - j * 0.01f;
                iarray[base + j] = iarray[base + j] + j * 3;
                mdata[base + j].d = mdata[base + j].d * 0.99 + j * 0.001;
            }
        }
        
        /* Switch statement with many cases */
        switch (i % 10) {
            case 0: {
                double a = accumulator * 0.1;
                double b = faccum * 2.0;
                accumulator = funcs[0](a, b, i);
                break;
            }
            case 1: {
                int idx = i % ARRAY_SIZE;
                double temp = darray[idx] + farray[idx] + iarray[idx];
                accumulator += temp * 0.01;
                break;
            }
            case 2: {
                /* Nested loop with dependency */
                for (int k = 0; k < 4; k++) {
                    accumulator = accumulator * 1.001 + k * 0.0001;
                }
                break;
            }
            case 3: {
                /* Memory intensive */
                memcpy(&darray[i % 16], &darray[(i + 1) % 16], 8 * sizeof(double));
                break;
            }
            case 4: {
                /* Floating point chain */
                double x = sin(accumulator * 0.01);
                double y = cos(faccum * 0.01);
                accumulator = x * x + y * y - 2 * x * y;
                break;
            }
            case 5: {
                /* Integer arithmetic */
                iaccum = iaccum * 1103515245 + 12345;
                accumulator += iaccum * 0.0000001;
                break;
            }
            case 6: {
                /* Mixed operations */
                struct misaligned_data *md = &mdata[i % ARRAY_SIZE];
                accumulator += md->d + md->i * 0.001 + md->s * 0.0001;
                break;
            }
            case 7: {
                /* Another function call */
                accumulator = funcs[2](accumulator, faccum, i);
                break;
            }
            case 8: {
                /* Conditional store */
                if (accumulator > 0) {
                    darray[i % ARRAY_SIZE] = accumulator;
                } else {
                    darray[i % ARRAY_SIZE] = -accumulator;
                }
                break;
            }
            case 9: {
                /* Complex expression */
                accumulator = (accumulator * accumulator - 
                              faccum * faccum) / 
                             (fabs(accumulator) + fabs(faccum) + 1.0);
                break;
            }
        }
        
        /* Volatile write to prevent reordering */
        vol_double = accumulator;
    }
    
    /* Reduction across all data */
    uint64_t result = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        /* Mix different types into final result */
        result ^= *(uint64_t*)&darray[i];
        result ^= *(uint32_t*)&farray[i];
        result ^= iarray[i];
        result ^= *(uint64_t*)&mdata[i].d;
        
        /* Create more scheduling opportunities */
        if (i & 1) {
            result = (result << 3) | (result >> 61);
        } else {
            result = (result >> 2) | (result << 62);
        }
    }
    
    /* Cleanup */
    free(darray);
    free(farray);
    free(iarray);
    free(mdata);
    
    return result ^ (uint64_t)accumulator ^ (uint64_t)faccum ^ iaccum;
}

int main(int argc, char *argv[]) {
    int iterations = 1000;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations < 100) iterations = 100;
        if (iterations > 1000000) iterations = 1000000;
    }
    
    printf("Running %d iterations...\n", iterations);
    
    uint64_t result = compute(iterations);
    
    /* Print result to prevent dead code elimination */
    printf("Result: 0x%016llx\n", (unsigned long long)result);
    
    return 0;
}
