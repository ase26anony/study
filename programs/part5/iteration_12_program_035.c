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
    int32_t indices[16];
    volatile int16_t flags;
    char padding[7];
};

/* Multi-dimensional array with padding */
struct MultiDim {
    struct ComplexData rows[4][3];
    int64_t metadata[2];
};

/* Global volatile structures to prevent optimization */
volatile struct MultiDim global_data;
volatile int64_t *volatile global_ptr_array[8];

/* Explicit register variables - force specific register allocation */
register int64_t reg_a asm("r10");
register int64_t reg_b asm("r11");
register int64_t reg_c asm("r12");
register int64_t reg_d asm("r13");
register int64_t reg_e asm("r14");
register int64_t reg_f asm("r15");

/* Function with complex addressing patterns */
void complex_addressing_loop(int iterations) {
    /* Local arrays with different alignments */
    struct ComplexData local_array[5] __attribute__((aligned(32)));
    volatile int64_t *ptr_buffer[4];
    
    /* Initialize pointers */
    for (int i = 0; i < 4; i++) {
        ptr_buffer[i] = &local_array[i].values[0];
    }
    
    /* Use explicit register variables in computations */
    reg_a = (int64_t)&local_array[0];
    reg_b = (int64_t)&global_data;
    reg_c = iterations * sizeof(struct ComplexData);
    
    /* Main loop with complex addressing */
    for (int i = 0; i < iterations; i++) {
        int idx1 = i & 3;
        int idx2 = (i * 7) & 15;
        int64_t offset = i * 24;
        
        /* Pattern 1: Complex memory operand with register index
         * Forces RELOAD_FOR_INPUT_ADDRESS and RELOAD_FOR_OTHER_ADDRESS */
        asm volatile (
            "lea (%[base], %[idx], 8), %[temp]\n\t"
            "mov (%[temp], %[off]), %[out]\n\t"
            : [out] "=r" (reg_d)
            : [base] "r" (reg_a),
              [idx] "r" (idx1),
              [off] "r" (offset),
              [temp] "r" (reg_e)
            : "memory"
        );
        
        /* Pattern 2: Multiple memory constraints with fixed registers
         * Forces RELOAD_FOR_INPADDR_ADDRESS */
        asm volatile (
            "imulq $7, %[val1], %[val2]\n\t"
            "addq %[val2], (%[mem1])\n\t"
            "movq (%[mem2]), %[val3]\n\t"
            : [val2] "=&r" (reg_e),
              [val3] "=r" (reg_f)
            : [val1] "0" (reg_d),
              [mem1] "m" (local_array[idx1].values[idx2]),
              [mem2] "m" (*(volatile int64_t*)(reg_b + offset))
            : "cc", "memory"
        );
        
        /* Pattern 3: Nested address computation with immediate
         * Forces RELOAD_FOR_OPADDR_ADDR */
        int64_t complex_offset = 128 + i * 16;
        asm volatile (
            "movq %[imm], %%rax\n\t"
            "addq %%rax, %[addr]\n\t"
            "movq (%[addr]), %%rbx\n\t"
            "addq %%rbx, %[result]\n\t"
            : [result] "+r" (reg_a),
              [addr] "+r" (reg_b)
            : [imm] "irm" (complex_offset)
            : "rax", "rbx", "cc", "memory"
        );
        
        /* Pattern 4: Multiple output operands with memory inputs
         * Forces RELOAD_FOR_OPERAND_ADDRESS */
        int64_t temp1, temp2;
        asm volatile (
            "movq (%[ptr1]), %[out1]\n\t"
            "movq (%[ptr2]), %[out2]\n\t"
            "addq %[out2], %[out1]\n\t"
            : [out1] "=&r" (temp1),
              [out2] "=&r" (temp2)
            : [ptr1] "r" (&ptr_buffer[idx1]),
              [ptr2] "r" (&global_ptr_array[idx2])
            : "memory"
        );
        
        /* Pattern 5: Address reload for volatile struct member
         * Forces RELOAD_FOR_OTHER_ADDRESS specifically */
        volatile struct ComplexData *volatile_ptr = &local_array[idx1];
        asm volatile (
            "movzwl %[flag], %%eax\n\t"
            "orl $1, %%eax\n\t"
            "movw %%ax, %[flag]\n\t"
            : [flag] "+m" (volatile_ptr->flags)
            : 
            : "rax", "cc", "memory"
        );
        
        /* Mix register usage to increase pressure */
        reg_c = reg_a + reg_b;
        reg_d = reg_c * reg_f;
        
        /* Access multi-dimensional array with complex index */
        int row = (i >> 2) & 3;
        int col = (i >> 1) & 2;
        global_data.rows[row][col].values[idx2] = reg_d;
    }
}

/* Secondary function with different addressing patterns */
void mixed_operand_types(void) {
    /* Immediate, register, and memory mixed */
    int64_t imm_val = 0x12345678;
    int64_t reg_val;
    int64_t mem_buffer[16] __attribute__((aligned(64)));
    
    /* Force RELOAD_FOR_OUTPUT_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
    for (int i = 0; i < 8; i++) {
        /* Complex constraint: immediate OR register OR memory */
        asm volatile (
            "movq %[src], %%rax\n\t"
            "addq $0x100, %%rax\n\t"
            "movq %%rax, %[dst]\n\t"
            : [dst] "=m" (mem_buffer[i])
            : [src] "irm" (imm_val + i)
            : "rax", "cc"
        );
        
        /* Memory operand with register-indirect addressing */
        asm volatile (
            "movq (%[base], %[idx], 8), %[out]\n\t"
            "incq %[out]\n\t"
            "movq %[out], (%[base], %[idx], 8)\n\t"
            : [out] "=&r" (reg_val)
            : [base] "r" (mem_buffer),
              [idx] "r" (i),
              "m" (*(struct { int64_t x[16]; } *)mem_buffer)
            : "cc", "memory"
        );
    }
}

/* Main function sets up data and calls complex functions */
int main(void) {
    /* Initialize global data */
    for (int i = 0; i < 8; i++) {
        global_ptr_array[i] = (int64_t*)&global_data + i;
    }
    
    /* Call functions with different iteration counts
     * to trigger various reload scenarios */
    complex_addressing_loop(100);
    mixed_operand_types();
    
    /* Final barrier to prevent dead code elimination */
    asm volatile ("" : : : "memory");
    
    return 0;
}
