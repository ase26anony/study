/* caller-save-test.c
 * 
 * This program is designed to trigger GCC's caller-save optimization pass
 * to move an instruction that is currently marked as BB_END of its basic block,
 * specifically covering lines 905-913 in caller-save.cc.
 *
 * Compile with: gcc -O2 -fno-optimize-sibling-calls -march=x86-64 -fdump-rtl-caller_save -S caller-save-test.c
 * Or for more detailed analysis: gcc -O2 -fno-omit-frame-pointer -da -fdump-rtl-all caller-save-test.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Helper function that creates register pressure around a function call */
static int __attribute__((noinline))
caller_save_hot_path(volatile int *arr, int idx) {
    /* Use multiple call-clobbered registers in calculations */
    register long r10_val asm("r10") = arr[idx] * 3;
    register long r11_val asm("r11") = arr[idx + 1] * 5;
    register long r12_val asm("r12") = arr[idx + 2] * 7;
    
    /* Volatile variable to prevent optimization of control flow */
    volatile int condition = arr[idx] & 1;
    
    /* This creates a basic block that ends with a conditional jump */
    if (condition) {
        /* Inline assembly that clobbers call-clobbered registers */
        asm volatile("" : : : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10");
        
        /* Function call that clobbers registers - creates caller-save pressure */
        int r = rand();
        
        /* More calculations using call-clobbered registers after the call */
        r10_val ^= r;
        r11_val += r;
        r12_val -= r;
        
        /* Another inline assembly clobber to force save/restore */
        asm volatile("" : : : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10");
        
        /* Store results back to array */
        arr[idx] = (int)(r10_val & 0x7FFFFFFF);
        arr[idx + 1] = (int)(r11_val & 0x7FFFFFFF);
        arr[idx + 2] = (int)(r12_val & 0x7FFFFFFF);
        
        return 1;
    }
    
    return 0;
}

/* Main function with nested loops to increase caller-save opportunities */
int main(void) {
    volatile int outer_counter = 0;
    const int ARR_SIZE = 100;
    volatile int *array = (volatile int*)malloc(ARR_SIZE * sizeof(int));
    
    /* Initialize array with random values */
    srand(time(NULL));
    for (int i = 0; i < ARR_SIZE; i++) {
        array[i] = rand() % 100;
    }
    
    int total_changed = 0;
    
    /* Outer loop to create multiple passes through caller-save */
    for (outer_counter = 0; outer_counter < 1000; outer_counter++) {
        /* Inner loop with multiple basic blocks */
        for (int i = 0; i < ARR_SIZE - 3; i++) {
            /* Create register pressure before the call */
            register long pre_calc asm("rax") = array[i];
            register long pre_calc2 asm("rcx") = array[i + 1];
            
            /* Call the hot path function */
            total_changed += caller_save_hot_path((volatile int*)array, i);
            
            /* Use the registers after the call to force caller-save */
            pre_calc ^= array[i];
            pre_calc2 += array[i + 1];
            
            /* Store results to prevent optimization */
            array[i] = (int)(pre_calc & 0x7FFFFFFF);
            array[i + 1] = (int)(pre_calc2 & 0x7FFFFFFF);
            
            /* Additional control flow to split basic blocks */
            if (i % 10 == 0) {
                /* Another function call to increase pressure */
                int r = rand();
                array[i] ^= r;
                
                /* More register usage */
                register long temp asm("rdx") = r;
                temp *= array[i];
                array[i] = (int)(temp & 0x7FFFFFFF);
            }
        }
        
        /* Periodically print to prevent optimization */
        if (outer_counter % 100 == 0) {
            printf("Iteration %d, total_changed: %d\n", outer_counter, total_changed);
        }
    }
    
    /* Final computation to use all values */
    long final_sum = 0;
    for (int i = 0; i < ARR_SIZE; i++) {
        final_sum += array[i];
    }
    
    printf("Final sum: %ld, total_changed: %d\n", final_sum, total_changed);
    
    free((void*)array);
    return 0;
}
