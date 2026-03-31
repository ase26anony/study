/* mcf_test.c - Test program to trigger min-cost flow debug output in GCC */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

#define ARRAY_SIZE 15000
#define ITERATIONS 100
#define NUM_CASES 15

/* Volatile variables to prevent optimization and extend live ranges */
volatile int global_seed = 42;
volatile int global_barrier = 0;

/* Complex struct to stress register allocation with mixed types */
typedef struct {
    int a;
    double b;
    short c;
    long d;
    float e;
    char f;
    int64_t g;
    uint32_t h;
} MixedData;

/* Function 1: Deeply nested loops with many live ranges */
int complex_loop_pattern(int *data, int size) {
    int sum = 0;
    int prod = 1;
    int diff = 0;
    int xor_val = 0;
    int temp1, temp2, temp3, temp4, temp5;
    double fp_sum = 0.0;
    float fp_prod = 1.0f;
    
    /* Declare variables at function scope but use in nested blocks */
    int outer_counter = 0;
    int middle_counter = 0;
    int inner_counter = 0;
    
    for (int i = 0; i < size; i += 4) {
        outer_counter++;
        temp1 = data[i];
        
        for (int j = i + 1; j < size && j < i + 100; j += 3) {
            middle_counter++;
            temp2 = data[j];
            
            for (int k = j + 1; k < size && k < j + 50; k += 2) {
                inner_counter++;
                temp3 = data[k];
                
                /* Complex expression with many intermediates */
                sum += temp1 * temp2 - temp3 + (temp1 >> 2) * (temp2 << 1);
                prod *= (temp1 & 0xFF) + (temp2 & 0xF0) - (temp3 & 0x0F);
                diff -= temp1 - temp2 + temp3;
                xor_val ^= temp1 ^ temp2 ^ temp3;
                
                /* Floating point operations stress different register classes */
                fp_sum += (double)temp1 * 0.5 + (double)temp2 * 0.25;
                fp_prod *= (float)temp3 * 0.1f;
                
                /* Early continue creates complex control flow */
                if ((k % 7) == 0) {
                    continue;
                }
                
                /* Nested if-else chain */
                if (temp1 > temp2) {
                    if (temp2 > temp3) {
                        sum += temp1 * 100;
                    } else if (temp1 > temp3) {
                        sum += temp2 * 50;
                    } else {
                        sum += temp3 * 25;
                    }
                } else {
                    if (temp1 > temp3) {
                        diff -= temp1 * 10;
                    } else if (temp2 > temp3) {
                        diff -= temp2 * 5;
                    } else {
                        diff -= temp3 * 2;
                    }
                }
                
                /* Break at different nesting levels */
                if (inner_counter > 1000) {
                    break;
                }
            }
            
            if (middle_counter > 5000) {
                continue;
            }
        }
        
        /* Use all temporaries in final computation */
        temp4 = sum ^ diff;
        temp5 = prod & xor_val;
        sum = (temp4 + temp5) * outer_counter;
    }
    
    /* Force all variables to be live at return */
    asm volatile("" : : "r"(sum), "r"(prod), "r"(diff), "r"(xor_val),
                   "r"(outer_counter), "r"(middle_counter), "r"(inner_counter));
    
    return sum + (int)fp_sum + (int)fp_prod;
}

/* Function 2: Switch statement with many cases and fall-through */
int complex_switch_pattern(int value, int *data, int size) {
    int result = 0;
    
    switch (value % NUM_CASES) {
        case 0:
            result = data[0] * 2;
            /* Fall through */
        case 1:
            result += data[1] * 3;
            break;
        case 2:
            result = data[2] - data[3];
            /* Fall through */
        case 3:
            result *= data[4];
            /* Fall through */
        case 4:
            result /= (data[5] + 1);
            break;
        case 5:
            result = data[6] ^ data[7];
            /* Fall through */
        case 6:
            result |= data[8];
            /* Fall through */
        case 7:
            result &= data[9];
            break;
        case 8:
            result = data[10] << (data[11] & 0x7);
            /* Fall through */
        case 9:
            result >>= (data[12] & 0x7);
            break;
        case 10:
            result = ~data[13];
            /* Fall through */
        case 11:
            result += data[14] * 2;
            /* Fall through */
        case 12:
            result -= data[15];
            break;
        case 13:
            result = data[16] + data[17] - data[18];
            /* Fall through */
        case 14:
            result *= 2;
            break;
        default:
            result = -1;
    }
    
    /* Complex loop after switch */
    for (int i = 0; i < size; i++) {
        switch (result % 5) {
            case 0: result += data[i]; break;
            case 1: result -= data[i] * i; break;
            case 2: result ^= data[i]; break;
            case 3: result |= data[i] & 0xFF; break;
            case 4: result = (result << 1) | (data[i] & 1); break;
        }
        
        /* Early return creates additional control flow edges */
        if (result > 1000000) {
            return result;
        }
        
        if (result < -1000000) {
            result = -result;
            continue;
        }
    }
    
    return result;
}

/* Function 3: Inline assembly with explicit register constraints */
int asm_register_pressure(int *data, int size) {
    int a, b, c, d, e, f, g, h;
    int result = 0;
    
    /* Force specific registers with inline asm */
    for (int i = 0; i < size; i += 8) {
        /* Compete for EAX/RAX */
        asm volatile (
            "movl %1, %%eax\n\t"
            "imull %%eax, %%eax\n\t"
            "movl %%eax, %0\n\t"
            : "=r" (a)
            : "r" (data[i])
            : "%eax", "memory"
        );
        
        /* Compete for EBX/RBX */
        asm volatile (
            "movl %1, %%ebx\n\t"
            "addl $100, %%ebx\n\t"
            "movl %%ebx, %0\n\t"
            : "=r" (b)
            : "r" (data[i+1])
            : "%ebx", "memory"
        );
        
        /* Compete for ECX/RCX */
        asm volatile (
            "movl %1, %%ecx\n\t"
            "shrl $2, %%ecx\n\t"
            "movl %%ecx, %0\n\t"
            : "=r" (c)
            : "r" (data[i+2])
            : "%ecx", "memory"
        );
        
        /* Compete for EDX/RDX */
        asm volatile (
            "movl %1, %%edx\n\t"
            "mull %2\n\t"
            "movl %%eax, %0\n\t"
            : "=r" (d)
            : "r" (data[i+3]), "r" (data[i+4])
            : "%eax", "%edx", "memory"
        );
        
        /* More register pressure */
        asm volatile (
            "movl %1, %%esi\n\t"
            "movl %2, %%edi\n\t"
            "addl %%esi, %%edi\n\t"
            "movl %%edi, %0\n\t"
            : "=r" (e)
            : "r" (data[i+5]), "r" (data[i+6])
            : "%esi", "%edi", "memory"
        );
        
        /* Use all constrained values */
        f = a + b;
        g = c - d;
        h = e ^ f;
        
        result += (f * g) / (h + 1);
        
        /* Memory barrier to force spills */
        asm volatile("" ::: "memory");
    }
    
    return result;
}

/* Function 4: Vector-like operations using multiple SIMD registers */
void vector_operations(float *fa, float *fb, float *fc, int size) {
    float temp1, temp2, temp3, temp4, temp5, temp6, temp7, temp8;
    float sum1 = 0.0f, sum2 = 0.0f, sum3 = 0.0f, sum4 = 0.0f;
    
    for (int i = 0; i < size; i += 8) {
        /* Unrolled to create more register pressure */
        temp1 = fa[i] * fb[i] + fc[i];
        temp2 = fa[i+1] * fb[i+1] - fc[i+1];
        temp3 = fa[i+2] / (fb[i+2] + 1.0f) * fc[i+2];
        temp4 = fa[i+3] + fb[i+3] * fc[i+3];
        temp5 = fa[i+4] - fb[i+4] / fc[i+4];
        temp6 = fa[i+5] * 2.0f + fb[i+5] * 3.0f;
        temp7 = fa[i+6] / 4.0f - fb[i+6] / 5.0f;
        temp8 = fa[i+7] * fb[i+7] * fc[i+7];
        
        /* Complex dependency chain */
        sum1 += temp1 * temp2 - temp3;
        sum2 += temp4 / temp5 + temp6;
        sum3 += temp7 * temp8 * 0.5f;
        sum4 = sum1 * sum2 - sum3;
        
        /* Conditional that creates control flow */
        if (sum4 > 1000.0f) {
            sum1 *= 0.9f;
            sum2 *= 0.8f;
            sum3 *= 0.7f;
        } else if (sum4 < -1000.0f) {
            sum1 /= 0.9f;
            sum2 /= 0.8f;
            sum3 /= 0.7f;
        }
        
        /* Write back results */
        fc[i] = sum1;
        fc[i+1] = sum2;
        fc[i+2] = sum3;
        fc[i+3] = sum4;
        
        /* Rotate values */
        float t = sum1;
        sum1 = sum2;
        sum2 = sum3;
        sum3 = sum4;
        sum4 = t;
    }
    
    /* Force all floats to be processed */
    asm volatile("" : : "f"(sum1), "f"(sum2), "f"(sum3), "f"(sum4));
}

/* Function 5: Mixed data types and pointer aliasing */
long mixed_type_stress(MixedData *md, int count) {
    long total = 0;
    volatile MixedData *volatile_ptr = md; /* Prevent optimizations */
    
    for (int i = 0; i < count; i++) {
        /* Access through multiple pointer aliases */
        MixedData *ptr1 = &md[i];
        MixedData *ptr2 = ptr1 + 1;
        
        /* Force different register classes */
        int int_val = ptr1->a + ptr1->h;
        double dbl_val = ptr1->b * 2.0;
        short short_val = ptr1->c;
        long long_val = ptr1->d;
        float float_val = ptr1->e;
        char char_val = ptr1->f;
        int64_t i64_val = ptr1->g;
        
        /* Complex expression mixing all types */
        total += (long)(int_val * 2) +
                 (long)(dbl_val * 10.0) +
                 (long)(short_val * 3) +
                 long_val +
                 (long)(float_val * 5.0f) +
                 (long)(char_val * 7) +
                 (long)(i64_val >> 2);
        
        /* Pointer arithmetic that extends live ranges */
        if (i < count - 1) {
            ptr1->a = ptr2->h;
            ptr1->b = (double)ptr2->a;
            ptr1->c = (short)ptr2->b;
            ptr1->d = (long)ptr2->c;
            ptr1->e = (float)ptr2->d;
            ptr1->f = (char)ptr2->e;
            ptr1->g = (int64_t)ptr2->f;
            ptr1->h = (uint32_t)ptr2->g;
        }
        
        /* Memory clobber to force spills */
        asm volatile("" ::: "memory");
    }
    
    return total;
}

/* Function 6: Many function arguments to stress calling convention */
int many_arguments(int a, int b, int c, int d, int e, int f, int g, int h,
                   int i, int j, int k, int l, int m, int n, int o) {
    /* Use all arguments in complex ways */
    int sum = a + b + c;
    int prod = d * e * f;
    int diff = g - h - i;
    int xor_val = j ^ k ^ l;
    int and_val = m & n & o;
    
    /* Deeply nested conditionals */
    if (sum > prod) {
        if (diff > xor_val) {
            if (and_val > 0) {
                return sum * 2 - prod;
            } else {
                return diff * 3 + xor_val;
            }
        } else if (xor_val > and_val) {
            for (int x = 0; x < 10; x++) {
                sum += a * x;
                prod -= b * x;
                if (sum > 1000) break;
            }
            return sum + prod;
        }
    } else if (prod > diff) {
        switch (and_val % 4) {
            case 0: return a + c + e + g;
            case 1: return b + d + f + h;
            case 2: return i + j + k + l;
            case 3: return m + n + o + sum;
        }
    }
    
    /* Final complex computation */
    return (sum * prod) / (diff + 1) ^ (xor_val & and_val);
}

/* Function 7: Irreducible control flow with computed goto */
int irreducible_cfg(int *data, int size) {
    static void *labels[] = {
        &&label0, &&label1, &&label2, &&label3, &&label4,
        &&label5, &&label6, &&label7, &&label8, &&label9
    };
    
    int result = 0;
    int index = 0;
    
    for (int i = 0; i < size; i++) {
        index = data[i] % 10;
        
        /* Computed goto creates irreducible control flow */
        goto *labels[index];
        
    label0:
        result += data[i] * 2;
        continue;
    label1:
        result -= data[i] * 3;
        continue;
    label2:
        result ^= data[i];
        continue;
    label3:
        result |= data[i] & 0xFF;
        continue;
    label4:
        result &= data[i];
        continue;
    label5:
        result = result << 1;
        continue;
    label6:
        result = result >> 1;
        continue;
    label7:
        result = ~result;
        continue;
    label8:
        result += i * data[i];
        continue;
    label9:
        result -= i * data[i];
        continue;
    }
    
    return result;
}

/* Main function that orchestrates all tests */
int main() {
    /* Initialize large arrays */
    int *int_data = (int*)malloc(ARRAY_SIZE * sizeof(int));
    float *float_a = (float*)malloc(ARRAY_SIZE * sizeof(float));
    float *float_b = (float*)malloc(ARRAY_SIZE * sizeof(float));
    float *float_c = (float*)malloc(ARRAY_SIZE * sizeof(float));
    MixedData *mixed = (MixedData*)malloc((ARRAY_SIZE/10) * sizeof(MixedData));
    
    if (!int_data || !float_a || !float_b || !float_c || !mixed) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random data */
    srand(global_seed);
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_data[i] = rand() % 1000;
        float_a[i] = (float)(rand() % 1000) / 10.0f;
        float_b[i] = (float)(rand() % 1000) / 10.0f;
        float_c[i] = (float)(rand() % 1000) / 10.0f;
    }
    
    for (int i = 0; i < ARRAY_SIZE/10; i++) {
        mixed[i].a = rand() % 1000;
        mixed[i].b = (double)(rand() % 1000) / 10.0;
        mixed[i].c = (short)(rand() % 1000);
        mixed[i].d = (long)(rand() % 1000);
        mixed[i].e = (float)(rand() % 1000) / 10.0f;
        mixed[i].f = (char)(rand() % 256);
        mixed[i].g = (int64_t)(rand() % 1000);
        mixed[i].h = (uint32_t)(rand() % 1000);
    }
    
    int total_result = 0;
    
    /* Warm-up iterations for profile feedback */
    for (int warmup = 0; warmup < 5; warmup++) {
        total_result ^= complex_loop_pattern(int_data, ARRAY_SIZE/10);
        
        /* Memory barrier between tests */
        asm volatile("" ::: "memory");
        global_barrier = warmup;
    }
    
    /* Run all test patterns multiple times */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Test 1: Complex loops */
        total_result += complex_loop_pattern(int_data, ARRAY_SIZE/2);
        
        /* Memory barrier */
        asm volatile("" ::: "memory");
        
        /* Test 2: Switch statements */
        total_result ^= complex_switch_pattern(iter, int_data, ARRAY_SIZE/4);
        
        /* Memory barrier */
        asm volatile("" ::: "memory");
        
        /* Test 3: Inline assembly */
        total_result += asm_register_pressure(int_data, ARRAY_SIZE/8);
        
        /* Memory barrier */
        asm volatile("" ::: "memory");
        
        /* Test 4: Vector operations */
        vector_operations(float_a, float_b, float_c, ARRAY_SIZE/2);
        total_result += (int)float_c[0];
        
        /* Memory barrier */
        asm volatile("" ::: "memory");
        
        /* Test 5: Mixed types */
        total_result += (int)mixed_type_stress(mixed, ARRAY_SIZE/100);
        
        /* Memory barrier */
        asm volatile("" ::: "memory");
        
        /* Test 6: Many arguments */
        total_result += many_arguments(
            int_data[0], int_data[1], int_data[2], int_data[3], int_data[4],
            int_data[5], int_data[6], int_data[7], int_data[8], int_data[9],
            int_data[10], int_data[11], int_data[12], int_data[13], int_data[14]
        );
        
        /* Memory barrier */
        asm volatile("" ::: "memory");
        
        /* Test 7: Irreducible CFG */
        total_result ^= irreducible_cfg(int_data, ARRAY_SIZE/16);
        
        /* Update seed for next iteration */
        global_seed = total_result;
    }
    
    /* Print verifiable result */
    printf("Final checksum: %d\n", total_result);
    printf("Array element at mid-point: %d\n", int_data[ARRAY_SIZE/2]);
    printf("Float result: %f\n", float_c[ARRAY_SIZE/2]);
    
    /* Cleanup */
    free(int_data);
    free(float_a);
    free(float_b);
    free(float_c);
    free(mixed);
    
    return 0;
}
