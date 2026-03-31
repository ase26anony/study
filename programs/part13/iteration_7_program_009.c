/* early-remat-trigger.c
 * Designed to trigger early rematerialization with virtual register spills
 * Targets uncovered lines 930-937 in early-remat.cc
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Global variables to create rematerialization candidates */
volatile int global_base = 100;
volatile long global_offset = 1000;
int global_array[256] = {0};
static volatile int sink; /* Prevent dead code elimination */

/* No-op function to clobber registers */
static void __attribute__((noinline)) clobber_registers(void) {
    __asm__ volatile ("" : : : "memory", "rax", "rbx", "rcx", "rdx", 
                      "rsi", "rdi", "r8", "r9", "r10", "r11", "r12", 
                      "r13", "r14", "r15");
}

/* Another no-op with different clobbers for variety */
static void __attribute__((noinline)) clobber_more(void) {
    __asm__ volatile ("# dummy asm" : : : "xmm0", "xmm1", "xmm2", "xmm3",
                      "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9");
}

/* Complex function with many short-lived values */
static int __attribute__((noinline)) 
compute_many_values(int param1, int param2, long param3, int iterations) {
    volatile int result = 0;
    
    /* Outer loop to increase register pressure */
    for (int outer = 0; outer < 10; outer++) {
        /* Inner hot loop with many basic blocks */
        for (int i = 0; i < iterations; i++) {
            /* --- Basic Block 1: Create many remat candidates --- */
            /* Small integer constants (require multiple instructions) */
            int const1 = 0x7FFFFFFF;  /* Large constant */
            int const2 = 0x80000000;  /* Another large constant */
            long const3 = 0x123456789ABCDEF0L; /* 64-bit constant */
            
            /* Constants derived from function arguments */
            int derived1 = param1 + 0x1000;  /* param + constant */
            int derived2 = param2 << 4;      /* param shifted */
            long derived3 = param3 * 3L;     /* param multiplied */
            
            /* Symbol addresses (good remat candidates) */
            int* addr1 = &global_array[i % 256];
            int* addr2 = &global_array[(i + 1) % 256];
            int* addr3 = &global_array[(i + param1) % 256];
            
            /* Constants with different modes */
            short const_short = 0x7FFF;
            char const_char = 0x7F;
            float const_float = 3.14159f;
            double const_double = 2.718281828459045;
            
            /* Force register pressure with many live values */
            int temp1 = const1 + derived1;
            int temp2 = const2 - derived2;
            long temp3 = const3 ^ derived3;
            
            /* --- Basic Block 2: Conditional branch --- */
            if (i & 1) {
                /* Use addresses in computations */
                int idx = (int)(addr1 - global_array);
                int idx2 = (int)(addr2 - global_array);
                
                /* More derived values */
                int val1 = idx * param1;
                int val2 = idx2 * param2;
                int val3 = val1 + val2;
                
                /* Mix data types */
                long val4 = (long)val3 * const3;
                double val5 = (double)val4 * const_double;
                
                /* Force spills with many operations */
                temp1 = val1 + const_short;
                temp2 = val2 - const_char;
                temp3 = (long)(val5 * 100.0);
                
                /* Clobber registers between uses */
                clobber_registers();
                
                /* More computations */
                int val6 = temp1 * 2;
                int val7 = temp2 / 2;
                long val8 = temp3 >> 2;
                
                /* Store to prevent elimination */
                sink = val6 + val7 + (int)val8;
                
                /* Use global variables */
                temp1 += global_base;
                temp2 += global_offset;
                
            } else {
                /* Alternative path with different computations */
                
                /* Pointer arithmetic creating remat candidates */
                int* addr4 = addr1 + param1;
                int* addr5 = addr2 + param2;
                
                /* More constants */
                int const4 = 0x55555555;
                int const5 = 0xAAAAAAAA;
                long const6 = 0x3333333333333333L;
                
                /* Complex expressions */
                int val9 = (int)(addr4 - global_array) ^ const4;
                int val10 = (int)(addr5 - global_array) ^ const5;
                long val11 = (long)val9 * const6;
                
                /* Force different modes */
                float val12 = (float)val10 * const_float;
                double val13 = (double)val11 + const_double;
                
                /* Clobber different registers */
                clobber_more();
                
                /* Continue computations */
                int val14 = (int)val12;
                long val15 = (long)val13;
                int val16 = val14 ^ val9;
                long val17 = val15 | val11;
                
                /* Store results */
                sink = val16 + (int)val17;
                
                /* Update temps for next iteration */
                temp1 = val16;
                temp2 = val10;
                temp3 = val17;
            }
            
            /* --- Basic Block 3: More computations after branch --- */
            
            /* Create more intermediate values */
            int sum1 = temp1 + temp2;
            long sum2 = temp3 + sum1;
            
            /* Use global in computation */
            int offset = global_base + i;
            long product = sum2 * offset;
            
            /* Another conditional */
            if (i % 3 == 0) {
                int scaled = product / (param1 + 1);
                long shifted = product << (param2 & 3);
                
                /* Mix operations */
                int combined = scaled ^ (int)shifted;
                result += combined;
                
                /* Another clobber */
                clobber_registers();
            } else if (i % 3 == 1) {
                /* Different computation path */
                double fp_val = (double)product * 1.5;
                int int_val = (int)fp_val;
                result += int_val;
            } else {
                /* Third path */
                long masked = product & 0xFFFFFFFFL;
                result += (int)masked;
            }
            
            /* Final store to volatile to prevent elimination */
            sink = result;
            
            /* Use all created addresses to keep them live */
            if (addr1 != NULL) sink += *addr1;
            if (addr2 != NULL) sink += *(addr2 - 1);
            if (addr3 != NULL) sink += *(addr3 + 1);
        }
    }
    
    return result;
}

/* Main driver with command line arguments */
int main(int argc, char *argv[]) {
    int iterations = 1000;
    int param1 = 42;
    int param2 = 73;
    long param3 = 123456789L;
    
    /* Use command line arguments for variability */
    if (argc > 1) {
        param1 = atoi(argv[1]);
    }
    if (argc > 2) {
        param2 = atoi(argv[2]);
    }
    if (argc > 3) {
        iterations = atoi(argv[3]);
    }
    
    printf("Starting computation with params: %d, %d, %ld, iterations: %d\n",
           param1, param2, param3, iterations);
    
    int result = compute_many_values(param1, param2, param3, iterations);
    
    printf("Result: %d (sink: %d)\n", result, sink);
    
    return 0;
}
