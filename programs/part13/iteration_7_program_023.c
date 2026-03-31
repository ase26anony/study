/* early-remat-test.c
 * Compile with: gcc -O2 -fearly-remat -fdump-rtl-early-remat -fdump-rtl-all -fno-omit-frame-pointer -fno-schedule-insns -fno-schedule-insns2 --param=max-early-remat-iterations=5 -o early-remat-test early-remat-test.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Global variables to create rematerialization candidates */
volatile int global_base = 100;
int global_array[256] = {0};
static long static_counter = 0;

/* Function to clobber registers */
static void __attribute__((noinline)) clobber_registers(void) {
    __asm__ volatile ("" : : : "memory", "rax", "rbx", "rcx", "rdx", "rsi", "rdi", 
                      "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15");
}

/* Dummy function to prevent optimization */
static void __attribute__((noinline)) use_value(volatile int *ptr) {
    __asm__ volatile ("" : : "r" (ptr) : "memory");
}

/* Main function with complex loop structure */
int main(int argc, char *argv[]) {
    int iterations = 1000;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 1000;
    }
    
    int param1 = (argc > 2) ? atoi(argv[2]) : 42;
    int param2 = (argc > 3) ? atoi(argv[3]) : 73;
    
    volatile int sink = 0;
    
    /* Outer loop to create register pressure */
    for (int outer = 0; outer < 10; outer++) {
        /* Inner hot loop with many basic blocks */
        for (int i = 0; i < iterations; i++) {
            /* --- Basic Block 1: Create many rematerialization candidates --- */
            
            /* Small integer constants (require multiple instructions on some arches) */
            const int const1 = 0x7FFFFFFF;  /* Large constant */
            const int const2 = 0x55555555;  /* Bit pattern constant */
            const long const3 = 0x123456789ABCDEF0L; /* 64-bit constant */
            
            /* Constants derived from function arguments (remat candidates) */
            int derived1 = param1 + 1;           /* param + constant */
            int derived2 = param1 << 2;          /* shift operation */
            int derived3 = param1 * 3;           /* multiplication */
            int derived4 = param2 - param1;      /* subtraction */
            long derived5 = (long)param1 * param2; /* widening */
            
            /* Symbol addresses (likely rematerialized) */
            int *addr1 = &global_array[i & 0xFF];
            int *addr2 = &global_array[(i + 1) & 0xFF];
            long *addr3 = (long *)&global_array[(i + 2) & 0xFF];
            
            /* Loop-variant values */
            int variant1 = i * 7;
            int variant2 = i + outer;
            int variant3 = (i << 3) | 0x1F;
            
            /* --- Basic Block 2: Conditional branch creating new BB --- */
            if (i & 1) {
                /* Use constants in computations */
                int temp1 = derived1 + const1;
                int temp2 = derived2 * const2;
                long temp3 = derived5 + const3;
                
                /* Pointer arithmetic with addresses */
                int *ptr1 = addr1 + (param1 >> 2);
                int *ptr2 = addr2 + (param2 >> 1);
                
                /* More computations mixing types */
                variant1 = temp1 ^ variant2;
                variant2 = (temp2 & 0xFF) + *ptr1;
                variant3 = (int)(temp3 >> 32) + *ptr2;
                
                /* Call to clobber registers */
                clobber_registers();
                
                /* Store to volatile to prevent elimination */
                sink = variant1 + variant2;
            } else {
                /* Alternative path with different computations */
                int temp4 = derived3 - const1;
                int temp5 = derived4 / 2;
                long temp6 = (long)derived3 * derived4;
                
                /* Different address computations */
                int offset = (param1 * i) & 0xF;
                int *ptr3 = addr1 + offset;
                int *ptr4 = addr2 - offset;
                
                /* More mixed-type operations */
                variant1 = temp4 | variant3;
                variant2 = temp5 ^ (int)(temp6 & 0xFFFFFFFF);
                variant3 = *ptr3 + *ptr4;
                
                /* Another register-clobbering call */
                clobber_registers();
                
                /* Use values to prevent elimination */
                use_value(&variant1);
                sink = variant2;
            }
            
            /* --- Basic Block 3: More computations after branch merge --- */
            
            /* Create more intermediate values */
            int intermediate1 = variant1 * derived1;
            int intermediate2 = variant2 + derived2;
            int intermediate3 = variant3 - derived3;
            int intermediate4 = intermediate1 ^ intermediate2;
            int intermediate5 = intermediate3 & intermediate4;
            int intermediate6 = intermediate5 | derived4;
            
            /* Pointer computations with different modes */
            char *char_ptr = (char *)addr3;
            short *short_ptr = (short *)&global_array[i & 0xFF];
            int *int_ptr = &global_array[(i + 3) & 0xFF];
            
            /* Mixed-size operations */
            long long_intermediate = (long)intermediate6 * const3;
            int truncated = (int)(long_intermediate >> 16);
            
            /* Another conditional creating another basic block */
            if (i & 2) {
                *char_ptr = (char)truncated;
                *short_ptr = (short)(truncated >> 8);
                *int_ptr = truncated;
                
                clobber_registers();
            } else {
                int_ptr[0] = truncated + 1;
                int_ptr[1] = truncated - 1;
                
                use_value(int_ptr);
            }
            
            /* --- Basic Block 4: Final computations with many live values --- */
            
            /* Use all intermediates simultaneously */
            int final1 = intermediate1 + intermediate2 + intermediate3;
            int final2 = intermediate4 - intermediate5 + intermediate6;
            int final3 = truncated * final1;
            int final4 = final2 ^ final3;
            
            /* Global variable access (forces address computation) */
            global_array[i & 0xFF] = final4 + global_base;
            
            /* Update static variable */
            static_counter += final1;
            
            /* Final clobber to increase register pressure */
            clobber_registers();
            
            /* Prevent dead code elimination */
            sink = final4;
        }
    }
    
    /* Use results to prevent optimization */
    printf("Result: sink=%d, static_counter=%ld\n", sink, static_counter);
    return sink != 0 ? 0 : 1;
}
