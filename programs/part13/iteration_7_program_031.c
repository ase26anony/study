/* early-remat-test.c
 * Designed to trigger early rematerialization with virtual register spills
 * Compile with: gcc -O2 -fearly-remat -fdump-rtl-early-remat -fdump-rtl-all -fno-omit-frame-pointer -fno-schedule-insns -fno-schedule-insns2 -o early-remat-test early-remat-test.c
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Global variables to create rematerialization candidates */
volatile int global_counter = 0;
int global_array[256] = {0};
const long global_const = 0x123456789ABCDEF0LL;

/* Dummy function to clobber registers */
static void __attribute__((noinline)) clobber_registers(void) {
    /* Use inline asm to clobber many registers */
    __asm__ volatile (
        "# Clobber registers\n"
        :
        :
        : "memory", "rax", "rbx", "rcx", "rdx", "rsi", "rdi", 
          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15"
    );
}

/* Another noinline function to prevent optimization */
static int __attribute__((noinline)) compute_offset(int base, int idx) {
    return base + idx * 4;
}

/* Main function with complex loop structure */
int main(int argc, char *argv[]) {
    int iterations = 1000;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 1000;
    }
    
    int base_arg = (argc > 2) ? atoi(argv[2]) : 42;
    
    volatile int sink = 0;  /* Prevent dead code elimination */
    
    /* Outer loop to create register pressure */
    for (int outer = 0; outer < 10; outer++) {
        /* Inner loop with many intermediate values */
        for (int i = 0; i < iterations; i++) {
            /* Create many rematerialization candidates with different modes */
            
            /* SImode constants (32-bit) */
            int const_small = 0x7FFFFFFF;  /* Expensive to materialize */
            int const_neg = -0x80000000;
            int const_shifted = base_arg << 3;  /* Derived from argument */
            int const_add = base_arg + 0x1234;
            
            /* DImode constants (64-bit) */
            long const_large = 0x123456789ABCDEF0LL;
            long const_derived = (long)base_arg * 0x100000000LL;
            long const_addr_like = (long)&global_array[i & 255];
            
            /* Pointer arithmetic (Pmode) */
            int *ptr1 = &global_array[i & 255];
            int *ptr2 = ptr1 + (base_arg >> 2);
            int *ptr3 = &global_array[(i + base_arg) & 255];
            
            /* Complex expressions that are rematerialization candidates */
            int expr1 = (base_arg * 3) / 2;
            int expr2 = (base_arg << 4) | 0xF;
            int expr3 = ~base_arg;
            int expr4 = (base_arg ^ 0x55555555) + 0xAAAAAAAA;
            
            /* Use in conditional branches to create multiple basic blocks */
            if (i & 1) {
                /* Branch 1: Use some values */
                int temp1 = const_small + expr1;
                int temp2 = const_neg * expr2;
                long temp3 = const_large + (long)expr3;
                
                /* More computations */
                int *temp_ptr = ptr1 + (temp1 >> 2);
                int val1 = *temp_ptr;
                int val2 = global_array[expr4 & 255];
                
                sink += val1 + val2 + (int)temp3;
                
                /* Create more intermediate values */
                int chain1 = temp1 + temp2;
                int chain2 = chain1 * base_arg;
                long chain3 = (long)chain2 + const_derived;
                
                /* Use volatile to prevent optimization */
                global_counter += chain1;
                
            } else {
                /* Branch 2: Use different values */
                int temp4 = const_add - expr3;
                long temp5 = const_addr_like - (long)expr4;
                int temp6 = compute_offset(base_arg, i);
                
                /* Pointer arithmetic with different modes */
                int *temp_ptr2 = (int*)((uintptr_t)ptr2 + temp4);
                int val3 = *temp_ptr2;
                int val4 = global_array[temp6 & 255];
                
                sink += val3 + val4 + (int)(temp5 >> 32);
                
                /* Another chain of computations */
                int chain4 = expr4 - expr1;
                int chain5 = chain4 ^ const_shifted;
                long chain6 = (long)chain5 * const_large;
                
                global_counter -= chain4;
            }
            
            /* Third basic block (always executed) */
            {
                /* Mix all types and modes */
                int mixed1 = const_small & const_neg;
                long mixed2 = const_large | const_derived;
                int *mixed_ptr = ptr3 - (mixed1 >> 1);
                
                /* Complex expression spanning multiple instructions */
                int complex_expr = ((base_arg * 13) << 3) + 
                                  ((~base_arg) >> 2) - 
                                  (0x12345678 / (base_arg + 1));
                
                /* Use in address calculation */
                int idx = (complex_expr ^ mixed1) & 255;
                int val5 = global_array[idx];
                
                sink += val5 + (int)mixed2 + *mixed_ptr;
                
                /* Force register pressure with many live values */
                int live1 = const_small;
                int live2 = const_neg;
                int live3 = const_shifted;
                int live4 = const_add;
                long live5 = const_large;
                long live6 = const_derived;
                long live7 = const_addr_like;
                int live8 = expr1;
                int live9 = expr2;
                int live10 = expr3;
                int live11 = expr4;
                
                /* Use all live values to prevent elimination */
                int sum = live1 + live2 + live3 + live4 + 
                         (int)live5 + (int)live6 + (int)live7 +
                         live8 + live9 + live10 + live11;
                
                global_array[i & 255] = sum & 0xFF;
            }
            
            /* Call function that clobbers registers */
            clobber_registers();
            
            /* More computations after clobber */
            int revived1 = base_arg + i;
            long revived2 = (long)base_arg * i;
            int revived3 = global_array[(i + base_arg) & 255];
            
            sink += revived1 + (int)revived2 + revived3;
            
            /* Create spill candidates with different addressing modes */
            for (int j = 0; j < 4; j++) {
                int offset = base_arg + (i << j);
                int *addr = &global_array[offset & 255];
                int val = *addr + j;
                
                /* Complex addressing mode */
                int idx2 = (offset * 3) & 255;
                int val2 = global_array[idx2] + global_array[(idx2 + 1) & 255];
                
                sink += val + val2;
            }
        }
    }
    
    printf("Result: sink=%d, global_counter=%d\n", sink, global_counter);
    return 0;
}
