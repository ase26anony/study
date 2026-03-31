/* reload_stress_test.c */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Opaque functions to prevent optimization */
#define NOINLINE __attribute__((noinline))

NOINLINE int use_int(int x) { return x ^ 0x55AA55AA; }
NOINLINE float use_float(float x) { return x * 1.5f; }
NOINLINE double use_double(double x) { return x * 2.5; }
NOINLINE long use_long(long x) { return x ^ 0x123456789ABCDEF0L; }
NOINLINE void* use_ptr(void* p) { return (void*)((uintptr_t)p ^ 0xF0F0F0F0); }

/* Helper functions that take many arguments to force register pressure */
NOINLINE int helper1(int a, int b, int c, int d, int e, float f, double g, long h) {
    return a + b - c + d - e + (int)f + (int)g + (int)h;
}

NOINLINE float helper2(float a, float b, float c, double d, double e, int f, int g, long h) {
    return a * b - c + (float)d + (float)e + (float)f + (float)g + (float)h;
}

NOINLINE double helper3(double a, double b, double c, int d, int e, float f, float g, long h) {
    return a / b + c - d + e + f + g + h;
}

NOINLINE long helper4(long a, long b, long c, int d, int e, float f, double g, int h) {
    return (a ^ b) | (c & ~d) + e + (long)f + (long)g + h;
}

/* Volatile sink to prevent dead code elimination */
static volatile int sink_int;
static volatile float sink_float;
static volatile double sink_double;
static volatile long sink_long;
static volatile void* sink_ptr;

NOINLINE int stress_reload(int* arr_int, double* arr_dbl, float* arr_flt, 
                           long* arr_long, int size) {
    /* Declare many local variables to exhaust registers */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    float f1, f2, f3, f4, f5, f6, f7, f8, f9, f10;
    double d1, d2, d3, d4, d5, d6, d7, d8, d9, d10;
    long l1, l2, l3, l4, l5, l6, l7, l8, l9, l10;
    
    /* Initialize with complex expressions */
    v1 = arr_int[0] ^ 1;
    v2 = arr_int[1] + 2;
    v3 = arr_int[2] * 3;
    v4 = arr_int[3] / 4;
    v5 = arr_int[4] | 5;
    v6 = arr_int[5] & 6;
    v7 = arr_int[6] ^ 7;
    v8 = arr_int[7] + 8;
    v9 = arr_int[8] * 9;
    v10 = arr_int[9] / 10;
    
    f1 = arr_flt[0] * 1.1f;
    f2 = arr_flt[1] + 2.2f;
    f3 = arr_flt[2] - 3.3f;
    f4 = arr_flt[3] / 4.4f;
    f5 = arr_flt[4] * 5.5f;
    
    d1 = arr_dbl[0] * 1.11;
    d2 = arr_dbl[1] + 2.22;
    d3 = arr_dbl[2] - 3.33;
    d4 = arr_dbl[3] / 4.44;
    d5 = arr_dbl[4] * 5.55;
    
    l1 = arr_long[0] ^ 0x11111111;
    l2 = arr_long[1] + 0x22222222;
    l3 = arr_long[2] * 3;
    l4 = arr_long[3] | 0x44444444;
    l5 = arr_long[4] & 0x55555555;
    
    /* More variables */
    v11 = v1 + v2; v12 = v3 - v4; v13 = v5 * v6; v14 = v7 ^ v8; v15 = v9 + v10;
    v16 = v11 * v12; v17 = v13 - v14; v18 = v15 ^ v16; v19 = v17 + v18; v20 = v19 * 2;
    
    f6 = f1 + f2; f7 = f3 - f4; f8 = f5 * f6; f9 = f7 + f8; f10 = f9 / 2.0f;
    
    d6 = d1 + d2; d7 = d3 - d4; d8 = d5 * d6; d9 = d7 + d8; d10 = d9 / 2.0;
    
    l6 = l1 ^ l2; l7 = l3 & l4; l8 = l5 | l6; l9 = l7 + l8; l10 = l9 * 2;
    
    int result = 0;
    
    /* Complex loop with extreme register pressure */
    for (int i = 0; i < 1000; i++) {
        /* Complex array indexing with multi-term expressions */
        int idx1 = (i * 7 + v1 * 3 + v2 * 5) % size;
        int idx2 = (i * 11 + v3 * 13 + v4 * 17) % size;
        int idx3 = (i * 19 + v5 * 23 + v6 * 29) % size;
        int idx4 = (i * 31 + v7 * 37 + v8 * 41) % size;
        int idx5 = (i * 43 + v9 * 47 + v10 * 53) % size;
        
        /* Volatile pointers to force memory accesses */
        volatile int* volatile_ptr1 = &arr_int[idx1];
        volatile double* volatile_ptr2 = &arr_dbl[idx2];
        volatile float* volatile_ptr3 = &arr_flt[idx3];
        volatile long* volatile_ptr4 = &arr_long[idx4];
        
        /* Complex control flow that splits live ranges */
        switch (i % 8) {
            case 0: {
                /* RELOAD_FOR_INPUT and RELOAD_FOR_INPUT_ADDRESS patterns */
                int temp1 = *volatile_ptr1 + v1;
                float temp2 = *volatile_ptr3 + f1;
                
                /* Inline assembly with conflicting constraints */
                int asm_out;
                asm volatile (
                    "addl %[in1], %[in2]\n\t"
                    "movl %[in2], %[out]"
                    : [out] "=r" (asm_out)
                    : [in1] "r" (temp1), [in2] "r" (v2)
                    : "cc"
                );
                
                result += asm_out;
                result += helper1(v1, v2, v3, v4, v5, f1, d1, l1);
                break;
            }
            case 1: {
                /* RELOAD_FOR_OUTPUT_ADDRESS patterns */
                double temp_d = *volatile_ptr2 * d1;
                long temp_l = *volatile_ptr4 ^ l1;
                
                /* Inline assembly that uses memory operand */
                long asm_out2;
                asm volatile (
                    "movq (%[addr]), %%rax\n\t"
                    "xorq %[val], %%rax\n\t"
                    "movq %%rax, %[out]"
                    : [out] "=r" (asm_out2)
                    : [addr] "r" (volatile_ptr4), [val] "r" (temp_l)
                    : "rax", "cc"
                );
                
                result += (int)asm_out2;
                result += helper2(f1, f2, f3, d1, d2, v1, v2, l2);
                break;
            }
            case 2: {
                /* RELOAD_FOR_OPERAND_ADDRESS patterns */
                int* addr1 = &arr_int[idx1];
                double* addr2 = &arr_dbl[idx2];
                
                /* Use addresses in computations */
                int offset = (v1 * v2 + v3 * v4) % 64;
                int val1 = *(addr1 + offset);
                double val2 = *(addr2 + (offset % 16));
                
                result += val1 + (int)val2;
                result += helper3(d1, d2, d3, v3, v4, f3, f4, l3);
                break;
            }
            case 3: {
                /* RELOAD_FOR_INPADDR_ADDRESS patterns */
                int complex_idx = (idx1 * 3 + idx2 * 5 + idx3 * 7) % size;
                float* complex_addr = &arr_flt[complex_idx];
                
                /* Multiple levels of address computation */
                float* addr_of_addr = complex_addr + v1;
                float val = *(addr_of_addr + v2);
                
                result += (int)val;
                result += helper4(l1, l2, l3, v5, v6, f5, d5, v7);
                break;
            }
            case 4: {
                /* RELOAD_FOR_OUTADDR_ADDRESS patterns */
                long* out_addr = &arr_long[idx4];
                *out_addr = l1 + i;
                
                /* Chain of address computations */
                int* int_ptr = (int*)out_addr;
                int_ptr[idx1 % 2] = v1 + v2;
                
                result += *int_ptr;
                break;
            }
            case 5: {
                /* RELOAD_FOR_OPADDR_ADDR patterns */
                /* Take address of local variable */
                int local_var = v1 * v2 + v3;
                int* local_addr = &local_var;
                
                /* Use in inline assembly */
                int asm_result;
                asm volatile (
                    "movl (%[addr]), %%eax\n\t"
                    "addl %[inc], %%eax\n\t"
                    "movl %%eax, %[out]"
                    : [out] "=r" (asm_result)
                    : [addr] "r" (local_addr), [inc] "r" (v4)
                    : "eax", "cc"
                );
                
                result += asm_result;
                break;
            }
            case 6: {
                /* RELOAD_FOR_OTHER_ADDRESS patterns */
                /* Complex addressing across basic blocks */
                int* ptr1 = &arr_int[idx1];
                int* ptr2 = &arr_int[idx2];
                int* ptr3 = &arr_int[idx3];
                
                /* Use computed goto to create complex control flow */
                void* labels[] = { &&label1, &&label2, &&label3 };
                goto *labels[i % 3];
                
            label1:
                result += *ptr1 + v1;
                goto end_case;
            label2:
                result += *ptr2 + v2;
                goto end_case;
            label3:
                result += *ptr3 + v3;
                goto end_case;
            end_case:
                break;
            }
            case 7: {
                /* RELOAD_OTHER patterns - mixed operations */
                /* Force spilling by using all variables */
                int sum1 = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
                int sum2 = v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20;
                float fsum = f1 + f2 + f3 + f4 + f5 + f6 + f7 + f8 + f9 + f10;
                double dsum = d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8 + d9 + d10;
                long lsum = l1 + l2 + l3 + l4 + l5 + l6 + l7 + l8 + l9 + l10;
                
                /* Call multiple helpers to force register shuffling */
                result += helper1(sum1, sum2, v1, v2, v3, fsum, dsum, lsum);
                result += helper2(f1, f2, f3, d1, d2, sum1, sum2, l1);
                result += helper3(d1, d2, d3, v4, v5, f4, f5, l2);
                result += helper4(l1, l2, l3, v6, v7, f6, d6, v8);
                break;
            }
        }
        
        /* Update most variables to keep them live */
        v1 = (v1 * 3 + i) ^ result;
        v2 = (v2 * 5 + i) | result;
        v3 = (v3 * 7 + i) & result;
        v4 = (v4 * 11 + i) + result;
        v5 = (v5 * 13 + i) - result;
        v6 = (v6 * 17 + i) ^ result;
        v7 = (v7 * 19 + i) | result;
        v8 = (v8 * 23 + i) & result;
        v9 = (v9 * 29 + i) + result;
        v10 = (v10 * 31 + i) - result;
        
        f1 = f1 * 1.01f + (float)result;
        f2 = f2 * 1.02f - (float)result;
        f3 = f3 * 1.03f + (float)(result % 100);
        f4 = f4 * 1.04f - (float)(result % 100);
        f5 = f5 * 1.05f + (float)result;
        
        d1 = d1 * 1.001 + (double)result;
        d2 = d2 * 1.002 - (double)result;
        d3 = d3 * 1.003 + (double)(result % 1000);
        d4 = d4 * 1.004 - (double)(result % 1000);
        d5 = d5 * 1.005 + (double)result;
        
        l1 = (l1 * 3 + i) ^ (long)result;
        l2 = (l2 * 5 + i) | (long)result;
        l3 = (l3 * 7 + i) & (long)result;
        l4 = (l4 * 11 + i) + (long)result;
        l5 = (l5 * 13 + i) - (long)result;
        
        /* Update derived variables */
        v11 = v1 + v2; v12 = v3 - v4; v13 = v5 * v6; v14 = v7 ^ v8; v15 = v9 + v10;
        v16 = v11 * v12; v17 = v13 - v14; v18 = v15 ^ v16; v19 = v17 + v18; v20 = v19 * 2;
        
        f6 = f1 + f2; f7 = f3 - f4; f8 = f5 * f6; f9 = f7 + f8; f10 = f9 / 2.0f;
        
        d6 = d1 + d2; d7 = d3 - d4; d8 = d5 * d6; d9 = d7 + d8; d10 = d9 / 2.0;
        
        l6 = l1 ^ l2; l7 = l3 & l4; l8 = l5 | l6; l9 = l7 + l8; l10 = l9 * 2;
        
        /* Use volatile sinks to prevent optimization */
        sink_int = result;
        sink_float = f10;
        sink_double = d10;
        sink_long = l10;
        sink_ptr = &arr_int[idx1];
    }
    
    return result;
}

int main() {
    const int SIZE = 1024;
    
    /* Allocate and initialize arrays with pattern data */
    int* arr_int = (int*)malloc(SIZE * sizeof(int));
    double* arr_dbl = (double*)malloc(SIZE * sizeof(double));
    float* arr_flt = (float*)malloc(SIZE * sizeof(float));
    long* arr_long = (long*)malloc(SIZE * sizeof(long));
    
    for (int i = 0; i < SIZE; i++) {
        arr_int[i] = i ^ 0x12345678;
        arr_dbl[i] = i * 1.23456789;
        arr_flt[i] = i * 0.987654321f;
        arr_long[i] = (long)i * 0x1122334455667788L;
    }
    
    /* Call the stress function */
    int result = stress_reload(arr_int, arr_dbl, arr_flt, arr_long, SIZE);
    
    printf("Result: %d\n", result);
    
    /* Cleanup */
    free(arr_int);
    free(arr_dbl);
    free(arr_flt);
    free(arr_long);
    
    return 0;
}
