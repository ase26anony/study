/* caller-save-test.c
 * 
 * This program is designed to trigger GCC's caller-save optimization pass
 * to move an instruction that is currently marked as BB_END for its basic block,
 * specifically covering lines 905-913 in caller-save.cc.
 *
 * The strategy:
 * 1. Create register pressure with multiple call-clobbered registers
 * 2. Force a function call inside a conditional block
 * 3. Use inline assembly to clobber registers around the call
 * 4. Create a control flow where BB_END needs updating
 * 5. Use volatile variables to prevent optimization
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Helper function to create register pressure */
int __attribute__((noinline)) 
caller_save_target(volatile int *arr, int idx) {
    /* Use multiple call-clobbered registers in calculations */
    register long r10_val asm("r10") = arr[idx] * 3;
    register long r11_val asm("r11") = arr[idx + 1] * 5;
    register long r12_val asm("r12") = arr[idx + 2] * 7;
    
    /* Volatile condition to maintain control flow */
    volatile int condition = arr[idx] & 1;
    
    if (condition) {
        /* 
         * This block will become a basic block ending with 
         * the function call or a related instruction.
         * The caller-save pass may need to insert save/restore
         * instructions around the rand() call.
         */
        
        /* Clobber call-clobbered registers before call */
        asm volatile (
            "clobber_before:"
            : 
            : "r" (r10_val), "r" (r11_val), "r" (r12_val)
            : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11", "cc"
        );
        
        /* Function call that clobbers registers */
        int r = rand();
        
        /* Clobber again after call */
        asm volatile (
            "clobber_after:"
            : 
            : "r" (r10_val), "r" (r11_val), "r" (r12_val)
            : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11", "cc"
        );
        
        /* More computations using the clobbered registers */
        r10_val = (r10_val ^ r) + 1;
        r11_val = (r11_val ^ r) + 2;
        r12_val = (r12_val ^ r) + 3;
        
        /* Store results - creates side effects */
        arr[idx] = r10_val;
        arr[idx + 1] = r11_val;
        arr[idx + 2] = r12_val;
        
        /* Return with computation to prevent tail call optimization */
        return (r10_val + r11_val + r12_val) & 0xFF;
    }
    
    /* Alternative path - also uses registers */
    r10_val = r10_val >> 1;
    r11_val = r11_val >> 2;
    r12_val = r12_val >> 3;
    
    arr[idx] = r10_val;
    arr[idx + 1] = r11_val;
    arr[idx + 2] = r12_val;
    
    return (r10_val - r11_val - r12_val) & 0xFF;
}

/* Outer function with loop to increase chances */
void __attribute__((noinline))
caller_save_loop(volatile int *arr, int size) {
    /* Outer loop - creates multiple opportunities */
    for (int outer = 0; outer < 3; outer++) {
        /* Inner loop with register pressure */
        for (int i = 0; i < size - 3; i += 3) {
            /* 
             * Mix of conditions to create different basic block structures.
             * The volatile variable forces the compiler to keep the condition.
             */
            volatile int selector = arr[i] % 4;
            
            if (selector == 0) {
                /* Path 1: Direct computation */
                arr[i] = arr[i] * 2 + 1;
            } 
            else if (selector == 1) {
                /* Path 2: Function call with register pressure */
                int result = caller_save_target(arr, i);
                
                /* Additional computation to use result */
                arr[i + 1] ^= result;
                
                /* Another volatile condition */
                volatile int check = result & 1;
                if (check) {
                    /* Nested condition creates more block boundaries */
                    arr[i + 2] = result * 3;
                } else {
                    arr[i + 2] = result * 7;
                }
            }
            else if (selector == 2) {
                /* Path 3: More register-intensive operations */
                register long rax_sim asm("rax") = arr[i];
                register long rcx_sim asm("rcx") = arr[i + 1];
                register long rdx_sim asm("rdx") = arr[i + 2];
                
                /* Inline assembly that might get moved */
                asm volatile (
                    "mix_regs:"
                    : "+r" (rax_sim), "+r" (rcx_sim), "+r" (rdx_sim)
                    : 
                    : "cc"
                );
                
                /* Function call in the middle */
                int r = rand() % 100;
                
                /* More operations */
                rax_sim = (rax_sim * r) | 1;
                rcx_sim = (rcx_sim * r) | 2;
                rdx_sim = (rdx_sim * r) | 3;
                
                arr[i] = rax_sim;
                arr[i + 1] = rcx_sim;
                arr[i + 2] = rdx_sim;
            }
            else {
                /* Path 4: Simple store */
                arr[i] = arr[i] ^ arr[i + 1] ^ arr[i + 2];
            }
        }
        
        /* Shuffle array between outer iterations */
        for (int j = 0; j < size; j++) {
            arr[j] = (arr[j] * 1103515245 + 12345) & 0x7FFFFFFF;
        }
    }
}

int main() {
    /* Initialize random seed */
    srand(time(NULL));
    
    /* Use volatile to prevent optimization */
    const int SIZE = 100;
    volatile int *array = malloc(SIZE * sizeof(int));
    
    if (!array) {
        return 1;
    }
    
    /* Initialize with random values */
    for (int i = 0; i < SIZE; i++) {
        array[i] = rand() % 1000;
    }
    
    /* Run the test multiple times */
    for (int trial = 0; trial < 5; trial++) {
        caller_save_loop(array, SIZE);
        
        /* Compute checksum to ensure side effects */
        long sum = 0;
        for (int i = 0; i < SIZE; i++) {
            sum += array[i];
        }
        
        /* Print to prevent dead code elimination */
        printf("Trial %d: checksum = %ld\n", trial, sum);
        
        /* Re-initialize for next trial */
        for (int i = 0; i < SIZE; i++) {
            array[i] = (array[i] * 1664525 + 1013904223) & 0x7FFFFFFF;
        }
    }
    
    free((void*)array);
    return 0;
}
