/* early-remat-trigger.c
 * Designed to trigger early rematerialization with virtual register spills
 * Compile with: gcc -O2 -fearly-remat -fdump-rtl-early-remat -fdump-rtl-all -fno-omit-frame-pointer -fno-schedule-insns -fno-schedule-insns2 --param=max-early-remat-iterations=5
 */

#include <stdio.h>
#include <stdlib.h>

/* Global variables to create rematerialization candidates */
volatile int global_base = 100;
volatile long global_offset = 1000;
int global_array[256] = {0};

/* Dummy function to clobber registers */
static void __attribute__((noinline)) clobber_registers(void) {
    __asm__ volatile ("" : : : "memory", "rax", "rbx", "rcx", "rdx", "rsi", "rdi", 
                       "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15");
}

/* Another dummy function to prevent optimization */
static int __attribute__((noinline)) use_value(int x) {
    volatile int sink = x;
    return sink;
}

/* Main function with high register pressure loop */
int main(int argc, char *argv[]) {
    /* Use command line arguments to create variant values */
    int loop_count = (argc > 1) ? atoi(argv[1]) : 1000;
    int param_base = (argc > 2) ? atoi(argv[2]) : 42;
    
    /* Volatile sink to prevent elimination */
    volatile int result_sink = 0;
    
    /* Outer loop to create many intermediate values */
    for (int outer = 0; outer < 10; outer++) {
        /* Inner loop with high register pressure */
        for (int i = 0; i < loop_count; i++) {
            /* --- Create many rematerialization candidates --- */
            
            /* Small integer constants (require materialization) */
            int const1 = 0x12345678;      /* Large constant */
            int const2 = 0x87654321;      /* Another large constant */
            long const3 = 0xFFFFFFFFL;    /* Different mode (DImode) */
            
            /* Constants derived from function arguments */
            int derived1 = param_base + 1;        /* param + 1 */
            int derived2 = param_base << 2;       /* param << 2 */
            int derived3 = param_base * 3;        /* param * 3 */
            long derived4 = (long)param_base * 5; /* Different type */
            
            /* Symbol addresses (pointer constants) */
            int *addr1 = &global_array[i & 0xFF];
            int *addr2 = &global_array[(i + 1) & 0xFF];
            long *addr3 = (long *)&global_array[(i + 2) & 0xFF];
            
            /* Loop-variant constants */
            int loop_const1 = i * 4;              /* i << 2 */
            int loop_const2 = i + 0x1000;         /* i + constant */
            long loop_const3 = (long)i * 8;       /* Different mode */
            
            /* Global-derived constants */
            int global_derived1 = global_base + i;
            int global_derived2 = global_offset - i;
            
            /* --- Create complex control flow with multiple basic blocks --- */
            
            /* First basic block: compute some values */
            int temp1 = const1 + derived1;
            int temp2 = const2 + derived2;
            long temp3 = const3 + derived4;
            
            /* Force spill point with volatile */
            volatile int spill_point = temp1;
            
            /* Branch to create new basic block */
            if (i & 1) {
                /* Second basic block: more computations */
                int temp4 = temp1 * loop_const1;
                int temp5 = temp2 + global_derived1;
                long temp6 = temp3 - loop_const3;
                
                /* Use addresses */
                *addr1 = temp4;
                *addr2 = temp5;
                
                /* More computations */
                int temp7 = derived3 + (int)global_offset;
                int temp8 = loop_const2 * 2;
                
                /* Another branch */
                if (temp4 > temp5) {
                    /* Third basic block */
                    int temp9 = temp7 - temp8;
                    long temp10 = (long)temp9 * derived4;
                    
                    /* Store result */
                    global_array[(i + 3) & 0xFF] = temp9;
                    
                    /* More computations with different modes */
                    int temp11 = (int)(temp10 >> 16);
                    int temp12 = temp11 + const1;
                    
                    result_sink += temp12;
                } else {
                    /* Fourth basic block (else path) */
                    int temp13 = temp7 + temp8;
                    long temp14 = (long)temp13 / (derived4 | 1);
                    
                    /* Pointer arithmetic */
                    int *addr4 = addr1 + (temp13 & 0xF);
                    *addr4 = temp13;
                    
                    int temp15 = (int)temp14 + const2;
                    result_sink += temp15;
                }
            } else {
                /* Fifth basic block (outer else path) */
                int temp16 = derived1 * derived2;
                int temp17 = derived3 - loop_const1;
                
                /* Mix pointer and integer operations */
                long temp18 = (long)(*addr3) + temp16;
                int temp19 = (int)temp18 + temp17;
                
                /* Another nested branch */
                if (global_derived2 > 0) {
                    /* Sixth basic block */
                    int temp20 = temp19 * 3;
                    long temp21 = (long)temp20 + global_offset;
                    
                    /* Use different addressing mode */
                    global_array[(i + 4) & 0xFF] = (int)temp21;
                    
                    int temp22 = temp20 + global_base;
                    result_sink += temp22;
                } else {
                    /* Seventh basic block */
                    int temp23 = temp19 / 2;
                    long temp24 = (long)temp23 - global_offset;
                    
                    /* More pointer operations */
                    int *addr5 = &global_array[(i + 5) & 0xFF];
                    *addr5 = temp23;
                    
                    int temp25 = (int)temp24 + param_base;
                    result_sink += temp25;
                }
            }
            
            /* --- Call clobber function to force register spills --- */
            clobber_registers();
            
            /* --- More computations after clobber --- */
            int temp26 = const1 + i;
            int temp27 = derived1 - i;
            long temp28 = (long)const3 + i;
            
            /* Use all remaining variables to keep them live */
            int temp29 = temp26 + temp27 + (int)temp28;
            
            /* Final store to volatile to prevent elimination */
            volatile int final_sink = temp29;
            result_sink += final_sink;
            
            /* Another clobber to increase pressure */
            clobber_registers();
        }
    }
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", result_sink);
    
    return 0;
}
