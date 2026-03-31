/* reload1_trigger.c
 * Designed to trigger specific reload types in GCC's reload pass,
 * particularly RELOAD_FOR_OTHER_ADDRESS and related cases.
 */

#include <stdint.h>
#include <stdio.h>

/* Complex structure to force non-trivial addressing */
struct DataBlock {
    int32_t values[16];
    int64_t counters[8];
    void* pointers[4];
    volatile int32_t flags;
};

/* Global volatile structure to prevent optimizations */
volatile struct DataBlock global_block;

/* Function with complex addressing patterns */
void trigger_reloads(int iterations) {
    /* Explicit register variables - will conflict with inline asm */
    register int64_t index_reg asm("r10") = 0;
    register int64_t base_reg asm("r11") = (int64_t)&global_block;
    register int64_t temp_reg asm("r12") = 0x1234;
    
    /* Local arrays with different alignments */
    int32_t local_array[32] __attribute__((aligned(64)));
    volatile int64_t volatile_buffer[16];
    
    /* Multi-dimensional array for complex indexing */
    int32_t matrix[8][8] __attribute__((aligned(32)));
    
    /* Initialize some data */
    for (int i = 0; i < 32; i++) {
        local_array[i] = i * 3;
    }
    
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            matrix[i][j] = i * 8 + j;
        }
    }
    
    /* Main loop with complex addressing patterns */
    for (int iter = 0; iter < iterations; iter++) {
        /* Pattern 1: RELOAD_FOR_OTHER_ADDRESS
         * Complex memory constraint with address computation that needs reloading
         */
        asm volatile (
            "/* Complex address reload pattern */\n\t"
            "add %[idx], %[base], %[idx], lsl #2\n\t"  /* Hypothetical ARM-like */
            "ldr %[out], [%[idx], %[offset]]\n\t"
            : [out] "=r" (temp_reg)
            : [idx] "r" (index_reg),
              [base] "r" (base_reg),
              [offset] "r" (iter * 4),
              "m" (*(struct DataBlock*)&global_block.values[index_reg % 16])
            : "memory", "cc"
        );
        
        /* Pattern 2: RELOAD_FOR_INPUT_ADDRESS and RELOAD_FOR_INPADDR_ADDRESS
         * Multiple memory operands with different address requirements
         */
        int32_t* ptr1 = &local_array[iter % 32];
        int32_t* ptr2 = &matrix[iter % 8][0];
        
        asm volatile (
            "/* Multiple memory address reloads */\n\t"
            "ldp %[val1], %[val2], [%[addr1]]\n\t"
            "stp %[val1], %[val2], [%[addr2], %[off]]\n\t"
            : [val1] "=&r" (temp_reg),
              [val2] "=&r" (index_reg)
            : [addr1] "r" (ptr1),
              [addr2] "r" (ptr2),
              [off] "r" (iter * 4),
              "m" (*ptr1),
              "m" (*(volatile int64_t*)&volatile_buffer[iter % 8])
            : "memory"
        );
        
        /* Pattern 3: RELOAD_FOR_OUTPUT_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS
         * Output memory operand with complex address computation
         */
        int64_t* out_ptr = &volatile_buffer[(index_reg + iter) % 8];
        
        asm volatile (
            "/* Output address reload */\n\t"
            "str %[val], [%[out], %[idx], lsl #3]\n\t"
            : "=m" (*out_ptr)
            : [val] "r" (temp_reg),
              [out] "r" (out_ptr),
              [idx] "r" (index_reg)
            : "memory"
        );
        
        /* Pattern 4: RELOAD_FOR_OPERAND_ADDRESS and RELOAD_FOR_OPADDR_ADDR
         * Mixed immediate, register, and memory constraints
         */
        int32_t immediate = 0xDEADBEEF;
        int64_t large_offset = 0x1000;
        
        asm volatile (
            "/* Mixed operand types */\n\t"
            "mov x0, %[imm]\n\t"
            "add x0, x0, %[large]\n\t"
            "str x0, [%[base], %[idx]]\n\t"
            : 
            : [imm] "i" (immediate),
              [large] "r" (large_offset),
              [base] "r" (base_reg),
              [idx] "r" (index_reg),
              "m" (global_block.counters[index_reg % 8])
            : "x0", "memory"
        );
        
        /* Pattern 5: RELOAD_FOR_INPUT with complex indexing
         * Multi-dimensional array access with computed index
         */
        int32_t row = iter % 8;
        int32_t col = (iter * 7) % 8;
        register int32_t* matrix_ptr asm("r13") = &matrix[0][0];
        
        asm volatile (
            "/* Complex array indexing */\n\t"
            "madd %[idx], %[row], %[cols], %[col]\n\t"
            "ldr %[out], [%[base], %[idx], lsl #2]\n\t"
            : [out] "=r" (temp_reg),
              [idx] "=&r" (index_reg)
            : [row] "r" (row),
              [col] "r" (col),
              [cols] "i" (8),
              [base] "r" (matrix_ptr)
            : "memory"
        );
        
        /* Update registers to create live range conflicts */
        index_reg = (index_reg * 1664525 + 1013904223) & 0x7FFFFFFF;
        base_reg = base_reg + (iter * 16);
        
        /* Force spill/reload of explicit register variables */
        asm volatile (
            "/* Clobber explicit registers */\n\t"
            "mov %[tmp], %[idx]\n\t"
            "add %[tmp], %[tmp], %[base]\n\t"
            : [tmp] "=&r" (temp_reg)
            : [idx] "r" (index_reg),
              [base] "r" (base_reg)
            : "cc"
        );
    }
    
    /* Final pattern: RELOAD_OTHER case
     * Standalone complex address computation
     */
    struct DataBlock* block_ptr = (struct DataBlock*)base_reg;
    
    asm volatile (
        "/* Other reload pattern */\n\t"
        "ldr x0, [%[ptr], %[offset]]\n\t"
        "str x0, [%[out]]\n\t"
        : 
        : [ptr] "r" (block_ptr),
          [offset] "r" (temp_reg & 0xFF),
          [out] "r" (&global_block.flags),
          "m" (block_ptr->pointers[0])
        : "x0", "memory"
    );
}

/* Secondary function to create more reload contexts */
void nested_reload_context(int depth, struct DataBlock* block) {
    if (depth <= 0) return;
    
    /* Use explicit register in nested context */
    register void* addr_reg asm("r14") = block->pointers[depth % 4];
    
    asm volatile (
        "/* Nested address reload */\n\t"
        "ldr x1, [%[addr], #8]\n\t"
        "add x1, x1, %[depth]\n\t"
        "str x1, [%[addr]]\n\t"
        : 
        : [addr] "r" (addr_reg),
          [depth] "r" (depth),
          "m" (*(volatile void**)addr_reg)
        : "x1", "memory"
    );
    
    nested_reload_context(depth - 1, block);
}

int main() {
    /* Initialize global block */
    for (int i = 0; i < 16; i++) {
        global_block.values[i] = i * 2;
    }
    
    for (int i = 0; i < 8; i++) {
        global_block.counters[i] = i * 1000;
        global_block.pointers[i % 4] = (void*)&global_block.values[i];
    }
    
    global_block.flags = 0;
    
    printf("Starting reload trigger patterns...\n");
    
    /* Trigger main reload patterns */
    trigger_reloads(10);
    
    /* Trigger nested patterns */
    nested_reload_context(5, &global_block);
    
    /* Final complex pattern with all constraint types */
    {
        register int64_t r10 asm("r10") = 0x100;
        register int64_t r11 asm("r11") = 0x200;
        register int64_t r12 asm("r12") = 0x300;
        
        asm volatile (
            "/* Ultimate reload test */\n\t"
            "add %[r10], %[r10], %[r11]\n\t"
            "add %[r12], %[r12], %[r10], lsl #2\n\t"
            "str %[r12], [%[base], %[idx]]\n\t"
            : [r10] "+r" (r10),
              [r12] "+r" (r12)
            : [r11] "r" (r11),
              [base] "r" (&global_block),
              [idx] "r" (r10),
              "m" (global_block),
              "m" (*(struct DataBlock*)&global_block)
            : "memory", "cc"
        );
    }
    
    printf("Reload patterns completed.\n");
    return 0;
}
