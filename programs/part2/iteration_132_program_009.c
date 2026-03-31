/* mcf_test.c - Test program to trigger min-cost flow debug output in GCC */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

#define ARRAY_SIZE 10000
#define ITERATIONS 100
#define WARMUP_ITERATIONS 10

/* Volatile variables to prevent optimization */
volatile int global_seed = 42;
volatile int global_barrier = 0;

/* Complex data structure to stress register allocation */
typedef struct {
    int a, b, c, d;
    double x, y, z;
    float f1, f2, f3;
    char buffer[32];
} MixedData;

/* Function 1: Deeply nested loops with many live ranges */
int complex_loop_pattern(int *data, int size) {
    int sum = 0;
    int prod = 1;
    int diff = 0;
    int xor_val = 0;
    int temp1, temp2, temp3, temp4, temp5;
    
    /* Outer loop with many live variables */
    for (int i = 0; i < size; i++) {
        temp1 = data[i];
        
        /* First inner loop */
        for (int j = 0; j < 8; j++) {
            temp2 = temp1 * j;
            sum += temp2;
            
            /* Second inner loop */
            for (int k = 0; k < 4; k++) {
                temp3 = temp2 + k;
                prod *= (temp3 % 17) + 1;
                
                /* Complex expression with many intermediates */
                temp4 = (temp3 << 3) | (temp3 >> 5);
                temp5 = temp4 ^ (j * k);
                diff += temp5 - temp1;
                
                /* Keep variables alive across loop iterations */
                xor_val ^= temp5;
            }
            
            /* Use all temporaries in a complex expression */
            sum = (sum * 31 + prod * 17 + diff * 13 + xor_val * 7) % 1000000;
        }
        
        /* Conditional with early continue */
        if (temp1 % 7 == 0) {
            continue;
        }
        
        /* More computations keeping variables alive */
        prod = (prod * 1103515245 + 12345) & 0x7fffffff;
        diff = diff ^ (temp1 * 0x5bd1e995);
    }
    
    /* Final complex expression using all variables */
    return (sum + prod + diff + xor_val) & 0xffff;
}

/* Function 2: Switch statement with many cases and fall-through */
int complex_switch_pattern(int value, int *counter) {
    int result = 0;
    
    switch (value % 20) {
        case 0:
            result = value * 2;
            /* Fall through */
        case 1:
            result += value / 3;
            break;
        case 2:
            result = value << 4;
            /* Fall through */
        case 3:
        case 4:
            result |= 0xff;
            break;
        case 5:
            result = value ^ 0xaaaaaaaa;
            /* Fall through */
        case 6:
            result = ~result;
            break;
        case 7:
            result = value % 17;
            /* Fall through */
        case 8:
            result = result * 3 + 1;
            break;
        case 9:
            result = value & 0x55555555;
            /* Fall through */
        case 10:
            result = result >> 2;
            break;
        case 11:
            result = value + 0x12345678;
            /* Fall through */
        case 12:
            result = result - 0x87654321;
            break;
        case 13:
            result = value | 0xf0f0f0f0;
            /* Fall through */
        case 14:
            result = result & 0x0f0f0f0f;
            break;
        case 15:
            result = value * value;
            /* Fall through */
        case 16:
            result = result % 10007;
            break;
        case 17:
            result = ~value;
            /* Fall through */
        case 18:
            result = result + 1;
            break;
        case 19:
            result = 0;
            for (int i = 0; i < 8; i++) {
                result = (result << 4) | (value & 0xf);
                value >>= 4;
            }
            break;
        default:
            result = -1;
    }
    
    (*counter)++;
    return result;
}

/* Function 3: Inline assembly with register constraints */
void inline_asm_stress(int *input, int *output, int size) {
    int a, b, c, d, e, f, g, h;
    
    for (int i = 0; i < size; i += 8) {
        /* Force specific register allocations */
        asm volatile (
            "movl %1, %%eax\n\t"
            "movl %2, %%ebx\n\t"
            "movl %3, %%ecx\n\t"
            "movl %4, %%edx\n\t"
            "addl %%ebx, %%eax\n\t"
            "subl %%ecx, %%edx\n\t"
            "imull %%edx, %%eax\n\t"
            "movl %%eax, %0\n\t"
            : "=r" (a)
            : "r" (input[i]), "r" (input[i+1]), "r" (input[i+2]), "r" (input[i+3])
            : "%eax", "%ebx", "%ecx", "%edx", "memory"
        );
        
        /* More assembly with different constraints */
        asm volatile (
            "mov %1, %%r8d\n\t"
            "mov %2, %%r9d\n\t"
            "mov %3, %%r10d\n\t"
            "mov %4, %%r11d\n\t"
            "xor %%r9d, %%r8d\n\t"
            "ror $13, %%r8d\n\t"
            "add %%r10d, %%r11d\n\t"
            "sub %%r11d, %%r8d\n\t"
            "mov %%r8d, %0\n\t"
            : "=r" (b)
            : "r" (input[i+4]), "r" (input[i+5]), "r" (input[i+6]), "r" (input[i+7])
            : "%r8", "%r9", "%r10", "%r11", "memory"
        );
        
        /* Mix results */
        asm volatile (
            "movl %1, %%esi\n\t"
            "movl %2, %%edi\n\t"
            "leal (%%esi,%%edi,2), %%eax\n\t"
            "movl %%eax, %0\n\t"
            : "=r" (c)
            : "r" (a), "r" (b)
            : "%eax", "%esi", "%edi", "memory"
        );
        
        output[i/8] = c;
    }
}

/* Function 4: Vector-like operations with many SIMD-like calculations */
void vector_operations(float *data, int size, float *result) {
    float a0, a1, a2, a3, a4, a5, a6, a7;
    float b0, b1, b2, b3, b4, b5, b6, b7;
    float c0, c1, c2, c3, c4, c5, c6, c7;
    
    for (int i = 0; i < size; i += 16) {
        /* Load 8 values (simulating SIMD) */
        a0 = data[i];     a1 = data[i+1];   a2 = data[i+2];   a3 = data[i+3];
        a4 = data[i+4];   a5 = data[i+5];   a6 = data[i+6];   a7 = data[i+7];
        
        b0 = data[i+8];   b1 = data[i+9];   b2 = data[i+10];  b3 = data[i+11];
        b4 = data[i+12];  b5 = data[i+13];  b6 = data[i+14];  b7 = data[i+15];
        
        /* Complex vector operations keeping all values live */
        c0 = a0 * b0 + a1 * b1;
        c1 = a2 * b2 + a3 * b3;
        c2 = a4 * b4 + a5 * b5;
        c3 = a6 * b6 + a7 * b7;
        
        c4 = a0 - b0 + a1 - b1;
        c5 = a2 - b2 + a3 - b3;
        c6 = a4 - b4 + a5 - b5;
        c7 = a6 - b6 + a7 - b7;
        
        /* Cross-mix results */
        result[i/2] = c0 + c1 + c2 + c3;
        result[i/2 + 1] = c4 + c5 + c6 + c7;
        
        /* More operations to extend live ranges */
        a0 = c0 * 1.1f; a1 = c1 * 1.2f; a2 = c2 * 1.3f; a3 = c3 * 1.4f;
        a4 = c4 * 1.5f; a5 = c5 * 1.6f; a6 = c6 * 1.7f; a7 = c7 * 1.8f;
        
        /* Use all variables in final computation */
        result[i/2 + 2] = a0 + a1 + a2 + a3 + a4 + a5 + a6 + a7;
    }
}

/* Function 5: Mixed data types and complex control flow */
double mixed_data_operations(MixedData *data, int count) {
    double total = 0.0;
    float f_total = 0.0f;
    int i_total = 0;
    long l_total = 0;
    
    for (int i = 0; i < count; i++) {
        /* Complex conditional with early returns simulated */
        if (data[i].a % 3 == 0) {
            total += data[i].x * 2.5;
            f_total += data[i].f1 * 1.5f;
            
            if (data[i].b > 100) {
                i_total += data[i].c * 3;
                continue;
            }
        } else if (data[i].a % 3 == 1) {
            total -= data[i].y / 1.7;
            f_total -= data[i].f2 / 2.3f;
            
            if (data[i].d < 50) {
                i_total -= data[i].c / 2;
                break;
            }
        } else {
            total *= data[i].z * 0.9;
            f_total *= data[i].f3 * 0.8f;
        }
        
        /* More computations with mixed types */
        l_total += (long)data[i].a * data[i].b;
        i_total += data[i].c - data[i].d;
        
        /* Pointer aliasing to prevent optimization */
        int *alias1 = &data[i].a;
        int *alias2 = &data[i].b;
        *alias1 = (*alias1 ^ *alias2) + i;
        *alias2 = (*alias1 & *alias2) | i;
        
        /* Complex expression with many intermediates */
        double temp1 = data[i].x + data[i].y;
        double temp2 = data[i].z - data[i].x;
        double temp3 = temp1 * temp2;
        double temp4 = temp3 / (data[i].y + 1.0);
        double temp5 = temp4 * temp4;
        
        total += temp5;
    }
    
    /* Combine all totals */
    return total + f_total + i_total + l_total;
}

/* Function 6: Many function arguments to stress register/stack passing */
int many_arguments(int a, int b, int c, int d, int e, int f, int g, int h,
                   int i, int j, int k, int l, int m, int n, int o, int p) {
    /* Use all arguments in complex ways */
    int sum1 = a + b + c + d;
    int sum2 = e + f + g + h;
    int sum3 = i + j + k + l;
    int sum4 = m + n + o + p;
    
    int prod1 = a * b * c * d;
    int prod2 = e * f * g * h;
    int prod3 = i * j * k * l;
    int prod4 = m * n * o * p;
    
    int xor1 = a ^ b ^ c ^ d;
    int xor2 = e ^ f ^ g ^ h;
    int xor3 = i ^ j ^ k ^ l;
    int xor4 = m ^ n ^ o ^ p;
    
    /* Nested conditionals using all variables */
    if (sum1 > sum2) {
        if (prod1 < prod3) {
            return xor1 | xor2;
        } else {
            return xor3 & xor4;
        }
    } else {
        if (sum3 < sum4) {
            return prod1 ^ prod2;
        } else {
            return prod3 | prod4;
        }
    }
}

/* Function 7: Irreducible control flow with computed goto */
void irreducible_cfg(int *data, int size, int *result) {
    void *labels[] = {
        &&label0, &&label1, &&label2, &&label3, &&label4,
        &&label5, &&label6, &&label7, &&label8, &&label9
    };
    
    int state = 0;
    int counter = 0;
    
    for (int i = 0; i < size; i++) {
        int val = data[i];
        
        /* Jump table based on complex condition */
        int index = (val * 1103515245 + 12345) & 0x7fffffff;
        index = index % 10;
        
        goto *labels[index];
        
    label0:
        result[counter++] = val * 2;
        state = (state + 1) % 5;
        continue;
        
    label1:
        result[counter++] = val + 1;
        state = (state * 3) % 7;
        continue;
        
    label2:
        result[counter++] = val - 1;
        state = (state << 1) & 0xf;
        continue;
        
    label3:
        result[counter++] = val ^ 0xff;
        state = state ^ 0x0f;
        continue;
        
    label4:
        result[counter++] = ~val;
        state = (state + 3) % 8;
        continue;
        
    label5:
        result[counter++] = val & 0x5555;
        state = state | 0x10;
        continue;
        
    label6:
        result[counter++] = val | 0xaaaa;
        state = state & 0x0f;
        continue;
        
    label7:
        result[counter++] = val << 3;
        state = state >> 1;
        continue;
        
    label8:
        result[counter++] = val >> 2;
        state = (state + 5) % 9;
        continue;
        
    label9:
        result[counter++] = val % 17;
        state = (state * 2 + 1) % 6;
        continue;
    }
}

/* Main test driver */
int main() {
    /* Initialize large arrays */
    int *int_data = (int*)malloc(ARRAY_SIZE * sizeof(int));
    float *float_data = (float*)malloc(ARRAY_SIZE * sizeof(float));
    int *int_result = (int*)malloc((ARRAY_SIZE/8) * sizeof(int));
    float *float_result = (float*)malloc((ARRAY_SIZE/2) * sizeof(float));
    MixedData *mixed_data = (MixedData*)malloc((ARRAY_SIZE/10) * sizeof(MixedData));
    
    /* Initialize with pseudo-random data */
    srand(global_seed);
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_data[i] = rand();
        float_data[i] = (float)rand() / RAND_MAX * 100.0f;
    }
    
    for (int i = 0; i < ARRAY_SIZE/10; i++) {
        mixed_data[i].a = rand();
        mixed_data[i].b = rand();
        mixed_data[i].c = rand();
        mixed_data[i].d = rand();
        mixed_data[i].x = (double)rand() / RAND_MAX;
        mixed_data[i].y = (double)rand() / RAND_MAX * 2.0;
        mixed_data[i].z = (double)rand() / RAND_MAX * 3.0;
        mixed_data[i].f1 = (float)rand() / RAND_MAX;
        mixed_data[i].f2 = (float)rand() / RAND_MAX * 1.5f;
        mixed_data[i].f3 = (float)rand() / RAND_MAX * 2.0f;
    }
    
    int final_result = 0;
    int switch_counter = 0;
    
    /* Warm-up phase (allow profile feedback to activate) */
    printf("Starting warm-up phase...\n");
    for (int warmup = 0; warmup < WARMUP_ITERATIONS; warmup++) {
        asm volatile("" ::: "memory");  /* Memory barrier */
        
        int r1 = complex_loop_pattern(int_data, ARRAY_SIZE/10);
        asm volatile("" ::: "memory");
        
        int r2 = complex_switch_pattern(warmup, &switch_counter);
        asm volatile("" ::: "memory");
        
        final_result ^= r1 ^ r2;
    }
    
    /* Main stress testing phase */
    printf("Starting main stress phase...\n");
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Memory barrier between different test patterns */
        asm volatile("" ::: "memory");
        
        /* Test 1: Complex loops */
        int r1 = complex_loop_pattern(int_data + iter, ARRAY_SIZE/20);
        final_result = (final_result * 31 + r1) & 0xffffffff;
        
        asm volatile("" ::: "memory");
        
        /* Test 2: Switch patterns */
        int r2 = complex_switch_pattern(iter, &switch_counter);
        final_result ^= r2 * 0x5bd1e995;
        
        asm volatile("" ::: "memory");
        
        /* Test 3: Inline assembly */
        inline_asm_stress(int_data + iter * 8, int_result, ARRAY_SIZE/100);
        for (int i = 0; i < 10; i++) {
            final_result += int_result[i];
        }
        
        asm volatile("" ::: "memory");
        
        /* Test 4: Vector operations */
        vector_operations(float_data + iter * 16, ARRAY_SIZE/50, float_result);
        for (int i = 0; i < 10; i++) {
            final_result += (int)float_result[i];
        }
        
        asm volatile("" ::: "memory");
        
        /* Test 5: Mixed data operations */
        double r5 = mixed_data_operations(mixed_data, ARRAY_SIZE/100);
        final_result += (int)r5;
        
        asm volatile("" ::: "memory");
        
        /* Test 6: Many arguments */
        int r6 = many_arguments(iter, iter+1, iter+2, iter+3,
                               iter+4, iter+5, iter+6, iter+7,
                               iter+8, iter+9, iter+10, iter+11,
                               iter+12, iter+13, iter+14, iter+15);
        final_result = (final_result ^ r6) * 1103515245 + 12345;
        
        asm volatile("" ::: "memory");
        
        /* Test 7: Irreducible CFG */
        irreducible_cfg(int_data + iter * 32, ARRAY_SIZE/200, int_result);
        for (int i = 0; i < 10; i++) {
            final_result += int_result[i];
        }
        
        /* Progress indicator */
        if (iter % 10 == 0) {
            printf("Iteration %d, intermediate result: %d\n", iter, final_result);
        }
    }
    
    /* Cleanup and final output */
    free(int_data);
    free(float_data);
    free(int_result);
    free(float_result);
    free(mixed_data);
    
    printf("Final result: %d\n", final_result);
    printf("Switch counter: %d\n", switch_counter);
    
    return final_result != 0 ? 0 : 1;
}
