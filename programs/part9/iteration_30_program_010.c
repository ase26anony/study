/* reload1_stress.c - Extreme register pressure with complex addressing modes */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define ARRAY_SIZE 1024
#define NUM_LOCALS 40
#define ITERATIONS 1000

/* Opaque noinline functions to prevent optimization */
__attribute__((noinline)) int helper1(int a, int b, int c, int d, int e, int f) {
    volatile int sink = a + b - c + d - e + f;
    return sink & 0xFF;
}

__attribute__((noinline)) float helper2(float a, float b, float c, float* addr) {
    volatile float sink = *addr + a * b - c;
    return sink;
}

__attribute__((noinline)) double helper3(double a, double b, double* addr1, double* addr2) {
    volatile double sink = *addr1 + *addr2 + a * b;
    return sink;
}

__attribute__((noinline)) long helper4(long a, long b, long c, long* addr, int offset) {
    volatile long sink = addr[offset] + a * b - c;
    return sink;
}

__attribute__((noinline)) void* helper5(void* p1, void* p2, void* p3, int idx) {
    volatile char* sink = (char*)p1 + idx;
    *sink = ((char*)p2)[idx] + ((char*)p3)[idx];
    return (void*)sink;
}

/* Main stress function with extreme register pressure */
__attribute__((noinline)) int stress_reload(int* arr_int, double* arr_dbl, 
                                          float* arr_flt, long* arr_lng) {
    /* Declare many local variables to exhaust registers */
    volatile int v0, v1, v2, v3, v4, v5, v6, v7, v8, v9;
    volatile float f0, f1, f2, f3, f4, f5, f6, f7, f8, f9;
    volatile double d0, d1, d2, d3, d4, d5, d6, d7, d8, d9;
    volatile long l0, l1, l2, l3, l4, l5, l6, l7, l8, l9;
    volatile int* ptr_int;
    volatile double* ptr_dbl;
    volatile float* ptr_flt;
    volatile long* ptr_lng;
    
    /* Additional volatile locals for address computations */
    volatile int addr_temp1, addr_temp2, addr_temp3;
    volatile long addr_offset1, addr_offset2;
    
    int checksum = 0;
    
    /* Initialize locals with array values */
    v0 = arr_int[0]; v1 = arr_int[1]; v2 = arr_int[2]; v3 = arr_int[3];
    v4 = arr_int[4]; v5 = arr_int[5]; v6 = arr_int[6]; v7 = arr_int[7];
    v8 = arr_int[8]; v9 = arr_int[9];
    
    f0 = arr_flt[0]; f1 = arr_flt[1]; f2 = arr_flt[2]; f3 = arr_flt[3];
    f4 = arr_flt[4]; f5 = arr_flt[5]; f6 = arr_flt[6]; f7 = arr_flt[7];
    f8 = arr_flt[8]; f9 = arr_flt[9];
    
    d0 = arr_dbl[0]; d1 = arr_dbl[1]; d2 = arr_dbl[2]; d3 = arr_dbl[3];
    d4 = arr_dbl[4]; d5 = arr_dbl[5]; d6 = arr_dbl[6]; d7 = arr_dbl[7];
    d8 = arr_dbl[8]; d9 = arr_dbl[9];
    
    l0 = arr_lng[0]; l1 = arr_lng[1]; l2 = arr_lng[2]; l3 = arr_lng[3];
    l4 = arr_lng[4]; l5 = arr_lng[5]; l6 = arr_lng[6]; l7 = arr_lng[7];
    l8 = arr_lng[8]; l9 = arr_lng[9];
    
    ptr_int = arr_int;
    ptr_dbl = arr_dbl;
    ptr_flt = arr_flt;
    ptr_lng = arr_lng;
    
    /* Main loop with complex addressing and control flow */
    for (int i = 0; i < ITERATIONS; i++) {
        /* Complex array indexing with multi-term expressions */
        int idx1 = (i * 7 + v0 * 3 + v1) % ARRAY_SIZE;
        int idx2 = (i * 11 + v2 * 5 + v3 * 2) % ARRAY_SIZE;
        int idx3 = (i * 13 + v4 * 7 + v5 * 3) % ARRAY_SIZE;
        int idx4 = (i * 17 + v6 * 11 + v7 * 5) % ARRAY_SIZE;
        
        /* Address computations that need registers */
        long offset1 = (l0 * i + l1 * idx1 + l2 * idx2) % (ARRAY_SIZE - 100);
        long offset2 = (l3 * i + l4 * idx3 + l5 * idx4) % (ARRAY_SIZE - 100);
        
        /* Use computed addresses in memory accesses */
        volatile int* addr1 = &arr_int[idx1 + offset1 % 50];
        volatile double* addr2 = &arr_dbl[idx2 + offset2 % 50];
        volatile float* addr3 = &arr_flt[idx3 + (offset1 * 2) % 50];
        volatile long* addr4 = &arr_lng[idx4 + (offset2 * 3) % 50];
        
        /* Inline assembly with conflicting constraints to force reloads */
        int temp1, temp2;
        double dtemp1, dtemp2;
        
        /* RELOAD_FOR_INPUT and RELOAD_FOR_INPUT_ADDRESS patterns */
        asm volatile (
            "mov %[val1], %[tmp1]\n\t"
            "add %[val2], %[tmp1]\n\t"
            "mov %[tmp1], %[out1]\n\t"
            : [out1] "=r" (temp1)
            : [val1] "r" (v0), [val2] "r" (v1), [tmp1] "r" (0)
            : "memory"
        );
        
        /* RELOAD_FOR_OUTPUT_ADDRESS pattern */
        asm volatile (
            "mov %[addr], %%rsi\n\t"
            "mov (%%rsi), %[out]\n\t"
            : [out] "=r" (temp2)
            : [addr] "r" (addr1)
            : "rsi", "memory"
        );
        
        /* Mixed operand types causing different reload types */
        asm volatile (
            "movsd %[dbl1], %%xmm0\n\t"
            "addsd %[dbl2], %%xmm0\n\t"
            "movsd %%xmm0, %[outdbl]\n\t"
            : [outdbl] "=m" (dtemp1)
            : [dbl1] "m" (*addr2), [dbl2] "r" (d0)
            : "xmm0", "memory"
        );
        
        /* Control flow that splits live ranges */
        switch (i % 8) {
            case 0: {
                /* Address computation in this block, use in next */
                addr_temp1 = idx1 * 3 + idx2 * 7;
                addr_temp2 = idx3 * 5 + idx4 * 11;
                /* Use helper that takes addresses */
                checksum += helper1(v0, v1, v2, v3, addr_temp1, addr_temp2);
                break;
            }
            case 1: {
                /* Different address pattern */
                addr_offset1 = offset1 * 2 + offset2 * 3;
                dtemp2 = helper3(d1, d2, 
                               (double*)&arr_dbl[addr_offset1 % ARRAY_SIZE],
                               (double*)&arr_dbl[(addr_offset1 * 2) % ARRAY_SIZE]);
                checksum += (int)dtemp2;
                break;
            }
            case 2: {
                /* Complex addressing with pointer arithmetic */
                volatile int* complex_addr = &arr_int[(i * 19 + v8 * 13) % ARRAY_SIZE];
                volatile double* complex_dbl_addr = &arr_dbl[(i * 23 + v9 * 17) % ARRAY_SIZE];
                
                /* Force address reloads through inline asm */
                long result;
                asm volatile (
                    "mov %[addr1], %%rbx\n\t"
                    "mov (%%rbx), %%eax\n\t"
                    "mov %[addr2], %%rcx\n\t"
                    "add (%%rcx), %%eax\n\t"
                    "mov %%eax, %[res]\n\t"
                    : [res] "=r" (result)
                    : [addr1] "r" (complex_addr), [addr2] "r" (&arr_int[i % ARRAY_SIZE])
                    : "rax", "rbx", "rcx", "memory"
                );
                checksum += result;
                break;
            }
            case 3:
            case 4: {
                /* Nested addressing */
                int nested_idx = (idx1 * idx2 + idx3 * idx4) % ARRAY_SIZE;
                volatile float* nested_addr = &arr_flt[nested_idx];
                f0 = helper2(f1, f2, f3, (float*)nested_addr);
                checksum += (int)f0;
                break;
            }
            case 5: {
                /* Address of address computation */
                volatile int** ptr_to_ptr = &ptr_int;
                volatile long* laddr = &arr_lng[(offset1 + offset2) % ARRAY_SIZE];
                l0 = helper4(l1, l2, l3, laddr, i % 50);
                checksum += (int)l0;
                break;
            }
            case 6: {
                /* Multiple address computations in one expression */
                void* result = helper5(
                    (void*)&arr_int[idx1],
                    (void*)&arr_int[idx2],
                    (void*)&arr_int[idx3],
                    idx4
                );
                checksum += *(int*)result;
                break;
            }
            case 7: {
                /* All address types in one case */
                addr_temp3 = (v0 * v1 + v2 * v3) % ARRAY_SIZE;
                volatile int* addr_array[3] = {
                    &arr_int[addr_temp3],
                    &arr_int[(addr_temp3 * 2) % ARRAY_SIZE],
                    &arr_int[(addr_temp3 * 3) % ARRAY_SIZE]
                };
                
                /* Use all three addresses */
                int sum = 0;
                for (int j = 0; j < 3; j++) {
                    sum += *addr_array[j];
                }
                checksum += sum;
                break;
            }
        }
        
        /* Update most local variables to keep them live */
        v0 = v1 + arr_int[idx1];
        v1 = v2 + arr_int[idx2];
        v2 = v3 + arr_int[idx3];
        v3 = v4 + arr_int[idx4];
        v4 = v5 + temp1;
        v5 = v6 + temp2;
        v6 = v7 + addr_temp1;
        v7 = v8 + addr_temp2;
        v8 = v9 + addr_temp3;
        v9 = i + checksum;
        
        f0 = f1 + arr_flt[idx1 % ARRAY_SIZE];
        f1 = f2 + arr_flt[idx2 % ARRAY_SIZE];
        f2 = f3 + arr_flt[idx3 % ARRAY_SIZE];
        f3 = f4 + arr_flt[idx4 % ARRAY_SIZE];
        
        d0 = d1 + arr_dbl[idx1 % ARRAY_SIZE];
        d1 = d2 + arr_dbl[idx2 % ARRAY_SIZE];
        d2 = d3 + arr_dbl[idx3 % ARRAY_SIZE];
        d3 = d4 + arr_dbl[idx4 % ARRAY_SIZE];
        
        l0 = l1 + arr_lng[idx1 % ARRAY_SIZE];
        l1 = l2 + arr_lng[idx2 % ARRAY_SIZE];
        l2 = l3 + arr_lng[idx3 % ARRAY_SIZE];
        l3 = l4 + arr_lng[idx4 % ARRAY_SIZE];
        l4 = l5 + offset1;
        l5 = l6 + offset2;
        
        /* Force spill/reload around loop backedge */
        if (i % 100 == 0) {
            /* Additional function call to force register shuffling */
            checksum += helper1(v0, v1, v2, v3, v4, v5);
            checksum += helper1(v6, v7, v8, v9, temp1, temp2);
        }
    }
    
    return checksum;
}

int main() {
    /* Allocate and initialize arrays with pattern data */
    int* arr_int = (int*)malloc(ARRAY_SIZE * sizeof(int));
    double* arr_dbl = (double*)malloc(ARRAY_SIZE * sizeof(double));
    float* arr_flt = (float*)malloc(ARRAY_SIZE * sizeof(float));
    long* arr_lng = (long*)malloc(ARRAY_SIZE * sizeof(long));
    
    if (!arr_int || !arr_dbl || !arr_flt || !arr_lng) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr_int[i] = i * 3 + 1;
        arr_dbl[i] = i * 0.5 + 2.0;
        arr_flt[i] = i * 0.25f + 1.5f;
        arr_lng[i] = i * 7L + 3L;
    }
    
    /* Call the stress function */
    int result = stress_reload(arr_int, arr_dbl, arr_flt, arr_lng);
    
    printf("Checksum result: %d\n", result);
    
    /* Cleanup */
    free(arr_int);
    free(arr_dbl);
    free(arr_flt);
    free(arr_lng);
    
    return 0;
}
