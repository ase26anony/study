/* caller-save-test.c
 * 
 * This program is designed to trigger GCC's caller-save optimization pass
 * to move an instruction that is currently marked as BB_END of its basic block,
 * specifically covering lines 905-913 in caller-save.cc.
 *
 * Compile with: gcc -O2 -fno-optimize-sibling-calls -march=x86-64 -fdump-rtl-caller_save -S caller-save-test.c -o caller-save-test.s
 * Or for more detailed analysis: gcc -O2 -fno-omit-frame-pointer -da -fdump-rtl-all caller-save-test.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Helper function that creates register pressure around a function call */
static int __attribute__((noinline)) 
pressure_function(volatile int x, volatile int y) {
    /* Use multiple call-clobbered registers in calculations */
    long long rax_val, rcx_val, rdx_val, rsi_val, rdi_val, r8_val, r9_val, r10_val;
    
    /* Initial computations using volatile inputs */
    rax_val = x * 3;
    rcx_val = y * 5;
    rdx_val = x + y;
    rsi_val = x - y;
    rdi_val = x * y;
    r8_val = x << 2;
    r9_val = y >> 1;
    r10_val = x ^ y;
    
    /* Clobber call-clobbered registers before the call */
    asm volatile (
        "movq $0x123456789ABCDEF0, %%rax\n\t"
        "movq $0xFEDCBA9876543210, %%rcx\n\t"
        "movq $0xAAAAAAAAAAAAAAAA, %%rdx\n\t"
        "movq $0xBBBBBBBBBBBBBBBB, %%rsi\n\t"
        "movq $0xCCCCCCCCCCCCCCCC, %%rdi\n\t"
        "movq $0xDDDDDDDDDDDDDDDD, %%r8\n\t"
        "movq $0xEEEEEEEEEEEEEEEE, %%r9\n\t"
        "movq $0xFFFFFFFFFFFFFFFF, %%r10\n\t"
        :
        :
        : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "cc"
    );
    
    /* Function call that clobbers registers - rand() is a good candidate */
    int result = rand();
    
    /* More computations using the clobbered registers */
    asm volatile (
        "addq %%rcx, %%rax\n\t"
        "subq %%rdx, %%rsi\n\t"
        "xorq %%rdi, %%r8\n\t"
        "orq  %%r9, %%r10\n\t"
        : 
        : 
        : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "cc"
    );
    
    /* Use the results to prevent optimization */
    return result + (rax_val & 0xFF) + (rcx_val & 0xFF) + (rdx_val & 0xFF);
}

/* Main test function with complex control flow */
static void __attribute__((noinline))
test_caller_save_scenario(void) {
    volatile int outer_counter = 100;  /* Prevent loop unrolling */
    volatile int condition_var = 0;
    int sum = 0;
    
    /* Outer loop to increase chances of hitting the BB_END scenario */
    for (int i = 0; i < outer_counter; i++) {
        /* Vary the condition to create different block structures */
        condition_var = i & 0xF;
        
        /* Critical control flow structure */
        if (condition_var < 8) {
            /* This block should end with a conditional or jump */
            volatile int x = i * 3;
            volatile int y = i * 7;
            
            /* Multiple calls with register pressure */
            int r1 = pressure_function(x, y);
            
            /* Additional computation to create more register pressure */
            long long temp1 = x * y;
            long long temp2 = x + y;
            long long temp3 = x - y;
            
            /* Another function call to increase save/restore needs */
            int r2 = rand();
            
            /* Use results to prevent optimization */
            sum += r1 + r2 + (temp1 & 0xFF) + (temp2 & 0xFF) + (temp3 & 0xFF);
            
            /* This should create a block ending with a computation */
            if (r1 > r2) {
                /* Nested condition to split blocks further */
                sum += pressure_function(y, x);
            }
        } else {
            /* Alternative path to create control flow complexity */
            sum += pressure_function(i, i * 2);
        }
        
        /* Intermediate computation that might be placed in call-clobbered regs */
        volatile int intermediate = i * 11;
        asm volatile (
            "movl %0, %%eax\n\t"
            "imull $13, %%eax\n\t"
            : 
            : "r" (intermediate)
            : "rax", "cc"
        );
        
        /* Another conditional to create more block boundaries */
        if ((i & 3) == 0) {
            int r3 = rand();
            sum += r3 + pressure_function(intermediate, i);
        }
    }
    
    /* Use the sum to prevent dead code elimination */
    asm volatile ("" : : "r" (sum));
}

/* Additional test with switch statement to create multiple block ends */
static void __attribute__((noinline))
test_switch_scenario(void) {
    volatile int switch_var = 0;
    int result = 0;
    
    for (int i = 0; i < 50; i++) {
        switch_var = i & 0x3;
        
        switch (switch_var) {
            case 0:
                /* Block ending with function call */
                result += pressure_function(i, i+1);
                break;
                
            case 1:
                /* Block with computation then call */
                result += i * 2;
                result += rand();
                break;
                
            case 2:
                /* Block with multiple calls */
                result += pressure_function(i, i*2);
                result += rand();
                result += pressure_function(i*3, i*4);
                break;
                
            default:
                /* Complex block with condition inside */
                if (i & 1) {
                    result += pressure_function(i, 0);
                } else {
                    result += rand();
                }
                break;
        }
    }
    
    asm volatile ("" : : "r" (result));
}

int main(void) {
    /* Initialize random seed */
    srand(time(NULL));
    
    printf("Starting caller-save test...\n");
    
    /* Run the primary test scenario */
    test_caller_save_scenario();
    
    /* Run additional scenario for more coverage */
    test_switch_scenario();
    
    printf("Test completed.\n");
    
    return 0;
}
