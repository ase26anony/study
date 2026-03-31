/* early-remat-trigger.c
 * Designed to trigger early rematerialization with virtual register spills
 * Targets uncovered lines 930-937 in early-remat.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Global variables to create rematerialization candidates */
volatile int global_base = 42;
int global_array[256] = {0};
static volatile long static_counter = 0;

/* Dummy function to clobber registers */
static void __attribute__((noinline)) clobber_registers(void) {
    __asm__ volatile ("" : : : "memory", "rax", "rbx", "rcx", "rdx", 
                      "rsi", "rdi", "r8", "r9", "r10", "r11", "r12", 
                      "r13", "r14", "r15");
}

/* Another noinline function to force spills */
static int __attribute__((noinline)) use_value(int x) {
    __asm__ volatile ("" : "+r" (x));
    return x;
}

/* Function with many parameters to create argument-derived constants */
static long __attribute__((noinline)) compute_offset(int a, int b, long c, void *d) {
    /* Create various constant expressions from arguments */
    long offset1 = (long)a + 1;          /* Constant derived from arg */
    long offset2 = (long)b << 2;         /* Shift constant */
    long offset3 = c & 0xFFF;            /* Mask constant */
    uintptr_t offset4 = (uintptr_t)d & ~0x7; /* Aligned address */
    
    __asm__ volatile ("" : : "r" (offset1), "r" (offset2), "r" (offset3), "r" (offset4));
    return offset1 + offset2 + offset3 + offset4;
}

int main(int argc, char *argv[]) {
    /* Use arguments to create variant constants */
    int loop_count = argc > 1 ? atoi(argv[1]) : 1000;
    int base_value = argc > 2 ? atoi(argv[2]) : 12345;
    
    volatile int sink = 0;
    long total = 0;
    
    /* Outer loop to create hot region */
    for (int outer = 0; outer < 100; outer++) {
        /* Inner loop with high register pressure */
        for (int i = 0; i < loop_count; i++) {
            /* Create many short-lived intermediate values with different modes */
            
            /* SImode constants (32-bit) */
            int const1 = global_base + 1;          /* Global-derived constant */
            int const2 = base_value << 3;          /* Argument-derived constant */
            int const3 = 0x7FFFFFFF;               /* Large constant */
            int const4 = -1000;                    /* Negative constant */
            
            /* DImode constants (64-bit) */
            long const5 = (long)global_base * 1000;
            long const6 = (long)base_value + 0x12345678;
            long const7 = ~0UL >> 4;               /* Complex bit pattern */
            
            /* Pmode constants (pointer-width) */
            uintptr_t const8 = (uintptr_t)&global_array[i & 255];
            uintptr_t const9 = (uintptr_t)&global_base + i * 4;
            
            /* More derived constants */
            int const10 = const1 + const2;
            long const11 = const5 - const6;
            uintptr_t const12 = const8 + const9;
            
            /* Use in conditional branches to create multiple basic blocks */
            if (i & 1) {
                /* Branch 1: Use some constants */
                int temp1 = const1 * const3;
                long temp2 = const5 + const11;
                uintptr_t temp3 = const12 - const8;
                
                sink += temp1;
                total += temp2 + temp3;
                
                /* More computations to increase live range */
                int temp4 = const2 / (const4 + 2);
                long temp5 = const6 ^ const7;
                
                /* Call to clobber registers */
                clobber_registers();
                
                sink += temp4;
                total += temp5;
            } else {
                /* Branch 2: Use different constants */
                int temp6 = const10 - const3;
                long temp7 = const11 * 2;
                uintptr_t temp8 = const9 >> 2;
                
                sink += temp6;
                total += temp7 + temp8;
                
                /* Different computations */
                int temp9 = const4 | const1;
                long temp10 = const7 & 0xFFFF;
                
                /* Another clobbering call */
                clobber_registers();
                
                sink += temp9;
                total += temp10;
            }
            
            /* Third basic block after branches */
            if (i & 2) {
                /* Mix all types again */
                int temp11 = const1 + const10;
                long temp12 = const5 - const7;
                uintptr_t temp13 = const8 + i;
                
                /* Function call with multiple arguments */
                long offset = compute_offset(const1, const2, const11, (void*)const8);
                
                sink += temp11 + offset;
                total += temp12 + temp13;
            } else {
                /* Alternative path */
                int temp14 = const3 * const4;
                long temp15 = const6 / 16;
                
                /* Use the values to prevent elimination */
                use_value(temp14);
                use_value((int)temp15);
                
                sink += temp14;
                total += temp15;
            }
            
            /* Final computations using all constants */
            int final1 = const1 + const2 + const3 + const4 + const10;
            long final2 = const5 + const6 + const7 + const11;
            uintptr_t final3 = const8 + const9 + const12;
            
            /* Force register pressure by using many values simultaneously */
            __asm__ volatile (""
                : 
                : "r" (final1), "r" (final2), "r" (final3),
                  "r" (const1), "r" (const2), "r" (const3),
                  "r" (const4), "r" (const5), "r" (const6)
                : "memory");
            
            sink += final1 + final2 + final3;
            
            /* Store to global to prevent elimination */
            global_array[i & 255] = sink;
        }
        
        /* Modify globals to prevent constant propagation */
        global_base++;
        static_counter += outer;
    }
    
    printf("Result: %ld (sink=%d)\n", total, sink);
    return 0;
}
