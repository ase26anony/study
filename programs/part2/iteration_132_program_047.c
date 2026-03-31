/* mcf_test.c - Test program to trigger min-cost flow debug output in GCC */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#define ARRAY_SIZE 10000
#define ITERATIONS 100
#define WARMUP_ITERATIONS 10

/* Global volatile variables to prevent optimization */
volatile int global_seed = 42;
volatile double global_accumulator = 0.0;

/* Function 1: Deeply nested loops with many live ranges */
double test_nested_loops(int *data, int size) {
    double sum = 0.0;
    double product = 1.0;
    double diff = 0.0;
    double ratio = 0.0;
    double temp1, temp2, temp3, temp4, temp5;
    
    /* Complex loop structure with many intermediate values */
    for (int i = 0; i < size; i++) {
        temp1 = data[i] * 1.5;
        temp2 = temp1 / (i + 1);
        
        for (int j = 0; j < 5; j++) {
            temp3 = temp2 * j;
            temp4 = sin(temp3);
            
            for (int k = 0; k < 3; k++) {
                temp5 = temp4 * cos(k * 0.5);
                sum += temp5;
                
                if (k % 2 == 0) {
                    product *= temp5;
                    diff -= temp5;
                } else {
                    product /= (temp5 + 1.0);
                    diff += temp5;
                }
                
                /* More intermediate calculations */
                ratio = (sum > 0) ? sum / (product + 1.0) : product / (sum + 1.0);
            }
            
            /* Early continue creates complex CFG */
            if (j == 2) continue;
            
            /* Nested if-else chain */
            if (temp3 > 100) {
                sum *= 1.1;
            } else if (temp3 > 50) {
                sum *= 1.05;
            } else if (temp3 > 25) {
                sum *= 1.02;
            } else {
                sum *= 0.99;
            }
        }
        
        /* Break at different levels */
        if (i > size / 2 && sum > 1000000) {
            break;
        }
    }
    
    return sum + product + diff + ratio;
}

/* Function 2: Complex switch statement with fall-through */
int test_switch_complex(int value, int *counter) {
    int result = 0;
    
    switch (value % 15) {
        case 0:
            result = value * 2;
            /* Fall through */
        case 1:
            result += value / 2;
            break;
        case 2:
            result = value << 1;
            /* Fall through */
        case 3:
            result |= 0xFF;
            /* Fall through */
        case 4:
            result ^= value;
            break;
        case 5:
            result = ~value;
            /* Fall through */
        case 6:
            result &= 0x7F;
            /* Fall through */
        case 7:
            result >>= 2;
            break;
        case 8:
            result = value * value;
            /* Fall through */
        case 9:
            result %= 256;
            /* Fall through */
        case 10:
            result += 128;
            break;
        case 11:
            result = abs(value);
            /* Fall through */
        case 12:
            result = -result;
            break;
        case 13:
            result = value & 0xF0;
            /* Fall through */
        case 14:
            result |= 0x0F;
            break;
        default:
            result = 0;
    }
    
    /* Multiple updates to extend live ranges */
    (*counter)++;
    (*counter) *= 2;
    (*counter) -= result;
    (*counter) &= 0xFFF;
    
    return result;
}

/* Function 3: Inline assembly with register constraints */
void test_inline_asm(int *data, int size) {
    int temp1, temp2, temp3, temp4;
    long long temp5, temp6;
    double fp1, fp2;
    
    for (int i = 0; i < size; i += 4) {
        /* Force use of specific registers */
        asm volatile (
            "movl %1, %%eax\n\t"
            "imull %%eax, %%eax\n\t"
            "movl %%eax, %0\n\t"
            : "=r" (temp1)
            : "r" (data[i])
            : "%eax", "memory"
        );
        
        asm volatile (
            "movl %1, %%ebx\n\t"
            "addl %%ebx, %%ebx\n\t"
            "movl %%ebx, %0\n\t"
            : "=r" (temp2)
            : "r" (data[i + 1])
            : "%ebx", "memory"
        );
        
        asm volatile (
            "movq %1, %%rax\n\t"
            "movq %2, %%rbx\n\t"
            "addq %%rbx, %%rax\n\t"
            "movq %%rax, %0\n\t"
            : "=r" (temp5)
            : "r" ((long long)temp1), "r" ((long long)temp2)
            : "%rax", "%rbx", "memory"
        );
        
        /* Floating point with SSE registers */
        fp1 = (double)temp1;
        fp2 = (double)temp2;
        
        asm volatile (
            "movsd %1, %%xmm0\n\t"
            "movsd %2, %%xmm1\n\t"
            "addsd %%xmm1, %%xmm0\n\t"
            "movsd %%xmm0, %0\n\t"
            : "=m" (fp1)
            : "m" (fp1), "m" (fp2)
            : "%xmm0", "%xmm1", "memory"
        );
        
        /* Compete for same registers */
        asm volatile (
            "movl %1, %%ecx\n\t"
            "movl %2, %%edx\n\t"
            "subl %%edx, %%ecx\n\t"
            "movl %%ecx, %0\n\t"
            : "=r" (temp3)
            : "r" (data[i + 2]), "r" (data[i + 3])
            : "%ecx", "%edx", "memory"
        );
        
        data[i] = temp1;
        data[i + 1] = temp2;
        data[i + 2] = temp3;
        
        /* Memory barrier */
        asm volatile("" ::: "memory");
    }
}

/* Function 4: Mixed data types and many function arguments */
double test_mixed_types(char c, short s, int i, long l, 
                        float f, double d, int *ptr, 
                        long long ll, unsigned int ui, 
                        unsigned long ul) {
    /* Force different register classes */
    double result = 0.0;
    
    result += (double)c * 1.5;
    result += (double)s * 2.5;
    result += (double)i * 3.5;
    result += (double)l * 4.5;
    result += (double)f * 5.5;
    result += d * 6.5;
    result += (double)(*ptr) * 7.5;
    result += (double)ll * 8.5;
    result += (double)ui * 9.5;
    result += (double)ul * 10.5;
    
    /* Complex expression with many temporaries */
    double temp1 = sin(result);
    double temp2 = cos(result);
    double temp3 = tan(result);
    double temp4 = exp(result);
    double temp5 = log(fabs(result) + 1.0);
    
    result = temp1 * temp2 + temp3 / temp4 - temp5;
    
    /* Pointer aliasing to prevent optimization */
    volatile double *alias = &result;
    *alias += 1.0;
    
    return result;
}

/* Function 5: Irreducible control flow with computed goto */
void test_computed_goto(int *data, int size, int *result) {
    static void *labels[] = {
        &&label0, &&label1, &&label2, &&label3,
        &&label4, &&label5, &&label6, &&label7
    };
    
    int sum = 0;
    int product = 1;
    int i = 0;
    
    /* Variables declared at function scope but used in goto blocks */
    int temp_a, temp_b, temp_c, temp_d;
    
    start_loop:
    if (i >= size) goto end;
    
    int index = data[i] & 0x7;
    goto *labels[index];
    
    label0:
        temp_a = data[i] * 2;
        temp_b = temp_a + 1;
        sum += temp_b;
        product *= (temp_b & 0xFF);
        i++;
        goto start_loop;
    
    label1:
        temp_a = data[i] >> 1;
        temp_c = temp_a - 1;
        sum += temp_c;
        product *= (temp_c & 0xFF);
        i++;
        goto start_loop;
    
    label2:
        temp_b = data[i] & 0xF;
        temp_d = temp_b * 3;
        sum += temp_d;
        product *= (temp_d & 0xFF);
        i++;
        goto start_loop;
    
    label3:
        temp_c = data[i] | 0x80;
        temp_a = temp_c / 2;
        sum += temp_a;
        product *= (temp_a & 0xFF);
        i++;
        goto start_loop;
    
    label4:
        temp_d = ~data[i];
        temp_b = temp_d + 100;
        sum += temp_b;
        product *= (temp_b & 0xFF);
        i++;
        goto start_loop;
    
    label5:
        temp_a = data[i] ^ 0x55;
        temp_c = temp_a * 5;
        sum += temp_c;
        product *= (temp_c & 0xFF);
        i++;
        goto start_loop;
    
    label6:
        temp_b = abs(data[i]);
        temp_d = temp_b - 50;
        sum += temp_d;
        product *= (temp_d & 0xFF);
        i++;
        goto start_loop;
    
    label7:
        temp_c = data[i] % 256;
        temp_a = temp_c << 2;
        sum += temp_a;
        product *= (temp_a & 0xFF);
        i++;
        goto start_loop;
    
    end:
    result[0] = sum;
    result[1] = product;
}

/* Function 6: Vector operations with many SIMD temporaries */
void test_vector_ops(float *data, int size) {
    /* Many vector temporaries to stress register allocation */
    float temp1[4], temp2[4], temp3[4], temp4[4];
    float temp5[4], temp6[4], temp7[4], temp8[4];
    
    for (int i = 0; i < size; i += 4) {
        /* Load data */
        for (int j = 0; j < 4; j++) {
            temp1[j] = data[i + j];
            temp2[j] = data[i + j] * 2.0f;
        }
        
        /* Multiple vector operations */
        for (int j = 0; j < 4; j++) {
            temp3[j] = temp1[j] + temp2[j];
            temp4[j] = temp1[j] - temp2[j];
            temp5[j] = temp1[j] * temp2[j];
            temp6[j] = temp1[j] / (temp2[j] + 0.001f);
        }
        
        /* More operations creating dependency chains */
        for (int j = 0; j < 4; j++) {
            temp7[j] = sinf(temp3[j]) + cosf(temp4[j]);
            temp8[j] = tanf(temp5[j]) * expf(temp6[j]);
        }
        
        /* Store results */
        for (int j = 0; j < 4; j++) {
            data[i + j] = temp7[j] + temp8[j];
        }
    }
}

/* Main function with warm-up and verification */
int main() {
    /* Initialize large arrays */
    int *int_data = (int*)malloc(ARRAY_SIZE * sizeof(int));
    float *float_data = (float*)malloc(ARRAY_SIZE * sizeof(float));
    int switch_counter = 0;
    int computed_goto_result[2];
    
    if (!int_data || !float_data) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random data */
    srand(global_seed);
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_data[i] = rand() % 1000;
        float_data[i] = (float)rand() / RAND_MAX * 100.0f;
    }
    
    printf("Starting register pressure stress test...\n");
    
    /* Warm-up phase for profile feedback */
    printf("Warm-up phase (%d iterations)...\n", WARMUP_ITERATIONS);
    for (int iter = 0; iter < WARMUP_ITERATIONS; iter++) {
        double result1 = test_nested_loops(int_data, 100);
        global_accumulator += result1;
        
        for (int i = 0; i < 100; i++) {
            int result2 = test_switch_complex(int_data[i], &switch_counter);
            global_accumulator += result2;
        }
        
        test_inline_asm(int_data, 100);
        
        /* Memory barrier between functions */
        asm volatile("" ::: "memory");
    }
    
    /* Main test phase */
    printf("Main test phase (%d iterations)...\n", ITERATIONS);
    double total_checksum = 0.0;
    
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Test 1: Nested loops */
        double result1 = test_nested_loops(int_data, ARRAY_SIZE / 10);
        total_checksum += result1;
        
        /* Memory barrier */
        asm volatile("" ::: "memory");
        
        /* Test 2: Switch statements */
        for (int i = 0; i < ARRAY_SIZE / 100; i++) {
            int result2 = test_switch_complex(int_data[i], &switch_counter);
            total_checksum += result2;
        }
        
        /* Memory barrier */
        asm volatile("" ::: "memory");
        
        /* Test 3: Inline assembly */
        test_inline_asm(int_data, ARRAY_SIZE / 20);
        
        /* Memory barrier */
        asm volatile("" ::: "memory");
        
        /* Test 4: Mixed types with many arguments */
        for (int i = 0; i < ARRAY_SIZE / 1000; i++) {
            double result4 = test_mixed_types(
                (char)(int_data[i] & 0xFF),
                (short)(int_data[i] & 0xFFFF),
                int_data[i],
                (long)int_data[i] * 2,
                (float)int_data[i] / 100.0f,
                (double)int_data[i] / 1000.0,
                &int_data[i],
                (long long)int_data[i] * 1000,
                (unsigned int)int_data[i],
                (unsigned long)int_data[i] * 2
            );
            total_checksum += result4;
        }
        
        /* Memory barrier */
        asm volatile("" ::: "memory");
        
        /* Test 5: Computed goto */
        test_computed_goto(int_data, ARRAY_SIZE / 50, computed_goto_result);
        total_checksum += computed_goto_result[0] + computed_goto_result[1];
        
        /* Memory barrier */
        asm volatile("" ::: "memory");
        
        /* Test 6: Vector operations */
        test_vector_ops(float_data, ARRAY_SIZE / 10);
        
        /* Update global to prevent dead code elimination */
        global_accumulator += total_checksum;
        
        /* Progress indicator */
        if ((iter + 1) % 10 == 0) {
            printf("Completed %d/%d iterations\n", iter + 1, ITERATIONS);
        }
    }
    
    /* Final verification */
    printf("\nTest completed successfully!\n");
    printf("Total checksum: %f\n", total_checksum);
    printf("Global accumulator: %f\n", (double)global_accumulator);
    printf("Switch counter: %d\n", switch_counter);
    printf("Computed goto result: [%d, %d]\n", 
           computed_goto_result[0], computed_goto_result[1]);
    
    /* Cleanup */
    free(int_data);
    free(float_data);
    
    return 0;
}
