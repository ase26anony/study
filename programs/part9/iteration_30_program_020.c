/* reload1_stress.c - Stress test for GCC reload pass */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define ARRAY_SIZE 1024
#define NUM_LOCALS 40
#define ITERATIONS 1000

/* Opaque noinline functions to prevent optimization */
int __attribute__((noinline)) helper1(int a, int b, int c, int d, int e, int f) {
    volatile int sink = a + b + c + d + e + f;
    return sink;
}

double __attribute__((noinline)) helper2(double a, double b, double c, 
                                        double d, double e) {
    volatile double sink = a * b + c * d - e;
    return sink;
}

void __attribute__((noinline)) helper3(int* addr1, int* addr2, 
                                      double* addr3, double* addr4) {
    volatile int tmp1 = *addr1 + *addr2;
    volatile double tmp2 = *addr3 * *addr4;
    (void)tmp1; (void)tmp2;
}

void __attribute__((noinline)) helper4(long long a, long long b, 
                                      float c, float d, int* addr) {
    volatile long long sum = a + b;
    volatile float prod = c * d;
    *addr += (int)(sum % 1000) + (int)(prod * 100);
}

/* Main stress function */
int __attribute__((noinline)) 
stress_reload(int* arr_int, double* arr_dbl, float* arr_flt, 
              long long* arr_ll, volatile int* checksum) {
    /* Declare many local variables to exhaust registers */
    int v0, v1, v2, v3, v4, v5, v6, v7, v8, v9;
    int v10, v11, v12, v13, v14, v15, v16, v17, v18, v19;
    double d0, d1, d2, d3, d4, d5, d6, d7, d8, d9;
    float f0, f1, f2, f3, f4, f5, f6, f7, f8, f9;
    long long ll0, ll1, ll2, ll3, ll4;
    
    /* Initialize with complex expressions */
    v0 = arr_int[0] * 3;
    v1 = arr_int[1] + v0;
    v2 = arr_int[2] - v1;
    v3 = arr_int[3] / (v2 ? v2 : 1);
    v4 = arr_int[4] % 17;
    v5 = arr_int[5] ^ v4;
    v6 = arr_int[6] | v5;
    v7 = arr_int[7] & v6;
    v8 = arr_int[8] << 2;
    v9 = arr_int[9] >> 1;
    
    v10 = v0 + v1 - v2;
    v11 = v3 * v4 + v5;
    v12 = v6 / (v7 ? v7 : 1) + v8;
    v13 = v9 ^ v10;
    v14 = v11 | v12;
    v15 = v13 & v14;
    v16 = v15 << 3;
    v17 = v16 >> 2;
    v18 = v17 + v0 - v9;
    v19 = v18 * 7 % 31;
    
    d0 = arr_dbl[0] * 1.5;
    d1 = arr_dbl[1] + d0;
    d2 = arr_dbl[2] - d1;
    d3 = arr_dbl[3] * d2;
    d4 = arr_dbl[4] / (d3 != 0.0 ? d3 : 1.0);
    d5 = arr_dbl[5] + d4 * 2.0;
    d6 = arr_dbl[6] - d5 / 3.0;
    d7 = arr_dbl[7] * d6;
    d8 = arr_dbl[8] + d7;
    d9 = arr_dbl[9] - d8;
    
    f0 = arr_flt[0] * 1.1f;
    f1 = arr_flt[1] + f0;
    f2 = arr_flt[2] - f1;
    f3 = arr_flt[3] * f2;
    f4 = arr_flt[4] / (f3 != 0.0f ? f3 : 1.0f);
    f5 = arr_flt[5] + f4 * 1.5f;
    f6 = arr_flt[6] - f5 / 2.0f;
    f7 = arr_flt[7] * f6;
    f8 = arr_flt[8] + f7;
    f9 = arr_flt[9] - f8;
    
    ll0 = arr_ll[0] * 3LL;
    ll1 = arr_ll[1] + ll0;
    ll2 = arr_ll[2] - ll1;
    ll3 = arr_ll[3] ^ ll2;
    ll4 = arr_ll[4] | ll3;
    
    volatile int* volatile_ptr = checksum;
    volatile double* volatile_dbl = (volatile double*)arr_dbl;
    
    /* Complex loop with multiple addressing modes */
    for (int i = 0; i < ITERATIONS; i++) {
        /* Complex array indexing with multi-term expressions */
        int idx1 = (i * 7 + v0 * 3 + v1 * 5) % ARRAY_SIZE;
        int idx2 = (i * 11 + v2 * 2 + v3 * 7) % ARRAY_SIZE;
        int idx3 = (i * 13 + v4 * 3 + v5 * 11) % ARRAY_SIZE;
        int idx4 = (i * 17 + v6 * 5 + v7 * 13) % ARRAY_SIZE;
        int idx5 = (i * 19 + v8 * 7 + v9 * 17) % ARRAY_SIZE;
        
        /* Force address computations into registers */
        int* addr1 = &arr_int[idx1];
        int* addr2 = &arr_int[idx2];
        double* addr3 = &arr_dbl[idx3];
        double* addr4 = &arr_dbl[idx4];
        float* addr5 = &arr_flt[idx5];
        
        /* Inline assembly with conflicting constraints */
        int temp1, temp2;
        double temp3;
        
        /* RELOAD_FOR_INPUT and RELOAD_FOR_INPUT_ADDRESS */
        asm volatile (
            "mov %[val1], %[src1]\n\t"
            "add %[val2], %[src2]\n\t"
            : [val1] "=r" (temp1), [val2] "=r" (temp2)
            : [src1] "m" (*addr1), [src2] "r" (v10)
            : "memory"
        );
        
        /* RELOAD_FOR_OUTPUT_ADDRESS */
        asm volatile (
            "movsd %[src], %%xmm0\n\t"
            "addsd %%xmm0, %%xmm0\n\t"
            "movsd %%xmm0, %[dst]\n\t"
            : [dst] "=m" (*addr3)
            : [src] "m" (*addr4)
            : "xmm0", "memory"
        );
        
        /* Switch with multiple basic blocks */
        switch (i % 8) {
            case 0: {
                /* RELOAD_FOR_INPADDR_ADDRESS */
                int idx6 = (idx1 * 3 + idx2 * 7) % ARRAY_SIZE;
                int* addr6 = &arr_int[idx6];
                int** addr_of_addr = &addr6;
                
                asm volatile (
                    "mov %[addr], %%rax\n\t"
                    "mov (%%rax), %%rbx\n\t"
                    : 
                    : [addr] "r" (addr_of_addr)
                    : "rax", "rbx", "memory"
                );
                
                v0 = *addr6 + v1;
                break;
            }
            case 1: {
                /* RELOAD_FOR_OUTADDR_ADDRESS */
                double complex_result;
                double* result_ptr = &complex_result;
                
                asm volatile (
                    "mov %[in1], %%xmm0\n\t"
                    "mov %[in2], %%xmm1\n\t"
                    "mulsd %%xmm1, %%xmm0\n\t"
                    "movsd %%xmm0, %[out]\n\t"
                    : [out] "=m" (*result_ptr)
                    : [in1] "m" (d0), [in2] "m" (d1)
                    : "xmm0", "xmm1"
                );
                
                d2 = complex_result + d3;
                break;
            }
            case 2: {
                /* RELOAD_FOR_OPERAND_ADDRESS */
                long long* ll_addr = &arr_ll[idx1 % 10];
                long long temp_ll;
                
                asm volatile (
                    "mov %[src], %%rax\n\t"
                    "add $100, %%rax\n\t"
                    "mov %%rax, %[dst]\n\t"
                    : [dst] "=r" (temp_ll)
                    : [src] "m" (*ll_addr)
                    : "rax"
                );
                
                ll0 = temp_ll ^ ll1;
                break;
            }
            case 3: {
                /* RELOAD_FOR_OPADDR_ADDR */
                int* volatile volatile_addr = (int*)&arr_int[idx3];
                int** addr_of_volatile = &volatile_addr;
                
                asm volatile (
                    "mov %[ptr], %%rcx\n\t"
                    "mov (%%rcx), %%rdx\n\t"
                    "mov (%%rdx), %%eax\n\t"
                    : 
                    : [ptr] "r" (addr_of_volatile)
                    : "rcx", "rdx", "rax", "memory"
                );
                
                v2 = v3 * 2;
                break;
            }
            case 4: {
                /* RELOAD_FOR_OTHER_ADDRESS */
                float* fptr1 = &arr_flt[idx4 % 10];
                float* fptr2 = &arr_flt[idx5 % 10];
                float* fptr_array[2] = {fptr1, fptr2};
                
                asm volatile (
                    "mov %[arr], %%rsi\n\t"
                    "mov (%%rsi), %%rdi\n\t"
                    "mov 8(%%rsi), %%r8\n\t"
                    : 
                    : [arr] "r" (fptr_array)
                    : "rsi", "rdi", "r8", "memory"
                );
                
                f0 = *fptr1 + *fptr2;
                break;
            }
            default: {
                /* RELOAD_OTHER - mixed operations */
                int idx_mixed = (i * 23 + v10 * 11) % ARRAY_SIZE;
                int* mixed_addr = &arr_int[idx_mixed];
                
                /* Force spill/reload around function calls */
                v10 = helper1(v0, v1, v2, v3, v4, *mixed_addr);
                d10 = helper2(d0, d1, d2, d3, d4);
                helper3(&v5, &v6, &d5, &d6);
                helper4(ll0, ll1, f0, f1, &v7);
                
                /* Complex update keeping variables live */
                v11 = v10 + v5 - v6 + v7;
                break;
            }
        }
        
        /* Update most variables to keep them live */
        v0 = v1 + temp1;
        v1 = v2 - temp2;
        v2 = v3 * arr_int[idx1];
        v3 = v4 / (arr_int[idx2] ? arr_int[idx2] : 1);
        v4 = v5 ^ arr_int[idx3];
        v5 = v6 | arr_int[idx4];
        v6 = v7 & arr_int[idx5];
        v7 = v8 << (i % 4);
        v8 = v9 >> (i % 4);
        v9 = v10 + i;
        
        d0 = d1 + arr_dbl[idx1 % 10];
        d1 = d2 - arr_dbl[idx2 % 10];
        d2 = d3 * arr_dbl[idx3 % 10];
        d3 = d4 / (arr_dbl[idx4 % 10] != 0.0 ? arr_dbl[idx4 % 10] : 1.0);
        d4 = d5 + arr_dbl[idx5 % 10];
        
        f0 = f1 + arr_flt[idx1 % 10];
        f1 = f2 - arr_flt[idx2 % 10];
        f2 = f3 * arr_flt[idx3 % 10];
        f3 = f4 / (arr_flt[idx4 % 10] != 0.0f ? arr_flt[idx4 % 10] : 1.0f);
        f4 = f5 + arr_flt[idx5 % 10];
        
        ll0 = ll1 + arr_ll[idx1 % 5];
        ll1 = ll2 - arr_ll[idx2 % 5];
        ll2 = ll3 ^ arr_ll[idx3 % 5];
        ll3 = ll4 | arr_ll[idx4 % 5];
        ll4 = ll0 << 1;
        
        /* Accumulate checksum */
        *volatile_ptr += v0 + v5 + v10 + (int)d0 + (int)f0 + (int)(ll0 & 0xFFFFFFFF);
    }
    
    /* Final complex computation */
    int result = v0 + v5 + v10 + v15 + v19;
    result += (int)d0 + (int)d5 + (int)d9;
    result += (int)f0 + (int)f5 + (int)f9;
    result += (int)(ll0 & 0xFFFFFFFF) + (int)(ll4 & 0xFFFFFFFF);
    
    return result;
}

int main() {
    /* Allocate and initialize arrays */
    int* arr_int = (int*)malloc(ARRAY_SIZE * sizeof(int));
    double* arr_dbl = (double*)malloc(ARRAY_SIZE * sizeof(double));
    float* arr_flt = (float*)malloc(ARRAY_SIZE * sizeof(float));
    long long* arr_ll = (long long*)malloc(ARRAY_SIZE * sizeof(long long));
    
    if (!arr_int || !arr_dbl || !arr_flt || !arr_ll) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr_int[i] = i * 3 + 1;
        arr_dbl[i] = i * 1.5 + 0.5;
        arr_flt[i] = i * 0.7f + 0.3f;
        arr_ll[i] = i * 5LL + 2LL;
    }
    
    volatile int checksum = 0;
    
    /* Call stress function */
    int result = stress_reload(arr_int, arr_dbl, arr_flt, arr_ll, &checksum);
    
    printf("Result: %d\n", result);
    printf("Checksum: %d\n", checksum);
    
    /* Cleanup */
    free(arr_int);
    free(arr_dbl);
    free(arr_flt);
    free(arr_ll);
    
    return 0;
}
