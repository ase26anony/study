/* early-remat-trigger.c
 * Designed to trigger early rematerialization with virtual register spills
 * Specifically targets lines 930-937 in early-remat.cc
 */

#include <stdint.h>
#include <stdlib.h>

/* Global variables to create rematerialization candidates */
volatile int global_counter = 0;
int global_array[256] = {0};
const long global_const = 0x7FFFFFFFFFFFFFFFL;

/* Dummy function to clobber registers */
static void __attribute__((noinline)) clobber_registers(void) {
    /* Inline asm to clobber many registers */
    __asm__ volatile (
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        : : : "memory", "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
              "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15"
    );
}

/* Another noinline function to force register pressure */
static int __attribute__((noinline)) use_value(int x, long y, void *z) {
    volatile int sink;
    sink = x + (int)((intptr_t)z >> 2);
    clobber_registers();
    return sink + (int)y;
}

int main(int argc, char *argv[]) {
    /* Use arguments to create variant values */
    int base = argc > 1 ? atoi(argv[1]) : 1000;
    int iterations = argc > 2 ? atoi(argv[2]) : 100000;
    
    volatile int result_sink = 0;
    
    /* Outer loop to create many register pressures */
    for (int outer = 0; outer < 1000; outer++) {
        /* Inner hot loop with many basic blocks */
        for (int i = 0; i < iterations; i++) {
            /* Create many rematerialization candidates */
            
            /* 1. Small integer constants (require multiple instructions) */
            const int small_const_1 = 0x12345678;  /* Large immediate */
            const long small_const_2 = 0x7FFFFFFFFFFFFFFFL;  /* Max signed long */
            const int small_const_3 = -0x76543210;  /* Negative large immediate */
            
            /* 2. Symbol addresses (can be recomputed) */
            int *addr1 = &global_array[i & 0xFF];
            int *addr2 = &global_array[(i + 1) & 0xFF];
            int *addr3 = &global_array[(i * 3) & 0xFF];
            
            /* 3. Constants derived from function arguments */
            int derived1 = base + 1;          /* param + 1 */
            int derived2 = base << 2;         /* param << 2 */
            int derived3 = (base * 3) + 7;    /* param * 3 + 7 */
            long derived4 = (long)base * 0x1000L;  /* Scaled constant */
            
            /* 4. Loop-variant values */
            int variant1 = i * 2;
            int variant2 = i + outer;
            long variant3 = (long)i * (long)outer;
            int variant4 = (i << 3) | 0xF;
            
            /* Mix different data types and operations */
            int temp1, temp2, temp3, temp4, temp5;
            long ltemp1, ltemp2;
            void *ptemp1, *ptemp2;
            
            /* Complex conditional branching to create multiple basic blocks */
            if (i & 1) {
                /* Branch 1: Use integer constants */
                temp1 = small_const_1 + derived1;
                temp2 = (int)small_const_2 + derived2;
                temp3 = small_const_3 * variant1;
                
                /* Pointer arithmetic with different modes */
                ptemp1 = (void *)((intptr_t)addr1 + derived3);
                ptemp2 = (void *)((intptr_t)addr2 + (variant2 << 2));
                
                /* Force use of long values */
                ltemp1 = derived4 + variant3;
                ltemp2 = global_const - (long)variant4;
                
                /* Store to prevent elimination */
                global_array[i & 0xFF] = temp1 + temp2;
                
            } else if (i & 2) {
                /* Branch 2: Different computation pattern */
                temp1 = derived1 - small_const_1;
                temp2 = derived2 ^ variant1;
                temp3 = (small_const_3 >> 4) + variant2;
                
                /* More pointer operations */
                ptemp1 = (void *)((intptr_t)addr3 - derived3);
                ptemp2 = (void *)((intptr_t)addr1 + (i * sizeof(int)));
                
                /* Long operations */
                ltemp1 = variant3 / (derived4 >> 12);
                ltemp2 = (long)temp1 * (long)temp2;
                
                global_array[(i + 1) & 0xFF] = temp3;
                
            } else {
                /* Branch 3: Yet another pattern */
                temp1 = small_const_1 | derived1;
                temp2 = small_const_2 & 0xFFFF;
                temp3 = variant1 ^ variant2;
                temp4 = derived3 + (i & 0xF);
                temp5 = (small_const_3 << 1) + outer;
                
                /* Complex pointer chain */
                ptemp1 = (void *)((intptr_t)addr2 + (temp1 * 4));
                ptemp2 = (void *)((intptr_t)ptemp1 + temp2);
                
                /* Mixed long/int operations */
                ltemp1 = (long)temp3 * (long)temp4;
                ltemp2 = (ltemp1 >> 16) + (long)temp5;
                
                global_array[(i + 2) & 0xFF] = temp4;
            }
            
            /* Common block after branches - use all computed values */
            int combined1 = temp1 + temp2 + temp3;
            long combined2 = ltemp1 + ltemp2;
            void *combined_ptr = (void *)((intptr_t)ptemp1 + (intptr_t)ptemp2);
            
            /* Force register clobbering with function call */
            int func_result = use_value(combined1, combined2, combined_ptr);
            
            /* More computations after clobber */
            int post1 = func_result + derived1;
            int post2 = (variant1 * post1) >> 4;
            long post3 = (long)post2 * derived4;
            
            /* Use volatile sink to prevent elimination */
            result_sink = post1 ^ post2 ^ (int)post3;
            
            /* Additional conditional to create more blocks */
            if (i % 3 == 0) {
                int extra1 = (small_const_1 >> 16) + post1;
                int extra2 = (derived2 & 0xFF) * post2;
                result_sink += extra1 * extra2;
                
                /* Another clobber */
                clobber_registers();
            }
            
            /* Final store with address computation */
            int store_idx = (i + outer) & 0xFF;
            int *store_addr = &global_array[store_idx];
            *store_addr = result_sink + global_counter++;
        }
    }
    
    return result_sink & 0xFF;
}
