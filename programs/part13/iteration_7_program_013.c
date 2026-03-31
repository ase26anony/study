/* early-remat-trigger.c
 * Compile with: gcc -O2 -fearly-remat -fdump-rtl-early-remat -fdump-rtl-all -fno-omit-frame-pointer -fno-schedule-insns -fno-schedule-insns2 --param=max-early-remat-iterations=5 -o early_remat_test early-remat-trigger.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Global variables to create rematerialization candidates */
volatile int global_base = 1000;
volatile long global_offset = 2000;
int global_array[256] = {0};
static const int const_table[8] = {1, 2, 4, 8, 16, 32, 64, 128};

/* Dummy function to clobber registers */
static void __attribute__((noinline)) clobber_registers(void) {
    __asm__ volatile ("" : : : "memory", "rax", "rbx", "rcx", "rdx", "rsi", "rdi", 
                      "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15");
}

/* Another dummy function to prevent optimization */
static void __attribute__((noinline)) use_value(volatile int *dest, int value) {
    *dest = value;
    __asm__ volatile ("" : : "r"(value) : "memory");
}

/* Main function with complex loop structure */
int main(int argc, char *argv[]) {
    volatile int sink = 0;
    int iterations = (argc > 1) ? atoi(argv[1]) : 1000;
    int param1 = (argc > 2) ? atoi(argv[2]) : 42;
    
    /* Loop with high register pressure */
    for (int outer = 0; outer < 10; outer++) {
        for (int i = 0; i < iterations; i++) {
            /* Basic block 1: Create many rematerialization candidates */
            int base = param1 + outer;  /* Derived from function argument */
            long offset = global_offset + i;  /* Global + loop variant */
            
            /* Various constants that are expensive to materialize */
            int const1 = 0x7FFFFFFF;  /* Large constant */
            int const2 = 0x55555555;  /* Another large constant */
            long const3 = 0x123456789ABCDEF0LL;  /* 64-bit constant */
            int const4 = const_table[i & 7];  /* Constant from table */
            
            /* Address calculations - good remat candidates */
            int *ptr1 = &global_array[(base + i) & 255];
            int *ptr2 = &global_array[(base * 2 + i) & 255];
            long *ptr3 = (long *)&global_array[(base * 3 + i) & 255];
            
            /* More derived values */
            int val1 = base << 2;      /* Shift operation */
            int val2 = base * 3 + 1;   /* Arithmetic */
            int val3 = (base ^ 0xFF) + const1;
            long val4 = offset * 5 - const3;
            
            /* Conditional branch creating multiple basic blocks */
            if (i & 1) {
                /* Basic block 2: Use values in different ways */
                int temp1 = val1 + const2;
                int temp2 = val2 * const4;
                long temp3 = val4 >> 3;
                
                /* More address calculations */
                int *ptr4 = ptr1 + (temp1 & 15);
                int *ptr5 = ptr2 + (temp2 & 7);
                
                /* Store to prevent elimination */
                *ptr4 = temp1;
                *ptr5 = temp2;
                
                /* Use volatile sink */
                use_value(&sink, temp1 + temp2);
                
                /* More computations */
                int val5 = (temp1 << 1) | (temp2 & 0xFF);
                int val6 = val5 * 7 + const1;
                long val7 = (long)val6 * 11 + const3;
                
                /* Another conditional */
                if (val5 > 1000) {
                    /* Basic block 3 */
                    *ptr3 = val7;
                    int val8 = val6 / 3 + const4;
                    use_value(&sink, val8);
                    
                    /* More remat candidates */
                    int const5 = 0xAAAAAAAA;
                    int val9 = val8 ^ const5;
                    long val10 = (long)val9 * 13;
                    
                    /* Function call clobbers registers */
                    clobber_registers();
                    
                    /* Use values after clobber */
                    *ptr1 = val9;
                    *ptr2 = (int)val10;
                } else {
                    /* Basic block 4 */
                    int val8 = val6 * 5 - const4;
                    long val9 = (long)val8 + 0xDEADBEEF;
                    
                    /* Different computation pattern */
                    int val10 = (val8 & 0xFFFF) + const2;
                    int val11 = val10 * 19;
                    
                    clobber_registers();
                    
                    *ptr1 = val10;
                    *ptr2 = val11;
                }
            } else {
                /* Basic block 5: Alternative path */
                int temp1 = val1 - const2;
                int temp2 = val3 / const4;
                long temp3 = val4 << 2;
                
                /* Different address mode */
                int *ptr4 = &global_array[(temp1 + i) & 255];
                int *ptr5 = &global_array[(temp2 * 2) & 255];
                
                /* More computations */
                int val5 = temp1 ^ temp2;
                int val6 = val5 + 0x88888888;
                long val7 = temp3 + 0x1111111111111111LL;
                
                /* Nested condition */
                if (val6 < 0) {
                    /* Basic block 6 */
                    *ptr4 = val6;
                    *ptr5 = (int)val7;
                    
                    int val8 = val6 * 3 + const1;
                    long val9 = val7 - 0x1000;
                    
                    clobber_registers();
                    
                    use_value(&sink, val8);
                    *ptr3 = val9;
                } else {
                    /* Basic block 7 */
                    int val8 = val6 >> 4;
                    long val9 = val7 * 3;
                    
                    /* More remat candidates */
                    int const5 = 0xCCCCCCCC;
                    int val10 = val8 & const5;
                    long val11 = val9 + 0x2000;
                    
                    clobber_registers();
                    
                    *ptr4 = val10;
                    *ptr5 = (int)val11;
                }
            }
            
            /* Basic block 8: Common tail code */
            int final1 = sink + i;
            int final2 = final1 * param1;  /* Use function argument again */
            long final3 = (long)final2 + global_base;  /* Use global */
            
            /* Force use of all types of values */
            use_value(&global_array[i & 255], final1);
            use_value(&global_array[(i + 1) & 255], final2);
            
            /* More register pressure */
            int extra1 = final1 ^ 0xF0F0F0F0;
            int extra2 = extra1 + const_table[(i >> 1) & 7];
            long extra3 = (long)extra2 * 23;
            int extra4 = (int)(extra3 >> 8);
            
            /* Final clobber */
            clobber_registers();
            
            /* Ensure values are used */
            sink = extra4 + (int)(final3 & 0xFFFF);
        }
    }
    
    printf("Result: %d\n", sink);
    return 0;
}
