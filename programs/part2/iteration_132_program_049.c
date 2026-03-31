/* mcf_test.c - Test program to trigger min-cost flow debug output */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#define ARRAY_SIZE 15000
#define ITERATIONS 100
#define SWITCH_CASES 15

/* Volatile variables to extend live ranges */
volatile int global_seed = 42;
volatile double global_temp = 0.0;

/* Complex structure to force register pressure */
struct DataNode {
    int values[8];
    double fp_values[4];
    struct DataNode* next;
    char padding[32];
};

/* Function 1: Deeply nested loops with many live ranges */
int nested_loop_pressure(int* data, int size) {
    int sum = 0;
    int prod = 1;
    int diff = 0;
    int xor_val = 0;
    int temp1, temp2, temp3, temp4, temp5;
    
    /* Variables declared at function scope but used in nested blocks */
    int outer_acc = 0;
    double fp_acc = 0.0;
    long long big_acc = 0;
    
    for (int i = 0; i < size; i++) {
        outer_acc += data[i];
        
        for (int j = 0; j < i % 100; j++) {
            temp1 = data[j] * 3;
            temp2 = data[j] + 7;
            temp3 = temp1 - temp2;
            
            for (int k = 0; k < j % 10; k++) {
                temp4 = data[k] << 2;
                temp5 = temp4 ^ temp3;
                fp_acc += sin(temp5 * 0.01);
                
                if (k % 3 == 0) {
                    sum += temp5;
                    prod *= (temp5 & 0xFF) + 1;
                    diff -= temp4;
                    xor_val ^= temp1;
                } else if (k % 3 == 1) {
                    continue;  /* Creates complex CFG */
                } else {
                    break;  /* More CFG complexity */
                }
                
                /* Inline asm with register constraints */
                asm volatile (
                    "movl %1, %%eax\n\t"
                    "imull %%eax, %%eax\n\t"
                    "movl %%eax, %0\n\t"
                    : "=r" (temp4)
                    : "r" (temp5)
                    : "%eax", "memory"
                );
                
                big_acc += temp4;
            }
            
            /* Another asm block competing for registers */
            asm volatile (
                "addl $1, %0\n\t"
                : "+r" (temp2)
                :
                : "cc"
            );
        }
        
        /* Early return in some cases */
        if (i == size / 2) {
            return sum ^ prod;
        }
    }
    
    /* Complex expression with many intermediates */
    int result = (sum * prod) - (diff << 2) + (xor_val & 0xFFFF);
    result += (int)(fp_acc * 1000) + (int)(big_acc % 1000000);
    return result + outer_acc;
}

/* Function 2: Switch statement with many cases and fall-through */
int complex_switch(int value, int* data, int size) {
    int result = 0;
    
    switch (value % SWITCH_CASES) {
        case 0:
            result = data[0] + data[1];
            /* Fall through */
        case 1:
            result *= data[2];
            break;
        case 2:
            result = data[3] - data[4];
            /* Fall through */
        case 3:
        case 4:
            result += data[5] * data[6];
            break;
        case 5:
            result = data[7] ^ data[8];
            /* Fall through */
        case 6:
            result <<= 2;
            break;
        case 7:
            result = data[9] | data[10];
            /* Fall through */
        case 8:
            result >>= 1;
            break;
        case 9:
            result = ~data[11];
            /* Fall through */
        case 10:
            result += 0x7FFFFFFF;
            break;
        case 11:
            result = data[12] & data[13];
            /* Fall through */
        case 12:
            result *= 3;
            break;
        case 13:
            result = abs(data[14]);
            /* Fall through */
        case 14:
            result = -result;
            break;
        default:
            result = value;
    }
    
    /* Loop with break/continue at different levels */
    for (int i = 0; i < size; i++) {
        if (data[i] < 0) {
            continue;
        }
        
        for (int j = 0; j < 5; j++) {
            if (data[i] > 1000) {
                break;
            }
            result += data[i] * j;
            
            if (j == 3) {
                goto early_exit;
            }
        }
        
        if (i % 7 == 0) {
            return result * 2;
        }
    }
    
early_exit:
    return result;
}

/* Function 3: Mixed data types and many function arguments */
double mixed_types_reg_pressure(
    char c1, short s1, int i1, long l1,
    float f1, double d1,
    char c2, short s2, int i2, long l2,
    float f2, double d2,
    int* arr, int size
) {
    /* Many local variables of different types */
    char c3 = c1 ^ c2;
    short s3 = s1 + s2;
    int i3 = i1 * i2;
    long l3 = l1 - l2;
    float f3 = f1 / f2;
    double d3 = d1 + d2;
    
    double acc = 0.0;
    float fp_acc = 0.0f;
    
    /* Complex expression mixing types */
    for (int idx = 0; idx < size; idx++) {
        c3 += arr[idx] & 0xFF;
        s3 += (arr[idx] >> 8) & 0xFFFF;
        i3 ^= arr[idx];
        l3 += arr[idx] * 1000L;
        f3 += sin(arr[idx] * 0.01f);
        d3 += cos(arr[idx] * 0.001);
        
        /* Force register pressure with many simultaneous computations */
        double t1 = d3 * f3;
        double t2 = l3 * 0.0001;
        double t3 = i3 * 0.01;
        float t4 = s3 * 0.1f;
        char t5 = c3 * 2;
        
        acc += t1 + t2 + t3 + t4 + t5;
        fp_acc += t4;
        
        /* Address calculation with multiple indexing */
        int* ptr1 = arr + idx;
        int* ptr2 = arr + (idx ^ 0x7FF);
        int* ptr3 = &arr[idx % 1000];
        
        *ptr1 += *ptr2;
        *ptr3 += idx;
        
        /* Memory barrier to prevent optimization */
        asm volatile("" ::: "memory");
    }
    
    /* Return complex expression */
    return acc + fp_acc + d3 + f3 + l3 + i3 + s3 + c3;
}

/* Function 4: Vector-like operations using multiple registers */
void simd_like_operations(int* data, int size, int* results) {
    /* Multiple accumulators for different operations */
    int sum[4] = {0};
    int prod[4] = {1, 1, 1, 1};
    int xor_val[4] = {0};
    int min_val[4] = {INT_MAX};
    int max_val[4] = {INT_MIN};
    
    /* Process 4 elements at a time (simulating SIMD) */
    for (int i = 0; i < size - 3; i += 4) {
        /* Load 4 elements */
        int v0 = data[i];
        int v1 = data[i + 1];
        int v2 = data[i + 2];
        int v3 = data[i + 3];
        
        /* Parallel operations on 4 elements */
        sum[0] += v0; sum[1] += v1; sum[2] += v2; sum[3] += v3;
        prod[0] *= (v0 & 0xFF) + 1;
        prod[1] *= (v1 & 0xFF) + 1;
        prod[2] *= (v2 & 0xFF) + 1;
        prod[3] *= (v3 & 0xFF) + 1;
        xor_val[0] ^= v0; xor_val[1] ^= v1; xor_val[2] ^= v2; xor_val[3] ^= v3;
        
        min_val[0] = v0 < min_val[0] ? v0 : min_val[0];
        min_val[1] = v1 < min_val[1] ? v1 : min_val[1];
        min_val[2] = v2 < min_val[2] ? v2 : min_val[2];
        min_val[3] = v3 < min_val[3] ? v3 : min_val[3];
        
        max_val[0] = v0 > max_val[0] ? v0 : max_val[0];
        max_val[1] = v1 > max_val[1] ? v1 : max_val[1];
        max_val[2] = v2 > max_val[2] ? v2 : max_val[2];
        max_val[3] = v3 > max_val[3] ? v3 : max_val[3];
        
        /* Inline asm with multiple constraints */
        asm volatile (
            "movl %1, %%eax\n\t"
            "addl %2, %%eax\n\t"
            "movl %%eax, %0\n\t"
            : "=r" (v0)
            : "r" (v0), "r" (i)
            : "%eax"
        );
        
        data[i] = v0;
    }
    
    /* Reduce results */
    results[0] = sum[0] + sum[1] + sum[2] + sum[3];
    results[1] = prod[0] * prod[1] * prod[2] * prod[3];
    results[2] = xor_val[0] ^ xor_val[1] ^ xor_val[2] ^ xor_val[3];
    results[3] = min_val[0] < min_val[1] ? 
                (min_val[0] < min_val[2] ? 
                 (min_val[0] < min_val[3] ? min_val[0] : min_val[3]) :
                 (min_val[2] < min_val[3] ? min_val[2] : min_val[3])) :
                (min_val[1] < min_val[2] ?
                 (min_val[1] < min_val[3] ? min_val[1] : min_val[3]) :
                 (min_val[2] < min_val[3] ? min_val[2] : min_val[3]));
    results[4] = max_val[0] > max_val[1] ?
                (max_val[0] > max_val[2] ?
                 (max_val[0] > max_val[3] ? max_val[0] : max_val[3]) :
                 (max_val[2] > max_val[3] ? max_val[2] : max_val[3])) :
                (max_val[1] > max_val[2] ?
                 (max_val[1] > max_val[3] ? max_val[1] : max_val[3]) :
                 (max_val[2] > max_val[3] ? max_val[2] : max_val[3]));
}

/* Function 5: Indirect calls and computed goto (irreducible CFG) */
int irreducible_cfg(int* data, int size, int mode) {
    static void* jump_table[] = {
        &&label_0, &&label_1, &&label_2, &&label_3,
        &&label_4, &&label_5, &&label_6, &&label_7,
        &&label_8, &&label_9, &&label_10, &&label_11
    };
    
    int result = 0;
    int i = 0;
    
    /* Indirect goto creates irreducible flow graph */
    goto *jump_table[mode % 12];
    
label_0:
    result = data[i++] * 2;
    goto *jump_table[(result + i) % 12];
    
label_1:
    result += data[i++] ^ 0x55;
    if (i >= size) goto end;
    goto *jump_table[(data[i] + 1) % 12];
    
label_2:
    result -= data[i++];
    goto *jump_table[(result & 7) % 12];
    
label_3:
    result |= data[i++];
    goto *jump_table[((result >> 4) & 7) % 12];
    
label_4:
    result &= data[i++];
    goto *jump_table[(i * 3) % 12];
    
label_5:
    result <<= (data[i++] & 3);
    goto *jump_table[(result % 5) + 5];
    
label_6:
    result >>= 1;
    i++;
    goto *jump_table[(data[i % size] % 8)];
    
label_7:
    result = ~result;
    i++;
    goto *jump_table[(i ^ result) % 12];
    
label_8:
    result += 0x1000;
    i++;
    if (result > 1000000) goto end;
    goto *jump_table[(result % 9)];
    
label_9:
    result *= 3;
    i++;
    goto *jump_table[10];
    
label_10:
    result /= 2;
    i++;
    goto *jump_table[11];
    
label_11:
    result %= 1000;
    i++;
    if (i < size) goto *jump_table[(data[i] % 12)];
    
end:
    return result;
}

/* Main function with warm-up and verification */
int main() {
    /* Allocate and initialize large arrays */
    int* data = (int*)malloc(ARRAY_SIZE * sizeof(int));
    int* results = (int*)malloc(5 * sizeof(int));
    double* fp_data = (double*)malloc(ARRAY_SIZE * sizeof(double));
    
    if (!data || !results || !fp_data) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random data */
    srand(global_seed);
    for (int i = 0; i < ARRAY_SIZE; i++) {
        data[i] = rand() % 10000 - 5000;
        fp_data[i] = (rand() % 10000) / 100.0 - 50.0;
    }
    
    int final_result = 0;
    double final_fp_result = 0.0;
    
    /* Warm-up phase (allow profile feedback if using FDO) */
    printf("Starting warm-up phase...\n");
    for (int warm = 0; warm < 10; warm++) {
        int temp = nested_loop_pressure(data, 1000);
        final_result ^= temp;
        
        /* Memory barrier between iterations */
        asm volatile("" ::: "memory");
    }
    
    /* Main test phase */
    printf("Starting main test phase...\n");
    
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Call each test function */
        int r1 = nested_loop_pressure(data + iter * 10, ARRAY_SIZE - iter * 10);
        final_result += r1;
        
        asm volatile("" ::: "memory");
        
        int r2 = complex_switch(iter, data, ARRAY_SIZE / 2);
        final_result ^= r2;
        
        asm volatile("" ::: "memory");
        
        double r3 = mixed_types_reg_pressure(
            iter & 0xFF, (iter >> 8) & 0xFFFF, iter, iter * 100L,
            iter * 0.1f, iter * 0.01,
            (iter ^ 0xFF) & 0xFF, ((iter ^ 0xFFFF) >> 8) & 0xFFFF,
            iter ^ 0xAAAAAAAA, iter * 200L,
            iter * 0.2f, iter * 0.02,
            data, ARRAY_SIZE / 10
        );
        final_fp_result += r3;
        
        asm volatile("" ::: "memory");
        
        simd_like_operations(data, ARRAY_SIZE, results);
        for (int i = 0; i < 5; i++) {
            final_result += results[i];
        }
        
        asm volatile("" ::: "memory");
        
        int r5 = irreducible_cfg(data, ARRAY_SIZE / 20, iter);
        final_result ^= r5;
        
        /* Progress indicator */
        if (iter % 10 == 0) {
            printf("Iteration %d/%d\n", iter, ITERATIONS);
        }
    }
    
    /* Compute final checksum */
    unsigned long long checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += (unsigned long long)data[i];
        checksum ^= (unsigned long long)(fp_data[i] * 1000);
    }
    
    checksum += (unsigned long long)final_result;
    checksum += (unsigned long long)fabs(final_fp_result * 1000);
    
    printf("Final result: %d\n", final_result);
    printf("Final FP result: %f\n", final_fp_result);
    printf("Checksum: %llu\n", checksum);
    
    /* Cleanup */
    free(data);
    free(results);
    free(fp_data);
    
    return 0;
}
