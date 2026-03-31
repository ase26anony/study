/* test_scheduler_coverage.c
 * 
 * This program creates complex basic blocks that force GCC's Haifa scheduler
 * to allocate and use the full scheduling context, ensuring the cleanup
 * code in free_sched_block() is executed.
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

/* Force inlining to create larger basic blocks */
#define ALWAYS_INLINE __attribute__((always_inline))

/* Vector types to create parallel operations */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Global variables to prevent optimization */
volatile int global_seed = 42;
volatile float global_accumulator = 0.0f;

/* Function with side effects to create scheduling barriers */
static ALWAYS_INLINE int get_next_value(int *state) {
    *state = (*state * 1103515245 + 12345) & 0x7fffffff;
    return *state;
}

/* Complex integer computation with dependency chain */
static ALWAYS_INLINE int test_integer_deps(int a, int b, int c, int d, int e) {
    /* Long dependency chain */
    int t1 = a + b;
    int t2 = t1 * c;
    int t3 = t2 - d;
    int t4 = t3 / (e + 1);
    int t5 = t4 << 2;
    int t6 = t5 ^ t1;
    int t7 = t6 & 0xFF;
    int t8 = t7 | t2;
    int t9 = t8 * t3;
    int t10 = t9 - t4;
    return t10;
}

/* Mixed integer/float operations */
static ALWAYS_INLINE float test_mixed_ops(int a, float b, double c, int d) {
    float f1 = (float)a * b;
    double d1 = (double)f1 + c;
    int i1 = (int)d1 * d;
    float f2 = (float)i1 / b;
    double d2 = d1 - (double)f2;
    float f3 = f1 + (float)d2;
    int i2 = i1 ^ (int)f3;
    return f3 * (float)i2;
}

/* Memory-intensive computation with potential aliasing */
static ALWAYS_INLINE void process_array(int *arr, float *farr, int n) {
    for (int i = 0; i < n - 1; i++) {
        /* Create dependencies between array elements */
        arr[i + 1] = arr[i] * 3 - arr[i + 1];
        farr[i] = sqrtf(fabsf((float)arr[i])) + farr[i + 1];
        
        /* Additional operations to increase block size */
        if (i % 4 == 0) {
            arr[i] ^= 0x55555555;
            farr[i] *= 1.5f;
        }
    }
}

/* Function with speculative scheduling opportunities */
static ALWAYS_INLINE int speculative_computation(int x, int y, int *branch_taken) {
    int result = 0;
    
    /* Multiple conditional updates */
    if (x > y) {
        result = x * y - (x >> 3);
        *branch_taken = 1;
    } else if (x < y) {
        result = y / (x + 1) + (y & 0xFF);
        *branch_taken = 2;
    } else {
        result = x ^ y;
        *branch_taken = 3;
    }
    
    /* More operations after conditionals */
    result = (result * 3) & 0xFFFF;
    result |= (x << 16);
    result ^= (y * 7);
    
    return result;
}

/* Wide basic block with many independent operations */
static ALWAYS_INLINE void wide_basic_block(int *inputs, float *outputs, int count) {
    /* Create many parallel computation chains */
    int chain1 = inputs[0];
    int chain2 = inputs[1];
    float fchain1 = outputs[0];
    float fchain2 = outputs[1];
    
    /* Independent integer chains */
    for (int i = 0; i < 8; i++) {
        chain1 = chain1 * 3 - i;
        chain2 = chain2 + chain1 ^ 0xAA;
    }
    
    /* Independent float chains */
    for (int i = 0; i < 8; i++) {
        fchain1 = fchain1 * 1.1f + sinf((float)i);
        fchain2 = fchain2 / 1.1f - cosf((float)i);
    }
    
    /* Mix them together */
    outputs[0] = fchain1 + (float)chain1;
    outputs[1] = fchain2 * (float)chain2;
    
    /* More independent operations */
    int temp[4];
    temp[0] = inputs[0] << 2;
    temp[1] = inputs[1] >> 1;
    temp[2] = inputs[0] & inputs[1];
    temp[3] = inputs[0] | inputs[1];
    
    /* Use all results to prevent elimination */
    outputs[2] = (float)(temp[0] + temp[1] + temp[2] + temp[3]);
}

/* Vector operations for SIMD scheduling */
static ALWAYS_INLINE v4sf vector_operations(v4si vi, v4sf vf) {
    v4si vi2 = vi + (v4si){1, 2, 3, 4};
    v4sf vf2 = vf * (v4sf){1.1f, 2.2f, 3.3f, 4.4f};
    
    v4si vi3 = vi2 * (v4si){2, 3, 4, 5};
    v4sf vf3 = vf2 + (v4sf){0.5f, 1.5f, 2.5f, 3.5f};
    
    /* Mix vector types */
    v4sf result = vf3 + __builtin_convertvector(vi3, v4sf);
    
    return result * vf;
}

/* Main test function with multiple complex basic blocks */
void test_function_1(int iterations) {
    int state = global_seed;
    float accum = 0.0f;
    
    /* Block 1: Mixed operations with dependencies */
    for (int i = 0; i < iterations; i++) {
        int a = get_next_value(&state);
        int b = get_next_value(&state);
        float c = (float)get_next_value(&state) / 1000.0f;
        
        int int_result = test_integer_deps(a, b, a & 0xFF, b >> 4, i);
        float float_result = test_mixed_ops(int_result, c, (double)c * 2.0, b);
        
        accum += float_result;
        
        /* Conditional with speculative scheduling */
        int branch_taken;
        int spec_result = speculative_computation(a, b, &branch_taken);
        accum += (float)spec_result / 1000.0f;
    }
    
    /* Block 2: Memory operations with aliasing */
    int arr[64];
    float farr[64];
    
    for (int i = 0; i < 64; i++) {
        arr[i] = get_next_value(&state);
        farr[i] = (float)arr[i] / 1000.0f;
    }
    
    process_array(arr, farr, 64);
    
    /* Block 3: Wide basic block */
    wide_basic_block(arr, farr, 64);
    
    /* Block 4: Vector operations */
    v4si vi = {arr[0], arr[1], arr[2], arr[3]};
    v4sf vf = {farr[0], farr[1], farr[2], farr[3]};
    v4sf vresult = vector_operations(vi, vf);
    
    /* Use results to prevent elimination */
    for (int i = 0; i < 4; i++) {
        accum += vresult[i];
    }
    
    global_accumulator += accum;
}

/* Second test function with different patterns */
void test_function_2(int size) {
    int *data = malloc(size * sizeof(int));
    float *fdata = malloc(size * sizeof(float));
    
    if (!data || !fdata) return;
    
    /* Initialize with pattern */
    for (int i = 0; i < size; i++) {
        data[i] = i * 7 + 3;
        fdata[i] = sinf((float)i * 0.1f);
    }
    
    /* Complex loop with small iteration count for software pipelining */
    for (int outer = 0; outer < 4; outer++) {
        for (int i = 0; i < size - 4; i++) {
            /* Create web of dependencies */
            int t1 = data[i] + data[i + 1];
            int t2 = data[i + 2] - data[i + 3];
            float ft1 = fdata[i] * fdata[i + 1];
            float ft2 = fdata[i + 2] / fdata[i + 3];
            
            data[i] = t1 ^ t2;
            fdata[i] = ft1 + ft2;
            
            /* More operations to increase scheduling complexity */
            if (i % 3 == 0) {
                data[i] = (data[i] << 1) | (data[i] >> 31);
                fdata[i] = fabsf(fdata[i]);
            }
        }
    }
    
    /* Another wide block */
    int sum = 0;
    float fsum = 0.0f;
    
    /* Many independent accumulations */
    for (int i = 0; i < size; i += 8) {
        sum += data[i] + data[i + 1] + data[i + 2] + data[i + 3] +
               data[i + 4] + data[i + 5] + data[i + 6] + data[i + 7];
        
        fsum += fdata[i] + fdata[i + 1] + fdata[i + 2] + fdata[i + 3] +
                fdata[i + 4] + fdata[i + 5] + fdata[i + 6] + fdata[i + 7];
    }
    
    global_accumulator += fsum + (float)sum;
    
    free(data);
    free(fdata);
}

/* Function with switch statement for state tracking */
void test_function_3(int mode) {
    int result = 0;
    float fresult = 0.0f;
    
    switch (mode % 4) {
        case 0:
            /* Integer-heavy path */
            for (int i = 0; i < 32; i++) {
                result = (result * 3 + i) & 0xFFFF;
                result ^= (i << 8);
            }
            fresult = (float)result;
            break;
            
        case 1:
            /* Float-heavy path */
            for (int i = 0; i < 32; i++) {
                fresult = fresult * 1.5f + sinf((float)i * 0.2f);
                fresult = fabsf(fresult);
            }
            result = (int)fresult;
            break;
            
        case 2:
            /* Mixed path */
            for (int i = 0; i < 32; i++) {
                result += i * 7;
                fresult += sqrtf((float)result);
                result = (int)fresult ^ result;
            }
            break;
            
        case 3:
            /* Memory-intensive path */
            {
                int temp[16];
                float ftemp[16];
                for (int i = 0; i < 16; i++) {
                    temp[i] = i * mode;
                    ftemp[i] = (float)temp[i] * 0.1f;
                }
                
                for (int i = 0; i < 15; i++) {
                    temp[i + 1] += temp[i];
                    ftemp[i] *= ftemp[i + 1];
                }
                
                result = temp[15];
                fresult = ftemp[0];
            }
            break;
    }
    
    global_accumulator += fresult;
}

/* Main driver that calls all test functions */
int main() {
    clock_t start = clock();
    
    printf("Starting scheduler coverage test...\n");
    
    /* Call test functions multiple times with different parameters */
    for (int i = 0; i < 10; i++) {
        test_function_1(50 + i * 2);
        test_function_2(128);
        test_function_3(i);
    }
    
    /* Additional complex computation to ensure scheduling happens */
    {
        /* Unrolled loop creating very wide basic block */
        int values[100];
        float fvalues[100];
        
        for (int i = 0; i < 100; i++) {
            values[i] = i * 3 + 7;
            fvalues[i] = (float)values[i] / 10.0f;
        }
        
        /* Manually unrolled processing */
        for (int i = 0; i < 100; i += 5) {
            /* Five independent chains */
            values[i] = values[i] * 2 - values[i + 1];
            values[i + 1] = values[i + 1] + values[i + 2] ^ 0xFF;
            values[i + 2] = values[i + 2] * values[i + 3] / 3;
            values[i + 3] = values[i + 3] | values[i + 4];
            values[i + 4] = values[i + 4] & values[i];
            
            fvalues[i] = fvalues[i] + sinf(fvalues[i + 1]);
            fvalues[i + 1] = fvalues[i + 1] * cosf(fvalues[i + 2]);
            fvalues[i + 2] = sqrtf(fabsf(fvalues[i + 3]));
            fvalues[i + 3] = fvalues[i + 3] / (fvalues[i + 4] + 1.0f);
            fvalues[i + 4] = fvalues[i] + fvalues[i + 1] + fvalues[i + 2];
        }
        
        /* Use results */
        int final_sum = 0;
        for (int i = 0; i < 100; i++) {
            final_sum += values[i];
            global_accumulator += fvalues[i];
        }
        global_accumulator += (float)final_sum;
    }
    
    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    
    printf("Test completed. Accumulator: %f\n", global_accumulator);
    printf("Elapsed time: %f seconds\n", elapsed);
    
    return 0;
}
