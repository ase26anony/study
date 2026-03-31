#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Prevent inlining to force scheduler state saves/restores */
__attribute__((noinline)) float helper_float(float a, float b, float c) {
    volatile float barrier = a * b;
    asm volatile("" ::: "memory");
    return barrier + c * 0.5f;
}

__attribute__((noinline)) int helper_int(int a, int b, int c) {
    volatile int barrier = a ^ b;
    asm volatile("" ::: "memory");
    return barrier + (c << 3);
}

__attribute__((noinline)) double helper_double(double a, double b, int c) {
    volatile double barrier = a / (b + 1.0);
    asm volatile("" ::: "memory");
    return barrier * c;
}

__attribute__((noinline)) void* helper_mem(void* ptr, int offset) {
    volatile char* p = (char*)ptr;
    asm volatile("" ::: "memory");
    return p + offset;
}

/* Main stress function with high register pressure */
__attribute__((noinline)) uint64_t stress_scheduler(void) {
    /* Many local variables to create register pressure */
    volatile int outer_limit = 1000; /* Prevent constant propagation */
    int i, j, k;
    
    /* Integer variables */
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
    int v11 = 11, v12 = 12, v13 = 13, v14 = 14, v15 = 15;
    int v16 = 16, v17 = 17, v18 = 18, v19 = 19, v20 = 20;
    
    /* Floating point variables */
    float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f, f5 = 5.5f;
    float f6 = 6.6f, f7 = 7.7f, f8 = 8.8f, f9 = 9.9f, f10 = 10.10f;
    
    /* Double precision variables */
    double d1 = 1.01, d2 = 2.02, d3 = 3.03, d4 = 4.04, d5 = 5.05;
    
    /* Memory/pointer variables */
    int array[256];
    volatile int* volatile_ptr = array;
    void* ptr1 = array;
    void* ptr2 = array + 128;
    
    /* Initialize array with pattern */
    for (i = 0; i < 256; i++) {
        array[i] = i ^ 0x55;
    }
    
    uint64_t checksum = 0;
    
    /* Outer loop with volatile limit */
    for (i = 0; i < outer_limit; i++) {
        /* Nested loops with variable bounds */
        volatile int inner_limit = (i % 64) + 10;
        
        for (j = 0; j < inner_limit; j++) {
            /* Innermost loop with data-dependent trip count */
            int innermost = (j % 8) + 3;
            
            for (k = 0; k < innermost; k++) {
                /* Complex dependency chain across different operation types */
                
                /* Integer arithmetic chain */
                v1 = v2 + v3;
                v2 = v3 ^ v4;
                v3 = v4 * v5;
                v4 = v5 - v6;
                v5 = v6 | v7;
                
                /* Memory barrier to prevent reordering */
                asm volatile("" ::: "memory");
                
                /* Floating point chain with integer inputs */
                f1 = (float)v1 * f2;
                f2 = f3 + (float)v2;
                f3 = f4 / (f5 + 1.0f);
                f4 = helper_float(f1, f2, f3);
                
                /* Memory access chain */
                int idx = (v1 + v2 + v3) & 0xFF;
                v6 = array[idx];
                array[(idx + 1) & 0xFF] = v6 + v7;
                volatile_ptr = array + ((idx + 2) & 0xFF);
                
                /* Double precision chain */
                d1 = d2 * (double)f1;
                d2 = helper_double(d1, d2, v4);
                d3 = d4 / (d5 + 1.0);
                
                /* More integer operations */
                v7 = v8 << (v9 & 3);
                v8 = v9 >> (v10 & 3);
                v9 = helper_int(v10, v11, v12);
                
                /* Conditional execution paths */
                switch ((i + j + k) & 7) {
                    case 0:
                        /* FP math path */
                        f5 = f6 * f7;
                        f6 = helper_float(f7, f8, f9);
                        v10 = (int)(f5 * 100.0f);
                        break;
                    case 1:
                        /* Integer bit manipulation path */
                        v11 = v12 ^ v13;
                        v12 = (v13 << 2) | (v14 >> 2);
                        v13 = helper_int(v14, v15, v16);
                        break;
                    case 2:
                        /* Memory intensive path */
                        ptr1 = helper_mem(array, v15 * sizeof(int));
                        v14 = *(int*)ptr1;
                        ptr2 = helper_mem(array, v16 * sizeof(int));
                        v15 = *(int*)ptr2;
                        break;
                    case 3:
                        /* Mixed type path */
                        f7 = (float)v16 * 0.25f;
                        d4 = (double)f7 * d5;
                        v16 = (int)d4;
                        break;
                    default:
                        /* Default arithmetic path */
                        v17 = v18 + v19;
                        v18 = v19 - v20;
                        v19 = v20 * v1;
                        f8 = (float)v17 * 0.5f;
                        break;
                }
                
                /* More barriers and dependencies */
                asm volatile("" ::: "memory");
                
                /* Additional register pressure operations */
                v20 = v1 + v2 + v3 + v4 + v5;
                f9 = f1 + f2 + f3 + f4 + f5;
                d5 = d1 + d2 + d3 + d4;
                
                /* Function calls with side effects */
                if ((k & 1) == 0) {
                    v1 = helper_int(v20, v19, v18);
                } else {
                    f10 = helper_float(f9, f8, f7);
                }
                
                /* Update checksum with various values */
                checksum ^= (uint64_t)v1;
                checksum ^= (uint64_t)(*(uint32_t*)&f1);
                checksum ^= (uint64_t)(*(uint64_t*)&d1);
                checksum ^= (uint64_t)array[(i + j + k) & 0xFF];
            }
            
            /* Additional operations between inner loops */
            v1 = v1 ^ v2 ^ v3;
            f1 = f1 * 0.99f;
            d1 = d1 * 0.999;
            
            /* Call helper with current state */
            if (j % 4 == 0) {
                v2 = helper_int(v1, v3, v4);
            }
        }
        
        /* Periodic state mixing */
        if (i % 100 == 0) {
            /* Shuffle array to create memory dependencies */
            for (int m = 0; m < 128; m += 4) {
                int temp = array[m];
                array[m] = array[m + 128];
                array[m + 128] = temp;
            }
        }
    }
    
    /* Final mixing of all variables */
    checksum ^= v1 ^ v2 ^ v3 ^ v4 ^ v5;
    checksum ^= v6 ^ v7 ^ v8 ^ v9 ^ v10;
    checksum ^= v11 ^ v12 ^ v13 ^ v14 ^ v15;
    checksum ^= v16 ^ v17 ^ v18 ^ v19 ^ v20;
    checksum ^= *(uint32_t*)&f1 ^ *(uint32_t*)&f2;
    checksum ^= *(uint32_t*)&f3 ^ *(uint32_t*)&f4 ^ *(uint32_t*)&f5;
    checksum ^= *(uint32_t*)&f6 ^ *(uint32_t*)&f7;
    checksum ^= *(uint32_t*)&f8 ^ *(uint32_t*)&f9 ^ *(uint32_t*)&f10;
    checksum ^= *(uint64_t*)&d1 ^ *(uint64_t*)&d2;
    checksum ^= *(uint64_t*)&d3 ^ *(uint64_t*)&d4 ^ *(uint64_t*)&d5;
    
    return checksum;
}

int main(void) {
    printf("Starting scheduler stress test...\n");
    
    uint64_t result = stress_scheduler();
    
    printf("Checksum: 0x%016llx\n", (unsigned long long)result);
    printf("Test completed.\n");
    
    return 0;
}
