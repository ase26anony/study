/* reload1_trigger.c
 * Designed to trigger complex reload scenarios in GCC's reload pass,
 * specifically targeting RELOAD_FOR_OTHER_ADDRESS and related types.
 */

#include <stdint.h>
#include <stdlib.h>

/* Complex structure to force non-trivial addressing */
struct DataBlock {
    int32_t values[16];
    int64_t counters[8];
    void* pointers[4];
    char padding[32];
};

/* Volatile structure to prevent optimization */
volatile struct DataBlock global_data[4];

/* Explicit register variables - using callee-saved registers on x86-64
 * to increase pressure on register allocator */
register int64_t reg_index asm("r12");
register int64_t reg_base asm("r13");
register int64_t reg_temp asm("r14");
register int64_t reg_addr asm("r15");

/* Function with complex addressing patterns */
void complex_addressing_loop(int iterations) {
    /* Local arrays with different alignments */
    struct DataBlock local_data[8] __attribute__((aligned(64)));
    int32_t* dynamic_ptr = (int32_t*)malloc(256 * sizeof(int32_t));
    
    /* Initialize some data */
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 16; j++) {
            local_data[i].values[j] = i * 100 + j;
        }
    }
    
    /* Force register variables to hold specific values */
    reg_index = 0;
    reg_base = (int64_t)&local_data[0];
    reg_temp = (int64_t)dynamic_ptr;
    
    /* Main loop with complex addressing */
    for (int i = 0; i < iterations; i++) {
        /* Pattern 1: RELOAD_FOR_OTHER_ADDRESS scenario
         * Complex memory constraint with register clobbering */
        asm volatile (
            /* Compute address using multiple registers */
            "lea (%[base], %[idx], 8), %[addr]\n\t"
            /* Use the computed address in memory operand */
            "movl (%[addr]), %%eax\n\t"
            /* Clobber address register to force reload */
            "addl $1, %%eax\n\t"
            "movl %%eax, (%[addr])"
            : [addr] "=&r" (reg_addr)  /* earlyclobber - can't share with inputs */
            : [base] "r" (reg_base),
              [idx] "r" (reg_index),
              "m" (*(struct DataBlock(*)[8])&local_data)  /* whole array as memory input */
            : "eax", "memory"
        );
        
        /* Pattern 2: Mix immediate, register, and memory constraints
         * Forces RELOAD_FOR_INPUT_ADDRESS and RELOAD_FOR_OTHER_ADDRESS */
        int32_t immediate_val = 42;
        asm volatile (
            "imull %[imm], (%[mem], %[idx], 4)\n\t"
            /* Additional operation that clobbers index register */
            "addq $4, %[idx]"
            : "+m" (*(int32_t(*)[256])dynamic_ptr),  /* memory operand with addressing */
              [idx] "+r" (reg_index)
            : [mem] "r" (reg_temp),
              [imm] "i" (immediate_val)  /* immediate constraint */
            : "cc"
        );
        
        /* Pattern 3: Nested address computation with multiple outputs
         * Forces RELOAD_FOR_OUTPUT_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
        int64_t offset1, offset2;
        asm volatile (
            /* Two different address computations */
            "lea (%[base], %[idx], 2), %[out1]\n\t"
            "lea 16(%[base], %[idx], 4), %[out2]"
            : [out1] "=&r" (offset1),  /* earlyclobber outputs */
              [out2] "=&r" (offset2)
            : [base] "r" (reg_base),
              [idx] "r" (reg_index)
            : /* no clobbers */
        );
        
        /* Pattern 4: Complex constraints with 'X' (any operand)
         * Forces RELOAD_FOR_OPERAND_ADDRESS */
        asm volatile (
            "movq %[any], %%r11\n\t"
            "addq %%r11, %[reg]"
            : [reg] "+r" (reg_temp)
            : [any] "X" (local_data[reg_index % 8].pointers[0])  /* any constraint */
            : "r11", "cc"
        );
        
        /* Pattern 5: Access volatile global with complex index
         * Forces RELOAD_FOR_INPADDR_ADDRESS */
        int complex_idx = (reg_index * 3 + 7) % 4;
        asm volatile (
            "movq %[ptr], %%rbx\n\t"
            "addl $1, (%%rbx, %[idx], 4)"
            : /* no outputs */
            : [ptr] "r" (&global_data[complex_idx].values[0]),
              [idx] "r" (reg_index % 16),
              "m" (global_data[complex_idx])  /* memory input */
            : "rbx", "memory", "cc"
        );
        
        /* Pattern 6: RELOAD_FOR_OPADDR_ADDR scenario
         * Address of an operand address computation */
        int32_t* addr_of_addr = &dynamic_ptr[reg_index % 128];
        asm volatile (
            /* Take address of memory location, then dereference */
            "movq %[addrptr], %%rcx\n\t"
            "movq (%%rcx), %%rdx\n\t"
            "addl $1, (%%rdx)"
            : /* no outputs */
            : [addrptr] "r" (&addr_of_addr),
              "m" (*addr_of_addr)  /* memory at computed address */
            : "rcx", "rdx", "memory"
        );
        
        /* Modify index for next iteration - creates loop-carried dependency */
        reg_index = (reg_index * 7 + 3) % 64;
    }
    
    /* Cleanup */
    free(dynamic_ptr);
}

/* Secondary function with different patterns */
void mixed_operand_types(void) {
    /* Force various operand types */
    register int32_t reg_a asm("r10");
    register int32_t reg_b asm("r11");
    
    int32_t stack_var = 99;
    const int32_t immediate = 255;
    int32_t* mem_ptr = &stack_var;
    
    /* Mixed constraints: immediate, register, memory */
    asm volatile (
        /* Operation requiring different operand types */
        "leal (%[imm], %[reg], 1), %[out]\n\t"
        "addl %[mem], %[out]"
        : [out] "=r" (reg_a)
        : [imm] "i" (immediate),    /* immediate */
          [reg] "r" (reg_b),        /* register */
          [mem] "m" (*mem_ptr)      /* memory */
        : "cc"
    );
    
    /* Complex case for RELOAD_FOR_OTHER_ADDRESS */
    struct {
        int32_t a;
        int32_t b[4];
    } __attribute__((packed)) packed_struct;
    
    asm volatile (
        /* Access with complex addressing that may need other address reload */
        "movl %[offset](%[base]), %%eax\n\t"
        "addl %%eax, %[reg]"
        : [reg] "+r" (reg_a)
        : [base] "r" (&packed_struct),
          [offset] "r" (reg_b),      /* register used as offset */
          "m" (packed_struct)        /* memory operand */
        : "eax", "cc", "memory"
    );
}

/* Main function sets up and calls complex patterns */
int main(void) {
    /* Initialize global data */
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 16; j++) {
            global_data[i].values[j] = i * 16 + j;
        }
    }
    
    /* Call functions with different complexities */
    complex_addressing_loop(100);
    mixed_operand_types();
    
    /* Additional loop with varying iteration count */
    for (int outer = 0; outer < 10; outer++) {
        reg_index = outer * 5;
        complex_addressing_loop(20);
    }
    
    return 0;
}
