/* reload_trigger.c
 * Designed to trigger specific reload types in GCC's reload pass:
 * - RELOAD_FOR_OTHER_ADDRESS
 * - RELOAD_FOR_INPUT_ADDRESS
 * - RELOAD_FOR_INPADDR_ADDRESS
 * - RELOAD_FOR_OPERAND_ADDRESS
 * - RELOAD_FOR_OPADDR_ADDR
 * And other address reload types
 */

#include <stdint.h>
#include <stdlib.h>

/* Complex structure to force non-trivial addressing */
struct DataBlock {
    int64_t values[8];
    int32_t indices[16];
    volatile int16_t flags[32];
    char padding[64];
};

/* Global volatile structures to prevent optimization */
volatile struct DataBlock global_block;
volatile struct DataBlock* volatile global_ptr = &global_block;

/* Explicit register variables - will force specific register usage */
register int64_t reg_a asm("r10");
register int64_t reg_b asm("r11");
register int64_t reg_c asm("r12");
register int64_t reg_d asm("r13");
register int64_t reg_e asm("r14");
register int64_t reg_f asm("r15");

/* Multi-dimensional array with complex access pattern */
static int32_t matrix[8][8][8];

/* Function with complex addressing modes */
void complex_addressing(int iterations) {
    /* Local volatile to force memory accesses */
    volatile int64_t local_accum = 0;
    volatile struct DataBlock local_block;
    
    /* Pointer with volatile qualification */
    volatile struct DataBlock* block_ptr = &local_block;
    
    /* Multiple index variables in explicit registers */
    reg_a = 0;
    reg_b = 1;
    reg_c = 2;
    reg_d = 3;
    reg_e = 4;
    reg_f = 5;
    
    /* Loop with complex address computations */
    for (int i = 0; i < iterations; i++) {
        /* Pattern 1: Complex memory operand with register-indirect addressing
         * Forces RELOAD_FOR_INPUT_ADDRESS and RELOAD_FOR_OTHER_ADDRESS */
        asm volatile (
            /* Use reg_a as base, reg_b as index with scale */
            "lea (%[base], %[index], 4), %[temp]\n\t"
            /* Memory access using computed address */
            "mov (%[temp]), %[temp]\n\t"
            /* Another memory access with different addressing */
            "add (%[ptr]), %[temp]\n\t"
            /* Store result back through different pointer */
            "mov %[temp], (%[dest])"
            : [temp] "=&r" (reg_a), [dest] "+r" (block_ptr)
            : [base] "r" (&matrix[0][0][0]), 
              [index] "r" (reg_b),
              [ptr] "m" (*(volatile int64_t*)&global_block.values[reg_c]),
              "0" (reg_a)
            : "memory", "cc"
        );
        
        /* Pattern 2: Multiple memory operands with conflicting constraints
         * Forces RELOAD_FOR_INPADDR_ADDRESS */
        int64_t offset = reg_d * 8 + reg_e;
        asm volatile (
            /* Complex addressing with multiple memory references */
            "imul %[off], %[val]\n\t"
            "add %[mem1], %[val]\n\t"
            "sub %[mem2], %[val]"
            : [val] "+r" (reg_f)
            : [off] "r" (offset),
              [mem1] "m" (local_block.values[reg_a % 8]),
              [mem2] "m" (global_ptr->indices[reg_b % 16])
            : "cc"
        );
        
        /* Pattern 3: Nested address computation in loop
         * Forces RELOAD_FOR_OPERAND_ADDRESS and RELOAD_FOR_OPADDR_ADDR */
        for (int j = 0; j < 4; j++) {
            /* Compute complex array index */
            int64_t idx = (reg_a * j + reg_b * i) % 64;
            
            /* Mixed operand types: immediate, register, and memory */
            asm volatile (
                /* Operation with mixed constraints */
                "mov %[imm], %%rax\n\t"
                "add %[reg], %%rax\n\t"
                "mov (%%rax), %%rbx\n\t"
                "add %%rbx, %[out]"
                : [out] "+r" (local_accum)
                : [imm] "i" (16),  /* Immediate */
                  [reg] "r" (idx), /* Register */
                  "m" (*(struct DataBlock*)((char*)&local_block + idx)) /* Memory */
                : "rax", "rbx", "memory", "cc"
            );
            
            /* Pattern 4: Address reload for output operand */
            volatile int64_t* output_ptr = &local_block.values[j];
            asm volatile (
                /* Store with complex address computation */
                "mov %[val], (%[ptr], %[idx], 8)"
                : 
                : [val] "r" (reg_f),
                  [ptr] "r" (output_ptr),
                  [idx] "r" (reg_c)
                : "memory"
            );
        }
        
        /* Pattern 5: Force RELOAD_FOR_OTHER_ADDRESS specifically
         * by using an address that needs reloading for "other" purposes */
        int64_t* addr_calc = (int64_t*)((char*)&matrix[0][0][0] + reg_a * 64 + reg_b * 8);
        
        asm volatile (
            /* Multiple uses of the same address register */
            "mov (%[addr]), %%rax\n\t"
            "add %%rax, %[sum]\n\t"
            /* Use address register for different addressing mode */
            "mov %[sum], 8(%[addr])"
            : [sum] "+r" (reg_e)
            : [addr] "r" (addr_calc)
            : "rax", "memory", "cc"
        );
        
        /* Rotate registers to create different conflicts */
        int64_t temp = reg_a;
        reg_a = reg_b;
        reg_b = reg_c;
        reg_c = reg_d;
        reg_d = reg_e;
        reg_e = reg_f;
        reg_f = temp;
    }
    
    /* Final pattern: Complex addressing with multiple reload types */
    volatile int64_t* volatile ptr_array[4];
    for (int i = 0; i < 4; i++) {
        ptr_array[i] = &local_block.values[i * 2];
    }
    
    asm volatile (
        /* Chain of operations requiring various address reloads */
        "mov (%[ptr1]), %%rax\n\t"
        "imul (%[ptr2]), %%rax\n\t"
        "add (%[ptr3], %[idx], 8), %%rax\n\t"
        "mov %%rax, (%[ptr4])"
        :
        : [ptr1] "r" (ptr_array[0]),
          [ptr2] "r" (ptr_array[1]),
          [ptr3] "r" (ptr_array[2]),
          [idx] "r" (reg_a),
          [ptr4] "r" (ptr_array[3])
        : "rax", "memory", "cc"
    );
}

/* Secondary function with different patterns */
void mixed_operand_types(void) {
    /* Force immediate, register, and memory operands in same asm */
    int64_t imm_val = 42;
    register int64_t reg_val asm("rbx") = 100;
    volatile int64_t mem_val = 1000;
    
    /* This pattern should trigger multiple reload types */
    asm volatile (
        /* Operation requiring different operand types */
        "add %[imm], %[reg]\n\t"
        "sub %[mem], %[reg]\n\t"
        "imul $2, %[reg]\n\t"
        /* Memory store with complex address */
        "mov %[reg], (%[addr], %[idx], 1)"
        : [reg] "+r" (reg_val)
        : [imm] "i" (imm_val),  /* Immediate constraint */
          [mem] "m" (mem_val),  /* Memory constraint */
          [addr] "r" (&global_block),
          [idx] "r" (reg_a)     /* Register constraint */
        : "memory", "cc"
    );
    
    /* Another complex pattern */
    struct {
        int64_t a;
        int64_t b[4];
        volatile int64_t c;
    } __attribute__((packed)) packed_struct;
    
    /* Access packed structure with misaligned address */
    int64_t* misaligned_ptr = (int64_t*)((char*)&packed_struct + 1);
    
    asm volatile (
        /* Misaligned access requiring special handling */
        "mov (%[mis]), %%rax\n\t"
        /* Use the value in another memory operation */
        "add %%rax, (%[base], %[offset], 8)"
        :
        : [mis] "r" (misaligned_ptr),
          [base] "r" (&packed_struct.b[0]),
          [offset] "r" (reg_b)
        : "rax", "memory", "cc"
    );
}

int main(void) {
    /* Initialize data */
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            for (int k = 0; k < 8; k++) {
                matrix[i][j][k] = i * 64 + j * 8 + k;
            }
        }
    }
    
    /* Call functions with complex addressing patterns */
    complex_addressing(8);  /* Enough iterations to create pressure */
    mixed_operand_types();
    
    /* Additional loop with different pattern */
    for (int i = 0; i < 4; i++) {
        /* Use explicit register variables in addressing */
        reg_a = i * 16;
        reg_b = i * 8 + 1;
        
        asm volatile (
            /* Complex addressing with multiple base registers */
            "mov (%[base1], %[idx1], 8), %%rax\n\t"
            "add (%[base2], %[idx2], 4), %%rax\n\t"
            "mov %%rax, (%[dest])"
            :
            : [base1] "r" (&matrix[0][0][0]),
              [idx1] "r" (reg_a),
              [base2] "r" (&global_block.indices[0]),
              [idx2] "r" (reg_b),
              [dest] "r" (&global_block.values[i])
            : "rax", "memory", "cc"
        );
    }
    
    return 0;
}
