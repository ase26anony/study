#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Prevent inlining to force scheduler state saves/restores */
__attribute__((noinline)) 
float helper_float(float a, float b, float c, int iter) {
    volatile float v1 = a * 1.5f;
    float v2 = b + v1;
    float v3 = c - v2;
    asm volatile("" : "+r"(iter) : : "memory");
    return v3 * (iter & 1 ? 0.8f : 1.2f);
}

__attribute__((noinline))
int helper_int(int a, int b, int c, float f) {
    int v1 = a ^ b;
    int v2 = v1 | c;
    int v3 = v2 & (int)(f * 100.0f);
    asm volatile("" : : : "memory");
    return v3 + (b >> 3);
}

__attribute__((noinline))
double helper_double(double a, double b, int iter, int* ptr) {
    double v1 = a / (b + 1.0);
    double v2 = v1 * (double)(*ptr);
    asm volatile("" : "+r"(iter) : : "memory");
    return v2 + (iter % 10) * 0.1;
}

/* Complex function with high register pressure and mixed operations */
__attribute__((noinline))
uint64_t complex_scheduling_test(volatile int outer_limit) {
    /* High register pressure: 30+ scalar variables */
    int i, j, k, m, n, p, q, r, s, t;
    float f1, f2, f3, f4, f5, f6, f7, f8, f9, f10;
    double d1, d2, d3, d4, d5, d6;
    int arr[32];
    volatile int* volatile_ptr = arr;
    uint64_t checksum = 0;
    
    /* Initialize arrays and variables with volatile to prevent optimization */
    for (i = 0; i < 32; i++) {
        arr[i] = i * 3 + 1;
    }
    
    /* Outer loop with volatile limit */
    for (i = 0; i < outer_limit; i++) {
        volatile int inner_limit = (i % 10) + 5;
        
        /* Nested loops with variable bounds */
        for (j = 0; j < inner_limit; j++) {
            volatile int middle_limit = (j % 5) + 3;
            
            for (k = 0; k < middle_limit; k++) {
                /* Mixed operation dependency chains */
                f1 = (float)i * 0.5f;
                f2 = (float)j * 1.5f;
                f3 = f1 + f2;
                
                /* Memory access creating dependencies */
                m = arr[k] + i;
                arr[k] = m ^ j;
                
                /* Integer arithmetic chain */
                n = (i << 3) | (j << 1);
                p = n ^ 0xABCD;
                q = p * 3 + 1;
                
                /* Memory barrier to force scheduler grouping */
                asm volatile("" : : : "memory");
                
                /* Floating-point chain */
                f4 = f3 * 2.0f;
                f5 = f4 - 1.0f;
                f6 = helper_float(f5, f3, f4, k);
                
                /* Conditional execution paths */
                switch (k % 4) {
                    case 0:
                        /* FP math path */
                        f7 = f6 * 3.14f;
                        d1 = (double)f7 / 2.71828;
                        d2 = helper_double(d1, d1 * 2.0, i, &arr[j]);
                        checksum += (uint64_t)(d2 * 1000.0);
                        break;
                    case 1:
                        /* Integer bit manipulation path */
                        r = helper_int(i, j, k, f6);
                        s = r ^ 0xDEADBEEF;
                        t = (s << 4) | (s >> 28);
                        arr[k] ^= t;
                        checksum += t;
                        break;
                    case 2:
                        /* Mixed operations path */
                        f8 = (float)arr[k] * 0.25f;
                        f9 = helper_float(f8, f6, f1, j);
                        d3 = (double)f9 * 1.414;
                        checksum += (uint64_t)(d3 * 100.0);
                        break;
                    case 3:
                        /* Memory intensive path */
                        for (m = 0; m < 4; m++) {
                            volatile_ptr[m] = volatile_ptr[m] * 2 + 1;
                            asm volatile("" : : : "memory");
                        }
                        checksum += arr[k % 4];
                        break;
                }
                
                /* More dependency chains */
                f10 = (float)(arr[(i + j) % 32]) * 0.1f;
                d4 = (double)f10 + 1.0;
                d5 = d4 * d4 - 2.0 * d4 + 1.0;
                
                /* Another memory barrier */
                asm volatile("" : : : "memory");
                
                /* Use results in next iteration */
                if (k > 0) {
                    arr[k] += (int)(d5 * 10.0);
                }
                
                /* Call helper with cross-type dependencies */
                int res = helper_int(arr[k], i, j, f10);
                checksum ^= (uint64_t)res << (k % 16);
            }
            
            /* Additional operations between loop levels */
            d6 = helper_double((double)j, (double)i, i + j, &arr[0]);
            checksum += (uint64_t)(d6 * 10000.0);
            
            /* Volatile store to prevent reordering */
            volatile int dummy = i * j;
            (void)dummy;
        }
        
        /* Periodic function call with many live variables */
        if (i % 7 == 0) {
            float fres = helper_float(f1, f3, f6, i);
            checksum += (uint64_t)(fres * 100.0);
        }
        
        /* Complex conditional with data dependencies */
        if (checksum & 1) {
            for (n = 0; n < 8; n++) {
                arr[(i + n) % 32] = helper_int(arr[n], i, n, f5);
                asm volatile("" : : : "memory");
            }
        }
    }
    
    /* Final checksum calculation using all variables */
    for (i = 0; i < 32; i++) {
        checksum ^= (uint64_t)arr[i] << (i % 16);
    }
    
    return checksum;
}

int main() {
    volatile int iterations = 1000;
    uint64_t result;
    
    printf("Starting complex scheduling stress test...\n");
    
    /* Run the test multiple times to increase coverage probability */
    for (int run = 0; run < 3; run++) {
        result = complex_scheduling_test(iterations);
        printf("Run %d: Checksum = 0x%016llx\n", run + 1, 
               (unsigned long long)result);
    }
    
    /* Additional test with different iteration count */
    iterations = 500;
    result = complex_scheduling_test(iterations);
    printf("Final run: Checksum = 0x%016llx\n", 
           (unsigned long long)result);
    
    return 0;
}
