/* reload1_stress_test.c
 * Designed to trigger RELOAD_FOR_INPUT, RELOAD_FOR_INPUT_ADDRESS,
 * RELOAD_FOR_INPADDR_ADDRESS, RELOAD_FOR_OUTPUT_ADDRESS,
 * RELOAD_FOR_OUTADDR_ADDRESS, RELOAD_OTHER, and other reload types
 * by creating extreme register pressure with complex addressing modes.
 */

#include <stdio.h>
#include <stdlib.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 1000

/* Opaque noinline functions to prevent optimization */
__attribute__((noinline)) int use_int(int a, int b, int c, int d, int e, int f) {
    volatile int sink = a + b - c + d - e + f;
    return sink;
}

__attribute__((noinline)) double use_double(double a, double b, double c, double d) {
    volatile double sink = a * b - c / d;
    return sink;
}

__attribute__((noinline)) long long use_long(long long a, long long b, long long c) {
    volatile long long sink = a ^ b ^ c;
    return sink;
}

__attribute__((noinline)) void* use_address(void* a, void* b, int offset) {
    volatile char* sink = (char*)a + offset;
    volatile char* sink2 = (char*)b - offset;
    return (void*)(sink > sink2 ? sink : sink2);
}

__attribute__((noinline)) float use_float(float a, float b, float c, float d, float e, float f, float g, float h) {
    volatile float sink = a + b - c + d - e + f - g + h;
    return sink;
}

/* Main stress function */
__attribute__((noinline)) unsigned long long stress_reload(
    volatile int* int_arr, 
    volatile double* dbl_arr,
    volatile long long* ll_arr,
    volatile float* flt_arr
) {
    /* Declare many local variables to exhaust registers */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    int v21, v22, v23, v24, v25, v26, v27, v28, v29, v30;
    int v31, v32, v33, v34, v35;
    
    double d1, d2, d3, d4, d5, d6, d7, d8, d9, d10;
    double d11, d12, d13, d14, d15;
    
    long long ll1, ll2, ll3, ll4, ll5, ll6, ll7, ll8;
    
    float f1, f2, f3, f4, f5, f6, f7, f8, f9, f10;
    
    /* Take addresses of some locals to force stack addressing */
    int* pv1 = &v1;
    int* pv2 = &v2;
    double* pd1 = &d1;
    double* pd2 = &d2;
    
    unsigned long long checksum = 0;
    
    /* Initialize with array values */
    v1 = int_arr[0]; v2 = int_arr[1]; v3 = int_arr[2]; v4 = int_arr[3];
    v5 = int_arr[4]; v6 = int_arr[5]; v7 = int_arr[6]; v8 = int_arr[7];
    v9 = int_arr[8]; v10 = int_arr[9];
    
    d1 = dbl_arr[0]; d2 = dbl_arr[1]; d3 = dbl_arr[2]; d4 = dbl_arr[3];
    d5 = dbl_arr[4]; d6 = dbl_arr[5]; d7 = dbl_arr[6]; d8 = dbl_arr[7];
    
    ll1 = ll_arr[0]; ll2 = ll_arr[1]; ll3 = ll_arr[2]; ll4 = ll_arr[3];
    
    f1 = flt_arr[0]; f2 = flt_arr[1]; f3 = flt_arr[2]; f4 = flt_arr[3];
    f5 = flt_arr[4]; f6 = flt_arr[5]; f7 = flt_arr[6]; f8 = flt_arr[7];
    
    for (int i = 0; i < ITERATIONS; i++) {
        /* Complex array indexing with multi-term expressions */
        int idx1 = (v1 * 7 + v2 * 3 + i) % ARRAY_SIZE;
        int idx2 = (v3 * 11 + v4 * 5 + i * 2) % ARRAY_SIZE;
        int idx3 = (v5 * 13 + v6 * 7 + i * 3) % ARRAY_SIZE;
        int idx4 = (v7 * 17 + v8 * 11 + i * 5) % ARRAY_SIZE;
        
        /* More complex indices for different reload types */
        int addr_idx1 = (v9 * 19 + v10 * 13 + idx1) % ARRAY_SIZE;
        int addr_idx2 = (v11 * 23 + v12 * 17 + idx2) % ARRAY_SIZE;
        int inpaddr_idx = (v13 * 29 + v14 * 19 + idx3) % ARRAY_SIZE;
        int outaddr_idx = (v15 * 31 + v16 * 23 + idx4) % ARRAY_SIZE;
        
        /* Control flow that splits live ranges */
        switch (i % 8) {
            case 0: {
                /* RELOAD_FOR_INPUT and RELOAD_FOR_INPUT_ADDRESS */
                int temp1 = int_arr[idx1];
                int temp2 = int_arr[idx2];
                
                /* Inline assembly with conflicting constraints */
                int result1, result2;
                asm volatile (
                    "addl %[in1], %[out1]\n\t"
                    "subl %[in2], %[out2]"
                    : [out1] "+r" (result1), [out2] "+r" (result2)
                    : [in1] "r" (temp1), [in2] "r" (temp2)
                    : "cc"
                );
                
                v1 = result1;
                v2 = result2;
                
                /* Use computed addresses */
                volatile int* addr1 = &int_arr[addr_idx1];
                volatile int* addr2 = &int_arr[addr_idx2];
                
                /* Force address reloads */
                asm volatile (
                    "movl (%[addr1]), %%eax\n\t"
                    "addl %%eax, %[val]"
                    : [val] "+r" (v3)
                    : [addr1] "r" (addr1)
                    : "eax", "cc"
                );
                
                checksum += *addr1 + *addr2;
                break;
            }
            
            case 1: {
                /* RELOAD_FOR_INPADDR_ADDRESS and RELOAD_FOR_OUTPUT_ADDRESS */
                double temp_d1 = dbl_arr[idx1];
                double temp_d2 = dbl_arr[idx2];
                
                /* Complex addressing in inline asm */
                double result_d;
                volatile double* inp_addr = &dbl_arr[inpaddr_idx];
                volatile double* out_addr = &dbl_arr[outaddr_idx];
                
                asm volatile (
                    "movsd (%[inp]), %%xmm0\n\t"
                    "addsd %[val], %%xmm0\n\t"
                    "movsd %%xmm0, (%[out])"
                    : 
                    : [inp] "r" (inp_addr), [val] "r" (temp_d1), [out] "r" (out_addr)
                    : "xmm0", "memory"
                );
                
                d1 = temp_d1 + temp_d2;
                checksum += (unsigned long long)(*inp_addr + *out_addr);
                break;
            }
            
            case 2: {
                /* RELOAD_FOR_OUTADDR_ADDRESS */
                long long temp_ll1 = ll_arr[idx1];
                long long temp_ll2 = ll_arr[idx2];
                
                volatile long long* outaddr_ptr = &ll_arr[outaddr_idx];
                
                /* Multiple constraints forcing reloads */
                long long ll_result;
                asm volatile (
                    "movq %[in1], %%rax\n\t"
                    "addq %[in2], %%rax\n\t"
                    "movq %%rax, %[out]"
                    : [out] "=m" (*outaddr_ptr)
                    : [in1] "r" (temp_ll1), [in2] "r" (temp_ll2)
                    : "rax", "cc"
                );
                
                ll1 = temp_ll1 ^ temp_ll2;
                checksum += *outaddr_ptr;
                break;
            }
            
            case 3: {
                /* RELOAD_OTHER and RELOAD_FOR_OTHER_ADDRESS */
                float temp_f1 = flt_arr[idx1];
                float temp_f2 = flt_arr[idx2];
                float temp_f3 = flt_arr[idx3];
                float temp_f4 = flt_arr[idx4];
                
                /* Many function calls to force register shuffling */
                f1 = use_float(temp_f1, temp_f2, temp_f3, temp_f4, f1, f2, f3, f4);
                f2 = use_float(temp_f2, temp_f3, temp_f4, temp_f1, f2, f3, f4, f1);
                f3 = use_float(temp_f3, temp_f4, temp_f1, temp_f2, f3, f4, f1, f2);
                
                /* Address computation that spans basic blocks */
                volatile float* other_addr = &flt_arr[(idx1 + idx2 + idx3) % ARRAY_SIZE];
                checksum += (unsigned long long)(*other_addr * 1000);
                break;
            }
            
            case 4: {
                /* RELOAD_FOR_OPERAND_ADDRESS and RELOAD_FOR_OPADDR_ADDR */
                int complex_idx = (v1 * v2 + v3 * v4 + i * i) % ARRAY_SIZE;
                int opaddr_idx = (v5 * v6 + v7 * v8 + complex_idx) % ARRAY_SIZE;
                
                volatile int* operand_addr = &int_arr[complex_idx];
                volatile int* opaddr_addr = &int_arr[opaddr_idx];
                
                /* Use both as data and address */
                int val1 = *operand_addr;
                int val2 = *opaddr_addr;
                
                asm volatile (
                    "leal (%[base], %[index], 4), %%eax\n\t"
                    "addl %%eax, %[result]"
                    : [result] "+r" (v10)
                    : [base] "r" (val1), [index] "r" (val2)
                    : "eax", "cc"
                );
                
                checksum += val1 + val2;
                break;
            }
            
            default: {
                /* Mix of all types */
                int mix_idx1 = (v1 + v3 + v5 + i) % ARRAY_SIZE;
                int mix_idx2 = (v2 + v4 + v6 + i * 2) % ARRAY_SIZE;
                
                /* Multiple memory accesses with different addressing */
                v11 = int_arr[mix_idx1] + int_arr[mix_idx2];
                v12 = int_arr[mix_idx2] - int_arr[mix_idx1];
                
                d11 = dbl_arr[mix_idx1] * dbl_arr[mix_idx2];
                d12 = dbl_arr[mix_idx2] / dbl_arr[mix_idx1];
                
                /* Function calls with mixed arguments */
                v13 = use_int(v11, v12, v1, v2, v3, v4);
                d13 = use_double(d11, d12, d1, d2);
                ll5 = use_long(ll1, ll2, ll3);
                
                /* Computed goto for complex control flow */
                void* labels[] = { &&label1, &&label2, &&label3 };
                goto *labels[i % 3];
                
                label1:
                    checksum += v11 + v12;
                    continue;
                label2:
                    checksum += (unsigned long long)(d11 + d12);
                    continue;
                label3:
                    checksum += ll5;
                    continue;
            }
        }
        
        /* Update most variables to keep them live */
        v1 = v1 * 3 + 1;
        v2 = v2 * 5 - 1;
        v3 = v3 * 7 + 2;
        v4 = v4 * 11 - 2;
        v5 = v5 ^ v6;
        v6 = v6 ^ v7;
        v7 = v7 ^ v8;
        v8 = v8 ^ v9;
        v9 = v9 + v10;
        v10 = v10 - v11;
        
        d1 = d1 * 1.1;
        d2 = d2 / 1.1;
        d3 = d3 + d4;
        d4 = d4 - d5;
        
        ll1 = ll1 << 1;
        ll2 = ll2 >> 1;
        ll3 = ll3 + ll4;
        ll4 = ll4 - ll5;
        
        f1 = f1 * 1.01f;
        f2 = f2 / 1.01f;
        f3 = f3 + f4;
        f4 = f4 - f5;
        
        /* More function calls to force spills around calls */
        if (i % 16 == 0) {
            v14 = use_int(v1, v2, v3, v4, v5, v6);
            d14 = use_double(d1, d2, d3, d4);
            ll6 = use_long(ll1, ll2, ll3);
            
            void* addr_result = use_address((void*)&int_arr[idx1], 
                                           (void*)&int_arr[idx2], 
                                           i);
            checksum += (unsigned long long)addr_result;
        }
    }
    
    /* Final mixing of all values */
    checksum += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
    checksum += (unsigned long long)(d1 + d2 + d3 + d4 + d5 + d6);
    checksum += ll1 + ll2 + ll3 + ll4;
    checksum += (unsigned long long)(f1 + f2 + f3 + f4 + f5 + f6);
    
    return checksum;
}

int main() {
    /* Allocate and initialize arrays with pattern data */
    volatile int* int_arr = (volatile int*)malloc(ARRAY_SIZE * sizeof(int));
    volatile double* dbl_arr = (volatile double*)malloc(ARRAY_SIZE * sizeof(double));
    volatile long long* ll_arr = (volatile long long*)malloc(ARRAY_SIZE * sizeof(long long));
    volatile float* flt_arr = (volatile float*)malloc(ARRAY_SIZE * sizeof(float));
    
    if (!int_arr || !dbl_arr || !ll_arr || !flt_arr) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_arr[i] = i * 3 + 1;
        dbl_arr[i] = i * 1.5 + 0.5;
        ll_arr[i] = (long long)i * 7LL + 3LL;
        flt_arr[i] = i * 0.7f + 0.3f;
    }
    
    printf("Starting reload stress test...\n");
    
    unsigned long long result = stress_reload(int_arr, dbl_arr, ll_arr, flt_arr);
    
    printf("Checksum result: %llu\n", result);
    printf("Test completed.\n");
    
    /* Cleanup */
    free((void*)int_arr);
    free((void*)dbl_arr);
    free((void*)ll_arr);
    free((void*)flt_arr);
    
    return 0;
}
