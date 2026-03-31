/* reload1_trigger.c
 * Designed to trigger RELOAD_FOR_OTHER_ADDRESS and other reload types
 * in GCC's reload1.cc during compilation
 */

#include <stdint.h>
#include <stdlib.h>

/* Complex structure to force various addressing modes */
struct MultiDim {
    int32_t data[8][8];
    int64_t offsets[16];
    volatile int32_t * volatile ptrs[4];
};

/* Explicit register variables - tie to specific registers */
register int64_t reg_index asm("r12");
register int64_t reg_base asm("r11");
register int64_t reg_temp asm("r10");
register int64_t reg_addr asm("r9");

/* Global volatile structures to prevent optimization */
volatile struct MultiDim global_struct;
volatile int64_t global_array[256] = {0};
volatile int32_t * volatile global_ptrs[8];

/* Function with complex addressing patterns */
void complex_addressing_loop(int iterations) {
    /* Local variables that will conflict with register variables */
    int64_t local_index = 0;
    int64_t local_offset = 0;
    volatile int32_t *local_ptr = NULL;
    
    /* Multi-dimensional array access pattern */
    int32_t md_array[4][8][16];
    
    /* Force address computations into registers */
    for (int i = 0; i < iterations; i++) {
        /* Pattern 1: Complex memory addressing with multiple components */
        asm volatile (
            /* Compute address using multiple registers */
            "lea (%[base], %[index], 4), %[temp]\n\t"
            "add %[offset], %[temp]\n\t"
            /* Memory operation with computed address */
            "mov (%[temp]), %[addr]\n\t"
            /* Use the address for another operation */
            "add %[addr], %[index]"
            : [temp] "=&r" (reg_temp), [addr] "=&r" (reg_addr), [index] "+r" (reg_index)
            : [base] "r" (reg_base), [offset] "r" (local_offset)
            : "memory", "cc"
        );
        
        /* Pattern 2: Mixed operand types with memory constraint */
        local_ptr = (volatile int32_t *)&md_array[i % 4][(i / 4) % 8][0];
        asm volatile (
            "imul %[imm], %[reg]\n\t"
            "add %[mem], %[reg]"
            : [reg] "+r" (reg_temp)
            : [imm] "i" (8), [mem] "m" (*(struct MultiDim*)local_ptr)
            : "cc"
        );
        
        /* Pattern 3: Nested address computation in loop */
        /* This should trigger RELOAD_FOR_OTHER_ADDRESS */
        int64_t complex_index = (reg_index * 3 + local_offset) % 256;
        asm volatile (
            /* Multiple address calculations */
            "mov %[idx], %[tmp]\n\t"
            "shl $3, %[tmp]\n\t"
            /* Memory access with complex addressing */
            "mov (%[arr], %[tmp]), %[addr]\n\t"
            /* Use address for further computation */
            "lea (%[addr], %[idx], 2), %[tmp]"
            : [tmp] "=&r" (reg_temp), [addr] "=&r" (reg_addr)
            : [idx] "r" (complex_index), [arr] "r" (global_array)
            : "memory", "cc"
        );
        
        /* Pattern 4: Forcing RELOAD_FOR_INPUT_ADDRESS and RELOAD_FOR_OTHER_ADDRESS */
        /* Complex struct member access */
        volatile int32_t *elem_ptr = &global_struct.data[i % 8][(i * 2) % 8];
        asm volatile (
            /* Multiple memory accesses with different addressing */
            "mov (%[ptr1]), %[tmp1]\n\t"
            "mov %[ptr2], %[tmp2]\n\t"
            "add (%[tmp2]), %[tmp1]\n\t"
            "mov %[tmp1], (%[ptr3])"
            : [tmp1] "=&r" (reg_temp), [tmp2] "=&r" (reg_addr)
            : [ptr1] "r" (elem_ptr), 
              [ptr2] "r" (&global_struct.offsets[i % 16]),
              [ptr3] "r" (&global_array[complex_index])
            : "memory", "cc"
        );
        
        /* Pattern 5: Address computation for pointer array */
        /* This should trigger various reload types */
        int ptr_idx = (i * 7) % 4;
        volatile int32_t **ptr_to_ptr = &global_struct.ptrs[ptr_idx];
        
        asm volatile (
            /* Load pointer, then load through it */
            "mov (%[pptr]), %[addr]\n\t"
            "test %[addr], %[addr]\n\t"
            "jz 1f\n\t"
            "mov (%[addr]), %[tmp]\n\t"
            "add %[val], %[tmp]\n\t"
            "mov %[tmp], (%[addr])\n\t"
            "1:"
            : [addr] "=&r" (reg_addr), [tmp] "=&r" (reg_temp)
            : [pptr] "r" (ptr_to_ptr), [val] "r" (i)
            : "memory", "cc"
        );
        
        /* Update loop variables in ways that force reloads */
        local_offset += reg_temp;
        reg_index = (reg_index * 1664525 + 1013904223) % 65536;
        
        /* Pattern 6: Immediate + memory + register constraints */
        /* Forces different reload decisions */
        int64_t immediate_val = 0x12345678;
        asm volatile (
            /* Complex operation mixing all operand types */
            "mov %[imm], %[tmp]\n\t"
            "imul %[reg], %[tmp]\n\t"
            "add (%[mem]), %[tmp]\n\t"
            "mov %[tmp], (%[mem2])"
            : [tmp] "=&r" (reg_temp)
            : [imm] "i" (immediate_val),
              [reg] "r" (reg_index),
              [mem] "r" (&global_struct.offsets[0]),
              [mem2] "r" (&global_array[i % 256])
            : "memory", "cc"
        );
    }
}

/* Second function with different addressing patterns */
void mixed_operand_types(void) {
    /* Force RELOAD_FOR_OPERAND_ADDRESS and RELOAD_FOR_OPADDR_ADDR */
    register int64_t r1 asm("r8");
    register int64_t r2 asm("r13");
    register int64_t r3 asm("r14");
    
    volatile int64_t * volatile ptr_array[4];
    for (int i = 0; i < 4; i++) {
        ptr_array[i] = &global_array[i * 16];
    }
    
    /* Complex addressing chain */
    r1 = 0;
    for (int i = 0; i < 100; i++) {
        /* Pattern triggering RELOAD_FOR_OPERAND_ADDRESS */
        asm volatile (
            /* Chain of address calculations */
            "mov %[idx], %[t1]\n\t"
            "lea (%[t1], %[t1], 2), %[t2]\n\t"  /* t2 = idx * 3 */
            "shl $3, %[t2]\n\t"                 /* Convert to byte offset */
            "add %[base], %[t2]\n\t"            /* Add base address */
            "mov (%[t2]), %[t3]\n\t"            /* Load through computed address */
            "add %[t3], %[t1]"                  /* Add to original index */
            : [t1] "+r" (r1), [t2] "=&r" (r2), [t3] "=&r" (r3)
            : [idx] "0" (r1), [base] "r" (global_array)
            : "memory", "cc"
        );
        
        /* Access through pointer array with offset */
        int idx = r1 % 4;
        volatile int64_t *current_ptr = ptr_array[idx];
        
        asm volatile (
            /* Multiple indirections */
            "mov %[ptr], %[addr]\n\t"
            "mov (%[addr], %[off], 8), %[val]\n\t"
            "add %[inc], %[val]\n\t"
            "mov %[val], (%[addr], %[off], 8)"
            : [addr] "=&r" (r2), [val] "=&r" (r3)
            : [ptr] "r" (current_ptr), [off] "r" (idx), [inc] "r" (i)
            : "memory", "cc"
        );
    }
}

/* Main function sets up data and calls complex functions */
int main(void) {
    /* Initialize global data */
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            global_struct.data[i][j] = i * 100 + j;
        }
    }
    
    for (int i = 0; i < 16; i++) {
        global_struct.offsets[i] = i * 8;
    }
    
    for (int i = 0; i < 4; i++) {
        global_struct.ptrs[i] = (volatile int32_t *)&global_array[i * 32];
    }
    
    for (int i = 0; i < 256; i++) {
        global_array[i] = i;
    }
    
    /* Initialize explicit register variables */
    reg_index = 42;
    reg_base = (int64_t)&global_struct;
    reg_temp = 0;
    reg_addr = 0;
    
    /* Call functions with complex addressing patterns */
    complex_addressing_loop(100);
    mixed_operand_types();
    
    /* Final pattern: Very complex addressing with multiple reload types */
    {
        volatile int64_t *final_ptr = &global_array[0];
        register int64_t final_idx asm("r15") = 128;
        
        asm volatile (
            /* This pattern should trigger multiple reload types */
            "mov %[idx], %[t1]\n\t"
            /* Address computation 1 */
            "lea (%[base], %[t1], 8), %[t2]\n\t"
            /* Address computation 2 (nested) */
            "mov (%[t2]), %[t3]\n\t"
            "lea (%[t3], %[t1], 2), %[t4]\n\t"
            /* Memory access with computed address */
            "mov (%[t4]), %[t5]\n\t"
            /* Another address computation */
            "shl $2, %[t5]\n\t"
            "add %[t5], %[t1]\n\t"
            /* Final store with complex addressing */
            "mov %[t1], (%[base], %[t5], 1)"
            : [t1] "+r" (final_idx), [t2] "=&r" (reg_temp), 
              [t3] "=&r" (reg_addr), [t4] "=&r" (r2), [t5] "=&r" (r3)
            : [base] "r" (final_ptr), [idx] "0" (final_idx)
            : "memory", "cc"
        );
    }
    
    return 0;
}
