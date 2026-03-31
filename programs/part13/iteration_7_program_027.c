/* early-remat-test.c
 * Designed to trigger early rematerialization with virtual register spills
 * Compile with: gcc -O2 -fearly-remat -fdump-rtl-early-remat -fdump-rtl-all -fno-omit-frame-pointer -fno-schedule-insns -fno-schedule-insns2 --param=max-early-remat-iterations=5 -o early-remat-test early-remat-test.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Global variables to create rematerialization candidates */
volatile int global_base = 100;
volatile long global_offset = 200;
int global_array[256] = {0};
volatile int *global_ptr = &global_array[0];

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
    volatile int result;
    result = x * y + 12345;  /* Constant that needs rematerialization */
    return result;
}

int main(int argc, char *argv[]) {
    int loop_iterations = 1000;
    int param1 = argc > 1 ? atoi(argv[1]) : 42;
    int param2 = argc > 2 ? atoi(argv[2]) : 73;
    
    volatile int sink = 0;  /* Prevent dead code elimination */
    
    /* Outer loop to create register pressure */
    for (int outer = 0; outer < 10; outer++) {
        /* Inner hot loop with many basic blocks */
        for (int i = 0; i < loop_iterations; i++) {
            /* --- Basic Block 1: Create many remat candidates --- */
            /* Small integer constants requiring multiple instructions */
            const int const1 = 0x12345678;  /* Large constant */
            const long const2 = 0x9876543210ABCDEFL;  /* 64-bit constant */
            const int const3 = -1;  /* All-ones pattern */
            
            /* Symbol addresses that can be recomputed */
            int *addr1 = &global_array[i & 0xFF];
            long *addr2 = (long*)&global_array[(i + 1) & 0xFF];
            
            /* Constants derived from function arguments */
            int derived1 = param1 + const1;  /* param + large constant */
            int derived2 = param2 << 2;      /* param << 2 */
            int derived3 = (param1 * param2) + 0xABCD;
            
            /* Loop-variant values */
            int variant1 = i * 7;
            int variant2 = i + 0x1000;
            long variant3 = (long)i * 0x1000000;
            
            /* --- Basic Block 2: Conditional branch --- */
            if (i & 1) {
                /* Use all the values in arithmetic */
                int temp1 = derived1 * variant1;
                int temp2 = derived2 + variant2;
                long temp3 = variant3 + const2;
                
                /* More computations creating register pressure */
                int temp4 = temp1 ^ temp2;
                long temp5 = temp3 >> 4;
                int temp6 = temp4 & 0xFF;
                
                /* Store to volatile to prevent elimination */
                sink = temp6;
                
                /* Pointer arithmetic with different modes */
                int *ptr_calc = addr1 + (temp6 >> 2);
                *ptr_calc = temp1;
                
                /* Function call clobbers registers */
                clobber_registers();
                
                /* More computations after clobber */
                int temp7 = compute_offset(temp1, temp2);
                int temp8 = temp7 + global_base;
                
                sink = temp8;
            } else {
                /* Alternative path with different computations */
                int temp1 = derived3 - variant1;
                long temp2 = (long)derived2 * variant3;
                int temp3 = const3 | variant2;
                
                /* Different data types mixed */
                int temp4 = (int)(temp2 >> 32);
                long temp5 = (long)temp1 * temp3;
                int temp6 = temp4 ^ temp3;
                
                sink = temp6;
                
                /* More pointer arithmetic */
                long *ptr_calc = addr2 + (temp6 & 0x3F);
                *ptr_calc = temp5;
                
                /* Clobber registers */
                clobber_registers();
                
                /* Recomputation of constants after clobber */
                int temp7 = param1 + 0x12345678;  /* Should be rematerialized */
                int temp8 = param2 << 2;          /* Should be rematerialized */
                int temp9 = temp7 * temp8 + global_offset;
                
                sink = temp9;
                
                /* Complex expression with many intermediates */
                int temp10 = (temp9 * 3) / 2;
                int temp11 = (temp10 + 0x5555) & 0xAAAA;
                long temp12 = (long)temp11 * 0x11111111;
                
                /* Use global pointer with offset */
                int offset = (i * 13) & 0xFF;
                global_ptr[offset] = (int)(temp12 >> 32);
            }
            
            /* --- Basic Block 3: More computations --- */
            /* Create more short-lived values */
            int final1 = sink + i;
            int final2 = final1 * 3;
            long final3 = (long)final2 * 0x10001;
            int final4 = (int)(final3 & 0xFFFFFFFF);
            
            /* Another conditional */
            if (i & 2) {
                int final5 = final4 ^ 0xAAAAAAAA;
                long final6 = (long)final5 | 0x5555555555555555L;
                sink = (int)final6;
            } else {
                int final5 = final4 & 0x55555555;
                long final6 = (long)final5 + const2;
                sink = (int)(final6 >> 16);
            }
            
            /* Final function call to increase register pressure */
            clobber_registers();
            
            /* Use global variable to prevent optimization */
            global_base += (sink & 1);
        }
    }
    
    /* Use results to prevent elimination of entire computation */
    printf("Result: %d %d\n", sink, global_base);
    return sink != 0 ? 0 : 1;
}
