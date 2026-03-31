/* early-remat-trigger.c
 * Designed to trigger early rematerialization with virtual register spills
 * Compile with: gcc -O2 -fearly-remat -fdump-rtl-early-remat -fdump-rtl-all -fno-omit-frame-pointer -fno-schedule-insns -fno-schedule-insns2 -o early_remat_test early-remat-trigger.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Global variables to create rematerialization candidates */
volatile int global_counter = 0;
int global_array[256] = {0};
const long global_const = 0x123456789ABCDEF0LL;

/* Dummy function to clobber registers */
static void __attribute__((noinline)) clobber_registers(void) {
    /* Inline asm to clobber many registers */
    __asm__ volatile (
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        :
        :
        : "memory", "rax", "rbx", "rcx", "rdx", "rsi", "rdi", 
          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15"
    );
}

/* Another dummy function to prevent optimization */
static void __attribute__((noinline)) use_value(volatile int *dest, int value) {
    *dest = value;
    __asm__ volatile("" : : "r"(value) : "memory");
}

/* Main function with complex loop structure */
int main(int argc, char *argv[]) {
    int iterations = 1000;
    if (argc > 1) {
        iterations = atoi(argv[1]);
    }
    
    int param1 = (argc > 2) ? atoi(argv[2]) : 42;
    long param2 = (long)param1 * 3;
    
    volatile int sink = 0;
    volatile long sink_long = 0;
    
    /* Outer loop to create register pressure */
    for (int outer = 0; outer < 10; outer++) {
        /* Inner hot loop with many basic blocks */
        for (int i = 0; i < iterations; i++) {
            /* Basic Block 1: Create many rematerialization candidates */
            
            /* Small integer constants requiring multiple instructions */
            int const1 = 0x7FFFFFFF;  /* Large constant */
            int const2 = -0x80000000; /* Large negative constant */
            long const3 = 0x12345678; /* Different mode (DImode) */
            
            /* Constants derived from function arguments */
            int derived1 = param1 + 1;          /* param + 1 */
            int derived2 = param1 << 2;         /* param << 2 */
            long derived3 = param2 * 3;         /* Different mode */
            int derived4 = (param1 & 0xFF) | 0x100;
            
            /* Symbol addresses as rematerialization candidates */
            int *addr1 = &global_array[i & 0xFF];
            int *addr2 = &global_array[(i + 1) & 0xFF];
            long *addr3 = (long *)&global_array[(i + 2) & 0xFF];
            
            /* Loop-variant constants */
            int loop_const1 = i * 4;
            int loop_const2 = i + 0x1000;
            long loop_const3 = (long)i * 0x1000000;
            
            /* Complex expressions mixing different types */
            int expr1 = const1 + derived1;
            int expr2 = const2 - derived2;
            long expr3 = const3 + derived3;
            int expr4 = (derived1 << 3) | (derived2 >> 1);
            
            /* Use volatile operations to prevent elimination */
            sink = expr1;
            sink_long = expr3;
            
            /* Basic Block 2: Conditional branch creating new basic block */
            if (i & 1) {
                /* More computations in this branch */
                int branch1 = expr1 * 2;
                int branch2 = expr2 / 2;
                long branch3 = expr3 & 0xFFFFFFFF;
                
                /* Use addresses */
                *addr1 = branch1;
                *addr2 = branch2;
                
                /* More constants */
                int branch_const1 = 0x55555555;
                int branch_const2 = 0xAAAAAAAA;
                
                /* Complex expression with multiple uses */
                int branch_expr = (branch_const1 & branch1) | 
                                  (branch_const2 & branch2);
                
                sink = branch_expr;
                
                /* Function call clobbering registers */
                clobber_registers();
                
                /* Use the clobbered values again - forcing rematerialization */
                int reused1 = branch_expr + const1;
                int reused2 = branch_const1 - derived1;
                
                use_value(&sink, reused1 + reused2);
            } else {
                /* Alternative branch with different computations */
                int alt1 = expr4 ^ 0xFF00FF00;
                long alt2 = loop_const3 | global_const;
                int alt3 = derived4 * 7;
                
                /* Pointer arithmetic with different modes */
                int *alt_addr = addr1 + (alt3 & 0xF);
                *alt_addr = alt1;
                
                /* More register pressure */
                int alt4 = alt1 + loop_const1;
                int alt5 = alt3 - loop_const2;
                long alt6 = alt2 + param2;
                
                sink = alt4;
                sink_long = alt6;
                
                /* Another function call */
                clobber_registers();
                
                /* Force reuse of constants after clobber */
                int alt_reused1 = const2 + alt4;
                int alt_reused2 = derived2 - alt5;
                
                /* Use global address again */
                global_array[(i + 3) & 0xFF] = alt_reused1 + alt_reused2;
            }
            
            /* Basic Block 3: Common code after branches */
            
            /* More computations using previously defined values */
            int common1 = (expr1 + expr2) * 3;
            long common2 = expr3 >> 4;
            int common3 = derived1 & derived2;
            
            /* Use all address modes */
            *addr3 = (int)common2;
            int *addr4 = &global_array[(i + 4) & 0xFF];
            *addr4 = common1;
            
            /* Create more intermediate values */
            int tmp1 = common1 + common3;
            int tmp2 = tmp1 * 2;
            int tmp3 = tmp2 - 1;
            int tmp4 = tmp3 & 0xFFFF;
            int tmp5 = tmp4 | 0x10000;
            int tmp6 = tmp5 << 1;
            int tmp7 = tmp6 >> 2;
            int tmp8 = tmp7 + param1;
            int tmp9 = tmp8 * 3;
            int tmp10 = tmp9 / 2;
            
            /* Volatile store to prevent optimization */
            sink = tmp10;
            
            /* Another register-clobbering call */
            clobber_registers();
            
            /* Force rematerialization of constants after clobber */
            int final1 = const1 + tmp10;
            int final2 = derived1 - tmp10;
            long final3 = const3 + (long)tmp10;
            
            /* Use results */
            global_counter += final1;
            sink_long = final3;
            
            /* Final check with another conditional */
            if ((i & 0xF) == 0) {
                /* Even more computations in this rare path */
                int rare1 = final1 * final2;
                int rare2 = rare1 + 0x123456;
                int rare3 = rare2 ^ 0x789ABC;
                
                /* Use global address with offset */
                global_array[(i + 5) & 0xFF] = rare3;
                
                /* One more clobber */
                clobber_registers();
            }
        }
    }
    
    printf("Result: %d %ld\n", sink, sink_long);
    printf("Global counter: %d\n", global_counter);
    
    return 0;
}
