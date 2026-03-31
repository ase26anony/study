/* mcf_test.c - Test program to trigger min-cost flow debug output in GCC */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#define ARRAY_SIZE 15000
#define ITERATIONS 100
#define WARMUP_ITERATIONS 10

/* Global volatile variables to prevent optimization */
volatile int global_counter = 0;
volatile double global_accumulator = 0.0;

/* Complex structure to force register pressure */
struct DataPacket {
    int id;
    double values[8];
    float weights[12];
    char metadata[64];
    short indices[16];
    long long timestamps[4];
};

/* Function 1: Deeply nested loops with many live ranges */
void test_nested_loops(struct DataPacket *data, int size) {
    int i, j, k, l;
    double sum1 = 0.0, sum2 = 0.0, sum3 = 0.0;
    float prod1 = 1.0f, prod2 = 1.0f, prod3 = 1.0f;
    long long counter1 = 0, counter2 = 0, counter3 = 0;
    
    /* Outer loop creates many live ranges */
    for (i = 0; i < size; i++) {
        /* Multiple intermediate calculations */
        double temp1 = data[i].values[0] * data[i].weights[0];
        double temp2 = data[i].values[1] * data[i].weights[1];
        double temp3 = data[i].values[2] * data[i].weights[2];
        
        /* Nested loop level 1 */
        for (j = 0; j < 8; j++) {
            float acc1 = 0.0f, acc2 = 0.0f, acc3 = 0.0f;
            
            /* Complex expression with many temporaries */
            acc1 = (float)(temp1 * j + temp2 * (j + 1) + temp3 * (j + 2));
            acc2 = (float)(temp2 * j * 0.5 + temp3 * j * 0.25);
            acc3 = (float)(temp1 * temp2 * temp3 / (j + 1));
            
            /* More nested loops */
            for (k = 0; k < 4; k++) {
                double inner_sum = 0.0;
                short idx = data[i].indices[k];
                
                /* Deepest loop with register pressure */
                for (l = 0; l < 3; l++) {
                    double a = acc1 * l + acc2 * (l + 1);
                    double b = acc3 * l * 0.333;
                    double c = a * b + a / (b + 1.0);
                    
                    inner_sum += c * idx * (l + 1);
                    counter1 += (long long)(c * 1000);
                }
                
                sum1 += inner_sum * data[i].values[k % 4];
                prod1 *= (float)inner_sum;
            }
            
            sum2 += acc1 + acc2 + acc3;
            prod2 *= acc1 * acc2 * acc3;
            counter2 += j * 1000;
        }
        
        sum3 += temp1 + temp2 + temp3;
        prod3 *= (float)(temp1 * temp2);
        counter3 += i;
    }
    
    /* Force all results to be used */
    global_accumulator += sum1 + sum2 + sum3;
    global_counter += (int)(prod1 + prod2 + prod3);
    global_counter += (int)(counter1 + counter2 + counter3) % 1000;
}

/* Function 2: Complex control flow with many basic blocks */
int test_complex_cfg(int *array, int size) {
    int result = 0;
    int i = 0;
    
    /* Complex if-else chain with early returns */
    if (size < 10) {
        for (i = 0; i < size; i++) {
            if (array[i] < 0) return -1;
            if (array[i] > 1000) return 1;
        }
        return 0;
    }
    
    /* Switch with many cases */
    while (i < size) {
        int val = array[i] % 20;
        
        switch (val) {
            case 0: result += array[i] * 2; break;
            case 1: result += array[i] / 2; break;
            case 2: result += array[i] << 1; break;
            case 3: result += array[i] >> 1; break;
            case 4: result += array[i] & 0xFF; break;
            case 5: result += array[i] | 0xAA; break;
            case 6: result += array[i] ^ 0x55; break;
            case 7: result += array[i] * array[i]; break;
            case 8: result += (int)sqrt(abs(array[i])); break;
            case 9: result += array[i] % 13; break;
            case 10: result += array[i] * 3; i++; continue; /* Fall through prevention */
            case 11: result += array[i] * 4; break;
            case 12: result += array[i] * 5; break;
            case 13: result += array[i] * 6; break;
            case 14: result += array[i] * 7; break;
            case 15: result += array[i] * 8; break;
            case 16: result += array[i] * 9; break;
            case 17: result += array[i] * 10; break;
            case 18: result += array[i] * 11; break;
            case 19: result += array[i] * 12; break;
            default: result -= array[i]; break;
        }
        
        /* Nested if with break/continue */
        if (result > 1000000) {
            for (int j = 0; j < 5; j++) {
                if (result % (j + 2) == 0) {
                    result /= 2;
                    break;
                } else {
                    result += j;
                    continue;
                }
            }
        }
        
        i++;
    }
    
    /* Irreducible control flow using computed goto */
    static void *labels[] = { &&L0, &&L1, &&L2, &&L3, &&L4, &&L5 };
    int idx = result % 6;
    
    goto *labels[idx];
    
    L0: result += 100; goto END;
    L1: result += 200; goto END;
    L2: result += 300; goto END;
    L3: result += 400; goto END;
    L4: result += 500; goto END;
    L5: result += 600; goto END;
    
    END:
    return result;
}

/* Function 3: Inline assembly with register constraints */
void test_asm_register_pressure(int *input, int *output, int size) {
    int i;
    int a, b, c, d, e, f, g, h;
    
    for (i = 0; i < size; i += 8) {
        /* Multiple asm statements competing for registers */
        asm volatile (
            "movl %1, %%eax\n\t"
            "imull %%eax, %%eax\n\t"
            "movl %%eax, %0\n\t"
            : "=r" (a)
            : "r" (input[i])
            : "%eax", "memory"
        );
        
        asm volatile (
            "movl %1, %%ebx\n\t"
            "addl $100, %%ebx\n\t"
            "movl %%ebx, %0\n\t"
            : "=r" (b)
            : "r" (input[i + 1])
            : "%ebx", "memory"
        );
        
        asm volatile (
            "movl %1, %%ecx\n\t"
            "subl $50, %%ecx\n\t"
            "movl %%ecx, %0\n\t"
            : "=r" (c)
            : "r" (input[i + 2])
            : "%ecx", "memory"
        );
        
        asm volatile (
            "movl %1, %%edx\n\t"
            "xorl $0xFFFF, %%edx\n\t"
            "movl %%edx, %0\n\t"
            : "=r" (d)
            : "r" (input[i + 3])
            : "%edx", "memory"
        );
        
        /* More register pressure */
        asm volatile (
            "movl %1, %%esi\n\t"
            "shrl $2, %%esi\n\t"
            "movl %%esi, %0\n\t"
            : "=r" (e)
            : "r" (input[i + 4])
            : "%esi", "memory"
        );
        
        asm volatile (
            "movl %1, %%edi\n\t"
            "negl %%edi\n\t"
            "movl %%edi, %0\n\t"
            : "=r" (f)
            : "r" (input[i + 5])
            : "%edi", "memory"
        );
        
        /* Force spilling with memory clobber */
        asm volatile (
            "movl %2, %%eax\n\t"
            "addl %3, %%eax\n\t"
            "addl %4, %%eax\n\t"
            "addl %5, %%eax\n\t"
            "addl %6, %%eax\n\t"
            "addl %7, %%eax\n\t"
            "movl %%eax, %0\n\t"
            "movl %%eax, %1\n\t"
            : "=r" (g), "=r" (h)
            : "r" (a), "r" (b), "r" (c), "r" (d), "r" (e), "r" (f)
            : "%eax", "memory"
        );
        
        /* Store results */
        output[i] = a + b;
        output[i + 1] = c + d;
        output[i + 2] = e + f;
        output[i + 3] = g;
        output[i + 4] = h;
        output[i + 5] = a * b;
        output[i + 6] = c * d;
        output[i + 7] = e * f;
    }
}

/* Function 4: Mixed data types and many function arguments */
double test_mixed_types_and_args(char c1, short s1, int i1, long l1, 
                                 float f1, double d1, char c2, short s2,
                                 int i2, long l2, float f2, double d2,
                                 int *arr, int size) {
    /* Many local variables of different types */
    char c3 = c1 + c2;
    short s3 = s1 - s2;
    int i3 = i1 * i2;
    long l3 = l1 / (l2 ? l2 : 1);
    float f3 = f1 + f2;
    double d3 = d1 * d2;
    
    double result = 0.0;
    
    /* Complex calculations mixing types */
    for (int i = 0; i < size; i++) {
        double temp = 0.0;
        
        temp += (double)c3 * i;
        temp += (double)s3 * (i % 100);
        temp += (double)i3 * sin(i * 0.01);
        temp += (double)l3 * cos(i * 0.02);
        temp += (double)f3 * tan(i * 0.001);
        temp += d3 * log(i + 1.0);
        
        /* Address calculation with multiple indexing */
        int idx1 = (i * 7) % size;
        int idx2 = (i * 13) % size;
        int idx3 = (i * 17) % size;
        
        temp += arr[idx1] * 0.3;
        temp += arr[idx2] * 0.5;
        temp += arr[idx3] * 0.7;
        
        result += temp;
        
        /* Pointer aliasing to prevent optimization */
        volatile double *alias = &result;
        *alias = *alias + 0.0001;
    }
    
    return result;
}

/* Function 5: Vector-like operations with many SIMD-like calculations */
void test_vector_operations(float *data, int size) {
    /* Many parallel accumulators */
    float sum1 = 0.0f, sum2 = 0.0f, sum3 = 0.0f, sum4 = 0.0f;
    float prod1 = 1.0f, prod2 = 1.0f, prod3 = 1.0f, prod4 = 1.0f;
    float max1 = -1e9f, max2 = -1e9f, max3 = -1e9f, max4 = -1e9f;
    float min1 = 1e9f, min2 = 1e9f, min3 = 1e9f, min4 = 1e9f;
    
    for (int i = 0; i < size; i += 4) {
        /* SIMD-like parallel processing */
        float val1 = data[i];
        float val2 = data[i + 1];
        float val3 = data[i + 2];
        float val4 = data[i + 3];
        
        /* Multiple parallel calculations */
        sum1 += val1 * 0.1f;
        sum2 += val2 * 0.2f;
        sum3 += val3 * 0.3f;
        sum4 += val4 * 0.4f;
        
        prod1 *= val1 + 1.0f;
        prod2 *= val2 + 1.0f;
        prod3 *= val3 + 1.0f;
        prod4 *= val4 + 1.0f;
        
        max1 = val1 > max1 ? val1 : max1;
        max2 = val2 > max2 ? val2 : max2;
        max3 = val3 > max3 ? val3 : max3;
        max4 = val4 > max4 ? val4 : max4;
        
        min1 = val1 < min1 ? val1 : min1;
        min2 = val2 < min2 ? val2 : min2;
        min3 = val3 < min3 ? val3 : min3;
        min4 = val4 < min4 ? val4 : min4;
        
        /* Cross-element dependencies */
        data[i] = (val1 + val2 + val3 + val4) * 0.25f;
        data[i + 1] = (val1 * val2 + val3 * val4) * 0.5f;
        data[i + 2] = sqrtf(fabsf(val1 - val2) + fabsf(val3 - val4));
        data[i + 3] = (val1 / (val2 + 1.0f)) + (val3 / (val4 + 1.0f));
    }
    
    /* Force all results to be used */
    global_accumulator += sum1 + sum2 + sum3 + sum4;
    global_accumulator += prod1 + prod2 + prod3 + prod4;
    global_accumulator += max1 + max2 + max3 + max4;
    global_accumulator += min1 + min2 + min3 + min4;
}

/* Main function that orchestrates all tests */
int main() {
    int i, j;
    long long total_checksum = 0;
    
    /* Allocate large arrays */
    struct DataPacket *data = (struct DataPacket*)malloc(
        ARRAY_SIZE * sizeof(struct DataPacket));
    int *int_array = (int*)malloc(ARRAY_SIZE * sizeof(int));
    int *output_array = (int*)malloc(ARRAY_SIZE * sizeof(int));
    float *float_array = (float*)malloc(ARRAY_SIZE * sizeof(float));
    
    if (!data || !int_array || !output_array || !float_array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with random data */
    srand(time(NULL));
    for (i = 0; i < ARRAY_SIZE; i++) {
        /* Initialize DataPacket */
        data[i].id = i;
        for (j = 0; j < 8; j++) {
            data[i].values[j] = (double)rand() / RAND_MAX * 100.0;
        }
        for (j = 0; j < 12; j++) {
            data[i].weights[j] = (float)rand() / RAND_MAX * 2.0f;
        }
        for (j = 0; j < 16; j++) {
            data[i].indices[j] = (short)(rand() % 1000);
        }
        
        /* Initialize other arrays */
        int_array[i] = rand() % 10000;
        float_array[i] = (float)rand() / RAND_MAX * 100.0f;
    }
    
    printf("Starting register pressure tests...\n");
    
    /* Warm-up phase for profile feedback */
    for (i = 0; i < WARMUP_ITERATIONS; i++) {
        test_nested_loops(data, 1000);
        asm volatile("" ::: "memory"); /* Memory barrier */
    }
    
    /* Main test iterations */
    for (i = 0; i < ITERATIONS; i++) {
        /* Test 1: Nested loops */
        test_nested_loops(data, ARRAY_SIZE / 10);
        asm volatile("" ::: "memory");
        
        /* Test 2: Complex CFG */
        int cfg_result = test_complex_cfg(int_array, ARRAY_SIZE / 20);
        total_checksum += cfg_result;
        asm volatile("" ::: "memory");
        
        /* Test 3: Inline assembly */
        test_asm_register_pressure(int_array, output_array, ARRAY_SIZE / 40);
        asm volatile("" ::: "memory");
        
        /* Test 4: Mixed types with many arguments */
        double mixed_result = test_mixed_types_and_args(
            'A', 100, 1000, 10000L, 1.5f, 2.5, 
            'B', 200, 2000, 20000L, 2.5f, 3.5,
            int_array, ARRAY_SIZE / 50);
        total_checksum += (long long)mixed_result;
        asm volatile("" ::: "memory");
        
        /* Test 5: Vector operations */
        test_vector_operations(float_array, ARRAY_SIZE / 30);
        asm volatile("" ::: "memory");
        
        /* Update global counter */
        global_counter += i;
    }
    
    /* Final checksum calculation */
    for (i = 0; i < ARRAY_SIZE; i += 100) {
        total_checksum += data[i].id;
        total_checksum += (long long)data[i].values[0];
        total_checksum += int_array[i];
        total_checksum += output_array[i % (ARRAY_SIZE / 40)];
        total_checksum += (long long)float_array[i];
    }
    
    total_checksum += global_counter;
    total_checksum += (long long)global_accumulator;
    
    printf("Test completed. Checksum: %lld\n", total_checksum);
    printf("Global counter: %d\n", global_counter);
    printf("Global accumulator: %f\n", global_accumulator);
    
    /* Cleanup */
    free(data);
    free(int_array);
    free(output_array);
    free(float_array);
    
    return 0;
}
