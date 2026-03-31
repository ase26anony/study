/* early-remat-test.c
 * Designed to trigger early rematerialization with virtual register spills
 * Compile with: gcc -O2 -fearly-remat -fdump-rtl-early-remat -fdump-rtl-all -fno-omit-frame-pointer -fno-schedule-insns -fno-schedule-insns2 --param=max-early-remat-iterations=5 -o early-remat-test early-remat-test.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Global variables to create rematerialization candidates */
volatile int global_base = 1000;
volatile long global_offset = 2000;
int global_array[1000] = {0};

/* Dummy function to clobber registers */
static void __attribute__((noinline)) clobber_registers(void) {
    __asm__ volatile ("" : : : "memory", "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
                      "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15");
}

/* Another dummy function to prevent optimization */
static int __attribute__((noinline)) use_value(int val) {
    volatile int sink = val;
    return sink;
}

/* Main function with complex loop structure */
int main(int argc, char *argv[]) {
    int loop_iterations = 1000;
    if (argc > 1) {
        loop_iterations = atoi(argv[1]);
        if (loop_iterations < 100) loop_iterations = 100;
    }
    
    int param_base = (argc > 2) ? atoi(argv[2]) : 42;
    
    volatile int result_sink = 0;
    
    /* Outer loop to create register pressure */
    for (int outer = 0; outer < 10; outer++) {
        /* Inner loop with many short-lived values */
        for (int i = 0; i < loop_iterations; i++) {
            /* Basic block 1: Create many rematerialization candidates */
            
            /* Small integer constants (expensive to materialize) */
            int const1 = 0x7FFFFFFF;  /* Large constant requiring multiple insns */
            int const2 = 0x12345678;
            long const3 = 0x7FFFFFFFFFFFFFFFLL;
            
            /* Constants derived from function arguments */
            int derived1 = param_base + 1;
            int derived2 = param_base << 2;
            int derived3 = param_base * 3;
            long derived4 = (long)param_base * 1000L;
            
            /* Symbol addresses that can be recomputed */
            int *addr1 = &global_array[i % 1000];
            int *addr2 = &global_array[(i + 1) % 1000];
            int *addr3 = &global_array[(i + param_base) % 1000];
            
            /* Loop-variant values */
            int var1 = i * 2;
            int var2 = i + outer;
            long var3 = (long)i * 100L;
            int var4 = i ^ 0x55AA55AA;
            
            /* Global-derived values */
            int gval1 = global_base + i;
            long gval2 = global_offset + (long)i;
            int gval3 = global_base * i;
            
            /* Force register clobbering */
            clobber_registers();
            
            /* Basic block 2: Conditional branch creating control flow */
            if (i & 1) {
                /* Use all values in arithmetic operations */
                int calc1 = const1 + derived1 + var1;
                int calc2 = const2 * derived2 - var2;
                long calc3 = const3 / (derived4 + 1) + var3;
                int calc4 = gval1 ^ derived3 ^ var4;
                
                /* Pointer arithmetic with different modes */
                int *ptr_calc = addr1 + (derived1 >> 2);
                long offset_calc = (long)addr2 - (long)addr3 + gval2;
                
                /* More computations mixing types */
                int mix1 = calc1 + (int)(calc3 & 0xFFFFFFFF);
                long mix2 = (long)calc2 * (long)calc4;
                int mix3 = (int)(offset_calc >> 3) + gval3;
                
                /* Store to volatile to prevent elimination */
                result_sink += calc1 + calc2 + (int)calc3 + calc4 + mix1 + (int)mix2 + mix3;
                
                /* Use addresses */
                *addr1 = mix1;
                addr2[0] = mix2;
                
            } else {
                /* Alternative path with different computations */
                
                /* Different set of operations */
                int alt1 = const1 - derived1 * var1;
                int alt2 = const2 ^ derived2 | var2;
                long alt3 = const3 % (derived4 | 1) - var3;
                int alt4 = gval1 & derived3 | var4;
                
                /* More pointer arithmetic */
                int *alt_ptr = addr3 - (derived2 >> 3);
                long alt_offset = (long)addr1 ^ (long)addr2 ^ gval2;
                
                /* Mixed computations */
                int alt_mix1 = alt1 * (int)(alt3 & 0xFFFF);
                long alt_mix2 = (long)alt2 + (long)alt4 * 17L;
                int alt_mix3 = (int)(alt_offset << 2) - gval3;
                
                /* More register pressure */
                int tmp1 = alt_mix1 + alt_mix3;
                long tmp2 = alt_mix2 * 3L;
                int tmp3 = tmp1 ^ (int)tmp2;
                long tmp4 = (long)tmp3 * (long)alt_mix1;
                
                /* Store results */
                result_sink += alt1 + alt2 + (int)alt3 + alt4 + alt_mix1 + (int)alt_mix2 + alt_mix3 + tmp1 + (int)tmp2 + tmp3 + (int)tmp4;
                
                /* Use addresses differently */
                addr3[0] = alt_mix1;
                *alt_ptr = tmp3;
            }
            
            /* Basic block 3: More computations after branch */
            
            /* Create additional short-lived values */
            int post1 = result_sink & 0xFF;
            long post2 = (long)result_sink * 59L;
            int post3 = post1 + (i & 0xF);
            long post4 = post2 - (outer * 1000L);
            
            /* Complex expression with many operands */
            int final1 = post1 + post3 + (int)(post4 & 0xFFF);
            long final2 = post2 * post4 / (post1 + 1);
            
            /* Force another clobber */
            clobber_registers();
            
            /* Use the values to prevent elimination */
            result_sink ^= final1 + (int)final2;
            
            /* More pointer work */
            int idx = (i * 17 + outer * 13) % 1000;
            global_array[idx] = final1;
            
            /* Use function call that can't be inlined */
            use_value(final1);
        }
    }
    
    printf("Result: %d\n", result_sink);
    return 0;
}
