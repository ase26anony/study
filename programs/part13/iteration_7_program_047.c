/* early-remat-trigger.c
 * Designed to trigger early rematerialization with virtual register spills
 * Compile with: gcc -O2 -fearly-remat -fdump-rtl-early-remat -fdump-rtl-all -fno-omit-frame-pointer -fno-schedule-insns -fno-schedule-insns2 --param=max-early-remat-iterations=5 -o early-remat-test early-remat-trigger.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Global variables to create rematerialization candidates */
volatile int global_base = 1000;
volatile long global_offset = 2000;
int global_array[256] = {0};

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

/* Another noinline function to force spills */
static int __attribute__((noinline)) compute_offset(int x, int y) {
    volatile int result = x + y;
    return result;
}

int main(int argc, char *argv[]) {
    int loop_count = 1000;
    if (argc > 1) {
        loop_count = atoi(argv[1]);
        if (loop_count <= 0) loop_count = 1000;
    }
    
    int param_base = (argc > 2) ? atoi(argv[2]) : 42;
    volatile int sink = 0;  /* Prevent dead code elimination */
    
    /* Outer loop to create register pressure */
    for (int outer = 0; outer < 10; outer++) {
        /* Inner hot loop with many basic blocks */
        for (int i = 0; i < loop_count; i++) {
            /* --- Basic Block 1: Create many rematerialization candidates --- */
            
            /* Small integer constants (require multiple instructions) */
            int const1 = 0x12345678;      /* Large constant */
            int const2 = 0x87654321;      /* Another large constant */
            long const3 = 0xFFFFFFFFL;    /* Different mode (DImode) */
            
            /* Constants derived from function arguments */
            int derived1 = param_base + 1;
            int derived2 = param_base << 2;
            int derived3 = param_base * 3;
            long derived4 = (long)param_base * 1000L;
            
            /* Symbol addresses that can be recomputed */
            int *addr1 = &global_array[i % 256];
            int *addr2 = &global_array[(i + 1) % 256];
            int *addr3 = &global_array[(i + param_base) % 256];
            
            /* Loop-variant values */
            int var1 = i * 2;
            int var2 = i + outer;
            long var3 = (long)i * (long)outer;
            
            /* --- Basic Block 2: Conditional branch creates new basic block --- */
            if (i & 1) {
                /* Use the constants in computations */
                int temp1 = const1 + derived1;
                int temp2 = const2 + derived2;
                long temp3 = const3 + derived4;
                
                /* Pointer arithmetic with different modes */
                int *ptr1 = addr1 + (derived1 >> 2);
                int *ptr2 = addr2 + (derived2 >> 2);
                
                /* More computations to increase live ranges */
                int sum1 = temp1 + var1;
                int sum2 = temp2 + var2;
                long sum3 = temp3 + var3;
                
                /* Store to volatile to prevent elimination */
                sink = sum1 + sum2 + (int)sum3;
                
                /* Use the pointers */
                *ptr1 = sum1;
                *ptr2 = sum2;
                
                /* Call function that clobbers registers */
                clobber_registers();
                
                /* More computations after clobber */
                int revived1 = const1 + sink;
                int revived2 = derived3 + *ptr1;
                long revived3 = derived4 + (long)*ptr2;
                
                sink = revived1 + revived2 + (int)revived3;
            } else {
                /* Alternative path with different computations */
                
                /* Different set of constants */
                int alt_const1 = 0x55555555;
                int alt_const2 = 0xAAAAAAAA;
                long alt_const3 = 0xCCCCCCCCCCCCCCCCl;
                
                /* Different derived values */
                int alt_derived1 = param_base - 1;
                int alt_derived2 = param_base >> 1;
                long alt_derived3 = (long)param_base << 4;
                
                /* Complex expression chain */
                int chain1 = alt_const1 * alt_derived1;
                int chain2 = chain1 + var1;
                int chain3 = chain2 * alt_const2;
                int chain4 = chain3 + alt_derived2;
                long chain5 = alt_const3 + alt_derived3;
                long chain6 = chain5 * var3;
                
                /* Multiple uses of same values to extend live ranges */
                int use1 = chain1 + chain2;
                int use2 = chain3 + chain4;
                long use3 = chain5 + chain6;
                
                /* Function call that returns a value */
                int offset = compute_offset(use1, use2);
                
                /* More computations with the result */
                int final1 = use1 + offset;
                int final2 = use2 + offset;
                long final3 = use3 + offset;
                
                sink = final1 + final2 + (int)final3;
                
                /* Another clobber call */
                clobber_registers();
                
                /* Revive values after clobber */
                int revived_alt1 = alt_const1 + sink;
                int revived_alt2 = alt_derived1 + final1;
                long revived_alt3 = alt_derived3 + final3;
                
                sink = revived_alt1 + revived_alt2 + (int)revived_alt3;
            }
            
            /* --- Basic Block 3: More computations merging both paths --- */
            
            /* Create more intermediate values */
            int merge1 = const1 + derived1;
            int merge2 = (i & 2) ? merge1 * 2 : merge1 / 2;
            long merge3 = const3 + (long)derived1;
            
            /* Pointer arithmetic with different scales */
            char *byte_ptr = (char*)addr3;
            int *int_ptr = (int*)byte_ptr;
            long *long_ptr = (long*)byte_ptr;
            
            /* Mixed mode operations */
            int mixed1 = merge1 + (int)merge3;
            long mixed2 = merge3 + merge1;
            
            /* Final store to prevent elimination */
            global_array[i % 256] = mixed1 + sink + (int)mixed2;
            
            /* One more clobber to force spills */
            clobber_registers();
        }
    }
    
    /* Use results to prevent complete optimization */
    printf("Result: %d (sink=%d)\n", global_array[0], sink);
    return 0;
}
