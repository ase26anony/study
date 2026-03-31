/* mcf_test.c - Test program to trigger min-cost flow debug output in GCC */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

#define ARRAY_SIZE 15000
#define ITERATIONS 100
#define SWITCH_CASES 15

/* Volatile variables to prevent optimization and extend live ranges */
volatile int global_seed = 42;
volatile int global_barrier = 0;

/* Complex structure to force different register classes */
typedef struct {
    char c;
    short s;
    int i;
    long l;
    float f;
    double d;
    void* p;
} MixedData;

/* Function 1: Deeply nested loops with many live ranges */
int complex_loop_pressure(int* data, int size) {
    int sum = 0;
    int prod = 1;
    int diff = 0;
    int xor_val = 0;
    int and_val = ~0;
    int or_val = 0;
    
    /* Variables declared at function scope but used in nested blocks */
    int temp1, temp2, temp3, temp4, temp5;
    float ftemp1, ftemp2;
    double dtemp1, dtemp2;
    
    for (int i = 0; i < size; i++) {
        temp1 = data[i];
        
        /* First nested loop */
        for (int j = 0; j < 5; j++) {
            temp2 = temp1 * j;
            ftemp1 = (float)temp2 / 3.14159f;
            
            /* Second level nesting */
            for (int k = 0; k < 3; k++) {
                temp3 = temp2 + k * 7;
                dtemp1 = (double)temp3 * 2.71828;
                
                /* Third level nesting */
                for (int m = 0; m < 2; m++) {
                    temp4 = temp3 - m * 11;
                    ftemp2 = ftemp1 + (float)dtemp1;
                    
                    /* Keep all these values alive across the innermost loop */
                    sum += temp4;
                    prod *= (temp4 != 0) ? temp4 : 1;
                    diff -= temp4;
                    xor_val ^= temp4;
                    and_val &= temp4;
                    or_val |= temp4;
                }
                
                /* Use all temporaries in complex expression */
                temp5 = (int)(ftemp1 * 100) + (int)(dtemp1 * 50) + temp3;
                sum += temp5;
            }
        }
        
        /* Early return in some cases to create complex CFG */
        if (i == size / 3) {
            if (sum > 1000000) return sum;
        }
        
        if (i == size / 2) {
            if (prod == 0) {
                prod = 1;
                continue; /* Skip rest of iteration */
            }
        }
        
        /* Break from outer loop under specific condition */
        if (i == size * 3 / 4) {
            if (xor_val == 0xFFFFFFFF) break;
        }
    }
    
    /* Final complex computation using all accumulated values */
    int result = (sum + prod + diff + xor_val + and_val + or_val) % 1000000;
    return result;
}

/* Function 2: Complex switch statement with fall-through */
int switch_cfg_pressure(int value, int* data, int size) {
    int result = 0;
    
    switch (value % SWITCH_CASES) {
        case 0:
            result = data[0] * 2;
            /* Fall through */
        case 1:
            result += data[1] * 3;
            break;
        case 2:
            result = data[2] - data[3];
            if (result < 0) {
                result = -result;
                /* Early return creates additional edge */
                return result;
            }
            /* Fall through */
        case 3:
        case 4:
            result *= data[4];
            break;
        case 5:
            for (int i = 0; i < 10; i++) {
                result += data[i % size];
                if (result > 1000) break;
            }
            break;
        case 6:
            result = data[6] ^ data[7];
            /* Fall through */
        case 7:
            result |= data[8];
            break;
        case 8:
            result = data[9] & data[10];
            /* Fall through */
        case 9:
            result = ~result;
            break;
        case 10:
            result = data[11] << 2;
            break;
        case 11:
            result = data[12] >> 1;
            /* Fall through */
        case 12:
            result += data[13];
            break;
        case 13:
            result = data[14] * data[15];
            if (result == 0) {
                return 0;
            }
            break;
        case 14:
            result = 1;
            for (int i = 0; i < 8; i++) {
                result *= data[16 + i];
            }
            break;
        default:
            result = -1;
    }
    
    /* Post-switch computation with many temporaries */
    int t1 = result * 3;
    int t2 = result / 2;
    int t3 = result + 7;
    int t4 = result - 5;
    int t5 = result ^ 0xAA;
    int t6 = result & 0x55;
    int t7 = result | 0xF0;
    
    /* Force all temporaries to be live simultaneously */
    asm volatile("" : : "r"(t1), "r"(t2), "r"(t3), "r"(t4), "r"(t5), "r"(t6), "r"(t7));
    
    return t1 + t2 + t3 + t4 + t5 + t6 + t7;
}

/* Function 3: Inline assembly with explicit register constraints */
void asm_register_pressure(MixedData* md, int count) {
    for (int i = 0; i < count; i++) {
        MixedData* current = &md[i];
        
        /* Force specific registers with inline asm */
        int a_val, b_val, c_val, d_val;
        
        /* Compete for EAX/RAX */
        asm volatile (
            "movl %1, %%eax\n\t"
            "addl $100, %%eax\n\t"
            "movl %%eax, %0\n\t"
            : "=r" (a_val)
            : "r" (current->i)
            : "%eax", "memory"
        );
        
        /* Compete for EBX/RBX */
        asm volatile (
            "movl %1, %%ebx\n\t"
            "imull $3, %%ebx\n\t"
            "movl %%ebx, %0\n\t"
            : "=r" (b_val)
            : "r" (current->i + 1)
            : "%ebx", "memory"
        );
        
        /* Compete for ECX/RCX */
        asm volatile (
            "movl %1, %%ecx\n\t"
            "xorl $0xFF, %%ecx\n\t"
            "movl %%ecx, %0\n\t"
            : "=r" (c_val)
            : "r" (current->i + 2)
            : "%ecx", "memory"
        );
        
        /* Compete for EDX/RDX */
        asm volatile (
            "movl %1, %%edx\n\t"
            "shrl $2, %%edx\n\t"
            "movl %%edx, %0\n\t"
            : "=r" (d_val)
            : "r" (current->i + 3)
            : "%edx", "memory"
        );
        
        /* Use all results to prevent dead code elimination */
        current->c = (char)((a_val + b_val + c_val + d_val) & 0xFF);
        current->s = (short)((a_val * b_val - c_val * d_val) & 0xFFFF);
        current->i = a_val ^ b_val ^ c_val ^ d_val;
        current->l = (long)a_val * b_val * c_val * d_val;
        current->f = (float)(a_val + b_val) / (float)(c_val + d_val + 1);
        current->d = (double)(a_val * b_val) / (double)(c_val * d_val + 1);
        
        /* Memory barrier between iterations */
        asm volatile("" ::: "memory");
    }
}

/* Function 4: Many function arguments to stress register/stack passing */
int multi_arg_pressure(int a1, int a2, int a3, int a4, int a5,
                       int a6, int a7, int a8, int a9, int a10,
                       float f1, float f2, float f3, double d1, double d2) {
    /* Force all arguments to be used in complex ways */
    int sum_int = a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10;
    float sum_float = f1 + f2 + f3;
    double sum_double = d1 + d2;
    
    /* Many temporary variables */
    int t1 = sum_int * 2;
    int t2 = sum_int / 3;
    int t3 = sum_int % 7;
    int t4 = sum_int << 1;
    int t5 = sum_int >> 2;
    
    float ft1 = sum_float * 1.5f;
    float ft2 = sum_float / 2.0f;
    float ft3 = sum_float + 3.14f;
    
    double dt1 = sum_double * 2.71828;
    double dt2 = sum_double / 1.41421;
    double dt3 = sum_double - 1.0;
    
    /* Complex expression mixing all types */
    int result = t1 + t2 + t3 + t4 + t5 +
                 (int)ft1 + (int)ft2 + (int)ft3 +
                 (int)dt1 + (int)dt2 + (int)dt3;
    
    /* Conditional with early returns */
    if (result > 1000) {
        return result % 1000;
    } else if (result < -1000) {
        return -result % 1000;
    } else if (result == 0) {
        return 1;
    }
    
    /* Loop with break/continue at different levels */
    for (int i = 0; i < 20; i++) {
        if (i % 3 == 0) continue;
        
        for (int j = 0; j < 10; j++) {
            if (j == i) break;
            
            for (int k = 0; k < 5; k++) {
                if (k == 2) continue;
                result += i * j * k;
                
                if (result > 10000) {
                    goto early_exit;
                }
            }
        }
    }
    
early_exit:
    return result;
}

/* Function 5: Vector/SIMD-like operations */
void vector_like_operations(float* data, int size) {
    /* Declare many variables to act as vector elements */
    float v0, v1, v2, v3, v4, v5, v6, v7;
    float w0, w1, w2, w3, w4, w5, w6, w7;
    
    for (int i = 0; i < size; i += 8) {
        /* Load vector elements */
        v0 = data[i];
        v1 = data[i + 1];
        v2 = data[i + 2];
        v3 = data[i + 3];
        v4 = data[i + 4];
        v5 = data[i + 5];
        v6 = data[i + 6];
        v7 = data[i + 7];
        
        /* Vector operations */
        w0 = v0 * 1.1f + v1;
        w1 = v1 * 1.2f + v2;
        w2 = v2 * 1.3f + v3;
        w3 = v3 * 1.4f + v4;
        w4 = v4 * 1.5f + v5;
        w5 = v5 * 1.6f + v6;
        w6 = v6 * 1.7f + v7;
        w7 = v7 * 1.8f + v0;
        
        /* More operations */
        v0 = w0 + w4;
        v1 = w1 + w5;
        v2 = w2 + w6;
        v3 = w3 + w7;
        v4 = w0 - w4;
        v5 = w1 - w5;
        v6 = w2 - w6;
        v7 = w3 - w7;
        
        /* Store results */
        data[i] = v0;
        data[i + 1] = v1;
        data[i + 2] = v2;
        data[i + 3] = v3;
        data[i + 4] = v4;
        data[i + 5] = v5;
        data[i + 6] = v6;
        data[i + 7] = v7;
        
        /* Pointer aliasing to prevent optimization */
        float* alias1 = data + i;
        float* alias2 = data + i + 4;
        *alias1 = *alias1 * 0.9f;
        *alias2 = *alias2 * 1.1f;
    }
}

/* Function 6: Computed goto for irreducible control flow */
int computed_goto_pressure(int* data, int size, int mode) {
    static void* jump_table[] = {
        &&label_0, &&label_1, &&label_2, &&label_3, &&label_4,
        &&label_5, &&label_6, &&label_7, &&label_8, &&label_9
    };
    
    int result = 0;
    int index = 0;
    
    /* Use computed goto to create irreducible CFG */
    goto *jump_table[mode % 10];
    
label_0:
    result = data[index++] * 2;
    if (index >= size) goto end;
    goto *jump_table[(result + 1) % 10];
    
label_1:
    result += data[index++] * 3;
    if (index >= size) goto end;
    goto *jump_table[(result + 2) % 10];
    
label_2:
    result -= data[index++] * 5;
    if (index >= size) goto end;
    goto *jump_table[(result + 3) % 10];
    
label_3:
    result ^= data[index++];
    if (index >= size) goto end;
    goto *jump_table[(result + 4) % 10];
    
label_4:
    result |= data[index++] << 2;
    if (index >= size) goto end;
    goto *jump_table[(result + 5) % 10];
    
label_5:
    result &= data[index++] | 0xF;
    if (index >= size) goto end;
    goto *jump_table[(result + 6) % 10];
    
label_6:
    result = ~result + data[index++];
    if (index >= size) goto end;
    goto *jump_table[(result + 7) % 10];
    
label_7:
    result = result * 7 + data[index++];
    if (index >= size) goto end;
    goto *jump_table[(result + 8) % 10];
    
label_8:
    result = result / 3 + data[index++];
    if (index >= size) goto end;
    goto *jump_table[(result + 9) % 10];
    
label_9:
    result = result % 100 + data[index++];
    if (index >= size) goto end;
    goto *jump_table[(result + 10) % 10];
    
end:
    return result;
}

/* Main function that orchestrates all tests */
int main() {
    printf("Starting MCF stress test...\n");
    
    /* Initialize large arrays with random data */
    int* int_data = (int*)malloc(ARRAY_SIZE * sizeof(int));
    float* float_data = (float*)malloc(ARRAY_SIZE * sizeof(float));
    MixedData* mixed_data = (MixedData*)malloc(ARRAY_SIZE / 10 * sizeof(MixedData));
    
    if (!int_data || !float_data || !mixed_data) {
        printf("Memory allocation failed\n");
        return 1;
    }
    
    srand(time(NULL));
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_data[i] = rand() % 1000;
        float_data[i] = (float)rand() / RAND_MAX * 100.0f;
    }
    
    for (int i = 0; i < ARRAY_SIZE / 10; i++) {
        mixed_data[i].c = (char)(rand() % 256);
        mixed_data[i].s = (short)(rand() % 65536);
        mixed_data[i].i = rand() % 10000;
        mixed_data[i].l = (long)rand() * rand();
        mixed_data[i].f = (float)rand() / RAND_MAX;
        mixed_data[i].d = (double)rand() / RAND_MAX;
        mixed_data[i].p = NULL;
    }
    
    int total_result = 0;
    
    /* Warm-up iterations for profile feedback */
    printf("Warm-up phase...\n");
    for (int warmup = 0; warmup < 5; warmup++) {
        total_result ^= complex_loop_pressure(int_data, ARRAY_SIZE / 10);
        asm volatile("" ::: "memory"); /* Prevent optimization across calls */
    }
    
    /* Main test iterations */
    printf("Main test phase...\n");
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Test 1: Complex loops */
        total_result += complex_loop_pressure(int_data, ARRAY_SIZE / 20);
        
        /* Memory barrier between tests */
        asm volatile("" ::: "memory");
        
        /* Test 2: Switch CFG */
        total_result += switch_cfg_pressure(iter, int_data, ARRAY_SIZE / 100);
        
        /* Test 3: Inline assembly register pressure */
        asm_register_pressure(mixed_data, ARRAY_SIZE / 1000);
        
        /* Test 4: Many arguments */
        total_result += multi_arg_pressure(
            iter, iter+1, iter+2, iter+3, iter+4,
            iter+5, iter+6, iter+7, iter+8, iter+9,
            (float)iter * 0.1f, (float)iter * 0.2f, (float)iter * 0.3f,
            (double)iter * 0.4, (double)iter * 0.5
        );
        
        /* Test 5: Vector operations */
        vector_like_operations(float_data, ARRAY_SIZE / 10);
        
        /* Test 6: Computed goto */
        total_result += computed_goto_pressure(int_data, ARRAY_SIZE / 50, iter);
        
        /* Update global barrier to prevent optimization */
        global_barrier = iter;
    }
    
    /* Final verification computation */
    int checksum = 0;
    for (int i = 0; i < ARRAY_SIZE / 100; i++) {
        checksum ^= int_data[i];
        checksum += (int)(float_data[i] * 1000);
    }
    
    total_result = (total_result + checksum) % 1000000000;
    
    printf("Test completed. Result: %d\n", total_result);
    printf("Checksum: %d\n", checksum);
    
    /* Cleanup */
    free(int_data);
    free(float_data);
    free(mixed_data);
    
    return 0;
}
