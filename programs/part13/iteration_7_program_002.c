/* early-remat-trigger.c
 * Compile with: gcc -O2 -fearly-remat -fdump-rtl-early-remat -fdump-rtl-all -fno-omit-frame-pointer -fno-schedule-insns -fno-schedule-insns2 --param=max-early-remat-iterations=5 -o early_remat_test early-remat-trigger.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Global variables to create rematerialization candidates */
volatile int global_base = 1000;
volatile long global_array[256] = {0};
volatile char global_flags[1024] = {0};

/* Dummy function to clobber registers */
static void __attribute__((noinline)) clobber_registers(void) {
    __asm__ volatile ("" : : : "memory", "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
                       "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15");
}

/* Another dummy function with arguments */
static int __attribute__((noinline)) dummy_compute(int a, long b) {
    __asm__ volatile ("" : "+r"(a), "+r"(b));
    return a + (int)b;
}

/* Main function with complex loop structure */
int main(int argc, char *argv[]) {
    int loop_count = (argc > 1) ? atoi(argv[1]) : 1000;
    int param1 = (argc > 2) ? atoi(argv[2]) : 42;
    volatile int sink = 0;
    
    /* Derived constants that are rematerialization candidates */
    const int const_derived1 = param1 + 1;      /* Candidate: param + 1 */
    const int const_derived2 = param1 << 2;     /* Candidate: param << 2 */
    const long const_derived3 = param1 * 3L;    /* Candidate: param * 3 */
    const intptr_t addr_offset = (intptr_t)&global_array[0] + 0x1000;
    
    /* Outer loop to create pressure */
    for (int outer = 0; outer < 10; outer++) {
        /* Inner hot loop with many basic blocks */
        for (int i = 0; i < loop_count; i++) {
            /* --- Basic Block 1: Create many intermediate values --- */
            int val1 = global_base + i;          /* Volatile load + computation */
            long val2 = const_derived1 * i;      /* Constant derived from param */
            int val3 = const_derived2 | i;       /* Another constant */
            long val4 = val1 + const_derived3;   /* Mix of values */
            intptr_t addr1 = addr_offset + i * sizeof(long); /* Address computation */
            int val5 = (i & 0xFF) + param1;      /* Loop variant + param */
            long val6 = val2 << 3;               /* Shift operation */
            int val7 = ~val3;                    /* Bitwise operation */
            
            /* Force register pressure with more values */
            int val8 = val1 * val5;
            long val9 = val4 - val6;
            int val10 = val7 ^ val8;
            intptr_t addr2 = (intptr_t)&global_flags[i % 1024];
            
            /* --- Basic Block 2: Conditional branch creates new BB --- */
            if (i & 1) {
                /* Even more values in this branch */
                int val11 = val8 + const_derived1;
                long val12 = val9 >> 2;
                int val13 = val10 * 3;
                int val14 = val11 | const_derived2;
                
                /* Use all values to prevent elimination */
                sink += val11 + val12 + val13 + val14;
                
                /* Address computation candidate */
                intptr_t addr3 = addr1 + (val11 * sizeof(long));
                sink += *(volatile int*)(addr3 & ~0x3); /* Mask to avoid segfault */
            } else {
                /* Different computations in else branch */
                int val15 = val8 - const_derived1;
                long val16 = val9 << 1;
                int val17 = val10 / 2;
                int val18 = val15 & const_derived2;
                
                sink += val15 + val16 + val17 + val18;
                
                /* Another address computation */
                intptr_t addr4 = addr2 + (val15 % 256);
                sink += *(volatile char*)addr4;
            }
            
            /* --- Basic Block 3: Another conditional --- */
            if (i % 3 == 0) {
                /* More intermediate values */
                int val19 = sink & 0xFF;
                long val20 = val19 * const_derived3;
                int val21 = val20 >> 4;
                int val22 = val21 + param1;
                
                /* Function call clobbers registers */
                clobber_registers();
                
                sink += val19 + val20 + val21 + val22;
            } else if (i % 3 == 1) {
                /* Alternative path with different computations */
                int val23 = sink | 0xAA;
                long val24 = val23 + const_derived3;
                int val25 = (int)val24 * 7;
                int val26 = val25 ^ param1;
                
                /* Another function call */
                sink += dummy_compute(val23, val24);
                
                sink += val25 + val26;
            } else {
                /* Third path */
                int val27 = sink ^ 0x55;
                long val28 = val27 - const_derived3;
                int val29 = (int)val28 & 0xFFF;
                int val30 = val29 << 1;
                
                sink += val27 + val28 + val29 + val30;
            }
            
            /* --- Basic Block 4: More computations before loop end --- */
            int val31 = (sink & 1) ? const_derived1 : const_derived2;
            long val32 = val31 * i;
            int val33 = val32 + param1;
            intptr_t addr5 = (intptr_t)&global_array[i % 256];
            
            /* Mix pointer and integer operations */
            sink += val31 + val32 + val33 + (int)(addr5 & 0xFF);
            
            /* Final clobber to force spills */
            clobber_registers();
            
            /* Store to global to prevent elimination */
            if (i % 128 == 0) {
                global_array[i % 256] = sink;
            }
        }
    }
    
    /* Use sink to prevent dead code elimination */
    printf("Result: %d\n", sink);
    return sink != 0;
}
