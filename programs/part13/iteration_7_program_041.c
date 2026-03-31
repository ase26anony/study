/* early-remat-trigger.c
 * Designed to trigger early rematerialization with virtual register spills
 * Compile with: gcc -O2 -fearly-remat -fdump-rtl-early-remat -fdump-rtl-all -fno-omit-frame-pointer -fno-schedule-insns -fno-schedule-insns2 --param=max-early-remat-iterations=5
 */

#include <stdint.h>
#include <stdlib.h>

/* Global variables to create rematerialization candidates */
volatile int global_counter = 0;
int global_array[256] = {0};
const long global_const = 0x123456789ABCDEF0L;

/* Dummy function to clobber registers */
static void __attribute__((noinline)) clobber_registers(void) {
    __asm__ volatile ("" : : : "memory", "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
                       "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15");
}

/* Another noinline function to force register pressure */
static int __attribute__((noinline)) use_value(int x, long y, void *z) {
    __asm__ volatile ("# dummy asm" : : "r"(x), "r"(y), "r"(z));
    return x + (int)((intptr_t)z >> 4);
}

/* Main function with complex loop structure */
int main(int argc, char *argv[]) {
    /* Use arguments to create variant values */
    int base = argc > 1 ? atoi(argv[1]) : 1000;
    int iterations = argc > 2 ? atoi(argv[2]) : 100000;
    
    /* Volatile sink to prevent optimizations */
    volatile int sink = 0;
    
    /* Outer loop to create register pressure */
    for (int outer = 0; outer < 100; outer++) {
        /* Inner hot loop with many basic blocks */
        for (int i = 0; i < iterations; i++) {
            /* --- Basic Block 1: Create many remat candidates --- */
            
            /* Small integer constants (expensive to materialize) */
            int const1 = 0x7FFFFFFF;  /* Large immediate */
            int const2 = 0x80000000;  /* Another large immediate */
            long const3 = 0x1234567890ABCDEFL;  /* 64-bit constant */
            
            /* Symbol addresses as remat candidates */
            int *addr1 = &global_array[i & 0xFF];
            int *addr2 = &global_array[(i + 1) & 0xFF];
            long *addr3 = (long *)&global_array[(i + 2) & 0xFF];
            
            /* Constants derived from function arguments */
            int derived1 = base + 1;          /* param + 1 */
            int derived2 = base << 2;         /* param << 2 */
            int derived3 = (base * 3) / 2;    /* complex derivation */
            long derived4 = (long)base * 0x1000L;  /* 64-bit derived */
            
            /* Loop-variant values */
            int variant1 = i * 7;
            int variant2 = i + outer;
            long variant3 = (long)i * 0x1000000L;
            
            /* Pointer arithmetic candidates */
            int *ptr1 = global_array + (i & 0xF);
            int *ptr2 = ptr1 + 4;
            int *ptr3 = ptr2 - 2;
            
            /* --- Basic Block 2: Conditional branch --- */
            if (i & 1) {
                /* Use some values in this branch */
                int temp1 = const1 + variant1;
                int temp2 = derived1 * derived2;
                long temp3 = const3 + variant3;
                
                /* More remat candidates in branch */
                int branch_const = 0x55555555;
                int branch_derived = base + 0x1000;
                
                /* Complex computation chain */
                for (int j = 0; j < 3; j++) {
                    temp1 = (temp1 * 3) + branch_const;
                    temp2 = (temp2 >> 1) + branch_derived;
                    temp3 = (temp3 & 0xFFFFFFFF) | ((long)j << 32);
                    
                    /* Force register pressure with mixed operations */
                    int mixed1 = (int)(temp3 >> 16) + temp1;
                    int mixed2 = (int)((uintptr_t)ptr1) + temp2;
                    long mixed3 = (long)mixed1 * (long)mixed2;
                    
                    /* Store to prevent elimination */
                    global_array[(i + j) & 0xFF] = mixed1;
                    sink = mixed2;
                }
                
                /* Call to clobber registers */
                clobber_registers();
                
                /* Use addresses */
                *addr1 = temp1;
                *addr2 = temp2;
                *addr3 = temp3;
                
            } else {
                /* --- Basic Block 3: Alternative path --- */
                
                /* Different set of computations */
                int alt1 = const2 - variant2;
                int alt2 = derived3 / 2;
                long alt3 = derived4 | const3;
                
                /* More pointer arithmetic */
                int *alt_ptr = ptr3 + (i & 7);
                int alt4 = *alt_ptr + alt1;
                
                /* Nested condition for more basic blocks */
                if (i & 2) {
                    /* Sub-branch with its own values */
                    int sub1 = alt2 + 0x11111111;
                    long sub2 = alt3 ^ 0xAAAAAAAAAAAAAAAAL;
                    
                    /* Complex expression chain */
                    for (int k = 0; k < 2; k++) {
                        sub1 = (sub1 << 1) | (sub1 >> 31);
                        sub2 = (sub2 + (long)sub1) & 0xFFFFFFFFL;
                        
                        /* Mixed mode operations */
                        int mixed = (int)sub2 + sub1 + (int)((uintptr_t)ptr2);
                        long mixed_long = (long)mixed * (long)alt4;
                        
                        global_array[(i + k + 64) & 0xFF] = mixed;
                        sink = (int)mixed_long;
                    }
                } else {
                    /* Another sub-branch */
                    int sub3 = alt1 * 5;
                    long sub4 = alt3 << 4;
                    
                    /* Use function call that returns value */
                    sub3 = use_value(sub3, sub4, ptr3);
                    
                    /* More computations */
                    sub4 = sub4 + (long)sub3 + (long)const1;
                    sub3 = sub3 ^ 0xCCCCCCCC;
                    
                    global_array[(i + 128) & 0xFF] = sub3;
                    sink = (int)sub4;
                }
                
                /* Another register clobber */
                clobber_registers();
                
                /* Use the addresses */
                *addr1 = alt1;
                *addr2 = alt2;
                *addr3 = alt3;
            }
            
            /* --- Basic Block 4: Merge point --- */
            
            /* Create more live values after branches merge */
            int merge1 = variant1 + (i & 0xFF);
            int merge2 = derived1 - (outer & 0xF);
            long merge3 = variant3 + (long)merge1;
            
            /* Pointer chasing to create address computations */
            int *chase = global_array + (merge1 & 0x7F);
            int chase_val = *chase + merge2;
            
            /* Final computation mixing all types */
            long final1 = (long)merge1 * (long)merge2;
            long final2 = merge3 + final1;
            int final3 = (int)final2 + chase_val + (int)((uintptr_t)addr1);
            
            /* Force use of all created values to keep them live */
            sink = final3;
            global_counter += final3 & 1;
            
            /* One more clobber at loop end */
            if (i % 16 == 0) {
                clobber_registers();
            }
            
            /* Prevent loop unrolling */
            __asm__ volatile ("# loop barrier" : : : "memory");
        }
    }
    
    return sink + global_counter;
}
