/* early-remat-trigger.c
 * Designed to trigger early rematerialization with virtual register spills
 * Compile with: gcc -O2 -fearly-remat -fdump-rtl-early-remat -fdump-rtl-all -fno-omit-frame-pointer -fno-schedule-insns -fno-schedule-insns2 --param=max-early-remat-iterations=5 -o trigger trigger.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Global variables to create rematerialization candidates */
volatile int global_base = 42;
volatile long global_offset = 1000;
int global_array[256] = {0};
volatile int *global_ptr = &global_array[0];

/* Dummy function to clobber registers */
static void __attribute__((noinline)) clobber_registers(void) {
    __asm__ volatile ("" : : : "memory", "rax", "rbx", "rcx", "rdx", "rsi", "rdi", 
                      "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15");
}

/* Another dummy function to prevent optimization */
static void __attribute__((noinline)) use_value(volatile int *ptr) {
    __asm__ volatile ("" : : "r" (ptr) : "memory");
}

/* Main function with complex loop structure */
int main(int argc, char *argv[]) {
    volatile int loop_counter;
    int iterations = 1000;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 1000;
    }
    
    /* Outer loop to create pressure */
    for (loop_counter = 0; loop_counter < iterations; loop_counter++) {
        /* Multiple basic blocks created via if/else */
        if (loop_counter & 1) {
            /* Block A: Many distinct integer values */
            int val1 = global_base + 1;          /* Constant derived from global */
            int val2 = global_base << 2;         /* Shift operation */
            int val3 = val1 * 3;                 /* Multiplication */
            long val4 = global_offset + 0x7FFF;  /* Large constant */
            int *ptr1 = &global_array[val1 & 0xFF];  /* Address computation */
            int *ptr2 = global_ptr + (val2 & 0x3F);  /* Pointer arithmetic */
            
            /* Use values to create dependencies */
            volatile int sink1 = val1 + val3;
            volatile long sink2 = val4 - global_offset;
            
            /* Complex conditional */
            if (val3 > 100) {
                int val5 = val2 | 0xABCD;        /* Bitwise operation */
                int val6 = val5 ^ val1;          /* Another operation */
                volatile int sink3 = val6;
                
                /* More pointer computations */
                int *ptr3 = ptr1 + (val6 & 0xF);
                use_value(ptr3);
            } else {
                int val7 = val2 & 0x7777;        /* Different operation */
                long val8 = (long)val7 * 17;     /* Long multiplication */
                volatile long sink4 = val8;
                
                int *ptr4 = ptr2 - (val7 & 0x7);
                use_value(ptr4);
            }
            
            /* Call to clobber registers */
            clobber_registers();
            
            /* More computations after clobber */
            int val9 = sink1 + global_base;
            int val10 = val9 * 5;
            volatile int sink5 = val10;
            
        } else {
            /* Block B: Different set of values */
            int val11 = global_base - 1;         /* Another constant */
            int val12 = global_base >> 1;        /* Shift right */
            long val13 = global_offset * 2;      /* Long operation */
            int *ptr5 = &global_array[val11 & 0xFF];
            
            /* Nested conditionals */
            if (val12 < 50) {
                int val14 = val11 + 0x1234;
                int val15 = val14 % 17;          /* Modulo operation */
                volatile int sink6 = val15;
                
                int *ptr6 = ptr5 + (val15 & 0x1F);
                use_value(ptr6);
                
                /* Inner if for more blocks */
                if (val15 & 1) {
                    int val16 = val14 * val15;
                    volatile int sink7 = val16;
                }
            }
            
            clobber_registers();
            
            /* More values after clobber */
            long val17 = val13 + 0xFFFFFFFF;
            volatile long sink8 = val17;
            
            int *ptr7 = global_ptr + (val17 & 0xFF);
            use_value(ptr7);
        }
        
        /* Common block with more computations */
        int val18 = loop_counter * 7;           /* Loop-variant constant */
        int val19 = val18 + global_base;
        long val20 = (long)val19 * val19;
        
        /* Address computation with different mode */
        int *ptr8 = &global_array[(val19 + loop_counter) & 0xFF];
        volatile int *volatile ptr9 = ptr8;     /* Force memory access */
        
        /* Use all computed values to prevent elimination */
        volatile int final_sink = val19;
        volatile long final_sink_long = val20;
        
        /* Another register clobber */
        clobber_registers();
        
        /* Final store to global to create side effect */
        global_array[loop_counter & 0xFF] = final_sink;
    }
    
    /* Use results to prevent dead code elimination */
    volatile int result = 0;
    for (int i = 0; i < 256; i++) {
        result ^= global_array[i];
    }
    
    return result & 0xFF;
}
