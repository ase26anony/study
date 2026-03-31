/* early-remat-trigger.c
 * Designed to trigger early rematerialization with virtual register spills
 * Compile with: gcc -O2 -fearly-remat -fdump-rtl-early-remat -fdump-rtl-all -fno-omit-frame-pointer -fno-schedule-insns -fno-schedule-insns2 --param=max-early-remat-iterations=5 -o trigger early-remat-trigger.c
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

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

/* Another dummy function with arguments */
static int __attribute__((noinline)) dummy_use(int a, long b, void *c) {
    __asm__ volatile ("" : : "r"(a), "r"(b), "r"(c) : "memory");
    return 0;
}

/* Main function with complex loop structure */
int main(int argc, char *argv[]) {
    /* Use arguments to create variant values */
    int loop_count = (argc > 1) ? atoi(argv[1]) : 1000;
    int param_base = (argc > 2) ? atoi(argv[2]) : 42;
    
    /* Volatile sink to prevent optimizations */
    volatile int sink = 0;
    
    /* Outer loop to create pressure */
    for (int outer = 0; outer < 10; outer++) {
        /* Inner hot loop with many basic blocks */
        for (int i = 0; i < loop_count; i++) {
            /* --- Basic Block 1: Create many remat candidates --- */
            
            /* Small integer constants (require multiple instructions) */
            int const1 = 0x7FFFFFFF;  /* Large constant */
            int const2 = 0x80000000;  /* Another large constant */
            long const3 = 0x123456789ABCDEF0LL;  /* 64-bit constant */
            
            /* Constants derived from function arguments */
            int derived1 = param_base + 1;      /* param + 1 */
            int derived2 = param_base << 2;     /* param << 2 */
            int derived3 = param_base * 3;      /* param * 3 */
            long derived4 = (long)param_base * 5; /* Different type */
            
            /* Symbol addresses (good remat candidates) */
            int *addr1 = &global_array[i & 0xFF];
            int *addr2 = global_ptr + (i & 0x7F);
            void *addr3 = (void*)((uintptr_t)&global_base + (i & 0x3F));
            
            /* Complex expressions with different modes */
            int expr1 = (global_base + i) * 2;
            long expr2 = (global_offset - i) / 3;
            int expr3 = (param_base ^ i) | 0xFF00;
            
            /* --- Basic Block 2: Conditional branch --- */
            if (i & 1) {
                /* More computations in this branch */
                int branch1 = derived1 * const1;
                long branch2 = derived4 + const3;
                int *branch_addr = addr1 + (param_base & 0xF);
                
                /* Use in arithmetic */
                expr1 = branch1 + (expr2 & 0xFFFF);
                expr3 = (int)((uintptr_t)branch_addr) ^ expr3;
                
                /* Function call clobbers registers */
                clobber_registers();
                
                /* More computations after call */
                derived2 = branch1 >> 4;
                derived3 = expr1 * 3;
            } else {
                /* Alternative computations */
                int branch3 = const2 - derived2;
                long branch4 = const3 ^ derived4;
                void *branch_addr2 = (void*)((uintptr_t)addr3 + 8);
                
                expr2 = branch4 / 7;
                int expr4 = branch3 & 0x00FF00FF;
                
                /* Another function call with arguments */
                dummy_use(branch3, branch4, branch_addr2);
                
                /* More computations */
                derived1 = expr4 + param_base;
                expr3 = (expr3 << 1) | 1;
            }
            
            /* --- Basic Block 3: More operations --- */
            
            /* Create pointer arithmetic with different scales */
            int idx1 = (i * 4) & 0x3FF;
            int idx2 = (i * 8) & 0x1FF;
            long idx3 = (i * 16) & 0xFF;
            
            /* More rematerializable addresses */
            int *ptr1 = &global_array[idx1 >> 2];
            int *ptr2 = global_ptr + (idx2 >> 3);
            char *ptr3 = (char*)&global_base + idx3;
            
            /* Complex expressions using all the values */
            int result1 = expr1 + derived1 + const1;
            long result2 = expr2 + derived4 + (const3 & 0xFFFFFFFF);
            int result3 = expr3 ^ derived2 ^ derived3;
            
            /* Mix pointer and integer operations */
            uintptr_t ptr_val = (uintptr_t)ptr1 ^ (uintptr_t)ptr2;
            int result4 = (int)ptr_val + (int)((uintptr_t)ptr3 >> 8);
            
            /* Another conditional */
            if (i & 2) {
                result1 = result1 * 2 - result3;
                result2 = result2 + (result4 * 3);
                
                /* Force spill/reload with volatile */
                sink = result1;
                __asm__ volatile ("" : : "r"(result2), "r"(result3) : "memory");
            } else {
                result3 = result3 | result4;
                result4 = result4 & result1;
                
                sink = result3;
                __asm__ volatile ("" : : "r"(result4), "r"(result2) : "memory");
            }
            
            /* --- Basic Block 4: Final computations --- */
            
            /* Use all results to prevent elimination */
            int final1 = result1 + result2;
            int final2 = result3 - result4;
            long final3 = (long)result1 * result3;
            
            /* Store to global array (prevent DCE) */
            global_array[i & 0xFF] = final1 + final2;
            
            /* Another register-clobbering call */
            clobber_registers();
            
            /* Use volatile to force register pressure */
            volatile int temp = final3;
            (void)temp;
            
            /* Mix in more address computations */
            if (i & 4) {
                int *dynamic_addr = &global_array[(i + param_base) & 0xFF];
                *dynamic_addr = final1;
            }
        }
    }
    
    /* Final sink to prevent entire loop elimination */
    printf("Result: %d\n", sink + global_array[0]);
    return 0;
}
