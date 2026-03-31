/* reload_stress.c - Designed to trigger various reload types in GCC reload1.cc */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 1000

/* Opaque noinline functions to prevent optimization */
int __attribute__((noinline)) helper1(int a, int b, int c, int d, int e, int f) {
    volatile int sink = a + b - c + d - e + f;
    return sink;
}

double __attribute__((noinline)) helper2(double a, double b, double c, 
                                        double d, double e, double f) {
    volatile double sink = a * b + c * d - e / f;
    return sink;
}

void __attribute__((noinline)) helper3(int* addr1, int* addr2, double* addr3) {
    volatile int sink1 = *addr1 + *addr2;
    volatile double sink2 = *addr3;
    (void)sink1; (void)sink2;
}

void __attribute__((noinline)) helper4(long long a, long long b, 
                                      float c, float d, int* addr) {
    volatile long long sink1 = a ^ b;
    volatile float sink2 = c * d;
    volatile int sink3 = *addr;
    (void)sink1; (void)sink2; (void)sink3;
}

/* Main stress function with extreme register pressure */
int __attribute__((noinline, optimize("O0"))) 
stress_reload(int* arr_int, double* arr_dbl, float* arr_flt, 
              long long* arr_ll, volatile int* volatile_arr) {
    
    /* Declare MANY local variables to exhaust registers */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    int v21, v22, v23, v24, v25, v26, v27, v28, v29, v30;
    double d1, d2, d3, d4, d5, d6, d7, d8, d9, d10;
    float f1, f2, f3, f4, f5, f6, f7, f8, f9, f10;
    long long ll1, ll2, ll3, ll4, ll5;
    int* ptr1, *ptr2, *ptr3, *ptr4, *ptr5;
    double* dptr1, *dptr2;
    volatile int result_sink = 0;
    
    /* Initialize with complex expressions */
    v1 = arr_int[0] ^ 0x12345678;
    v2 = arr_int[1] + v1 * 3;
    v3 = arr_int[2] - v2 / 7;
    v4 = arr_int[3] | (v1 & v2);
    v5 = arr_int[4] ^ v3;
    
    d1 = arr_dbl[0] * 1.234;
    d2 = arr_dbl[1] + d1 * 2.345;
    d3 = arr_dbl[2] - d2 / 3.456;
    
    f1 = arr_flt[0] * 1.234f;
    f2 = arr_flt[1] + f1 * 2.345f;
    
    ll1 = arr_ll[0] ^ 0xFFFFFFFF;
    ll2 = arr_ll[1] + ll1 * 5;
    
    /* Take addresses of locals to force stack addressing */
    ptr1 = &v1; ptr2 = &v2; ptr3 = &v3; ptr4 = &v4; ptr5 = &v5;
    dptr1 = &d1; dptr2 = &d2;
    
    /* Complex loop with extreme register pressure */
    for (int i = 0; i < ITERATIONS; i++) {
        /* Complex array indexing with multi-term expressions */
        int idx1 = (i * 7 + v1 * 3 + v2 * 5) % ARRAY_SIZE;
        int idx2 = (i * 11 + v3 * 13 + v4 * 17) % ARRAY_SIZE;
        int idx3 = (i * 19 + v5 * 23 + v1 * 29) % ARRAY_SIZE;
        int idx4 = (i * 31 + v2 * 37 + v3 * 41) % ARRAY_SIZE;
        int idx5 = (i * 43 + v4 * 47 + v5 * 53) % ARRAY_SIZE;
        
        /* More complex indices for double/float arrays */
        int idx_d1 = (i * 59 + v1 * 61 + idx1 * 67) % ARRAY_SIZE;
        int idx_d2 = (i * 71 + v2 * 73 + idx2 * 79) % ARRAY_SIZE;
        int idx_f1 = (i * 83 + v3 * 89 + idx3 * 97) % ARRAY_SIZE;
        
        /* Use volatile array to prevent optimization */
        volatile_arr[idx1] = i;
        
        /* Complex addressing modes that need reloads */
        int* addr1 = &arr_int[idx1];
        int* addr2 = &arr_int[idx2];
        int* addr3 = &arr_int[idx3];
        double* addr_d1 = &arr_dbl[idx_d1];
        double* addr_d2 = &arr_dbl[idx_d2];
        float* addr_f1 = &arr_flt[idx_f1];
        
        /* Inline assembly with conflicting constraints to force reloads */
        int temp1, temp2, temp3;
        double dtemp1, dtemp2;
        
        /* RELOAD_FOR_INPUT and RELOAD_FOR_INPUT_ADDRESS patterns */
        asm volatile (
            "mov %[val1], %[tmp1]\n\t"
            "add %[val2], %[tmp1]\n\t"
            "mov %[tmp1], %[out1]\n\t"
            : [out1] "=r" (temp1)
            : [val1] "r" (*addr1), 
              [val2] "r" (*addr2),
              [tmp1] "r" (0)
            : "memory"
        );
        
        /* More assembly with memory constraints */
        asm volatile (
            "mov %[addr], %%rax\n\t"
            "mov (%%rax), %[out]\n\t"
            : [out] "=r" (temp2)
            : [addr] "r" (addr3)
            : "rax", "memory"
        );
        
        /* Double precision with complex addressing */
        asm volatile (
            "movsd %[dbl1], %[tmpd1]\n\t"
            "addsd %[dbl2], %[tmpd1]\n\t"
            "movsd %[tmpd1], %[outd]\n\t"
            : [outd] "=x" (dtemp1)
            : [dbl1] "m" (*addr_d1),
              [dbl2] "m" (*addr_d2),
              [tmpd1] "x" (0.0)
            : "memory"
        );
        
        /* Control flow that splits live ranges */
        switch (i % 8) {
            case 0: {
                /* Branch with address computation */
                int* branch_addr = &arr_int[(idx1 + idx2 * 3) % ARRAY_SIZE];
                temp3 = *branch_addr + v1;
                helper1(v1, v2, v3, temp3, *addr1, *addr2);
                break;
            }
            case 1: {
                /* Different address computation */
                double* branch_daddr = &arr_dbl[(idx_d1 + idx_d2) % ARRAY_SIZE];
                dtemp2 = *branch_daddr * d1;
                helper2(d1, d2, d3, dtemp2, *addr_d1, *addr_d2);
                break;
            }
            case 2: {
                /* Address used in multiple ways */
                int complex_idx = (idx3 * 7 + idx4 * 11) % ARRAY_SIZE;
                int* addr_a = &arr_int[complex_idx];
                int* addr_b = &arr_int[(complex_idx + 5) % ARRAY_SIZE];
                helper3(addr_a, addr_b, addr_d1);
                break;
            }
            case 3: {
                /* Mixed types and addressing */
                long long* ll_addr = &arr_ll[i % ARRAY_SIZE];
                helper4(ll1, ll2, f1, f2, &arr_int[idx5]);
                break;
            }
            case 4: {
                /* More complex addressing chain */
                int* chain_addr1 = &arr_int[idx1];
                int* chain_addr2 = &arr_int[(idx1 + *chain_addr1) % ARRAY_SIZE];
                temp3 = *chain_addr1 + *chain_addr2;
                result_sink += temp3;
                break;
            }
            case 5: {
                /* Nested addressing */
                int nested_idx = (idx2 + arr_int[idx3]) % ARRAY_SIZE;
                int* nested_addr = &arr_int[nested_idx];
                asm volatile (
                    "mov %[addr], %%rbx\n\t"
                    "mov (%%rbx), %[out]\n\t"
                    : [out] "=r" (temp3)
                    : [addr] "r" (nested_addr)
                    : "rbx", "memory"
                );
                break;
            }
            case 6: {
                /* Multiple address computations in one block */
                int* addr4 = &arr_int[(idx4 * 2 + 1) % ARRAY_SIZE];
                int* addr5 = &arr_int[(idx5 * 3 + 2) % ARRAY_SIZE];
                helper1(*addr4, *addr5, v4, v5, temp1, temp2);
                break;
            }
            case 7: {
                /* Computed goto to create complex control flow */
                void* labels[] = { &&label1, &&label2, &&label3 };
                goto *labels[i % 3];
                
                label1:
                    temp3 = arr_int[idx1] + arr_int[idx2];
                    goto end_switch;
                label2:
                    temp3 = arr_int[idx3] - arr_int[idx4];
                    goto end_switch;
                label3:
                    temp3 = arr_int[idx5] * 2;
                    goto end_switch;
                end_switch:
                    break;
            }
        }
        
        /* Update many local variables to keep them live */
        v1 = v1 ^ temp1;
        v2 = v2 + temp2;
        v3 = v3 - temp3;
        v4 = v4 | arr_int[idx1];
        v5 = v5 & arr_int[idx2];
        
        d1 = d1 + dtemp1;
        d2 = d2 - *addr_d1;
        d3 = d3 * *addr_d2;
        
        f1 = f1 + *addr_f1;
        f2 = f2 - arr_flt[idx_f1];
        
        ll1 = ll1 ^ arr_ll[i % ARRAY_SIZE];
        ll2 = ll2 + (i * 1000);
        
        /* Force address computations to be reused */
        ptr1 = &arr_int[idx1];
        ptr2 = &arr_int[idx2];
        dptr1 = &arr_dbl[idx_d1];
        
        /* Call helpers with different argument combinations */
        if (i % 3 == 0) {
            helper1(v1, v2, v3, v4, v5, temp1);
        } else if (i % 3 == 1) {
            helper2(d1, d2, d3, *addr_d1, *addr_d2, 1.0);
        } else {
            helper3(ptr1, ptr2, dptr1);
        }
        
        /* More inline assembly with output constraints */
        int out1, out2;
        asm volatile (
            "lea (%[base], %[index], 4), %[out1]\n\t"
            "mov %[out1], %[out2]\n\t"
            : [out1] "=&r" (out1), [out2] "=r" (out2)
            : [base] "r" (arr_int), [index] "r" (idx1)
            : "memory"
        );
        
        /* Use the results */
        v6 = v6 + out1;
        v7 = v7 + out2;
        
        /* Chain of dependent computations */
        v8 = arr_int[out1 % ARRAY_SIZE];
        v9 = arr_int[out2 % ARRAY_SIZE];
        v10 = v8 + v9;
        
        /* Force spilling by using all variables */
        result_sink += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
        result_sink += (int)d1 + (int)d2 + (int)d3;
        result_sink += (int)f1 + (int)f2;
        result_sink += (int)(ll1 & 0xFFFFFFFF) + (int)(ll2 & 0xFFFFFFFF);
    }
    
    return result_sink;
}

int main() {
    /* Allocate and initialize arrays with pattern data */
    int* arr_int = (int*)malloc(ARRAY_SIZE * sizeof(int));
    double* arr_dbl = (double*)malloc(ARRAY_SIZE * sizeof(double));
    float* arr_flt = (float*)malloc(ARRAY_SIZE * sizeof(float));
    long long* arr_ll = (long long*)malloc(ARRAY_SIZE * sizeof(long long));
    volatile int* volatile_arr = (volatile int*)malloc(ARRAY_SIZE * sizeof(int));
    
    if (!arr_int || !arr_dbl || !arr_flt || !arr_ll || !volatile_arr) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr_int[i] = i ^ 0x55AA55AA;
        arr_dbl[i] = i * 1.23456789;
        arr_flt[i] = i * 0.987654321f;
        arr_ll[i] = (long long)i * 123456789LL;
        volatile_arr[i] = 0;
    }
    
    printf("Starting stress test...\n");
    
    /* Call the stress function */
    int result = stress_reload(arr_int, arr_dbl, arr_flt, arr_ll, volatile_arr);
    
    printf("Result checksum: %d\n", result);
    printf("Test completed.\n");
    
    /* Cleanup */
    free(arr_int);
    free(arr_dbl);
    free(arr_flt);
    free(arr_ll);
    free((void*)volatile_arr);
    
    return 0;
}
