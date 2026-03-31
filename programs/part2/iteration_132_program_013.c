/* test_mcf_coverage.c
 * 
 * This program creates register pressure intensive code patterns and complex
 * control flow to trigger GCC's min-cost flow solver during register allocation.
 * When compiled with appropriate debug flags, it should cause the print_edge
 * function to output ENTRY/EXIT node information.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#define ARRAY_SIZE 15000
#define ITERATIONS 100
#define WARMUP_ITERATIONS 10

/* Memory barrier to prevent cross-function optimization */
#define MEMORY_BARRIER() asm volatile("" ::: "memory")

/* Function 1: Deeply nested loops with many live ranges */
unsigned long long test1_nested_loops(int *data, int size) {
    unsigned long long sum = 0;
    volatile int keep_alive1, keep_alive2, keep_alive3;
    
    /* Variables declared at function scope but used in nested blocks */
    int temp1, temp2, temp3, temp4, temp5;
    float f1, f2, f3;
    double d1, d2;
    
    for (int i = 0; i < size; i++) {
        temp1 = data[i];
        
        for (int j = 0; j < 8; j++) {
            temp2 = temp1 * j;
            
            for (int k = 0; k < 4; k++) {
                temp3 = temp2 + k;
                f1 = temp3 * 0.5f;
                
                for (int l = 0; l < 2; l++) {
                    temp4 = (int)(f1 * l);
                    f2 = sinf(f1 + temp4);
                    
                    /* Complex expression with many intermediates */
                    d1 = (double)temp4 * 1.5;
                    d2 = cos(d1 * 0.3);
                    f3 = (float)(d1 + d2) * 2.0f;
                    
                    temp5 = (int)(f3 * 100.0f);
                    sum += temp5;
                }
            }
        }
        
        /* Keep variables alive across loop iterations */
        keep_alive1 = temp1;
        keep_alive2 = temp5;
        keep_alive3 = (int)f3;
    }
    
    return sum;
}

/* Function 2: Complex control flow with switch and early returns */
int test2_complex_cfg(int *data, int size) {
    int result = 0;
    int *ptr1 = data;
    int *ptr2 = data + size/2;
    
    /* Create pointer aliases to prevent optimization */
    volatile int *alias1 = ptr1;
    volatile int *alias2 = ptr2;
    
    for (int i = 0; i < size; i++) {
        int val = data[i];
        
        /* Nested if-else chain with early returns */
        if (val < 0) {
            if (val < -1000) return -1;
            if (val < -500) {
                result += val * 2;
                continue;
            }
        } else if (val > 0) {
            if (val > 1000) {
                result += val / 2;
                goto special_case;
            }
        }
        
        /* Switch with many cases and fall-through */
        switch (val % 13) {
            case 0: result += val; break;
            case 1: result += val * 2; /* fall through */
            case 2: result += val * 3; break;
            case 3: result += val * 4; /* fall through */
            case 4: result += val * 5; /* fall through */
            case 5: result += val * 6; break;
            case 6: result += val * 7; /* fall through */
            case 7: result += val * 8; /* fall through */
            case 8: result += val * 9; break;
            case 9: result += val * 10; /* fall through */
            case 10: result += val * 11; /* fall through */
            case 11: result += val * 12; break;
            case 12: result += val * 13; break;
            default: result -= val; break;
        }
        
        special_case:
        /* Use aliases to extend live ranges */
        *alias1 = result;
        *alias2 = val;
    }
    
    return result;
}

/* Function 3: Inline assembly with register constraints */
long long test3_asm_constraints(int *data, int size) {
    long long total = 0;
    int a, b, c, d;
    
    for (int i = 0; i < size; i += 4) {
        /* Multiple asm statements competing for registers */
        asm volatile (
            "movl %1, %%eax\n\t"
            "imull %2, %%eax\n\t"
            "movl %%eax, %0\n\t"
            : "=r" (a)
            : "r" (data[i]), "r" (i)
            : "%eax", "memory"
        );
        
        asm volatile (
            "movl %1, %%ebx\n\t"
            "addl %2, %%ebx\n\t"
            "movl %%ebx, %0\n\t"
            : "=r" (b)
            : "r" (data[i+1]), "r" (a)
            : "%ebx", "memory"
        );
        
        asm volatile (
            "movl %1, %%ecx\n\t"
            "subl %2, %%ecx\n\t"
            "movl %%ecx, %0\n\t"
            : "=r" (c)
            : "r" (data[i+2]), "r" (b)
            : "%ecx", "memory"
        );
        
        asm volatile (
            "movl %1, %%edx\n\t"
            "xorl %2, %%edx\n\t"
            "movl %%edx, %0\n\t"
            : "=r" (d)
            : "r" (data[i+3]), "r" (c)
            : "%edx", "memory"
        );
        
        total += (long long)a + b + c + d;
    }
    
    return total;
}

/* Function 4: Mixed data types and many function arguments */
double test4_mixed_types(char *cdata, short *sdata, int *idata, 
                         long *ldata, float *fdata, double *ddata, int size) {
    double result = 0.0;
    
    /* Many local variables of different types */
    char c1, c2, c3;
    short s1, s2, s3;
    int i1, i2, i3, i4, i5;
    long l1, l2;
    float f1, f2, f3, f4;
    double d1, d2, d3, d4, d5;
    
    for (int idx = 0; idx < size; idx++) {
        /* Mixed type computations */
        c1 = cdata[idx];
        s1 = sdata[idx];
        i1 = idata[idx];
        l1 = ldata[idx];
        f1 = fdata[idx];
        d1 = ddata[idx];
        
        /* Complex expression chain */
        c2 = c1 + (char)(idx % 256);
        s2 = s1 * (short)(c2 + 1);
        i2 = i1 + (int)s2 * 3;
        l2 = l1 * (long)i2 / 7;
        f2 = f1 * (float)l2 * 0.01f;
        d2 = d1 + (double)f2 * 1.5;
        
        /* More computations keeping many values live */
        c3 = (char)(d2 * 0.1);
        s3 = (short)(d2 * 0.2);
        i3 = (int)(d2 * 0.3);
        i4 = i3 * i2 + i1;
        i5 = i4 - i3 * 2;
        
        f3 = (float)i5 * 0.25f;
        f4 = f3 + f2 - f1;
        
        d3 = (double)f4 * 2.0;
        d4 = d3 + d2 + d1;
        d5 = sin(d4) * cos(d3);
        
        result += d5;
        
        /* Force all variables to be considered live */
        MEMORY_BARRIER();
        cdata[idx] = c3;
        sdata[idx] = s3;
        idata[idx] = i5;
        ldata[idx] = l2;
        fdata[idx] = f4;
        ddata[idx] = d5;
    }
    
    return result;
}

/* Function 5: Irreducible control flow with computed goto */
int test5_irreducible_cfg(int *data, int size) {
    static void *labels[] = {
        &&label0, &&label1, &&label2, &&label3, &&label4,
        &&label5, &&label6, &&label7, &&label8, &&label9
    };
    
    int result = 0;
    int state = 0;
    
    for (int i = 0; i < size; i++) {
        int val = data[i];
        
        /* Jump table based on complex condition */
        int label_idx = (abs(val) * 7 + i * 3) % 10;
        
        goto *labels[label_idx];
        
        label0:
            result += val * 2;
            state = (state + 1) % 5;
            continue;
        label1:
            result += val * 3;
            if (state == 0) goto label9;
            state = (state + 2) % 5;
            continue;
        label2:
            result += val * 4;
            if (val > 0) goto label5;
            state = (state + 3) % 5;
            continue;
        label3:
            result += val * 5;
            state = (state + 4) % 5;
            continue;
        label4:
            result += val * 6;
            if (val % 2 == 0) goto label7;
            state = (state + 1) % 5;
            continue;
        label5:
            result += val * 7;
            state = (state + 2) % 5;
            continue;
        label6:
            result += val * 8;
            if (state == 3) goto label2;
            state = (state + 3) % 5;
            continue;
        label7:
            result += val * 9;
            state = (state + 4) % 5;
            continue;
        label8:
            result += val * 10;
            if (val < 0) goto label4;
            state = (state + 1) % 5;
            continue;
        label9:
            result += val * 11;
            state = (state + 2) % 5;
            continue;
    }
    
    return result;
}

/* Function 6: Many function calls within loops */
float test6_many_calls(float *data, int size) {
    float result = 0.0f;
    
    for (int i = 0; i < size; i++) {
        /* Multiple function calls creating call-clobbered conflicts */
        float x = data[i];
        float y = sinf(x);
        float z = cosf(x + y);
        float w = tanf(x * y * z);
        
        result += y * z * w;
        
        /* More calls with different arguments */
        float a = expf(result);
        float b = logf(fabsf(a) + 1.0f);
        float c = powf(b, 2.0f);
        
        result += a + b + c;
        
        /* Store back to prevent dead code elimination */
        data[i] = result;
    }
    
    return result;
}

/* Function 7: Vector operations stressing SIMD registers */
#ifdef __SSE2__
#include <emmintrin.h>
__m128 test7_simd_ops(float *data, int size) {
    __m128 sum = _mm_setzero_ps();
    __m128 a, b, c, d, e, f, g, h;
    
    for (int i = 0; i < size; i += 8) {
        /* Load multiple vectors */
        a = _mm_loadu_ps(&data[i]);
        b = _mm_loadu_ps(&data[i+4]);
        
        /* Many SIMD operations keeping vectors live */
        c = _mm_add_ps(a, b);
        d = _mm_mul_ps(a, b);
        e = _mm_sub_ps(c, d);
        f = _mm_div_ps(e, a);
        g = _mm_sqrt_ps(f);
        h = _mm_max_ps(g, b);
        
        /* Complex dependency chain */
        a = _mm_add_ps(a, h);
        b = _mm_mul_ps(b, g);
        c = _mm_sub_ps(c, f);
        d = _mm_div_ps(d, e);
        
        /* Accumulate results */
        sum = _mm_add_ps(sum, a);
        sum = _mm_add_ps(sum, b);
        sum = _mm_add_ps(sum, c);
        sum = _mm_add_ps(sum, d);
    }
    
    return sum;
}
#endif

/* Main function that orchestrates all tests */
int main() {
    /* Initialize large arrays with random data */
    int *int_data = malloc(ARRAY_SIZE * sizeof(int));
    char *char_data = malloc(ARRAY_SIZE * sizeof(char));
    short *short_data = malloc(ARRAY_SIZE * sizeof(short));
    long *long_data = malloc(ARRAY_SIZE * sizeof(long));
    float *float_data = malloc(ARRAY_SIZE * sizeof(float));
    double *double_data = malloc(ARRAY_SIZE * sizeof(double));
    
    srand(time(NULL));
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_data[i] = rand() % 10000 - 5000;
        char_data[i] = rand() % 256;
        short_data[i] = rand() % 1000;
        long_data[i] = rand() % 10000;
        float_data[i] = (float)rand() / RAND_MAX * 100.0f;
        double_data[i] = (double)rand() / RAND_MAX * 100.0;
    }
    
    unsigned long long total_checksum = 0;
    
    /* Warm-up iterations for profile feedback */
    for (int warmup = 0; warmup < WARMUP_ITERATIONS; warmup++) {
        MEMORY_BARRIER();
        test1_nested_loops(int_data, 1000);
    }
    
    /* Run all test functions multiple times */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        MEMORY_BARRIER();
        
        total_checksum += test1_nested_loops(int_data, ARRAY_SIZE);
        
        MEMORY_BARRIER();
        total_checksum += test2_complex_cfg(int_data, ARRAY_SIZE);
        
        MEMORY_BARRIER();
        total_checksum += test3_asm_constraints(int_data, ARRAY_SIZE);
        
        MEMORY_BARRIER();
        total_checksum += (unsigned long long)test4_mixed_types(
            char_data, short_data, int_data, 
            long_data, float_data, double_data, 
            ARRAY_SIZE / 10
        );
        
        MEMORY_BARRIER();
        total_checksum += test5_irreducible_cfg(int_data, ARRAY_SIZE);
        
        MEMORY_BARRIER();
        total_checksum += (unsigned long long)test6_many_calls(float_data, ARRAY_SIZE);
        
        #ifdef __SSE2__
        MEMORY_BARRIER();
        __m128 simd_result = test7_simd_ops(float_data, ARRAY_SIZE);
        float simd_sum = simd_result[0] + simd_result[1] + simd_result[2] + simd_result[3];
        total_checksum += (unsigned long long)simd_sum;
        #endif
        
        /* Modify data slightly each iteration */
        for (int i = 0; i < ARRAY_SIZE; i += 7) {
            int_data[i] += iter;
            float_data[i] += iter * 0.1f;
        }
    }
    
    /* Print verifiable result */
    printf("Total checksum: %llu\n", total_checksum);
    
    /* Cleanup */
    free(int_data);
    free(char_data);
    free(short_data);
    free(long_data);
    free(float_data);
    free(double_data);
    
    return 0;
}
