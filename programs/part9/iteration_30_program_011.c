/* reload1_stress_test.c */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define ARRAY_SIZE 1024
#define LOCAL_VARS 40
#define ITERATIONS 1000

/* Opaque noinline functions to prevent optimization */
__attribute__((noinline)) int helper1(int a, int b, int c, int d, int e, int f) {
    volatile int sink = a + b - c + d - e + f;
    return sink;
}

__attribute__((noinline)) float helper2(float a, float b, float c, float d, 
                                       float e, float f, float g, float h) {
    volatile float sink = a * b + c * d - e * f + g / h;
    return sink;
}

__attribute__((noinline)) double helper3(double a, double b, double c, double d,
                                        double e, double f, double g, double h,
                                        double i, double j) {
    volatile double sink = a + b * c - d / e + f * g - h * i + j;
    return sink;
}

__attribute__((noinline)) long helper4(long a, long b, long c, long d,
                                      long e, long f, long g, long h) {
    volatile long sink = (a ^ b) | (c & d) ^ (e | f) & (g ^ h);
    return sink;
}

__attribute__((noinline)) void address_helper(volatile int* addr1, 
                                             volatile float* addr2,
                                             volatile double* addr3,
                                             volatile long* addr4) {
    *addr1 += 1;
    *addr2 += 1.0f;
    *addr3 += 1.0;
    *addr4 += 1;
}

/* Main stress function with extreme register pressure */
__attribute__((noinline)) 
unsigned long stress_reload(volatile int* int_arr, 
                           volatile float* float_arr,
                           volatile double* double_arr,
                           volatile long* long_arr) {
    /* Declare many local variables to exhaust registers */
    int v0, v1, v2, v3, v4, v5, v6, v7, v8, v9;
    int v10, v11, v12, v13, v14, v15, v16, v17, v18, v19;
    float f0, f1, f2, f3, f4, f5, f6, f7, f8, f9;
    double d0, d1, d2, d3, d4, d5, d6, d7, d8, d9;
    long l0, l1, l2, l3, l4, l5, l6, l7, l8, l9;
    
    /* Initialize with values from arrays */
    v0 = int_arr[0]; v1 = int_arr[1]; v2 = int_arr[2]; v3 = int_arr[3];
    v4 = int_arr[4]; v5 = int_arr[5]; v6 = int_arr[6]; v7 = int_arr[7];
    v8 = int_arr[8]; v9 = int_arr[9]; v10 = int_arr[10]; v11 = int_arr[11];
    v12 = int_arr[12]; v13 = int_arr[13]; v14 = int_arr[14]; v15 = int_arr[15];
    v16 = int_arr[16]; v17 = int_arr[17]; v18 = int_arr[18]; v19 = int_arr[19];
    
    f0 = float_arr[0]; f1 = float_arr[1]; f2 = float_arr[2]; f3 = float_arr[3];
    f4 = float_arr[4]; f5 = float_arr[5]; f6 = float_arr[6]; f7 = float_arr[7];
    f8 = float_arr[8]; f9 = float_arr[9];
    
    d0 = double_arr[0]; d1 = double_arr[1]; d2 = double_arr[2]; d3 = double_arr[3];
    d4 = double_arr[4]; d5 = double_arr[5]; d6 = double_arr[6]; d7 = double_arr[7];
    d8 = double_arr[8]; d9 = double_arr[9];
    
    l0 = long_arr[0]; l1 = long_arr[1]; l2 = long_arr[2]; l3 = long_arr[3];
    l4 = long_arr[4]; l5 = long_arr[5]; l6 = long_arr[6]; l7 = long_arr[7];
    l8 = long_arr[8]; l9 = long_arr[9];
    
    volatile unsigned long checksum = 0;
    
    /* Complex loop with multiple basic blocks */
    for (int i = 0; i < ITERATIONS; i++) {
        /* Complex addressing computations that need registers */
        int idx1 = (i * 7 + v0 * 3 + v1 * 5) % ARRAY_SIZE;
        int idx2 = (i * 11 + v2 * 13 + v3 * 17) % ARRAY_SIZE;
        int idx3 = (i * 19 + v4 * 23 + v5 * 29) % ARRAY_SIZE;
        int idx4 = (i * 31 + v6 * 37 + v7 * 41) % ARRAY_SIZE;
        int idx5 = (i * 43 + v8 * 47 + v9 * 53) % ARRAY_SIZE;
        
        /* Control flow to split live ranges */
        switch (i % 8) {
            case 0: {
                /* RELOAD_FOR_INPUT_ADDRESS and RELOAD_FOR_INPADDR_ADDRESS */
                volatile int* addr1 = &int_arr[idx1];
                volatile float* addr2 = &float_arr[idx2];
                
                /* Inline assembly with conflicting constraints */
                int temp1, temp2;
                asm volatile (
                    "mov %[val1], %[tmp1]\n\t"
                    "mov %[val2], %[tmp2]\n\t"
                    : [tmp1] "=r" (temp1), [tmp2] "=r" (temp2)
                    : [val1] "m" (*addr1), [val2] "m" (*addr2)
                    : "memory"
                );
                
                /* Use computed addresses in function calls */
                address_helper(addr1, addr2, &double_arr[idx3], &long_arr[idx4]);
                
                /* Update variables keeping them live */
                v0 = temp1 + v1;
                v1 = temp2 + v0;
                break;
            }
            
            case 1: {
                /* RELOAD_FOR_OUTPUT_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
                volatile double* addr3 = &double_arr[idx3];
                volatile long* addr4 = &long_arr[idx4];
                
                /* Complex expression requiring multiple registers */
                double complex_val = d0 * d1 + d2 * d3 - d4 * d5 + d6 * d7 - d8 * d9;
                long complex_long = l0 ^ l1 | l2 & l3 ^ l4 | l5 & l6 ^ l7 | l8 & l9;
                
                /* Inline assembly with output address constraints */
                double result_d;
                long result_l;
                asm volatile (
                    "fldl %[dbl]\n\t"
                    "fstpl %[out1]\n\t"
                    "movq %[lng], %[out2]\n\t"
                    : [out1] "=m" (result_d), [out2] "=r" (result_l)
                    : [dbl] "m" (complex_val), [lng] "r" (complex_long)
                    : "memory"
                );
                
                *addr3 = result_d;
                *addr4 = result_l;
                break;
            }
            
            case 2:
            case 3: {
                /* RELOAD_FOR_OPERAND_ADDRESS and RELOAD_FOR_OPADDR_ADDR */
                /* Take addresses of locals to force stack-based reloads */
                int* local_addr1 = &v10;
                int* local_addr2 = &v11;
                float* local_addr3 = &f0;
                double* local_addr4 = &d0;
                
                /* Use in arithmetic with array accesses */
                *local_addr1 = int_arr[idx1] + int_arr[idx2];
                *local_addr2 = int_arr[idx3] - int_arr[idx4];
                *local_addr3 = float_arr[idx1] * float_arr[idx2];
                *local_addr4 = double_arr[idx3] / double_arr[idx4];
                
                /* Chain of address computations */
                int** addr_of_addr = &local_addr1;
                float*** addr_of_addr_of_addr = (float***)&addr_of_addr;
                
                /* Force address into register then use */
                volatile int dummy;
                asm volatile (
                    "mov %[addr], %%rax\n\t"
                    "mov (%%rax), %%rbx\n\t"
                    "mov (%%rbx), %[val]\n\t"
                    : [val] "=r" (dummy)
                    : [addr] "r" (addr_of_addr)
                    : "rax", "rbx", "memory"
                );
                break;
            }
            
            case 4:
            case 5: {
                /* RELOAD_OTHER and RELOAD_FOR_OTHER_ADDRESS */
                /* Nested addressing with multiple terms */
                int complex_idx = (idx1 * 2 + idx2 * 3 + idx3 * 5 + idx4 * 7) % ARRAY_SIZE;
                volatile int* complex_addr = &int_arr[complex_idx];
                
                /* Multiple function calls with many arguments */
                int r1 = helper1(v0, v1, v2, v3, v4, v5);
                float r2 = helper2(f0, f1, f2, f3, f4, f5, f6, f7);
                double r3 = helper3(d0, d1, d2, d3, d4, d5, d6, d7, d8, d9);
                long r4 = helper4(l0, l1, l2, l3, l4, l5, l6, l7);
                
                /* Use results in address computation */
                int final_idx = (r1 + (int)r2 + (int)r3 + r4) % ARRAY_SIZE;
                *complex_addr = final_idx;
                
                /* Update many variables to keep them live */
                v0 += r1; v1 += r2; v2 += r3; v3 += r4;
                f0 += r2; f1 += r3; f2 += r4; f3 += r1;
                d0 += r3; d1 += r4; d2 += r1; d3 += r2;
                l0 ^= r4; l1 ^= r1; l2 ^= r2; l3 ^= r3;
                break;
            }
            
            case 6:
            case 7: {
                /* Mixed reload types with computed goto */
                static void* labels[] = { &&label1, &&label2, &&label3, &&label4 };
                int label_idx = (i * v0 + v1 * v2) % 4;
                
                goto *labels[label_idx];
                
            label1:
                /* RELOAD_FOR_INPUT with memory constraint */
                int mem_val1, mem_val2;
                asm volatile (
                    "mov %[in1], %[out1]\n\t"
                    "mov %[in2], %[out2]\n\t"
                    : [out1] "=r" (mem_val1), [out2] "=r" (mem_val2)
                    : [in1] "m" (int_arr[idx1]), [in2] "m" (int_arr[idx2])
                );
                v10 = mem_val1 + mem_val2;
                goto end_switch;
                
            label2:
                /* Output with '+' constraint */
                int out_val = v10 + v11;
                asm volatile (
                    "add %[inc], %[val]\n\t"
                    : [val] "+r" (out_val)
                    : [inc] "r" (v12)
                );
                int_arr[idx3] = out_val;
                goto end_switch;
                
            label3:
                /* Address of local in register */
                int local_val = v13;
                int* reg_addr;
                asm volatile (
                    "lea %[val], %[addr]\n\t"
                    : [addr] "=r" (reg_addr)
                    : [val] "m" (local_val)
                );
                *reg_addr = v14;
                goto end_switch;
                
            label4:
                /* Multiple constraints */
                double dbl_temp = d0;
                asm volatile (
                    "fldl %[in]\n\t"
                    "fadd %%st(0), %%st(0)\n\t"
                    "fstpl %[out]\n\t"
                    : [out] "=m" (dbl_temp)
                    : [in] "m" (d0)
                    : "st"
                );
                d0 = dbl_temp;
                goto end_switch;
                
            end_switch:
                break;
            }
        }
        
        /* Rotate and update all variables to keep them live */
        int temp_v = v19;
        v19 = v18; v18 = v17; v17 = v16; v16 = v15; v15 = v14;
        v14 = v13; v13 = v12; v12 = v11; v11 = v10; v10 = v9;
        v9 = v8; v8 = v7; v7 = v6; v6 = v5; v5 = v4; v4 = v3;
        v3 = v2; v2 = v1; v1 = v0; v0 = temp_v + i;
        
        float temp_f = f9;
        f9 = f8; f8 = f7; f7 = f6; f6 = f5; f5 = f4;
        f4 = f3; f3 = f2; f2 = f1; f1 = f0; f0 = temp_f + i;
        
        double temp_d = d9;
        d9 = d8; d8 = d7; d7 = d6; d6 = d5; d5 = d4;
        d4 = d3; d3 = d2; d2 = d1; d1 = d0; d0 = temp_d + i;
        
        long temp_l = l9;
        l9 = l8; l8 = l7; l7 = l6; l6 = l5; l5 = l4;
        l4 = l3; l3 = l2; l2 = l1; l1 = l0; l0 = temp_l ^ i;
        
        /* Accumulate checksum */
        checksum += v0 + (int)f0 + (int)d0 + l0;
    }
    
    return checksum;
}

int main() {
    /* Allocate and initialize arrays with pattern data */
    volatile int* int_arr = (volatile int*)malloc(ARRAY_SIZE * sizeof(int));
    volatile float* float_arr = (volatile float*)malloc(ARRAY_SIZE * sizeof(float));
    volatile double* double_arr = (volatile double*)malloc(ARRAY_SIZE * sizeof(double));
    volatile long* long_arr = (volatile long*)malloc(ARRAY_SIZE * sizeof(long));
    
    if (!int_arr || !float_arr || !double_arr || !long_arr) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_arr[i] = i * 3 + 7;
        float_arr[i] = i * 1.5f + 2.3f;
        double_arr[i] = i * 2.7 + 3.14;
        long_arr[i] = i * 5L + 11L;
    }
    
    printf("Starting reload stress test...\n");
    
    /* Call the stress function */
    unsigned long result = stress_reload(int_arr, float_arr, double_arr, long_arr);
    
    printf("Checksum result: %lu\n", result);
    
    /* Cleanup */
    free((void*)int_arr);
    free((void*)float_arr);
    free((void*)double_arr);
    free((void*)long_arr);
    
    return 0;
}
