/* reload_stress_test.c
 * Designed to trigger multiple reload types in GCC's reload pass
 * Compile with: gcc -O2 -fno-omit-frame-pointer -fno-schedule-insns -fno-move-loop-invariants reload_stress_test.c -o reload_test
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 1000

/* Opaque functions that compiler cannot analyze */
__attribute__((noinline)) int use_int(int a, int b, int c, int d, int e, int f) {
    volatile int sink = a + b - c + d - e + f;
    return sink;
}

__attribute__((noinline)) double use_double(double a, double b, double c, double d) {
    volatile double sink = a * b + c - d;
    return sink;
}

__attribute__((noinline)) void* use_address(void* a, void* b, void* c) {
    volatile void* sink = (void*)((uintptr_t)a + (uintptr_t)b - (uintptr_t)c);
    return sink;
}

__attribute__((noinline)) long long use_long_long(long long a, long long b, long long c, 
                                                  long long d, long long e) {
    volatile long long sink = a * b + c * d - e;
    return sink;
}

__attribute__((noinline)) float use_float(float a, float b, float c, float d, 
                                         float e, float f, float g) {
    volatile float sink = a + b * c - d / e + f - g;
    return sink;
}

/* Main stress function */
__attribute__((noinline)) int stress_reload(int* arr_int, double* arr_double, 
                                           long long* arr_ll, float* arr_float) {
    /* Declare many local variables to exhaust registers */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    int v21, v22, v23, v24, v25, v26, v27, v28, v29, v30;
    double d1, d2, d3, d4, d5, d6, d7, d8, d9, d10;
    float f1, f2, f3, f4, f5, f6, f7, f8, f9, f10;
    long long ll1, ll2, ll3, ll4, ll5, ll6, ll7, ll8;
    
    /* Initialize with values from arrays */
    v1 = arr_int[0]; v2 = arr_int[1]; v3 = arr_int[2]; v4 = arr_int[3];
    v5 = arr_int[4]; v6 = arr_int[5]; v7 = arr_int[6]; v8 = arr_int[7];
    v9 = arr_int[8]; v10 = arr_int[9];
    
    d1 = arr_double[0]; d2 = arr_double[1]; d3 = arr_double[2];
    d4 = arr_double[3]; d5 = arr_double[4];
    
    ll1 = arr_ll[0]; ll2 = arr_ll[1]; ll3 = arr_ll[2]; ll4 = arr_ll[3];
    
    f1 = arr_float[0]; f2 = arr_float[1]; f3 = arr_float[2]; f4 = arr_float[3];
    
    volatile int checksum = 0;
    
    /* Complex loop with extreme register pressure */
    for (int i = 0; i < ITERATIONS; i++) {
        /* Complex addressing calculations - will require address reloads */
        int idx1 = (i * 7 + v1 * 3 + v2 * 5) % ARRAY_SIZE;
        int idx2 = (i * 11 + v3 * 13 + v4 * 17) % ARRAY_SIZE;
        int idx3 = (i * 19 + v5 * 23 + v6 * 29) % ARRAY_SIZE;
        int idx4 = (i * 31 + v7 * 37 + v8 * 41) % ARRAY_SIZE;
        int idx5 = (i * 43 + v9 * 47 + v10 * 53) % ARRAY_SIZE;
        
        /* More complex indices with floating point conversions */
        int idx6 = (int)(d1 * 100 + d2 * 200 + i * 300) % ARRAY_SIZE;
        int idx7 = (int)(d3 * 400 + d4 * 500 + d5 * 600) % ARRAY_SIZE;
        int idx8 = (int)(f1 * 1000 + f2 * 2000 + f3 * 3000) % ARRAY_SIZE;
        
        /* Control flow that splits live ranges */
        switch (i % 8) {
            case 0: {
                /* RELOAD_FOR_INPUT_ADDRESS and RELOAD_FOR_INPADDR_ADDRESS */
                int* addr1 = &arr_int[idx1];
                int* addr2 = &arr_int[idx2];
                int* addr3 = &arr_int[idx3];
                
                /* Inline assembly with conflicting constraints */
                int val1, val2, val3;
                asm volatile (
                    "mov %[src1], %[dst1]\n\t"
                    "add %[src2], %[dst2]\n\t"
                    "sub %[src3], %[dst3]"
                    : [dst1] "=r" (val1), [dst2] "=r" (val2), [dst3] "=r" (val3)
                    : [src1] "m" (*addr1), [src2] "m" (*addr2), [src3] "m" (*addr3)
                    : "memory"
                );
                
                v1 = val1; v2 = val2; v3 = val3;
                checksum += v1 + v2 + v3;
                break;
            }
            
            case 1: {
                /* RELOAD_FOR_OUTPUT_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
                double* daddr1 = &arr_double[idx4];
                double* daddr2 = &arr_double[idx5];
                double* daddr3 = &arr_double[idx6];
                
                /* Complex computation requiring multiple registers */
                double temp1 = d1 * d2 + d3 - d4;
                double temp2 = d5 * d1 - d2 + d3;
                
                /* Inline assembly with output memory constraints */
                asm volatile (
                    "movsd %[in1], %[out1]\n\t"
                    "addsd %[in2], %[out2]"
                    : [out1] "=m" (*daddr1), [out2] "=m" (*daddr2)
                    : [in1] "x" (temp1), [in2] "x" (temp2)
                    : "memory"
                );
                
                d1 = *daddr1; d2 = *daddr2;
                break;
            }
            
            case 2: {
                /* RELOAD_FOR_OPERAND_ADDRESS and RELOAD_FOR_OPADDR_ADDR */
                long long* lladdr1 = &arr_ll[idx7];
                long long* lladdr2 = &arr_ll[idx8];
                
                /* Take addresses of locals - forces stack-based reloads */
                int* local_addr1 = &v4;
                int* local_addr2 = &v5;
                int* local_addr3 = &v6;
                
                /* Mixed type operations */
                ll1 = (long long)(*local_addr1) * (*local_addr2);
                ll2 = (long long)(*local_addr3) * v7;
                
                /* Store with complex addressing */
                *lladdr1 = ll1 + ll2;
                *lladdr2 = ll1 - ll2;
                
                /* Call with many arguments - forces register shuffling */
                ll3 = use_long_long(ll1, ll2, *lladdr1, *lladdr2, ll4);
                break;
            }
            
            case 3: {
                /* RELOAD_FOR_OTHER_ADDRESS and RELOAD_OTHER */
                float* faddr1 = &arr_float[idx1];
                float* faddr2 = &arr_float[idx2];
                float* faddr3 = &arr_float[idx3];
                
                /* Multiple function calls with overlapping arguments */
                f1 = use_float(*faddr1, *faddr2, *faddr3, f4, f5, f6, f7);
                f2 = use_float(f8, f9, f10, *faddr1, *faddr2, *faddr3, f1);
                
                /* Computed goto to create complex control flow */
                void* labels[] = { &&label1, &&label2, &&label3, &&label4 };
                goto *labels[i % 4];
                
            label1:
                v10 = use_int(v1, v2, v3, v4, v5, v6);
                goto end_case;
            label2:
                v11 = use_int(v7, v8, v9, v10, v1, v2);
                goto end_case;
            label3:
                v12 = use_int(v3, v4, v5, v6, v7, v8);
                goto end_case;
            label4:
                v13 = use_int(v9, v10, v1, v2, v3, v4);
                goto end_case;
            end_case:
                checksum += v10 + v11 + v12 + v13;
                break;
            }
            
            default: {
                /* Mix of all reload types */
                /* Multiple array accesses with complex addressing */
                int* addr_a = &arr_int[(idx1 + idx2 + idx3) % ARRAY_SIZE];
                int* addr_b = &arr_int[(idx4 + idx5 + idx6) % ARRAY_SIZE];
                int* addr_c = &arr_int[(idx7 + idx8 + i) % ARRAY_SIZE];
                
                /* Use addresses as both data and address operands */
                void* ptr1 = use_address(addr_a, addr_b, addr_c);
                void* ptr2 = use_address(addr_b, addr_c, addr_a);
                void* ptr3 = use_address(addr_c, addr_a, addr_b);
                
                /* Force spilling by using all variables */
                d3 = use_double(d1, d2, d3, d4);
                d4 = use_double(d5, d1, d2, d3);
                d5 = use_double(d4, d3, d2, d1);
                
                f3 = use_float(f1, f2, f3, f4, f5, f6, f7);
                f4 = use_float(f8, f9, f10, f1, f2, f3, f4);
                
                /* Update many variables to keep them live */
                v14 = *addr_a + v1;
                v15 = *addr_b + v2;
                v16 = *addr_c + v3;
                v17 = v4 + v5 + v6;
                v18 = v7 + v8 + v9;
                v19 = v10 + v11 + v12;
                v20 = v13 + v14 + v15;
                
                checksum += v14 + v15 + v16 + v17 + v18 + v19 + v20;
                break;
            }
        }
        
        /* Update most variables to keep them live across iterations */
        v1 = v1 * 3 + i;
        v2 = v2 * 5 - i;
        v3 = v3 * 7 + i * 2;
        v4 = v4 * 11 - i * 3;
        v5 = v5 * 13 + i * 5;
        v6 = v6 * 17 - i * 7;
        v7 = v7 * 19 + i * 11;
        v8 = v8 * 23 - i * 13;
        v9 = v9 * 29 + i * 17;
        v10 = v10 * 31 - i * 19;
        
        d1 = d1 * 1.1 + i * 0.01;
        d2 = d2 * 1.2 - i * 0.02;
        d3 = d3 * 1.3 + i * 0.03;
        d4 = d4 * 1.4 - i * 0.04;
        d5 = d5 * 1.5 + i * 0.05;
        
        f1 = f1 * 1.01f + i * 0.001f;
        f2 = f2 * 1.02f - i * 0.002f;
        f3 = f3 * 1.03f + i * 0.003f;
        f4 = f4 * 1.04f - i * 0.004f;
        
        ll1 = ll1 + i;
        ll2 = ll2 - i;
        ll3 = ll3 + i * 2;
        ll4 = ll4 - i * 3;
    }
    
    return checksum;
}

int main() {
    /* Allocate and initialize arrays with pattern data */
    int* arr_int = (int*)malloc(ARRAY_SIZE * sizeof(int));
    double* arr_double = (double*)malloc(ARRAY_SIZE * sizeof(double));
    long long* arr_ll = (long long*)malloc(ARRAY_SIZE * sizeof(long long));
    float* arr_float = (float*)malloc(ARRAY_SIZE * sizeof(float));
    
    if (!arr_int || !arr_double || !arr_ll || !arr_float) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr_int[i] = i * 3 - i / 2;
        arr_double[i] = i * 0.1 - i * 0.01;
        arr_ll[i] = (long long)i * 1000 - i * 500;
        arr_float[i] = i * 0.01f - i * 0.001f;
    }
    
    printf("Starting reload stress test...\n");
    
    /* Call the stress function */
    int result = stress_reload(arr_int, arr_double, arr_ll, arr_float);
    
    printf("Checksum result: %d\n", result);
    printf("Test completed.\n");
    
    /* Cleanup */
    free(arr_int);
    free(arr_double);
    free(arr_ll);
    free(arr_float);
    
    return 0;
}
