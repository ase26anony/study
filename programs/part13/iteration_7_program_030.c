/* early-remat-trigger.c
 * Compile with: gcc -O2 -fearly-remat -fdump-rtl-early-remat -fdump-rtl-all -fno-omit-frame-pointer -fno-schedule-insns -fno-schedule-insns2 --param=max-early-remat-iterations=5 -o early_remat_test early-remat-trigger.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Global variables to create rematerialization candidates */
volatile int global_base = 1000;
volatile long global_offset = 2000;
int global_array[256] = {0};
volatile int *global_ptr = &global_array[0];

/* Dummy function to clobber registers */
static void __attribute__((noinline)) clobber_registers(void) {
    __asm__ volatile ("" : : : "memory", "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
                       "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15");
}

/* Another noinline function to force spills */
static int __attribute__((noinline)) use_value(int x) {
    volatile int sink;
    sink = x;
    return sink;
}

/* Main function with complex loop structure */
int main(int argc, char *argv[]) {
    int iterations = (argc > 1) ? atoi(argv[1]) : 1000;
    int param1 = (argc > 2) ? atoi(argv[2]) : 42;
    volatile int result_sink = 0;
    
    /* Outer loop to create many virtual registers */
    for (int outer = 0; outer < 10; outer++) {
        /* Inner hot loop with high register pressure */
        for (int i = 0; i < iterations; i++) {
            /* Basic block 1: Create many rematerialization candidates */
            
            /* Small integer constants (require multiple instructions) */
            int const1 = 0x12345678;  /* Large constant */
            int const2 = -987654321;  /* Negative constant */
            long const3 = 0x7FFFFFFFFFFFFFFF;  /* Max positive 64-bit */
            
            /* Constants derived from function arguments */
            int derived1 = param1 + 1;          /* param + 1 */
            int derived2 = param1 << 2;         /* param << 2 */
            int derived3 = (param1 * 3) + 7;    /* Complex expression */
            long derived4 = (long)param1 * 1000;
            
            /* Symbol addresses that can be recomputed */
            int *addr1 = &global_array[i & 0xFF];
            int *addr2 = global_ptr + (i % 16);
            volatile int *addr3 = &global_base;
            
            /* Loop-variant values */
            int variant1 = i * 3;
            int variant2 = i + outer;
            long variant3 = (long)i * (long)outer;
            
            /* Mix different data types */
            short short_val = i & 0xFFFF;
            char char_val = i & 0xFF;
            float float_val = i * 1.5f;
            double double_val = i * 2.5;
            
            /* Conditional branch creating multiple basic blocks */
            if (i & 1) {
                /* Basic block 2: Use values in computations */
                
                /* Complex arithmetic creating many intermediate values */
                int temp1 = const1 + derived1;
                int temp2 = const2 - derived2;
                long temp3 = const3 / (derived4 + 1);
                int temp4 = variant1 * variant2;
                int temp5 = (temp1 + temp2) >> 1;
                long temp6 = temp3 + variant3;
                
                /* Pointer arithmetic with different modes */
                int *ptr1 = addr1 + temp1;
                int *ptr2 = addr2 + temp2;
                
                /* Use values to prevent elimination */
                result_sink += temp4;
                result_sink += (int)(temp6 & 0xFFFFFFFF);
                
                /* Function call clobbers registers */
                clobber_registers();
                
                /* More computations after clobber */
                int temp7 = temp5 * 2;
                long temp8 = temp6 * 3;
                int temp9 = *ptr1 + temp7;
                long temp10 = temp8 + (long)temp9;
                
                result_sink += temp7;
                result_sink += (int)temp10;
                
            } else {
                /* Basic block 3: Alternative computation path */
                
                /* Different set of computations */
                int alt1 = const1 ^ derived1;      /* XOR operation */
                int alt2 = const2 | derived3;      /* OR operation */
                long alt3 = const3 & derived4;     /* AND operation */
                int alt4 = variant1 ^ variant2;
                int alt5 = alt1 * alt2;
                long alt6 = alt3 + alt4;
                
                /* More pointer arithmetic */
                volatile int *ptr3 = addr3 + alt1;
                int *ptr4 = (int*)((uintptr_t)addr2 + alt2);
                
                /* Store to volatile to prevent optimization */
                result_sink += alt5;
                result_sink += (int)(alt6 & 0xFFFF);
                
                /* Another register-clobbering call */
                clobber_registers();
                
                /* Continue computations */
                int alt7 = alt5 << 3;
                long alt8 = alt6 >> 2;
                int alt9 = *ptr4 + alt7;
                long alt10 = alt8 * (long)alt9;
                
                result_sink += alt7;
                result_sink += (int)alt10;
            }
            
            /* Basic block 4: Common code after branches */
            
            /* Create more intermediate values */
            int common1 = derived1 + derived2;
            int common2 = derived3 * 2;
            long common3 = (long)derived4 * 3;
            int common4 = variant1 + variant2;
            
            /* Mix data types */
            int mixed1 = common1 + short_val;
            long mixed2 = common3 + char_val;
            int mixed3 = common2 + (int)float_val;
            long mixed4 = mixed2 + (long)double_val;
            
            /* Use in conditional */
            if (common4 > 100) {
                result_sink += mixed1;
                result_sink += (int)mixed4;
            } else {
                result_sink += mixed3;
                result_sink += (int)mixed2;
            }
            
            /* Final function call to increase register pressure */
            use_value(result_sink);
            
            /* Additional nested condition to create more basic blocks */
            switch (i % 4) {
                case 0: {
                    int case1 = const1 + i;
                    int case2 = derived1 * i;
                    result_sink += case1 + case2;
                    break;
                }
                case 1: {
                    long case3 = const3 - i;
                    int case4 = derived2 / (i + 1);
                    result_sink += (int)case3 + case4;
                    break;
                }
                case 2: {
                    int case5 = variant1 ^ variant2;
                    long case6 = variant3 & 0xFF;
                    result_sink += case5 + (int)case6;
                    break;
                }
                case 3: {
                    int case7 = *addr1;
                    long case8 = (long)*addr2;
                    result_sink += case7 + (int)case8;
                    break;
                }
            }
        }
    }
    
    printf("Result: %d\n", result_sink);
    return 0;
}
