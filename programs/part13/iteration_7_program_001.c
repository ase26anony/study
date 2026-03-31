/* early-remat-trigger.c
 * Compile with: gcc -O2 -fearly-remat -fdump-rtl-early-remat -fdump-rtl-all -fno-omit-frame-pointer -fno-schedule-insns -fno-schedule-insns2 -o early_remat_test early-remat-trigger.c
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Global variables to create rematerialization candidates */
volatile int global_base = 1000;
volatile long global_offset = 2000;
int global_array[1024] = {0};
const int const_array[8] = {1, 2, 3, 4, 5, 6, 7, 8};

/* Function to clobber registers - prevents optimization */
static void __attribute__((noinline)) clobber_registers(void) {
    __asm__ volatile ("" : : : "memory", "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
                      "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15");
}

/* Another noinline function to force register pressure */
static int __attribute__((noinline)) dummy_compute(int x, int y) {
    __asm__ volatile ("# dummy compute" : "+r" (x), "+r" (y));
    return x ^ y;
}

/* Main function with complex loop structure */
int main(int argc, char *argv[]) {
    /* Use arguments to create variant values */
    int loop_count = (argc > 1) ? atoi(argv[1]) : 1000;
    int seed = (argc > 2) ? atoi(argv[2]) : 42;
    
    /* Volatile sink to prevent elimination */
    volatile int sink = 0;
    
    /* Outer loop to create many iterations */
    for (int outer = 0; outer < 10; outer++) {
        /* Inner hot loop with high register pressure */
        for (int i = 0; i < loop_count; i++) {
            /* --- Create many rematerialization candidates --- */
            
            /* Small integer constants (require multiple instructions) */
            int const1 = 0x12345678;  /* Large constant */
            int const2 = 0x87654321;  /* Another large constant */
            long const3 = 0xFFFFFFFF00000000UL;  /* 64-bit constant */
            
            /* Constants derived from function arguments */
            int arg_derived1 = seed + 1;      /* param + 1 */
            int arg_derived2 = seed << 2;     /* param << 2 */
            int arg_derived3 = seed * 3;      /* param * 3 */
            
            /* Symbol addresses that can be recomputed */
            int *addr1 = &global_array[i % 1024];
            const int *addr2 = &const_array[i % 8];
            int *addr3 = &global_array[(i + seed) % 1024];
            
            /* Complex expressions mixing types */
            long mixed1 = (long)global_base + (long)arg_derived1;
            int mixed2 = global_base - arg_derived2;
            long mixed3 = (long)global_offset * (long)arg_derived3;
            
            /* More intermediate values with different modes */
            uintptr_t ptr_val1 = (uintptr_t)addr1;
            uintptr_t ptr_val2 = (uintptr_t)addr2;
            intptr_t ptr_diff = ptr_val1 - ptr_val2;
            
            /* --- Multiple basic blocks with conditional branches --- */
            
            if (i & 1) {
                /* Branch 1: Use some values */
                int temp1 = const1 + arg_derived1;
                long temp2 = mixed1 + const3;
                int *temp_addr = addr1 + (temp1 % 16);
                
                /* More computations */
                int result1 = dummy_compute(temp1, mixed2);
                long result2 = temp2 + ptr_diff;
                
                /* Store to volatile to prevent elimination */
                sink += result1;
                sink ^= (int)result2;
                
                /* Use the address */
                *temp_addr = result1;
                
            } else {
                /* Branch 2: Use different values */
                int temp3 = const2 + arg_derived2;
                long temp4 = mixed3 - const3;
                const int *temp_addr2 = addr2 + (temp3 % 4);
                
                /* Different computations */
                int result3 = dummy_compute(temp3, arg_derived3);
                long result4 = temp4 - ptr_diff;
                
                /* More operations to increase live ranges */
                int complex1 = result3 * 7;
                long complex2 = result4 / 3;
                int complex3 = complex1 ^ (int)complex2;
                
                /* Store results */
                sink += complex3;
                sink ^= result3;
                
                /* Read from constant address */
                int val = *temp_addr2;
                sink += val;
            }
            
            /* Third basic block (always executed) */
            if (i % 3 == 0) {
                /* More computations with remaining values */
                int combined = arg_derived1 + arg_derived2 + arg_derived3;
                long big_combined = mixed1 + mixed2 + mixed3;
                
                /* Pointer arithmetic */
                int *new_addr = addr3 + (combined % 32);
                *new_addr = combined ^ (int)big_combined;
                
                sink += *new_addr;
            }
            
            /* Fourth basic block with switch-like structure */
            switch (i % 4) {
                case 0:
                    sink += const1 + ptr_diff;
                    break;
                case 1:
                    sink += const2 - ptr_diff;
                    break;
                case 2:
                    sink += (int)mixed1;
                    break;
                case 3:
                    sink += (int)mixed3;
                    break;
            }
            
            /* Call function that clobbers registers */
            clobber_registers();
            
            /* Force reuse of computed values after clobber */
            if (i % 5 == 0) {
                /* These values need to be rematerialized after clobber */
                int reused1 = arg_derived1 * 2;  /* Should be remat candidate */
                long reused2 = mixed1 * 3;       /* Another candidate */
                int reused3 = seed + outer;      /* Function arg + loop var */
                
                /* Use them in computation */
                int final = reused1 + (int)reused2 + reused3;
                sink += final;
                
                /* More address computations */
                int *final_addr = &global_array[final % 1024];
                *final_addr = final;
            }
            
            /* Additional loop to increase pressure */
            for (int j = 0; j < 3; j++) {
                /* Create more short-lived values */
                int inner_val = (i * j) + seed;
                long inner_long = (long)inner_val * global_offset;
                int *inner_addr = &global_array[(inner_val + j) % 1024];
                
                /* Use them */
                *inner_addr = inner_val ^ (int)inner_long;
                sink += *inner_addr;
                
                /* Small conditional */
                if (j == 1) {
                    clobber_registers();
                }
            }
        }
    }
    
    printf("Result: %d\n", sink);
    return sink != 0;
}
