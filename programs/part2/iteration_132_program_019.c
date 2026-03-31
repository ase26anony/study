/* test_mcf_coverage.c - Program to trigger GCC's min-cost flow debug output */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define ARRAY_SIZE 10000
#define ITERATIONS 100
#define WARMUP_ITERATIONS 10

/* Volatile variables to prevent optimization */
volatile int global_counter = 0;
volatile double global_sum = 0.0;

/* Function 1: Deeply nested loops with many live ranges */
void test_nested_loops(int *data, int size) {
    int i, j, k, l, m;
    int sum1 = 0, sum2 = 0, sum3 = 0, sum4 = 0, sum5 = 0;
    int temp1, temp2, temp3, temp4, temp5;
    double fp1, fp2, fp3, fp4;
    
    /* Complex expression with many intermediate values */
    for (i = 0; i < size; i++) {
        temp1 = data[i] * 3;
        temp2 = data[i] / 2;
        temp3 = temp1 + temp2;
        temp4 = temp3 << 2;
        temp5 = temp4 ^ 0x55AA55AA;
        
        for (j = 0; j < 5; j++) {
            fp1 = (double)temp5 * 1.5;
            fp2 = fp1 / 3.14159;
            fp3 = fp2 * fp2;
            fp4 = fp3 + (double)j;
            
            for (k = 0; k < 3; k++) {
                sum1 += (int)fp4 + k;
                sum2 += temp5 >> k;
                
                for (l = 0; l < 2; l++) {
                    sum3 += data[(i + k + l) % size];
                    sum4 += temp1 * l - temp2;
                    
                    /* Inline assembly with register constraints */
                    asm volatile (
                        "movl %1, %%eax\n\t"
                        "addl %%eax, %0\n\t"
                        : "+r" (sum5)
                        : "r" (temp3)
                        : "%eax", "memory"
                    );
                }
            }
        }
    }
    
    global_sum += (double)(sum1 + sum2 + sum3 + sum4 + sum5);
}

/* Function 2: Complex control flow with switch and early returns */
int test_complex_cfg(int *data, int size) {
    int result = 0;
    int i = 0;
    
    /* Multiple nested if-else with early returns */
    if (size > 1000) {
        if (data[0] > 500) {
            result += 100;
            if (data[1] < 200) return result;  /* Early return */
        } else {
            result -= 50;
        }
    } else {
        result = size * 2;
    }
    
    /* Switch with many cases and fall-through */
    for (i = 0; i < size && i < 20; i++) {
        switch (data[i] % 15) {
            case 0: result += 1; break;
            case 1: result += 2; /* fall through */
            case 2: result += 3; break;
            case 3: result += 4; /* fall through */
            case 4: result += 5; /* fall through */
            case 5: result += 6; break;
            case 6: result += 7; /* fall through */
            case 7: result += 8; break;
            case 8: result += 9; /* fall through */
            case 9: result += 10; /* fall through */
            case 10: result += 11; break;
            case 11: result += 12; /* fall through */
            case 12: result += 13; break;
            case 13: result += 14; /* fall through */
            case 14: result += 15; break;
            default: result -= 1;
        }
        
        /* Loop with break/continue at different levels */
        for (int j = 0; j < 10; j++) {
            if (j == data[i] % 5) {
                continue;
            }
            for (int k = 0; k < 5; k++) {
                if (k == 3) break;
                result += j * k;
            }
        }
    }
    
    /* Computed goto to create irreducible control flow */
    static void *labels[] = { &&label0, &&label1, &&label2, &&label3, &&label4 };
    
    int idx = data[size-1] % 5;
    goto *labels[idx];
    
label0:
    result *= 2;
    goto end;
label1:
    result += 100;
    goto end;
label2:
    result -= 50;
    goto end;
label3:
    result /= 3;
    goto end;
label4:
    result <<= 2;
    goto end;
    
end:
    return result;
}

/* Function 3: Mixed data types and register classes */
void test_mixed_types(short *sdata, int *idata, long *ldata, 
                      float *fdata, double *ddata, int size) {
    /* Variables declared at function scope but used in nested blocks */
    int int_temp1, int_temp2, int_temp3, int_temp4, int_temp5;
    long long_temp1, long_temp2;
    float float_temp1, float_temp2, float_temp3;
    double double_temp1, double_temp2;
    char char_temp;
    
    for (int i = 0; i < size; i++) {
        /* Mixed type computations */
        int_temp1 = idata[i];
        int_temp2 = sdata[i] * 2;
        int_temp3 = int_temp1 + int_temp2;
        
        long_temp1 = ldata[i];
        long_temp2 = int_temp3 * 100L;
        
        float_temp1 = fdata[i];
        float_temp2 = (float)int_temp3 / 7.0f;
        float_temp3 = float_temp1 * float_temp2;
        
        double_temp1 = ddata[i];
        double_temp2 = (double)long_temp2 * 0.01;
        
        /* Complex address calculations */
        char_temp = (char)((idata[(i + 1) % size] + 
                           idata[(i + 2) % size] + 
                           idata[(i + 3) % size]) % 256);
        
        /* Force register pressure with many simultaneous values */
        global_sum += double_temp1 + double_temp2 + float_temp3;
        global_counter += int_temp1 + int_temp2 + int_temp3 + char_temp;
        
        /* Pointer aliasing to prevent optimization */
        int *alias1 = &idata[i];
        int *alias2 = &int_temp1;
        *alias1 = *alias2 + 1;
    }
}

/* Function 4: Many function arguments to stress calling convention */
int test_many_args(int a1, int a2, int a3, int a4, int a5,
                   int a6, int a7, int a8, int a9, int a10,
                   float f1, float f2, float f3, double d1, double d2) {
    /* All arguments compete for registers */
    int sum = a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10;
    float fsum = f1 + f2 + f3;
    double dsum = d1 + d2;
    
    /* Inline assembly competing for specific registers */
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl %2, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=r" (a1)
        : "r" (a2), "r" (a3)
        : "%eax"
    );
    
    asm volatile (
        "movq %1, %%xmm0\n\t"
        "addsd %2, %%xmm0\n\t"
        "movq %%xmm0, %0\n\t"
        : "=r" (d1)
        : "r" (d2), "r" (dsum)
        : "%xmm0"
    );
    
    return sum + (int)fsum + (int)dsum + a1;
}

/* Function 5: Vector/SIMD operations */
void test_simd_operations(float *data, int size) {
    /* Multiple vector accumulators */
    float sum1 = 0.0f, sum2 = 0.0f, sum3 = 0.0f, sum4 = 0.0f;
    float prod1 = 1.0f, prod2 = 1.0f, prod3 = 1.0f, prod4 = 1.0f;
    
    for (int i = 0; i < size; i += 4) {
        /* Simulate SIMD operations */
        float v1 = data[i];
        float v2 = data[i + 1];
        float v3 = data[i + 2];
        float v4 = data[i + 3];
        
        sum1 += v1; sum2 += v2; sum3 += v3; sum4 += v4;
        prod1 *= v1; prod2 *= v2; prod3 *= v3; prod4 *= v4;
        
        /* Cross-element operations */
        float temp1 = v1 * v2 + v3 * v4;
        float temp2 = v1 / v2 - v3 / v4;
        float temp3 = v1 + v2 + v3 + v4;
        float temp4 = v1 - v2 - v3 - v4;
        
        /* More register pressure */
        global_sum += temp1 + temp2 + temp3 + temp4;
    }
    
    global_counter += (int)(sum1 + sum2 + sum3 + sum4);
}

int main() {
    /* Initialize large arrays with random data */
    int *int_data = malloc(ARRAY_SIZE * sizeof(int));
    short *short_data = malloc(ARRAY_SIZE * sizeof(short));
    long *long_data = malloc(ARRAY_SIZE * sizeof(long));
    float *float_data = malloc(ARRAY_SIZE * sizeof(float));
    double *double_data = malloc(ARRAY_SIZE * sizeof(double));
    
    srand(time(NULL));
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_data[i] = rand() % 1000;
        short_data[i] = rand() % 1000;
        long_data[i] = rand() % 1000;
        float_data[i] = (float)rand() / RAND_MAX * 100.0f;
        double_data[i] = (double)rand() / RAND_MAX * 100.0;
    }
    
    printf("Starting register pressure tests...\n");
    
    /* Warm-up iterations for profile feedback */
    for (int iter = 0; iter < WARMUP_ITERATIONS; iter++) {
        test_nested_loops(int_data, ARRAY_SIZE / 10);
        asm volatile("" ::: "memory");  /* Memory barrier */
    }
    
    /* Main test iterations */
    long total_result = 0;
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Call test functions in sequence */
        test_nested_loops(int_data, ARRAY_SIZE / 20);
        asm volatile("" ::: "memory");
        
        total_result += test_complex_cfg(int_data, ARRAY_SIZE / 50);
        asm volatile("" ::: "memory");
        
        test_mixed_types(short_data, int_data, long_data, 
                        float_data, double_data, ARRAY_SIZE / 100);
        asm volatile("" ::: "memory");
        
        /* Test with many arguments */
        total_result += test_many_args(
            rand() % 100, rand() % 100, rand() % 100, rand() % 100, rand() % 100,
            rand() % 100, rand() % 100, rand() % 100, rand() % 100, rand() % 100,
            (float)rand() / RAND_MAX, (float)rand() / RAND_MAX, (float)rand() / RAND_MAX,
            (double)rand() / RAND_MAX, (double)rand() / RAND_MAX
        );
        asm volatile("" ::: "memory");
        
        test_simd_operations(float_data, ARRAY_SIZE / 40);
        asm volatile("" ::: "memory");
        
        /* Progress indicator */
        if (iter % 10 == 0) {
            printf("Iteration %d/%d\n", iter, ITERATIONS);
        }
    }
    
    /* Compute final checksum */
    long checksum = total_result + (long)global_sum + global_counter;
    printf("Final checksum: %ld\n", checksum);
    printf("Global sum: %f\n", global_sum);
    printf("Global counter: %d\n", global_counter);
    
    /* Cleanup */
    free(int_data);
    free(short_data);
    free(long_data);
    free(float_data);
    free(double_data);
    
    return 0;
}
