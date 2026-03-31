/* reload1_trigger.c
 * Designed to trigger specific reload types in GCC's reload pass:
 * - RELOAD_FOR_OTHER_ADDRESS
 * - RELOAD_FOR_INPUT_ADDRESS
 * - RELOAD_FOR_OPADDR_ADDR
 * - RELOAD_FOR_OPERAND_ADDRESS
 * - RELOAD_FOR_OUTPUT_ADDRESS
 */

#include <stdint.h>
#include <stdlib.h>

/* Complex data structure to force non-trivial addressing */
struct ComplexData {
    int64_t values[8];
    struct ComplexData *next;
    volatile int32_t flags[4];
    double fp_data[2];
};

/* Volatile globals to prevent optimization */
volatile struct ComplexData global_data[16];
volatile int64_t *volatile global_ptr_array[32];

/* Explicit register variables - force specific register allocation */
register int64_t reg_index asm("r10");
register struct ComplexData *reg_ptr asm("r11");
register int64_t reg_temp asm("r12");
register int64_t reg_addr asm("r13");
register int64_t reg_offset asm("r14");

/* Helper to create complex addressing expressions */
static inline int64_t compute_complex_offset(int64_t base, int64_t idx1, int64_t idx2) {
    return (base * 8 + idx1 * 16 + idx2 * 32) & 0xFF;
}

int main(void) {
    /* Initialize data */
    struct ComplexData local_data[8];
    volatile int64_t *mem_ptrs[16];
    int64_t accumulators[4] = {0};
    
    /* Initialize pointers */
    for (int i = 0; i < 8; i++) {
        local_data[i].next = &local_data[(i + 1) & 7];
        for (int j = 0; j < 8; j++) {
            local_data[i].values[j] = i * 100 + j;
        }
    }
    
    for (int i = 0; i < 16; i++) {
        mem_ptrs[i] = &local_data[i & 7].values[0];
    }
    
    /* Set up explicit register variables */
    reg_index = 0;
    reg_ptr = &local_data[0];
    reg_temp = 0x12345678;
    reg_addr = (int64_t)&local_data[0];
    reg_offset = 16;
    
    /* Loop 1: Complex addressing with multiple reload types */
    for (int outer = 0; outer < 4; outer++) {
        /* Force RELOAD_FOR_INPUT_ADDRESS and RELOAD_FOR_OTHER_ADDRESS */
        for (int inner = 0; inner < 8; inner++) {
            /* Complex inline assembly with conflicting constraints */
            int64_t idx = outer * 8 + inner;
            int64_t *base_ptr = &local_data[inner].values[0];
            int64_t offset = compute_complex_offset(outer, inner, reg_index);
            
            /* Pattern 1: Memory operand with address that needs reloading */
            asm volatile (
                "/* Complex memory access with address reload */\n\t"
                "movq %[base], %[temp]\n\t"
                "addq %[offset], %[temp]\n\t"
                "movq (%[temp]), %[result]\n\t"
                : [result] "=r" (accumulators[inner & 3])
                : [base] "irm" (base_ptr),  /* Immediate, register, or memory */
                  [offset] "r" (offset),
                  [temp] "r" (reg_temp)     /* Force use of specific register */
                : "memory"
            );
            
            /* Pattern 2: Multiple memory operands with different addressing */
            volatile int64_t *volatile_ptr = mem_ptrs[(idx + 1) & 15];
            
            asm volatile (
                "/* Multiple memory operands */\n\t"
                "movq %[src], %%r15\n\t"    /* Force address into register */
                "movq (%%r15), %[dst]\n\t"
                "addq %[imm], %[dst]\n\t"
                : [dst] "=r" (reg_temp)
                : [src] "m" (*volatile_ptr),  /* Memory constraint forces address reload */
                  [imm] "i" (256)             /* Immediate */
                : "r15", "memory"
            );
            
            /* Update register variables to force spills/reloads */
            reg_index = (reg_index + 1) & 7;
            reg_offset = (reg_offset + 8) & 31;
        }
        
        /* Pattern 3: Nested address computation with explicit registers */
        int64_t complex_addr = (int64_t)&local_data[outer];
        
        asm volatile (
            "/* Nested address computation */\n\t"
            "leaq 32(%[base], %[idx], 8), %[addr]\n\t"
            "movq %[addr], %[ptr]\n\t"
            "movq (%[ptr]), %[val]\n\t"
            : [addr] "=&r" (reg_addr),
              [ptr] "=&r" (reg_ptr),
              [val] "=r" (reg_temp)
            : [base] "r" (complex_addr),
              [idx] "r" (reg_index)
            : "memory"
        );
        
        /* Pattern 4: Output address reload */
        int64_t *output_addr = &accumulators[outer & 3];
        
        asm volatile (
            "/* Output address reload */\n\t"
            "movq %[val], (%[out_addr])\n\t"
            : 
            : [val] "r" (reg_temp),
              [out_addr] "m" (*output_addr)  /* Output address needs reload */
            : "memory"
        );
    }
    
    /* Loop 2: Mixed operand types with volatile accesses */
    for (int i = 0; i < 32; i++) {
        /* Force RELOAD_FOR_OPADDR_ADDR and RELOAD_FOR_OPERAND_ADDRESS */
        volatile struct ComplexData *volatile_data = &global_data[i & 15];
        int64_t immediate_offset = 64;  /* Compile-time constant */
        
        /* Complex constraints mixing immediate, register, and memory */
        asm volatile (
            "/* Mixed operand types */\n\t"
            "movq %[data], %%r15\n\t"
            "movq %[offset], %%r14\n\t"
            "addq %%r14, %%r15\n\t"
            "movq (%%r15), %[result]\n\t"
            : [result] "=r" (reg_temp)
            : [data] "m" (volatile_data->values[0]),  /* Memory with complex address */
              [offset] "irm" (immediate_offset)       /* Immediate, register, or memory */
            : "r14", "r15", "memory"
        );
        
        /* Use the result in another asm with fixed register constraints */
        asm volatile (
            "/* Fixed register usage */\n\t"
            "addq %[in], %%r12\n\t"
            "movq %%r12, %[out]\n\t"
            : [out] "=r" (accumulators[i & 3])
            : [in] "r" (reg_temp),
              "0" (accumulators[i & 3])  /* Tied operand */
            : "r12", "cc"
        );
        
        /* Update volatile memory through computed address */
        int64_t *target = (int64_t*)((char*)volatile_data + (i * 8));
        
        asm volatile (
            "/* Volatile store with address computation */\n\t"
            "movq %[addr], %%r13\n\t"
            "movq %[value], (%%r13)\n\t"
            : 
            : [addr] "r" (target),
              [value] "r" (reg_temp)
            : "r13", "memory"
        );
    }
    
    /* Final complex pattern: Deeply nested addressing */
    {
        struct ComplexData **ptr_ptr = (struct ComplexData**)&local_data[0].next;
        int64_t nested_offset = 128;
        
        asm volatile (
            "/* Deeply nested addressing */\n\t"
            "movq %[ptrptr], %%rbx\n\t"      /* Load pointer to pointer */
            "movq (%%rbx), %%rcx\n\t"        /* Dereference once */
            "movq %[off], %%rdx\n\t"         /* Load offset */
            "addq %%rdx, %%rcx\n\t"          /* Compute final address */
            "movq (%%rcx), %[result]\n\t"    /* Load from final address */
            : [result] "=r" (reg_temp)
            : [ptrptr] "m" (**ptr_ptr),      /* Complex memory operand */
              [off] "r" (nested_offset)
            : "rbx", "rcx", "rdx", "memory"
        );
    }
    
    /* Prevent dead code elimination */
    asm volatile ("" : : "r"(accumulators[0]), "r"(accumulators[1]),
                       "r"(accumulators[2]), "r"(accumulators[3]) : );
    
    return (int)(accumulators[0] + accumulators[1] + 
                 accumulators[2] + accumulators[3]);
}
