/* early-remat-trigger.c
 * Designed to trigger early rematerialization with virtual register spills
 * Specifically targets lines 930-937 in early-remat.cc
 */

#include <stdio.h>
#include <stdlib.h>

/* Global variables to create rematerialization candidates */
volatile int global_base = 100;
volatile long global_offset = 1000;
int global_array[256] = {0};
static volatile int sink; /* Prevent optimization */

/* Dummy function to clobber registers */
static void __attribute__((noinline)) clobber_registers(void) {
    __asm__ volatile ("" : : : "memory");
}

/* Another noinline function to force spills */
static int __attribute__((noinline)) use_value(int x) {
    __asm__ volatile ("" : "+r" (x) : : );
    return x;
}

/* Main function with complex loop structure */
int main(int argc, char *argv[]) {
    /* Use arguments to create variant values */
    int loop_count = (argc > 1) ? atoi(argv[1]) : 1000;
    int param1 = (argc > 2) ? atoi(argv[2]) : 42;
    
    /* Force many different constants that are remat candidates */
    const int small_const = 7;      /* Requires multiple instructions on some arches */
    const long large_const = 0x12345678ABCDEF00UL;
    const int shift_const = param1 << 2;  /* Derived from argument */
    const int add_const = param1 + 17;
    
    /* Results accumulator */
    volatile int total = 0;
    
    /* Outer loop to create pressure */
    for (int outer = 0; outer < 10; outer++) {
        /* Inner hot loop with many basic blocks */
        for (int i = 0; i < loop_count; i++) {
            /* Basic Block 1: Create many intermediate values */
            int val1 = small_const * i;           /* Constant remat candidate */
            long val2 = large_const + i;          /* Large constant */
            int val3 = shift_const | i;           /* Derived from param */
            int val4 = add_const - i;             /* Another derived constant */
            
            /* Address computation - good remat candidate */
            int *ptr1 = &global_array[i & 0xFF];
            int *ptr2 = &global_array[(i + 1) & 0xFF];
            
            /* More computations with different modes */
            long val5 = (long)val1 * val2;        /* DImode operation */
            int val6 = val3 ^ val4;               /* SImode operation */
            
            /* Force register pressure with many live values */
            int val7 = val1 + val2;
            int val8 = val3 - val4;
            long val9 = val5 >> 3;
            int val10 = val6 & 0xFF;
            
            /* Basic Block 2: Conditional branch creates new BB */
            if (i & 1) {
                /* Even more computations in this path */
                int val11 = global_base + i;      /* Global variable access */
                long val12 = global_offset * i;   /* Another global */
                
                /* Pointer arithmetic with different modes */
                int *ptr3 = ptr1 + (val11 % 16);
                int *ptr4 = ptr2 - (val12 % 16);
                
                /* Use the values to prevent elimination */
                val7 += *ptr3;
                val8 += *ptr4 ? 1 : 0;
                
                /* Another remat candidate: constant expression */
                int val13 = (param1 * 3) + (i << 1);
                val9 += val13;
            } else {
                /* Alternative path with different computations */
                int val14 = (param1 >> 1) + i;    /* Another derived constant */
                long val15 = (global_base * 2) + i;
                
                /* More address computations */
                int *ptr5 = &global_array[(i + val14) & 0xFF];
                int *ptr6 = &global_array[(i + 2) & 0xFF];
                
                val10 += *ptr5;
                val9 -= *ptr6 ? val14 : 0;
                
                /* Create spill pressure */
                int val16 = val14 * 3;
                int val17 = val16 + 255;          /* Constant addition */
                val7 += val17;
            }
            
            /* Basic Block 3: Another conditional */
            if (i % 3 == 0) {
                /* Complex expression that might be rematerialized */
                int val18 = (param1 * 7) + (global_base / 2);
                long val19 = (large_const >> 4) + i;
                
                val8 ^= val18;
                val9 |= val19;
                
                /* Force different mode operations */
                ptr1 = (int*)((long)ptr1 + val19);
            }
            
            /* Call function that clobbers registers */
            clobber_registers();
            
            /* Basic Block 4: Use all values to prevent elimination */
            int sum1 = val7 + val8 + val10;
            long sum2 = val9 + val5;
            
            /* More computations mixing types */
            int val20 = sum1 + (sum2 & 0xFFFFFFFF);
            long val21 = (sum2 << 2) + val20;
            
            /* Use noinline function to force spills */
            val20 = use_value(val20);
            
            /* Store to volatile to prevent optimization */
            sink = val20;
            total += val20 + (val21 & 0xFF);
            
            /* More register pressure at end of loop */
            int val22 = global_base++;
            long val23 = global_offset--;
            int val24 = val22 * 3;
            long val25 = val23 / 2;
            
            /* Final clobber */
            __asm__ volatile ("" : : : 
                "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
                "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15");
        }
    }
    
    printf("Result: %d\n", total);
    return total != 0;
}
