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
static volatile int sink;

/* Dummy function to clobber registers */
static void __attribute__((noinline)) clobber_registers(void) {
    __asm__ volatile ("" : : : "memory", "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
                       "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15");
}

/* Another noinline function to force register pressure */
static int __attribute__((noinline)) use_value(int x, long y, void *p) {
    __asm__ volatile ("" : "+r" (x), "+r" (y), "+r" (p));
    return x + (int)y + (long)p;
}

/* Main function with complex loop structure */
int main(int argc, char *argv[]) {
    int loop_count = (argc > 1) ? atoi(argv[1]) : 1000;
    int param1 = (argc > 2) ? atoi(argv[2]) : 42;
    long param2 = (argc > 3) ? atol(argv[3]) : 123456789L;
    
    /* Force these parameters to be in registers initially */
    volatile int force_param1 = param1;
    volatile long force_param2 = param2;
    param1 = force_param1;
    param2 = force_param2;
    
    /* Results accumulator */
    volatile long total = 0;
    
    /* Outer loop to create many iterations */
    for (int outer = 0; outer < 10; outer++) {
        /* Inner hot loop with high register pressure */
        for (int i = 0; i < loop_count; i++) {
            /* --- Create many rematerialization candidates --- */
            
            /* Small integer constants (require multiple instructions) */
            int const1 = 0x7FFFFFFF;  /* Large constant */
            int const2 = 0x80000000;  /* Another large constant */
            long const3 = 0x123456789ABCDEF0L; /* 64-bit constant */
            
            /* Constants derived from function arguments */
            int derived1 = param1 + 1;      /* param + 1 */
            int derived2 = param1 << 2;     /* param << 2 */
            long derived3 = param2 + 0x1000; /* param + offset */
            long derived4 = param2 * 3;     /* param * 3 */
            
            /* Symbol addresses (pointer arithmetic) */
            int *ptr1 = &global_array[i & 0xFF];
            int *ptr2 = &global_array[(i + 1) & 0xFF];
            int *ptr3 = &global_array[(i + param1) & 0xFF];
            
            /* More complex expressions with different modes */
            long mixed1 = (long)param1 * param2;
            long mixed2 = (long)derived1 * derived3;
            int mixed3 = derived1 + derived2;
            
            /* Global variable based expressions */
            int global_derived1 = global_base + i;
            long global_derived2 = global_offset * i;
            
            /* --- Multiple basic blocks to split live ranges --- */
            
            /* First basic block: use some values */
            if (i & 1) {
                /* Use integer constants */
                derived1 = const1 + const2;
                ptr1 = (int*)((long)ptr1 + const3);
            } else {
                /* Use derived values */
                derived2 = derived3 >> 2;
                ptr2 = (int*)((long)ptr2 + derived4);
            }
            
            /* Second basic block */
            if (i & 2) {
                /* More operations creating new values */
                mixed1 = mixed1 + global_derived2;
                mixed2 = mixed2 * 2;
            } else {
                mixed3 = mixed3 - global_derived1;
                global_derived2 = global_derived2 / 2;
            }
            
            /* Third basic block with switch-like structure */
            switch (i & 3) {
                case 0:
                    ptr3 = (int*)((long)ptr3 + mixed1);
                    break;
                case 1:
                    ptr3 = (int*)((long)ptr3 - mixed2);
                    break;
                case 2:
                    ptr3 = (int*)((long)ptr3 + mixed3);
                    break;
                default:
                    ptr3 = (int*)((long)ptr3 + global_derived1);
                    break;
            }
            
            /* --- Force register clobbering --- */
            clobber_registers();
            
            /* --- Use all values to keep them alive --- */
            
            /* Complex expression using many values */
            long complex_expr = (long)derived1 * derived2 + 
                               (long)derived3 / derived4 +
                               (long)mixed1 - mixed2 +
                               (long)mixed3 * 2 +
                               (long)global_derived1 + global_derived2;
            
            /* Use function call that needs many arguments */
            int func_result = use_value(derived1, derived3, ptr1);
            func_result += use_value(derived2, derived4, ptr2);
            func_result += use_value(mixed3, mixed1, ptr3);
            
            /* Store to volatile to prevent optimization */
            sink = const1 + const2 + (int)const3;
            sink = derived1 + derived2;
            sink = (int)derived3 + (int)derived4;
            sink = (int)mixed1 + (int)mixed2 + mixed3;
            sink = global_derived1 + (int)global_derived2;
            
            /* Accumulate results */
            total += complex_expr + func_result;
            
            /* More operations to extend live ranges */
            if (i & 4) {
                /* Create cross-basic-block dependencies */
                derived1 = derived1 + sink;
                derived3 = derived3 - sink;
            }
            
            /* Another clobber to force spills */
            clobber_registers();
            
            /* Final use of values */
            total += (long)ptr1 - (long)ptr2 + (long)ptr3;
        }
        
        /* Modify parameters slightly each outer iteration */
        param1 += outer;
        param2 -= outer;
    }
    
    printf("Result: %ld\n", total);
    return (int)(total & 0x7FFFFFFF);
}
