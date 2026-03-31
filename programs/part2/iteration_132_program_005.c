/* mcf_trigger.c - Program to trigger min-cost flow debug output in GCC */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <math.h>

#define ARRAY_SIZE 15000
#define ITERATIONS 100
#define SWITCH_CASES 15

/* Volatile variables to prevent optimization and extend live ranges */
volatile int global_counter = 0;
volatile double global_accumulator = 0.0;

/* Complex structure to force different register classes */
typedef struct {
    char c;
    short s;
    int i;
    long l;
    float f;
    double d;
    void *p;
} MixedData;

/* Function with deeply nested loops and many live ranges */
void test_register_pressure_1(int *data, int size) {
    int i, j, k, l;
    int sum1 = 0, sum2 = 0, sum3 = 0, sum4 = 0;
    int tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8;
    double d1, d2, d3, d4;
    float f1, f2, f3, f4;
    
    /* Multiple nested loops creating many overlapping live ranges */
    for (i = 0; i < size / 4; i++) {
        tmp1 = data[i] * 2;
        tmp2 = data[i] + tmp1;
        d1 = (double)tmp1 * 3.14159;
        
        for (j = 0; j < 8; j++) {
            tmp3 = tmp2 + j;
            tmp4 = tmp3 * tmp1;
            f1 = (float)tmp3 * 2.71828f;
            
            for (k = 0; k < 4; k++) {
                tmp5 = tmp4 + k;
                tmp6 = tmp5 * tmp3;
                d2 = d1 * (double)k;
                
                for (l = 0; l < 2; l++) {
                    tmp7 = tmp6 + l;
                    tmp8 = tmp7 * tmp5;
                    f2 = f1 * (float)l;
                    
                    /* Complex expression with many intermediates */
                    sum1 += tmp8 + (int)(d2 * 100.0) + (int)(f2 * 10.0f);
                }
                
                sum2 += tmp6;
                d3 = d2 * d1;
                f3 = f2 * f1;
            }
            
            sum3 += tmp4;
            d4 = d3 * 2.0;
            f4 = f3 * 2.0f;
        }
        
        sum4 += tmp2;
        global_accumulator += d4 + f4;
    }
    
    /* Force all values to be used */
    asm volatile("" : : "r"(sum1), "r"(sum2), "r"(sum3), "r"(sum4));
    global_counter += sum1 + sum2 + sum3 + sum4;
}

/* Function with complex control flow and switch statements */
int test_control_flow_2(int *data, int size) {
    int result = 0;
    int i, state = 0;
    
    for (i = 0; i < size; i++) {
        int val = data[i] % SWITCH_CASES;
        
        /* Large switch statement creating complex CFG */
        switch (val) {
            case 0:
                result += val * 2;
                if (result > 1000) {
                    result /= 2;
                    goto early_exit;
                }
                break;
            case 1:
                result += val + 1;
                for (int j = 0; j < 3; j++) {
                    result += j;
                    if (j == 1) continue;
                    result *= 2;
                }
                break;
            case 2:
                result += val << 2;
                if (i % 2 == 0) {
                    return result;  /* Early return */
                }
                break;
            case 3:
                result += val * val;
                while (state < 5) {
                    result += state;
                    state++;
                    if (state == 3) break;
                }
                break;
            case 4:
                result += val / 2;
                do {
                    result -= 1;
                } while (result % 10 != 0);
                break;
            case 5:
                result += val | 0xFF;
                if (result & 1) {
                    result ^= 0xAAAA;
                } else {
                    result ^= 0x5555;
                }
                break;
            case 6:
                result += val & 0xF;
                for (int k = 0; k < 4; k++) {
                    if (k == 2) {
                        result += 100;
                        goto inner_label;
                    }
                    result += k * 10;
                }
                inner_label:
                result += 5;
                break;
            case 7:
                result += val * 3;
                /* Fall through */
            case 8:
                result += 8;
                if (val == 7) {
                    result += 7;
                }
                break;
            case 9:
                result += val - 1;
                /* Nested switch */
                switch (result % 3) {
                    case 0: result += 10; break;
                    case 1: result += 20; break;
                    case 2: result += 30; break;
                }
                break;
            case 10:
                result += val + 10;
                /* Multiple levels of if-else */
                if (result < 0) {
                    result = -result;
                } else if (result < 100) {
                    result += 50;
                } else if (result < 200) {
                    result += 25;
                } else {
                    result -= 25;
                }
                break;
            case 11:
                result += val * val;
                /* Loop with break at different level */
                for (int x = 0; x < 5; x++) {
                    for (int y = 0; y < 5; y++) {
                        if (x + y == 6) {
                            goto double_break;
                        }
                        result += x * y;
                    }
                }
                double_break:
                break;
            case 12:
                result += val << 1;
                /* Computed goto simulation */
                static void *labels[] = { &&l0, &&l1, &&l2 };
                goto *labels[val % 3];
                l0: result += 1; goto end_label;
                l1: result += 2; goto end_label;
                l2: result += 3; goto end_label;
                end_label:
                break;
            case 13:
                result += val % 5;
                /* Multiple continues */
                for (int c = 0; c < 10; c++) {
                    if (c % 2 == 0) continue;
                    if (c % 3 == 0) continue;
                    result += c;
                }
                break;
            case 14:
                result += val + 14;
                /* Deeply nested if-else chain */
                if (result % 2 == 0) {
                    if (result % 3 == 0) {
                        if (result % 5 == 0) {
                            result += 100;
                        } else {
                            result += 50;
                        }
                    } else {
                        if (result % 7 == 0) {
                            result += 25;
                        }
                    }
                } else {
                    if (result % 11 == 0) {
                        result += 10;
                    }
                }
                break;
        }
        
        /* Keep variable alive across switch */
        asm volatile("" : : "r"(state));
    }
    
    early_exit:
    return result;
}

/* Function with inline assembly forcing specific register allocation */
void test_asm_register_constraints(int *data, int size) {
    int a, b, c, d, e, f;
    long la, lb, lc;
    double da, db;
    
    for (int i = 0; i < size / 100; i++) {
        /* Force use of specific registers */
        asm volatile (
            "movl %1, %%eax\n\t"
            "addl %%eax, %0\n\t"
            : "=r" (a) : "r" (data[i]), "0" (a) : "%eax"
        );
        
        asm volatile (
            "movl %1, %%ebx\n\t"
            "imull %%ebx, %0\n\t"
            : "=r" (b) : "r" (data[i] + 1), "0" (b) : "%ebx"
        );
        
        asm volatile (
            "movq %1, %%rax\n\t"
            "addq %%rax, %0\n\t"
            : "=r" (la) : "r" ((long)data[i]), "0" (la) : "%rax"
        );
        
        /* Compete for same registers */
        asm volatile (
            "movl %%eax, %0\n\t"
            "movl %%ebx, %1\n\t"
            : "=r" (c), "=r" (d) : : "%eax", "%ebx", "memory"
        );
        
        /* Floating point with register constraints */
        da = (double)data[i];
        asm volatile (
            "addsd %1, %0\n\t"
            : "=x" (da) : "x" (da), "0" (da)
        );
        
        /* Multiple asm statements with memory clobber */
        asm volatile (
            "movl %1, %%ecx\n\t"
            "shll $2, %%ecx\n\t"
            "movl %%ecx, %0\n\t"
            : "=r" (e) : "r" (data[i]) : "%ecx", "memory"
        );
        
        asm volatile (
            "movl %1, %%edx\n\t"
            "rorl $4, %%edx\n\t"
            "movl %%edx, %0\n\t"
            : "=r" (f) : "r" (data[i]) : "%edx", "memory"
        );
        
        /* Force spilling by using many variables */
        a = a + b + c + d + e + f;
        la = la + (long)a;
        da = da + (double)b;
        
        /* Memory barrier to prevent optimization */
        asm volatile("" ::: "memory");
    }
    
    global_counter += a + (int)la + (int)da;
}

/* Function with many function calls and arguments */
double test_function_calls(MixedData *md, int count) {
    double total = 0.0;
    
    for (int i = 0; i < count; i++) {
        /* Function call with many arguments stressing calling convention */
        total += process_mixed_data(
            md[i].c, md[i].s, md[i].i, md[i].l,
            md[i].f, md[i].d, md[i].p, i,
            total, (double)i, (void*)&md[i],
            md[i].c + md[i].s, md[i].i * 2,
            md[i].l / 3, md[i].f * 4.0f
        );
    }
    
    return total;
}

/* Helper function with many parameters */
double process_mixed_data(char c, short s, int i, long l,
                         float f, double d, void *p, int idx,
                         double accum, double scale, void *ctx,
                         int extra1, int extra2, long extra3, float extra4) {
    /* Complex calculations using all parameters */
    double result = (double)c + (double)s + (double)i + (double)l;
    result += (double)f + d;
    result += (double)(intptr_t)p * 0.0001;
    result += accum * scale;
    result += (double)(extra1 + extra2 + extra3 + (long)extra4);
    
    /* More register pressure */
    int tmp1 = c * s;
    int tmp2 = i / 2;
    long tmp3 = l + extra3;
    float tmp4 = f * extra4;
    double tmp5 = d * scale;
    
    result += tmp1 + tmp2 + tmp3 + tmp4 + tmp5;
    
    /* Pointer aliasing to prevent optimizations */
    volatile int *alias = (volatile int*)&i;
    *alias = *alias + 1;
    
    return result;
}

/* Function with vector-like operations */
void test_vector_ops(int *data, int size) {
    /* Manual vectorization stressing SIMD registers */
    for (int i = 0; i < size - 7; i += 8) {
        int v0 = data[i];
        int v1 = data[i+1];
        int v2 = data[i+2];
        int v3 = data[i+3];
        int v4 = data[i+4];
        int v5 = data[i+5];
        int v6 = data[i+6];
        int v7 = data[i+7];
        
        /* Parallel operations */
        int s0 = v0 + v1;
        int s1 = v2 + v3;
        int s2 = v4 + v5;
        int s3 = v6 + v7;
        
        int p0 = v0 * v1;
        int p1 = v2 * v3;
        int p2 = v4 * v5;
        int p3 = v6 * v7;
        
        /* More operations keeping many values live */
        int t0 = s0 + s1;
        int t1 = s2 + s3;
        int t2 = p0 + p1;
        int t3 = p2 + p3;
        
        int u0 = t0 * t1;
        int u1 = t2 * t3;
        
        int final = u0 + u1;
        
        /* Store result creating store-load dependencies */
        data[i] = final;
        
        /* Many live variables across loop iteration */
        asm volatile("" : : "r"(v0), "r"(v1), "r"(v2), "r"(v3),
                         "r"(v4), "r"(v5), "r"(v6), "r"(v7),
                         "r"(s0), "r"(s1), "r"(s2), "r"(s3));
    }
}

/* Main function that orchestrates all tests */
int main() {
    /* Allocate large arrays */
    int *data1 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    int *data2 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    MixedData *md = (MixedData*)malloc((ARRAY_SIZE/10) * sizeof(MixedData));
    
    if (!data1 || !data2 || !md) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with random data */
    srand(time(NULL));
    for (int i = 0; i < ARRAY_SIZE; i++) {
        data1[i] = rand() % 1000;
        data2[i] = rand() % 1000;
    }
    
    for (int i = 0; i < ARRAY_SIZE/10; i++) {
        md[i].c = rand() % 256;
        md[i].s = rand() % 65536;
        md[i].i = rand();
        md[i].l = (long)rand() * rand();
        md[i].f = (float)rand() / RAND_MAX;
        md[i].d = (double)rand() / RAND_MAX;
        md[i].p = (void*)(intptr_t)rand();
    }
    
    int checksum = 0;
    double fp_checksum = 0.0;
    
    /* Warm-up iterations for profile feedback */
    for (int warmup = 0; warmup < 5; warmup++) {
        test_register_pressure_1(data1, ARRAY_SIZE/4);
        asm volatile("" ::: "memory");  /* Memory barrier */
    }
    
    /* Main test iterations */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Test 1: Register pressure */
        test_register_pressure_1(data1, ARRAY_SIZE);
        checksum += global_counter;
        
        /* Memory barrier between tests */
        asm volatile("" ::: "memory");
        
        /* Test 2: Control flow complexity */
        checksum += test_control_flow_2(data2, ARRAY_SIZE/2);
        
        /* Test 3: Inline assembly */
        test_asm_register_constraints(data1, ARRAY_SIZE);
        checksum += global_counter;
        
        /* Test 4: Function calls */
        fp_checksum += test_function_calls(md, ARRAY_SIZE/100);
        
        /* Test 5: Vector operations */
        test_vector_ops(data2, ARRAY_SIZE);
        
        /* Mix data to create different patterns */
        for (int i = 0; i < ARRAY_SIZE; i++) {
            data1[i] = (data1[i] + data2[i]) % 1000;
            data2[i] = (data2[i] * 13 + 7) % 1000;
        }
        
        /* Progress indicator */
        if (iter % 10 == 0) {
            fprintf(stderr, "Iteration %d/%d\n", iter, ITERATIONS);
        }
    }
    
    /* Final computation for verifiable result */
    int final_result = checksum + (int)fp_checksum;
    
    /* Process arrays one more time */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        final_result ^= data1[i];
        final_result += data2[i];
    }
    
    /* Use all data to prevent dead code elimination */
    for (int i = 0; i < ARRAY_SIZE/10; i++) {
        final_result += md[i].c + md[i].s + md[i].i;
    }
    
    printf("Final checksum: %d\n", final_result % 1000000);
    printf("Global counter: %d\n", global_counter);
    printf("FP accumulator: %f\n", global_accumulator);
    
    /* Cleanup */
    free(data1);
    free(data2);
    free(md);
    
    return 0;
}
