/* mcf_test.c - Test program to trigger min-cost flow debug output in GCC */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <math.h>

#define ARRAY_SIZE 10000
#define ITERATIONS 100
#define WARMUP_ITERATIONS 10

/* Volatile variables to prevent optimization */
volatile int global_counter = 0;
volatile double global_sum = 0.0;

/* Complex structure to force register pressure */
struct DataBlock {
    int ints[16];
    double doubles[8];
    float floats[12];
    char bytes[32];
    short shorts[24];
    long long longs[4];
};

/* Function 1: Deeply nested loops with many live ranges */
void test_nested_loops(struct DataBlock *data, int size) {
    volatile double sum = 0.0;
    volatile int count = 0;
    
    /* Outer loop creates many live ranges */
    for (int i = 0; i < size; i++) {
        int temp1 = data[i].ints[0];
        int temp2 = data[i].ints[1];
        double dtemp1 = data[i].doubles[0];
        double dtemp2 = data[i].doubles[1];
        float ftemp1 = data[i].floats[0];
        float ftemp2 = data[i].floats[1];
        
        /* Middle loop with complex expressions */
        for (int j = 0; j < 8; j++) {
            int inner_temp1 = temp1 * j + temp2;
            int inner_temp2 = temp2 * j - temp1;
            double inner_dtemp = dtemp1 * j + dtemp2;
            float inner_ftemp = ftemp1 * j + ftemp2;
            
            /* Innermost loop with register-intensive operations */
            for (int k = 0; k < 4; k++) {
                /* Many intermediate values requiring registers */
                int result1 = inner_temp1 * k + inner_temp2;
                int result2 = inner_temp2 * k - inner_temp1;
                double dresult = inner_dtemp * k + inner_ftemp;
                float fresult = inner_ftemp * k + inner_dtemp;
                
                /* Complex expression chain */
                sum += (result1 * dresult) / (result2 + 1) + fresult;
                count += result1 ^ result2;
                
                /* Force register pressure with bit operations */
                data[i].bytes[k] = (result1 & 0xFF) | (result2 & 0xFF00) >> 8;
                data[i].shorts[j] = (short)(result1 + result2);
            }
            
            /* More register usage in middle loop */
            temp1 = inner_temp1 ^ inner_temp2;
            temp2 = inner_temp1 & inner_temp2;
            dtemp1 = inner_dtemp * 0.5;
            dtemp2 = inner_dtemp * 0.25;
        }
        
        /* Final calculations extending live ranges */
        data[i].longs[0] = (long long)sum * count;
        data[i].longs[1] = (long long)temp1 * temp2;
    }
    
    global_sum += sum;
    global_counter += count;
}

/* Function 2: Complex control flow with switch and early returns */
int test_complex_cfg(struct DataBlock *data, int size, int mode) {
    volatile int result = 0;
    volatile double accumulator = 0.0;
    
    /* Multiple variables declared at function scope */
    int var1 = 0, var2 = 0, var3 = 0, var4 = 0, var5 = 0;
    double dvar1 = 0.0, dvar2 = 0.0, dvar3 = 0.0;
    float fvar1 = 0.0f, fvar2 = 0.0f, fvar3 = 0.0f;
    
    for (int i = 0; i < size; i++) {
        /* Complex switch with many cases (creates CFG complexity) */
        switch (data[i].ints[0] % 15) {
            case 0:
                var1 = data[i].ints[1];
                var2 = data[i].ints[2];
                if (var1 > var2) return var1 - var2; /* Early return */
                break;
            case 1:
                var3 = data[i].ints[3];
                var4 = data[i].ints[4];
                dvar1 = data[i].doubles[0];
                if (dvar1 > 100.0) continue; /* Continue statement */
                break;
            case 2:
                var5 = data[i].ints[5];
                fvar1 = data[i].floats[0];
                if (fvar1 < 0.0) break;
                /* Fall through */
            case 3:
                dvar2 = data[i].doubles[1];
                fvar2 = data[i].floats[1];
                accumulator += dvar2 * fvar2;
                break;
            case 4:
                var1 = var2 * var3;
                var2 = var4 / (var5 + 1);
                if (var2 == 0) goto cleanup; /* goto creates irreducible CFG */
                break;
            case 5:
                dvar3 = data[i].doubles[2];
                fvar3 = data[i].floats[2];
                result += (int)(dvar3 * fvar3);
                break;
            case 6:
                /* Nested if-else chain */
                if (data[i].ints[6] > 0) {
                    if (data[i].ints[7] < 0) {
                        var1 = -var1;
                    } else if (data[i].ints[8] == 0) {
                        var2 = var2 * 2;
                    } else {
                        var3 = var3 / 2;
                    }
                } else {
                    if (data[i].ints[9] > 100) {
                        var4 = var4 + 100;
                    } else {
                        var5 = var5 - 50;
                    }
                }
                break;
            case 7:
                var1 = var2 + var3;
                var2 = var4 - var5;
                var3 = var1 * var2;
                var4 = var3 / (var1 + 1);
                var5 = var4 % 256;
                break;
            case 8:
                dvar1 = dvar2 * dvar3;
                dvar2 = dvar1 / (dvar3 + 1.0);
                dvar3 = sqrt(dvar1 * dvar1 + dvar2 * dvar2);
                break;
            case 9:
                fvar1 = fvar2 + fvar3;
                fvar2 = fvar1 * 0.5f;
                fvar3 = fvar2 * 0.25f;
                break;
            case 10:
                result += var1 + var2 + var3 + var4 + var5;
                break;
            case 11:
                accumulator += dvar1 + dvar2 + dvar3;
                break;
            case 12:
                /* Pointer aliasing to prevent optimization */
                int *alias1 = &var1;
                int *alias2 = &var2;
                *alias1 = *alias2 + data[i].ints[10];
                *alias2 = *alias1 - data[i].ints[11];
                break;
            case 13:
                /* Mixed type operations */
                result += (int)(dvar1 * fvar1) + var1;
                accumulator += var2 * dvar2 + fvar2;
                break;
            case 14:
                /* Complex expression with many intermediates */
                result += ((var1 * var2) + (var3 * var4)) / (var5 + 1) +
                          (int)((dvar1 * dvar2) / (dvar3 + 1.0)) +
                          (int)(fvar1 * fvar2 * fvar3);
                break;
        }
        
        /* More operations extending live ranges */
        data[i].ints[12] = var1 + var2;
        data[i].ints[13] = var3 * var4;
        data[i].ints[14] = var5 ^ result;
        data[i].doubles[3] = dvar1 + dvar2 + dvar3;
        data[i].floats[3] = fvar1 * fvar2 + fvar3;
    }
    
cleanup:
    return result + (int)accumulator;
}

/* Function 3: Inline assembly with explicit register constraints */
void test_inline_asm(struct DataBlock *data, int size) {
    volatile int a, b, c, d, e, f;
    volatile double x, y, z;
    
    for (int i = 0; i < size; i++) {
        /* Multiple asm statements competing for registers */
        asm volatile (
            "movl %1, %%eax\n\t"
            "addl %2, %%eax\n\t"
            "movl %%eax, %0\n\t"
            : "=r" (a)
            : "r" (data[i].ints[0]), "r" (data[i].ints[1])
            : "%eax", "memory"
        );
        
        asm volatile (
            "movl %1, %%ebx\n\t"
            "imull %2, %%ebx\n\t"
            "movl %%ebx, %0\n\t"
            : "=r" (b)
            : "r" (data[i].ints[2]), "r" (data[i].ints[3])
            : "%ebx", "memory"
        );
        
        asm volatile (
            "movq %1, %%xmm0\n\t"
            "addsd %2, %%xmm0\n\t"
            "movq %%xmm0, %0\n\t"
            : "=x" (x)
            : "x" (data[i].doubles[0]), "x" (data[i].doubles[1])
            : "%xmm0", "memory"
        );
        
        asm volatile (
            "movl %1, %%ecx\n\t"
            "xorl %2, %%ecx\n\t"
            "movl %%ecx, %0\n\t"
            : "=r" (c)
            : "r" (data[i].ints[4]), "r" (data[i].ints[5])
            : "%ecx", "memory"
        );
        
        asm volatile (
            "movl %1, %%edx\n\t"
            "andl %2, %%edx\n\t"
            "movl %%edx, %0\n\t"
            : "=r" (d)
            : "r" (data[i].ints[6]), "r" (data[i].ints[7])
            : "%edx", "memory"
        );
        
        /* More register pressure */
        asm volatile (
            "movss %1, %%xmm1\n\t"
            "mulss %2, %%xmm1\n\t"
            "movss %%xmm1, %0\n\t"
            : "=x" (y)
            : "x" (data[i].floats[0]), "x" (data[i].floats[1])
            : "%xmm1", "memory"
        );
        
        /* Complex sequence with multiple clobbered registers */
        asm volatile (
            "movl %2, %%eax\n\t"
            "movl %3, %%ebx\n\t"
            "addl %%ebx, %%eax\n\t"
            "movl %%eax, %0\n\t"
            "movl %4, %%ecx\n\t"
            "subl %%ecx, %%eax\n\t"
            "movl %%eax, %1\n\t"
            : "=r" (e), "=r" (f)
            : "r" (a), "r" (b), "r" (c)
            : "%eax", "%ebx", "%ecx", "memory"
        );
        
        /* Use all computed values */
        data[i].ints[15] = a + b + c + d + e + f;
        data[i].doubles[7] = x * y + z;
    }
}

/* Function 4: Many function arguments to stress calling convention */
double test_many_args(int a1, int a2, int a3, int a4, int a5,
                      int a6, int a7, int a8, int a9, int a10,
                      double d1, double d2, double d3, double d4,
                      float f1, float f2, float f3, float f4) {
    /* Many local variables competing with arguments */
    volatile int l1 = a1, l2 = a2, l3 = a3, l4 = a4, l5 = a5;
    volatile int l6 = a6, l7 = a7, l8 = a8, l9 = a9, l10 = a10;
    volatile double ld1 = d1, ld2 = d2, ld3 = d3, ld4 = d4;
    volatile float lf1 = f1, lf2 = f2, lf3 = f3, lf4 = f4;
    
    /* Complex expression using all variables */
    double result = (l1 * l2) + (l3 * l4) - (l5 * l6) + (l7 / l8) - (l9 % l10);
    result += (ld1 * ld2) / (ld3 + ld4);
    result += (lf1 * lf2) + (lf3 / lf4);
    
    /* Nested calls to increase register pressure */
    for (int i = 0; i < 10; i++) {
        result += sin(ld1) * cos(ld2) + tan(ld3);
        ld1 += 0.1;
        ld2 -= 0.1;
        ld3 *= 1.01;
    }
    
    return result;
}

/* Function 5: Vector/SIMD like operations */
void test_vector_ops(struct DataBlock *data, int size) {
    volatile int sum1 = 0, sum2 = 0, sum3 = 0, sum4 = 0;
    volatile double dsum1 = 0.0, dsum2 = 0.0, dsum3 = 0.0, dsum4 = 0.0;
    
    /* Unrolled loop with multiple accumulators */
    for (int i = 0; i < size; i += 4) {
        /* Process 4 elements at once (SIMD-like) */
        int v1 = data[i].ints[0];
        int v2 = data[i + 1].ints[0];
        int v3 = data[i + 2].ints[0];
        int v4 = data[i + 3].ints[0];
        
        double dv1 = data[i].doubles[0];
        double dv2 = data[i + 1].doubles[0];
        double dv3 = data[i + 2].doubles[0];
        double dv4 = data[i + 3].doubles[0];
        
        /* Multiple parallel operations */
        sum1 += v1 * v1;
        sum2 += v2 * v2;
        sum3 += v3 * v3;
        sum4 += v4 * v4;
        
        dsum1 += dv1 * dv1;
        dsum2 += dv2 * dv2;
        dsum3 += dv3 * dv3;
        dsum4 += dv4 * dv4;
        
        /* Cross-element operations */
        data[i].ints[1] = v1 + v2;
        data[i + 1].ints[1] = v2 + v3;
        data[i + 2].ints[1] = v3 + v4;
        data[i + 3].ints[1] = v4 + v1;
        
        data[i].doubles[1] = dv1 + dv2;
        data[i + 1].doubles[1] = dv2 + dv3;
        data[i + 2].doubles[1] = dv3 + dv4;
        data[i + 3].doubles[1] = dv4 + dv1;
    }
    
    /* Reduce accumulators */
    global_counter += sum1 + sum2 + sum3 + sum4;
    global_sum += dsum1 + dsum2 + dsum3 + dsum4;
}

/* Main function with warm-up and verification */
int main() {
    struct DataBlock *data = malloc(ARRAY_SIZE * sizeof(struct DataBlock));
    if (!data) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with random data */
    srand(time(NULL));
    for (int i = 0; i < ARRAY_SIZE; i++) {
        for (int j = 0; j < 16; j++) {
            data[i].ints[j] = rand() % 1000;
        }
        for (int j = 0; j < 8; j++) {
            data[i].doubles[j] = (double)rand() / RAND_MAX * 100.0;
        }
        for (int j = 0; j < 12; j++) {
            data[i].floats[j] = (float)rand() / RAND_MAX * 100.0f;
        }
        for (int j = 0; j < 32; j++) {
            data[i].bytes[j] = rand() % 256;
        }
        for (int j = 0; j < 24; j++) {
            data[i].shorts[j] = rand() % 65536;
        }
        for (int j = 0; j < 4; j++) {
            data[i].longs[j] = rand() % 1000000;
        }
    }
    
    printf("Starting register pressure tests...\n");
    
    /* Warm-up iterations for profile feedback */
    for (int iter = 0; iter < WARMUP_ITERATIONS; iter++) {
        test_nested_loops(data, 100);
        asm volatile("" ::: "memory"); /* Memory barrier */
    }
    
    long long total_checksum = 0;
    
    /* Main test iterations */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Test 1: Nested loops */
        test_nested_loops(data, ARRAY_SIZE / 10);
        asm volatile("" ::: "memory");
        
        /* Test 2: Complex control flow */
        int cfg_result = test_complex_cfg(data, ARRAY_SIZE / 20, iter % 3);
        total_checksum += cfg_result;
        asm volatile("" ::: "memory");
        
        /* Test 3: Inline assembly */
        test_inline_asm(data, ARRAY_SIZE / 40);
        asm volatile("" ::: "memory");
        
        /* Test 4: Many arguments */
        double arg_result = test_many_args(
            data[iter % ARRAY_SIZE].ints[0],
            data[iter % ARRAY_SIZE].ints[1],
            data[iter % ARRAY_SIZE].ints[2],
            data[iter % ARRAY_SIZE].ints[3],
            data[iter % ARRAY_SIZE].ints[4],
            data[iter % ARRAY_SIZE].ints[5],
            data[iter % ARRAY_SIZE].ints[6],
            data[iter % ARRAY_SIZE].ints[7],
            data[iter % ARRAY_SIZE].ints[8],
            data[iter % ARRAY_SIZE].ints[9],
            data[iter % ARRAY_SIZE].doubles[0],
            data[iter % ARRAY_SIZE].doubles[1],
            data[iter % ARRAY_SIZE].doubles[2],
            data[iter % ARRAY_SIZE].doubles[3],
            data[iter % ARRAY_SIZE].floats[0],
            data[iter % ARRAY_SIZE].floats[1],
            data[iter % ARRAY_SIZE].floats[2],
            data[iter % ARRAY_SIZE].floats[3]
        );
        total_checksum += (long long)arg_result;
        asm volatile("" ::: "memory");
        
        /* Test 5: Vector operations */
        test_vector_ops(data, ARRAY_SIZE / 5);
        asm volatile("" ::: "memory");
        
        /* Compute checksum of modified data */
        for (int i = 0; i < ARRAY_SIZE / 100; i++) {
            total_checksum += data[i].ints[0] + data[i].ints[15];
            total_checksum += (int)data[i].doubles[0] + (int)data[i].doubles[7];
        }
    }
    
    /* Final verification */
    printf("Final checksum: %lld\n", total_checksum);
    printf("Global counter: %d\n", global_counter);
    printf("Global sum: %f\n", global_sum);
    
    free(data);
    return 0;
}
