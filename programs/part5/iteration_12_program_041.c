/* reload1_trigger.c
 * Designed to trigger specific reload types in GCC's reload pass:
 * - RELOAD_FOR_OTHER_ADDRESS
 * - RELOAD_FOR_INPUT_ADDRESS
 * - RELOAD_FOR_INPADDR_ADDRESS
 * - RELOAD_FOR_OPADDR_ADDR
 * - RELOAD_FOR_OPERAND_ADDRESS
 */

#include <stdint.h>
#include <stdlib.h>

/* Complex structure to force non-trivial addressing */
struct ComplexData {
    int64_t values[8];
    struct ComplexData* next;
    volatile int32_t flags[4];
    double fp_data[2];
};

/* Global volatile structures to prevent optimization */
volatile struct ComplexData global_data[16];
volatile int64_t global_array[256];

/* Explicit register variables - force specific register allocation */
register int64_t reg_a asm("r10");
register int64_t reg_b asm("r11");
register int64_t reg_c asm("r12");
register int64_t reg_d asm("r13");
register int64_t reg_e asm("r14");
register int64_t reg_f asm("r15");

/* Function with complex addressing modes */
void complex_addressing_loop(int iterations) {
    /* Local arrays with different alignments */
    int64_t aligned_array[64] __attribute__((aligned(64)));
    int64_t unaligned_array[64] __attribute__((aligned(8)));
    
    /* Volatile pointers to force memory accesses */
    volatile int64_t* volatile_ptr = aligned_array;
    volatile struct ComplexData* volatile_struct = &global_data[0];
    
    /* Initialize arrays */
    for (int i = 0; i < 64; i++) {
        aligned_array[i] = i * 3;
        unaligned_array[i] = i * 7;
    }
    
    /* Initialize explicit register variables */
    reg_a = (int64_t)&aligned_array[0];
    reg_b = (int64_t)&unaligned_array[0];
    reg_c = (int64_t)&global_data[0];
    reg_d = 8;  /* stride */
    reg_e = 16; /* offset */
    reg_f = 0;  /* accumulator */
    
    /* Main loop with complex addressing patterns */
    for (int i = 0; i < iterations; i++) {
        /* Pattern 1: Memory operand with complex address computation
         * Forces RELOAD_FOR_INPUT_ADDRESS and RELOAD_FOR_OTHER_ADDRESS */
        asm volatile (
            "/* Complex memory access with multiple index registers */\n\t"
            "movq (%[base], %[idx1], 8), %[tmp]\n\t"
            "addq %[tmp], %[acc]\n\t"
            "movq %[acc], (%[base], %[idx2], 4)"
            : [acc] "+r" (reg_f), [tmp] "=&r" (reg_e)
            : [base] "r" (reg_a), [idx1] "r" (reg_d), [idx2] "r" (reg_b & 0x3F)
            : "memory"
        );
        
        /* Pattern 2: Inline assembly with memory constraint and register conflict
         * Forces RELOAD_FOR_INPADDR_ADDRESS */
        int64_t temp;
        asm volatile (
            "/* Memory constraint with conflicting register */\n\t"
            "lea (%[addr], %[offset], 1), %[temp]\n\t"
            "movq (%[temp]), %[temp]"
            : [temp] "=&r" (temp)
            : [addr] "r" (reg_c), [offset] "r" (reg_d * 2)
            : "memory"
        );
        
        /* Pattern 3: Multiple output operands with memory inputs
         * Forces RELOAD_FOR_OPERAND_ADDRESS */
        int64_t out1, out2;
        asm volatile (
            "/* Multiple outputs with complex addressing */\n\t"
            "movq (%[ptr1]), %[out1]\n\t"
            "imulq %[imm], %[out1]\n\t"
            "movq %[out1], (%[ptr2], %[idx], 1)"
            : [out1] "=&r" (out1), [out2] "=r" (out2)
            : [ptr1] "m" (*(volatile int64_t*)(reg_a + reg_d)),
              [ptr2] "r" (reg_b),
              [idx] "r" (reg_d),
              [imm] "i" (42)  /* immediate operand */
            : "memory", "cc"
        );
        
        /* Pattern 4: Nested address computation in loop
         * Forces RELOAD_FOR_OPADDR_ADDR */
        volatile int64_t* addr_ptr = (volatile int64_t*)(reg_a + (reg_d * i));
        asm volatile (
            "/* Nested address with register pressure */\n\t"
            "movq (%[addr]), %%r8\n\t"
            "addq %%r8, %[sum]\n\t"
            "movq %[sum], (%[dest])"
            : [sum] "+r" (reg_f)
            : [addr] "r" (addr_ptr),
              [dest] "m" (*(volatile int64_t*)(reg_b + (reg_d * 2)))
            : "r8", "memory", "cc"
        );
        
        /* Pattern 5: Complex struct access with volatile
         * Forces RELOAD_FOR_OUTPUT_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
        int64_t struct_offset = (i * sizeof(struct ComplexData)) & 0xFF;
        asm volatile (
            "/* Volatile struct member access */\n\t"
            "movq %[offset], %%r9\n\t"
            "andq $0x38, %%r9\n\t"
            "movq (%[base], %%r9, 1), %[val]\n\t"
            "orq %[val], (%[flags])"
            : [val] "=&r" (temp)
            : [base] "r" (reg_c),
              [offset] "r" (struct_offset),
              [flags] "m" (volatile_struct->flags[0])
            : "r9", "memory", "cc"
        );
        
        /* Modify register variables to change addressing patterns */
        reg_d = (reg_d + 1) & 0x7;
        reg_a = reg_a + 8;
        if (reg_a >= (int64_t)&aligned_array[64]) {
            reg_a = (int64_t)&aligned_array[0];
        }
        
        /* Force spill/reload of explicit registers */
        asm volatile (
            "/* Clobber explicit registers */\n\t"
            "movq $0x12345678, %%r10\n\t"
            "movq $0x87654321, %%r11\n\t"
            "xchgq %%r12, %%r13"
            :
            :
            : "r10", "r11", "r12", "r13", "cc"
        );
    }
}

/* Secondary function with different addressing patterns */
void mixed_operand_types(int count) {
    /* Mixed types: immediate, memory, register */
    int64_t buffer[32];
    volatile int64_t* volatile_buf = buffer;
    
    /* Initialize with pattern */
    for (int i = 0; i < 32; i++) {
        buffer[i] = i * i;
    }
    
    /* Use all explicit register variables */
    reg_a = (int64_t)buffer;
    reg_b = 4;
    reg_c = 8;
    reg_d = 12;
    reg_e = 0;
    reg_f = 0;
    
    for (int i = 0; i < count; i++) {
        /* Complex constraint: "irm" for one operand, fixed register for another */
        int64_t result;
        asm volatile (
            "/* Mixed constraint pattern */\n\t"
            "movq %[input], %%rax\n\t"
            "addq %%rax, %[result]\n\t"
            "movq %[result], (%[base], %[index], 8)"
            : [result] "=&r" (result)
            : [input] "irm" (buffer[i & 0x1F]),  /* immediate, register, or memory */
              [base] "r" (reg_a),
              [index] "r" (reg_b)
            : "rax", "memory", "cc"
        );
        
        /* Memory barrier to prevent optimization */
        asm volatile ("" ::: "memory");
        
        /* Access with complex offset calculation */
        int64_t offset = (reg_b * reg_c + reg_d) & 0x1F;
        asm volatile (
            "movq (%[base], %[off], 8), %[tmp]\n\t"
            "addq $1, %[tmp]"
            : [tmp] "=r" (result)
            : [base] "r" (reg_a),
              [off] "r" (offset)
            : "cc"
        );
        
        /* Rotate register values */
        int64_t tmp = reg_a;
        reg_a = reg_b;
        reg_b = reg_c;
        reg_c = reg_d;
        reg_d = reg_e;
        reg_e = reg_f;
        reg_f = tmp;
    }
}

/* Main function sets up data and calls complex functions */
int main() {
    /* Initialize global data */
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 8; j++) {
            global_data[i].values[j] = i * 100 + j;
        }
        global_data[i].next = &global_data[(i + 1) % 16];
        for (int j = 0; j < 4; j++) {
            global_data[i].flags[j] = i ^ j;
        }
        global_data[i].fp_data[0] = i * 1.5;
        global_data[i].fp_data[1] = i * 2.5;
    }
    
    for (int i = 0; i < 256; i++) {
        global_array[i] = i * 11;
    }
    
    /* Call functions with different parameters to trigger various reloads */
    complex_addressing_loop(100);
    mixed_operand_types(50);
    
    /* Additional loop with different characteristics */
    for (int outer = 0; outer < 10; outer++) {
        /* Vary the parameters to create different reload scenarios */
        complex_addressing_loop(20 + outer);
        mixed_operand_types(15 + outer);
        
        /* Direct inline assembly with complex constraints */
        int64_t x = outer * 100;
        int64_t y = outer * 200;
        asm volatile (
            "/* Final complex pattern */\n\t"
            "movq %[x], %%r10\n\t"
            "movq %[y], %%r11\n\t"
            "lea (%%r10, %%r11, 4), %%r12\n\t"
            "movq (%%r12), %%r13\n\t"
            "addq %%r13, %%r14"
            :
            : [x] "m" (*(volatile int64_t*)&x),
              [y] "m" (*(volatile int64_t*)&y)
            : "r10", "r11", "r12", "r13", "r14", "memory", "cc"
        );
    }
    
    return 0;
}
