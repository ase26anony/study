/* reload_stress_test.c */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define NOINLINE __attribute__((noinline))
#define VOLATILE_VAR volatile

/* Opaque functions to prevent optimization */
NOINLINE int use_int(int x) { return x ^ 0x55AA55AA; }
NOINLINE double use_double(double x) { return x * 1.0000001; }
NOINLINE long use_long(long x) { return x + 0x12345678; }
NOINLINE float use_float(float x) { return x / 2.0f; }
NOINLINE void* use_ptr(void* p) { return (void*)((uintptr_t)p + 1); }

/* Helper to take address and force address reloads */
NOINLINE void address_user(int* p1, double* p2, float* p3, long* p4) {
    VOLATILE_VAR int sink;
    sink = *p1 + (int)*p2 + (int)*p3 + (int)*p4;
}

/* Complex control flow helper */
NOINLINE int branch_helper(int x, int y) {
    switch (x % 7) {
        case 0: return y * 2;
        case 1: return y + x;
        case 2: return y - x;
        case 3: return y ^ x;
        case 4: return y & x;
        case 5: return y | x;
        default: return y >> (x & 3);
    }
}

NOINLINE int stress_reload(int* arr_int, double* arr_dbl, float* arr_flt, 
                           long* arr_lng, int size) {
    /* Declare many local variables to exhaust registers */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    double d1, d2, d3, d4, d5, d6, d7, d8, d9, d10;
    float f1, f2, f3, f4, f5, f6, f7, f8, f9, f10;
    long l1, l2, l3, l4, l5, l6, l7, l8, l9, l10;
    
    VOLATILE_VAR int result = 0;
    VOLATILE_VAR int* volatile ptr1;
    VOLATILE_VAR double* volatile ptr2;
    VOLATILE_VAR float* volatile ptr3;
    VOLATILE_VAR long* volatile ptr4;
    
    /* Initialize with complex expressions */
    v1 = arr_int[0] ^ 1; v2 = arr_int[1] + 2; v3 = arr_int[2] * 3;
    v4 = arr_int[3] / 4; v5 = arr_int[4] & 5; v6 = arr_int[5] | 6;
    v7 = arr_int[6] ^ 7; v8 = arr_int[7] + 8; v9 = arr_int[8] * 9;
    v10 = arr_int[9] / 10; v11 = v1 + v2; v12 = v3 - v4;
    v13 = v5 ^ v6; v14 = v7 & v8; v15 = v9 | v10;
    v16 = v11 * v12; v17 = v13 + v14; v18 = v15 - v16;
    v19 = v17 ^ v18; v20 = v19 & 0xFF;
    
    d1 = arr_dbl[0] * 1.1; d2 = arr_dbl[1] + 2.2;
    d3 = arr_dbl[2] - 3.3; d4 = arr_dbl[3] / 4.4;
    d5 = arr_dbl[4] * 5.5; d6 = arr_dbl[5] + 6.6;
    d7 = arr_dbl[6] - 7.7; d8 = arr_dbl[7] / 8.8;
    d9 = d1 + d2 + d3; d10 = d4 * d5 * d6;
    
    f1 = arr_flt[0] * 1.1f; f2 = arr_flt[1] + 2.2f;
    f3 = arr_flt[2] - 3.3f; f4 = arr_flt[3] / 4.4f;
    f5 = arr_flt[4] * 5.5f; f6 = arr_flt[5] + 6.6f;
    f7 = arr_flt[6] - 7.7f; f8 = arr_flt[7] / 8.8f;
    f9 = f1 + f2 + f3; f10 = f4 * f5 * f6;
    
    l1 = arr_lng[0] ^ 0x11111111; l2 = arr_lng[1] + 0x22222222;
    l3 = arr_lng[2] * 3; l4 = arr_lng[3] / 4;
    l5 = arr_lng[4] & 0x55555555; l6 = arr_lng[5] | 0xAAAAAAAA;
    l7 = arr_lng[6] ^ 0x33333333; l8 = arr_lng[7] + 0x44444444;
    l9 = l1 + l2 + l3; l10 = l4 * l5 * l6;
    
    /* Main stress loop */
    for (int i = 0; i < 1000; i++) {
        /* Complex array indexing with multi-term expressions */
        int idx1 = (i * 7 + v1 * 3 + v2 * 5) % size;
        int idx2 = (i * 11 + v3 * 13 + v4 * 17) % size;
        int idx3 = (i * 19 + v5 * 23 + v6 * 29) % size;
        int idx4 = (i * 31 + v7 * 37 + v8 * 41) % size;
        int idx5 = (i * 43 + v9 * 47 + v10 * 53) % size;
        int idx6 = (i * 59 + v11 * 61 + v12 * 67) % size;
        
        /* Force address computations that need registers */
        int* addr1 = &arr_int[idx1];
        double* addr2 = &arr_dbl[idx2];
        float* addr3 = &arr_flt[idx3];
        long* addr4 = &arr_lng[idx4];
        int* addr5 = &arr_int[idx5];
        double* addr6 = &arr_dbl[idx6];
        
        /* Inline assembly with conflicting constraints to force reloads */
        int temp1, temp2;
        double dtemp;
        float ftemp;
        long ltemp;
        
        /* RELOAD_FOR_INPUT and RELOAD_FOR_INPUT_ADDRESS patterns */
        asm volatile (
            "mov %[in1], %[out1]\n\t"
            "add %[in2], %[out1]\n\t"
            : [out1] "=r" (temp1)
            : [in1] "r" (*addr1), [in2] "r" (v1)
            : "cc"
        );
        
        /* RELOAD_FOR_OUTPUT_ADDRESS pattern */
        asm volatile (
            "mov %[val], (%[addr])\n\t"
            : 
            : [val] "r" (v2), [addr] "r" (addr5)
            : "memory"
        );
        
        /* Mixed operand types */
        asm volatile (
            "cvtsi2sd %[intval], %[dblout]\n\t"
            "addsd %[dblin], %[dblout]\n\t"
            : [dblout] "=x" (dtemp)
            : [intval] "r" (v3), [dblin] "x" (d1)
            : 
        );
        
        /* Control flow that splits live ranges */
        switch (i % 8) {
            case 0: {
                /* Address computation in this block */
                int* complex_addr = &arr_int[(idx1 + idx2 * 3) % size];
                ptr1 = complex_addr;
                /* Use in another computation */
                result += *complex_addr + v4;
                break;
            }
            case 1: {
                float* complex_addr = &arr_flt[(idx3 * 5 + idx4) % size];
                ptr3 = complex_addr;
                result += (int)(*complex_addr * f1);
                break;
            }
            case 2: {
                double* complex_addr = &arr_dbl[(idx5 + idx6 * 7) % size];
                ptr2 = complex_addr;
                result += (int)(*complex_addr + d2);
                break;
            }
            case 3: {
                long* complex_addr = &arr_lng[(idx1 * 11 + idx3 * 13) % size];
                ptr4 = complex_addr;
                result += (int)(*complex_addr ^ l1);
                break;
            }
            case 4: {
                /* Force RELOAD_FOR_OPERAND_ADDRESS */
                int offset = v5 % 32;
                int* base = arr_int;
                asm volatile (
                    "mov (%[base], %[offset], 4), %[out]\n\t"
                    : [out] "=r" (temp2)
                    : [base] "r" (base), [offset] "r" (offset)
                    : "memory"
                );
                result += temp2;
                break;
            }
            case 5: {
                /* RELOAD_FOR_INPADDR_ADDRESS pattern */
                int** addr_of_addr = &addr1;
                asm volatile (
                    "mov (%[addrptr]), %[reg]\n\t"
                    "mov (%[reg]), %[out]\n\t"
                    : [out] "=r" (temp1)
                    : [addrptr] "r" (addr_of_addr)
                    : "memory"
                );
                result += temp1;
                break;
            }
            case 6: {
                /* RELOAD_FOR_OUTADDR_ADDRESS pattern */
                int** addr_of_addr = &addr5;
                asm volatile (
                    "mov %[val], %%eax\n\t"
                    "mov %[addrptr], %%rbx\n\t"
                    "mov (%%rbx), %%rcx\n\t"
                    "mov %%eax, (%%rcx)\n\t"
                    : 
                    : [val] "r" (v6), [addrptr] "r" (addr_of_addr)
                    : "rax", "rbx", "rcx", "memory"
                );
                break;
            }
            default: {
                /* RELOAD_OTHER and RELOAD_FOR_OTHER_ADDRESS */
                /* Computed goto to create complex control flow */
                static void* labels[] = {
                    &&label0, &&label1, &&label2, &&label3
                };
                goto *labels[i % 4];
                
                label0:
                    result += v7 + arr_int[idx1];
                    goto end_switch;
                label1:
                    result += v8 + arr_int[idx2];
                    goto end_switch;
                label2:
                    result += v9 + arr_int[idx3];
                    goto end_switch;
                label3:
                    result += v10 + arr_int[idx4];
                    goto end_switch;
                end_switch:
                break;
            }
        }
        
        /* Call noinline functions with different argument combinations */
        v1 = use_int(v1 + *addr1);
        d1 = use_double(d1 + *addr2);
        f1 = use_float(f1 + *addr3);
        l1 = use_long(l1 + *addr4);
        
        /* Force address reloads through function calls */
        address_user(addr1, addr2, addr3, addr4);
        
        /* Complex update keeping variables live */
        v2 = branch_helper(v2, idx1);
        v3 = branch_helper(v3, idx2);
        v4 = branch_helper(v4, idx3);
        
        /* Update many variables to keep them live */
        v5 = v5 + arr_int[idx4] - v6;
        v6 = v6 ^ arr_int[idx5] | v7;
        v7 = v7 * arr_int[idx6] / (v8 + 1);
        v8 = v8 & arr_int[idx1] + v9;
        v9 = v9 | arr_int[idx2] ^ v10;
        v10 = v10 - arr_int[idx3] + v11;
        
        d2 = d2 + arr_dbl[idx4] * d3;
        d3 = d3 - arr_dbl[idx5] / d4;
        f2 = f2 + arr_flt[idx6] * f3;
        f3 = f3 - arr_flt[idx1] / f4;
        l2 = l2 ^ arr_lng[idx2] + l3;
        l3 = l3 & arr_lng[idx3] | l4;
        
        /* Sink results to prevent elimination */
        VOLATILE_VAR int sink = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
        result += sink;
    }
    
    return result;
}

int main() {
    const int SIZE = 1000;
    
    /* Allocate and initialize arrays with pattern data */
    int* arr_int = (int*)malloc(SIZE * sizeof(int));
    double* arr_dbl = (double*)malloc(SIZE * sizeof(double));
    float* arr_flt = (float*)malloc(SIZE * sizeof(float));
    long* arr_lng = (long*)malloc(SIZE * sizeof(long));
    
    for (int i = 0; i < SIZE; i++) {
        arr_int[i] = i * 3 + 1;
        arr_dbl[i] = i * 1.5 + 0.3;
        arr_flt[i] = i * 0.7f + 0.2f;
        arr_lng[i] = i * 5L + 2L;
    }
    
    /* Call the stress function */
    int result = stress_reload(arr_int, arr_dbl, arr_flt, arr_lng, SIZE);
    
    printf("Result: %d\n", result);
    
    free(arr_int);
    free(arr_dbl);
    free(arr_flt);
    free(arr_lng);
    
    return 0;
}
