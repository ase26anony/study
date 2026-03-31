/* early-remat-test.c
 * Designed to trigger early rematerialization with virtual register spills
 * Compile with: gcc -O2 -fearly-remat -fdump-rtl-early-remat -fdump-rtl-all -fno-omit-frame-pointer -fno-schedule-insns -fno-schedule-insns2 --param=max-early-remat-iterations=5 -o early-remat-test early-remat-test.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Global variables to create rematerialization candidates */
volatile int global_base = 1000;
volatile long global_offset = 2000;
int global_array[256] = {0};

/* Dummy function to clobber registers */
static void __attribute__((noinline)) clobber_registers(void) {
    __asm__ volatile ("" : : : "memory", "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
                       "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15");
}

/* Another dummy function with arguments */
static int __attribute__((noinline)) dummy_compute(int a, int b) {
    __asm__ volatile ("" : : "r"(a), "r"(b) : "memory");
    return 0;
}

/* Main function with complex loop structure */
int main(int argc, char *argv[]) {
    int loop_count = 1000;
    if (argc > 1) loop_count = atoi(argv[1]);
    
    int param1 = (argc > 2) ? atoi(argv[2]) : 42;
    long param2 = (argc > 3) ? atol(argv[3]) : 123456789L;
    
    volatile int sink = 0;
    
    /* Outer loop to create pressure */
    for (int outer = 0; outer < 10; outer++) {
        /* Inner hot loop with many basic blocks */
        for (int i = 0; i < loop_count; i++) {
            /* --- Basic Block 1: Create many remat candidates --- */
            /* Small integer constants requiring multiple instructions */
            const int small_const1 = 0x7FFFFFFF;  /* Max positive int */
            const int small_const2 = 0x80000000;  /* Min negative int */
            const long large_const = 0x123456789ABCDEF0L;
            
            /* Constants derived from function arguments */
            int derived1 = param1 + 1;           /* param + 1 */
            int derived2 = param1 << 2;          /* param << 2 */
            long derived3 = param2 * 3;          /* param * 3 */
            int derived4 = (param1 & 0xFF) | 0x100;
            
            /* Symbol addresses that can be recomputed */
            int *addr1 = &global_array[i & 0xFF];
            int *addr2 = &global_array[(i + 1) & 0xFF];
            long *addr3 = (long *)&global_array[(i + 2) & 0xFF];
            
            /* Loop-variant constants */
            int loop_const1 = i * 4;
            int loop_const2 = i + 0x1000;
            long loop_const3 = (long)i * 0x1000000L;
            
            /* Global-derived constants */
            int global_derived1 = global_base + i;
            long global_derived2 = global_offset - i;
            
            /* --- Basic Block 2: Conditional branch creating new BB --- */
            if (i & 1) {
                /* Use constants in arithmetic */
                int temp1 = derived1 + small_const1;
                int temp2 = derived2 - small_const2;
                long temp3 = derived3 + large_const;
                
                /* Pointer arithmetic with different modes */
                int *ptr1 = addr1 + loop_const1;
                int *ptr2 = addr2 + (derived4 >> 2);
                
                /* Mix operations */
                long mixed1 = (long)temp1 * temp2;
                long mixed2 = temp3 + global_derived2;
                
                /* Store to prevent elimination */
                sink += *ptr1 + temp1;
                global_array[i & 0xFF] = temp2;
                
                /* Call to clobber registers */
                clobber_registers();
                
                /* More computations */
                int temp4 = global_derived1 * 2;
                long temp5 = mixed1 / (loop_const3 ? loop_const3 : 1);
                
                sink += temp4 + (int)temp5;
            } else {
                /* --- Basic Block 3: Alternative path --- */
                /* Different set of operations */
                int temp6 = derived1 * derived2;
                long temp7 = (long)derived3 >> 4;
                
                /* More address computations */
                int *ptr3 = &global_array[derived4 & 0xFF];
                long *ptr4 = (long *)&global_array[(i * 3) & 0xFF];
                
                /* Complex expression chain */
                int chain1 = small_const1 - param1;
                int chain2 = chain1 * loop_const2;
                long chain3 = (long)chain2 + global_derived2;
                
                /* Use in conditional */
                if (chain2 > 0) {
                    /* Nested basic block */
                    int nested1 = chain1 + 0xABCD;
                    long nested2 = chain3 * 0x1234L;
                    
                    sink += nested1 + (int)nested2;
                    *ptr3 = nested1;
                }
                
                /* Another function call */
                dummy_compute(temp6, (int)temp7);
                
                /* More register pressure */
                int temp8 = global_base - i;
                long temp9 = (long)temp8 * temp8;
                int temp10 = (temp8 & 0xF) | ((temp8 >> 4) & 0xF0);
                
                sink += temp10 + (int)(temp9 & 0xFF);
            }
            
            /* --- Basic Block 4: Merge point with more computations --- */
            /* Create more simultaneously live values */
            int live1 = (i * param1) & 0xFFF;
            int live2 = live1 + 0x100;
            int live3 = live2 * 2;
            int live4 = live3 - 0x200;
            long live5 = (long)live4 * live4;
            int live6 = (live1 + live2 + live3 + live4) >> 2;
            
            /* Address computation with different modes */
            char *byte_ptr = (char *)&global_array[0];
            byte_ptr += i * sizeof(int);
            
            /* Use all live values */
            sink += live1 + live2 + live3 + live4 + live6;
            sink += (int)(live5 & 0xFFFFFFFF);
            
            /* Final clobber to force spills */
            clobber_registers();
            
            /* Store to global with address computation */
            int store_idx = (i * 7) & 0xFF;
            global_array[store_idx] = sink & 0xFF;
        }
    }
    
    printf("Result: %d\n", sink);
    return sink & 1;
}
