/* early-remat-trigger.c
 * Designed to trigger early rematerialization with virtual register spills
 * Compile with: gcc -O2 -fearly-remat -fdump-rtl-early-remat -fdump-rtl-all -fno-omit-frame-pointer -fno-schedule-insns -fno-schedule-insns2 --param=max-early-remat-iterations=5 -o early_remat_test early-remat-trigger.c
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

/* Another noinline function to force register pressure */
static int __attribute__((noinline)) compute_offset(int x, int y) {
    __asm__ volatile ("" : "+r" (x), "+r" (y));
    return x * y;
}

/* Main function with complex loop structure */
int main(int argc, char *argv[]) {
    int loop_count = 1000;
    if (argc > 1) loop_count = atoi(argv[1]);
    
    int param1 = (argc > 2) ? atoi(argv[2]) : 42;
    long param2 = (long)param1 * 2;
    
    volatile int sink = 0;  /* Prevent optimizations */
    int *volatile ptr_sink = &sink;
    
    /* Outer loop to create many iterations */
    for (int outer = 0; outer < 10; outer++) {
        /* Inner hot loop with high register pressure */
        for (int i = 0; i < loop_count; i++) {
            /* --- Basic Block 1: Create many rematerialization candidates --- */
            
            /* Small integer constants (expensive to materialize) */
            int const1 = 0x12345678;  /* Large constant requiring multiple insns */
            int const2 = 0x87654321;
            long const3 = 0xFFFFFFFF12345678L;
            
            /* Constants derived from function arguments */
            int derived1 = param1 + 1;          /* param + 1 */
            int derived2 = param1 << 2;         /* param << 2 */
            long derived3 = param2 * 3;         /* param * 3 */
            int derived4 = compute_offset(param1, i);  /* Function call result */
            
            /* Symbol addresses that can be recomputed */
            int *addr1 = &global_array[i & 0xFF];
            int *addr2 = &global_array[(i + 1) & 0xFF];
            long *addr3 = (long *)&global_array[(i + 2) & 0xFF];
            
            /* Loop-variant constants */
            int loop_const1 = i * 7;
            int loop_const2 = i + 0x1000;
            long loop_const3 = (long)i * 0x1000000;
            
            /* Complex expressions mixing types */
            int expr1 = (const1 & 0xFF) + derived1;
            long expr2 = (const3 >> 16) + derived3;
            int expr3 = loop_const1 * derived2;
            long expr4 = loop_const3 / (param2 + 1);
            
            /* --- Basic Block 2: Conditional branch creating control flow --- */
            if (i & 1) {
                /* Use all the values in this branch */
                *addr1 = expr1 + expr3;
                *addr2 = expr1 - expr3;
                derived4 = compute_offset(expr1, expr3);
                
                /* More computations with different modes */
                long mixed1 = (long)expr1 * expr2;
                int mixed2 = (int)(expr4 & 0xFFFFFFFF);
                long mixed3 = mixed1 + mixed2 + const3;
                
                /* Store to volatile to prevent elimination */
                *ptr_sink = mixed2;
                sink = (int)mixed3;
                
                /* Call to clobber registers */
                clobber_registers();
            } else {
                /* Alternative computations in else branch */
                *addr3 = expr2;
                global_array[(i + 3) & 0xFF] = expr3;
                
                /* Different set of computations */
                int alt1 = const2 + loop_const2;
                long alt2 = (long)alt1 * param2;
                int alt3 = derived4 ^ const1;
                
                /* More register pressure */
                for (int j = 0; j < 3; j++) {
                    alt3 += global_array[(i + j) & 0xFF];
                    alt2 -= j * 0x100;
                }
                
                /* Use inline asm to prevent optimization */
                __asm__ volatile ("# dummy asm %0 %1 %2" 
                                 : "+r" (alt1), "+r" (alt2), "+r" (alt3));
                
                *ptr_sink = alt3;
                sink = (int)alt2;
                
                /* Another register-clobbering call */
                clobber_registers();
            }
            
            /* --- Basic Block 3: More computations after the branch --- */
            
            /* Recompute some addresses (remat candidates) */
            int *addr4 = &global_array[(i + 4) & 0xFF];
            int *addr5 = &global_array[(i + 5) & 0xFF];
            
            /* Use all remaining live values */
            int final1 = *addr4 + *addr5 + expr1;
            long final2 = (long)final1 * expr2;
            int final3 = final1 ^ derived1;
            
            /* Force use of different modes */
            if (final2 > 0) {
                final3 += (int)(final2 >> 32);
            }
            
            /* Final store and register clobber */
            global_array[(i + 6) & 0xFF] = final3;
            clobber_registers();
            
            /* Prevent loop unrolling */
            __asm__ volatile ("# loop barrier %0" : : "r" (i));
        }
        
        /* Modify parameters slightly each outer iteration */
        param1 += outer;
        param2 += outer * 2L;
    }
    
    /* Use results to prevent dead code elimination */
    int sum = 0;
    for (int i = 0; i < 256; i++) {
        sum += global_array[i];
    }
    
    printf("Result: %d\n", sum);
    return sum != 0;
}
