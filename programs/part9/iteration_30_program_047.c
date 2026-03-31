/* reload_stress.c - Extreme register pressure test for GCC reload pass */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define ARRAY_SIZE 1024
#define NUM_LOCALS 40
#define ITERATIONS 1000

/* Opaque noinline functions to prevent optimization */
int __attribute__((noinline)) helper1(int a, int b, int c, int d, int e, int f) {
    volatile int sink = a + b - c + d - e + f;
    return sink;
}

double __attribute__((noinline)) helper2(double a, double b, double c, 
                                         double d, double e, double f) {
    volatile double sink = a * b / c + d - e * f;
    return sink;
}

void __attribute__((noinline)) helper3(int* addr1, double* addr2, 
                                       long* addr3, float* addr4) {
    volatile int sink1 = *addr1;
    volatile double sink2 = *addr2;
    volatile long sink3 = *addr3;
    volatile float sink4 = *addr4;
    (void)sink1; (void)sink2; (void)sink3; (void)sink4;
}

void __attribute__((noinline)) helper4(int a, double b, long c, float d,
                                       int e, double f, long g, float h) {
    volatile int sink = a + (int)b + (int)c + (int)d + e + (int)f + (int)g + (int)h;
    (void)sink;
}

/* Main stress function with extreme register pressure */
int __attribute__((noinline)) stress_reload(int* arr_int, double* arr_dbl, 
                                            long* arr_long, float* arr_flt) {
    /* Declare many local variables to exhaust registers */
    /* Group 1: Integer variables */
    volatile int v1 = arr_int[0];
    volatile int v2 = arr_int[1];
    volatile int v3 = arr_int[2];
    volatile int v4 = arr_int[3];
    volatile int v5 = arr_int[4];
    volatile int v6 = arr_int[5];
    volatile int v7 = arr_int[6];
    volatile int v8 = arr_int[7];
    volatile int v9 = arr_int[8];
    volatile int v10 = arr_int[9];
    
    /* Group 2: Floating point variables */
    volatile double d1 = arr_dbl[0];
    volatile double d2 = arr_dbl[1];
    volatile double d3 = arr_dbl[2];
    volatile double d4 = arr_dbl[3];
    volatile double d5 = arr_dbl[4];
    volatile double d6 = arr_dbl[5];
    volatile double d7 = arr_dbl[6];
    volatile double d8 = arr_dbl[7];
    
    /* Group 3: Long variables */
    volatile long l1 = arr_long[0];
    volatile long l2 = arr_long[1];
    volatile long l3 = arr_long[2];
    volatile long l4 = arr_long[3];
    volatile long l5 = arr_long[4];
    volatile long l6 = arr_long[5];
    
    /* Group 4: Float variables */
    volatile float f1 = arr_flt[0];
    volatile float f2 = arr_flt[1];
    volatile float f3 = arr_flt[2];
    volatile float f4 = arr_flt[3];
    volatile float f5 = arr_flt[4];
    volatile float f6 = arr_flt[5];
    
    /* Group 5: More mixed types */
    volatile int v11, v12, v13, v14, v15;
    volatile double d9, d10, d11, d12;
    volatile long l7, l8, l9, l10;
    volatile float f7, f8, f9, f10;
    
    /* Initialize remaining variables */
    v11 = arr_int[10]; v12 = arr_int[11]; v13 = arr_int[12];
    v14 = arr_int[13]; v15 = arr_int[14];
    d9 = arr_dbl[8]; d10 = arr_dbl[9]; d11 = arr_dbl[10]; d12 = arr_dbl[11];
    l7 = arr_long[6]; l8 = arr_long[7]; l9 = arr_long[8]; l10 = arr_long[9];
    f7 = arr_flt[6]; f8 = arr_flt[7]; f9 = arr_flt[8]; f10 = arr_flt[9];
    
    int checksum = 0;
    
    /* Complex loop with extreme register pressure */
    for (int i = 0; i < ITERATIONS; i++) {
        /* Complex array indexing with multi-term expressions */
        int idx1 = (i * 7 + v1 * 3 + v2 * 5) % ARRAY_SIZE;
        int idx2 = (i * 11 + v3 * 13 + v4 * 17) % ARRAY_SIZE;
        int idx3 = (i * 19 + v5 * 23 + v6 * 29) % ARRAY_SIZE;
        int idx4 = (i * 31 + v7 * 37 + v8 * 41) % ARRAY_SIZE;
        
        /* Address computations that will need reloads */
        int* addr_int1 = &arr_int[idx1];
        int* addr_int2 = &arr_int[idx2];
        double* addr_dbl1 = &arr_dbl[idx1];
        double* addr_dbl2 = &arr_dbl[idx2];
        long* addr_long1 = &arr_long[idx3];
        long* addr_long2 = &arr_long[idx4];
        float* addr_flt1 = &arr_flt[idx3];
        float* addr_flt2 = &arr_flt[idx4];
        
        /* More complex address computations with offsets */
        int* addr_int3 = &arr_int[(idx1 + idx2 * 3 + v9) % ARRAY_SIZE];
        double* addr_dbl3 = &arr_dbl[(idx3 + idx4 * 5 + v10) % ARRAY_SIZE];
        
        /* Control flow that splits live ranges */
        switch (i % 8) {
            case 0: {
                /* Use inline assembly with conflicting constraints */
                int temp1, temp2;
                asm volatile (
                    "mov %[src], %[dst1]\n\t"
                    "add %[val1], %[dst1]\n\t"
                    "mov %[dst1], %[dst2]"
                    : [dst1] "=r" (temp1), [dst2] "=r" (temp2)
                    : [src] "r" (v1), [val1] "r" (v2)
                    : "cc"
                );
                checksum += temp1 + temp2;
                
                /* Force address reloads */
                asm volatile (
                    "mov %[addr], %%rax\n\t"
                    "mov (%%rax), %[val]"
                    : [val] "=r" (temp1)
                    : [addr] "r" (addr_int1)
                    : "rax", "memory"
                );
                v1 = temp1;
                break;
            }
            
            case 1: {
                /* Different addressing mode */
                double temp_d;
                asm volatile (
                    "movsd (%[addr]), %[val]"
                    : [val] "=x" (temp_d)
                    : [addr] "r" (addr_dbl1)
                    : "memory"
                );
                d1 = temp_d;
                
                /* Complex address computation in separate statement */
                int offset = (v3 * 7 + v4 * 11) % 64;
                int* complex_addr = addr_int2 + offset;
                asm volatile (
                    "mov (%[addr]), %[val]"
                    : [val] "=r" (v3)
                    : [addr] "r" (complex_addr)
                    : "memory"
                );
                break;
            }
            
            case 2: {
                /* Multiple address computations */
                long* addr1 = addr_long1 + (v5 % 16);
                long* addr2 = addr_long2 + (v6 % 16);
                
                asm volatile (
                    "mov (%[a1]), %%rax\n\t"
                    "add (%[a2]), %%rax\n\t"
                    "mov %%rax, %[res]"
                    : [res] "=r" (l1)
                    : [a1] "r" (addr1), [a2] "r" (addr2)
                    : "rax", "memory"
                );
                break;
            }
            
            case 3: {
                /* Mixed type address computation */
                float* faddr = addr_flt1 + (v7 % 32);
                double* daddr = addr_dbl2 + (v8 % 32);
                
                /* Force spilling around function call */
                helper3(addr_int3, daddr, addr_long1, faddr);
                break;
            }
            
            case 4: {
                /* Output address reload */
                int output;
                asm volatile (
                    "mov %[val], (%[addr])"
                    : 
                    : [val] "r" (v9), [addr] "r" (addr_int2)
                    : "memory"
                );
                
                /* Input address reload for different operand */
                asm volatile (
                    "mov (%[addr]), %[out]"
                    : [out] "=r" (output)
                    : [addr] "r" (addr_int1)
                    : "memory"
                );
                v10 = output;
                break;
            }
            
            case 5: {
                /* Complex expression requiring many temps */
                int complex_idx = (v11 * v12 + v13 * v14 - v15) % ARRAY_SIZE;
                int* volatile complex_ptr = &arr_int[complex_idx];
                
                /* Use in inline asm with multiple constraints */
                int result;
                asm volatile (
                    "imul %[a], %[b]\n\t"
                    "add %[c], %[b]\n\t"
                    "mov (%[ptr]), %[res]"
                    : [res] "=r" (result), [b] "+r" (v11)
                    : [a] "r" (v12), [c] "r" (v13), [ptr] "r" (complex_ptr)
                    : "cc", "memory"
                );
                checksum += result;
                break;
            }
            
            case 6: {
                /* Force operand address reloads */
                helper4(v1, d1, l1, f1, v2, d2, l2, f2);
                
                /* Additional computation to keep variables live */
                v3 = v1 + v2;
                d3 = d1 * d2;
                l3 = l1 ^ l2;
                f3 = f1 / f2;
                break;
            }
            
            case 7: {
                /* Use computed goto for non-trivial control flow */
                static void* labels[] = { &&label1, &&label2, &&label3 };
                int label_idx = i % 3;
                
                goto *labels[label_idx];
                
            label1:
                /* Address computation in this block */
                int* addr = &arr_int[(v4 * 3 + v5 * 7) % ARRAY_SIZE];
                v4 = *addr;
                goto end_switch;
                
            label2:
                /* Use in different block */
                v5 = arr_int[(v6 * 5 + v7 * 11) % ARRAY_SIZE];
                goto end_switch;
                
            label3:
                /* Another block with different computation */
                v6 = arr_int[(v8 * 13 + v9 * 17) % ARRAY_SIZE];
                goto end_switch;
                
            end_switch:
                break;
            }
        }
        
        /* Update most variables to keep them live */
        v1 = v1 * 3 + 1;
        v2 = v2 * 5 - 1;
        v3 = v3 ^ v4;
        v4 = v4 + v5;
        v5 = v5 - v6;
        v6 = v6 * 7 + v7;
        v7 = v7 / 3 + v8;
        v8 = v8 ^ v9;
        v9 = v9 + v10;
        v10 = v10 - v11;
        v11 = v11 * 11 + v12;
        v12 = v12 ^ v13;
        v13 = v13 + v14;
        v14 = v14 - v15;
        
        d1 = d1 * 1.1;
        d2 = d2 / 1.2;
        d3 = d3 + d4;
        d4 = d4 - d5;
        d5 = d5 * d6;
        d6 = d6 / d7;
        d7 = d7 + d8;
        d8 = d8 - d9;
        
        l1 = l1 << 1;
        l2 = l2 >> 1;
        l3 = l3 ^ l4;
        l4 = l4 + l5;
        l5 = l5 - l6;
        l6 = l6 * 3;
        
        f1 = f1 * 1.5f;
        f2 = f2 / 2.0f;
        f3 = f3 + f4;
        f4 = f4 - f5;
        f5 = f5 * f6;
        f6 = f6 / f7;
        
        /* Call helpers with different argument combinations */
        if (i % 3 == 0) {
            checksum += helper1(v1, v2, v3, v4, v5, v6);
        } else if (i % 3 == 1) {
            d10 = helper2(d1, d2, d3, d4, d5, d6);
        } else {
            helper3(&v7, &d7, &l7, &f7);
        }
        
        /* Prevent loop invariant motion */
        asm volatile ("" : : "r"(v1), "r"(v2), "r"(v3), "r"(v4), "r"(v5),
                         "r"(d1), "r"(d2), "r"(d3), "r"(d4),
                         "r"(l1), "r"(l2), "r"(l3),
                         "r"(f1), "r"(f2), "r"(f3) : "memory");
    }
    
    /* Final checksum computation using all variables */
    checksum += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                v11 + v12 + v13 + v14 + v15 +
                (int)d1 + (int)d2 + (int)d3 + (int)d4 + (int)d5 +
                (int)d6 + (int)d7 + (int)d8 + (int)d9 + (int)d10 +
                (int)l1 + (int)l2 + (int)l3 + (int)l4 + (int)l5 +
                (int)l6 + (int)l7 + (int)l8 + (int)l9 + (int)l10 +
                (int)f1 + (int)f2 + (int)f3 + (int)f4 + (int)f5 +
                (int)f6 + (int)f7 + (int)f8 + (int)f9 + (int)f10;
    
    return checksum;
}

int main() {
    /* Allocate and initialize arrays with pattern data */
    int* arr_int = (int*)malloc(ARRAY_SIZE * sizeof(int));
    double* arr_dbl = (double*)malloc(ARRAY_SIZE * sizeof(double));
    long* arr_long = (long*)malloc(ARRAY_SIZE * sizeof(long));
    float* arr_flt = (float*)malloc(ARRAY_SIZE * sizeof(float));
    
    if (!arr_int || !arr_dbl || !arr_long || !arr_flt) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr_int[i] = i * 3 + 1;
        arr_dbl[i] = i * 1.5;
        arr_long[i] = i * 7L + 3L;
        arr_flt[i] = i * 2.5f;
    }
    
    printf("Starting reload stress test...\n");
    
    /* Call the stress function */
    int result = stress_reload(arr_int, arr_dbl, arr_long, arr_flt);
    
    printf("Result: %d\n", result);
    printf("Test completed.\n");
    
    /* Cleanup */
    free(arr_int);
    free(arr_dbl);
    free(arr_long);
    free(arr_flt);
    
    return 0;
}
