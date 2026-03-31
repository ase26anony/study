/* reload1_trigger.c - Complex program to trigger various reload types in GCC */
#include <stdint.h>

/* Volatile structures to prevent optimization */
struct ComplexData {
    volatile int64_t a;
    volatile int32_t b[4];
    volatile char c[32];
};

struct Nested {
    volatile struct ComplexData inner[8];
    volatile int64_t counters[16];
};

/* Explicit register variables for x86-64 */
register int64_t reg_a asm("r10");
register int64_t reg_b asm("r11");
register int64_t reg_c asm("r12");
register int64_t reg_d asm("r13");
register int64_t reg_e asm("r14");

/* Global arrays to create addressing complexity */
static struct Nested global_data[4];
static volatile int64_t global_index_array[256];
static volatile int32_t *global_ptr_array[128];

/* Function with complex addressing patterns */
void complex_addressing_loop(int iterations) {
    /* Local variables that will conflict with register variables */
    int64_t local_base = (int64_t)&global_data[0];
    volatile int64_t *volatile ptr1 = &global_index_array[0];
    volatile int32_t **volatile ptr2 = &global_ptr_array[0];
    
    /* Initialize explicit register variables with addresses */
    reg_a = (int64_t)&global_data[1];
    reg_b = (int64_t)&global_data[2];
    reg_c = (int64_t)&global_data[3];
    reg_d = (int64_t)ptr1;
    reg_e = (int64_t)ptr2;
    
    /* Loop with nested addressing computations */
    for (int i = 0; i < iterations; i++) {
        /* Complex inline assembly 1: Mixed operand types with memory constraint */
        /* Forces RELOAD_FOR_INPUT_ADDRESS and RELOAD_FOR_OTHER_ADDRESS */
        asm volatile (
            "/* Complex op 1 */\n\t"
            "movq %[addr1], %%r15\n\t"
            "addq %[imm], %%r15\n\t"
            "movq (%%r15), %%r15\n\t"
            "addq %%r15, %[out1]"
            : [out1] "+r" (reg_a)
            : [addr1] "irm" (*(volatile int64_t*)(reg_b + i * 8)), 
              [imm] "i" (16)
            : "r15", "memory"
        );
        
        /* Complex inline assembly 2: Multiple memory operands with different addressing */
        /* Forces RELOAD_FOR_INPADDR_ADDRESS and RELOAD_FOR_OUTPUT_ADDRESS */
        int64_t temp1, temp2;
        asm volatile (
            "/* Complex op 2 */\n\t"
            "leaq (%[base], %[index], 8), %%r15\n\t"
            "movq (%%r15), %[t1]\n\t"
            "movq %[t1], (%[dest])"
            : [t1] "=&r" (temp1), "=m" (*(volatile int64_t*)(reg_c))
            : [base] "r" (local_base),
              [index] "r" (reg_d),
              [dest] "r" (reg_e)
            : "r15", "memory"
        );
        
        /* Complex inline assembly 3: Nested address computation */
        /* Forces RELOAD_FOR_OPADDR_ADDR and RELOAD_FOR_OPERAND_ADDRESS */
        struct ComplexData *cd_ptr = (struct ComplexData*)reg_a;
        asm volatile (
            "/* Complex op 3 */\n\t"
            "movq %[struct_ptr], %%r15\n\t"
            "movq %[offset], %%r14\n\t"
            "addq %%r14, %%r15\n\t"
            "movl 8(%%r15), %%eax\n\t"
            "addl %%eax, %[result]"
            : [result] "+r" (reg_b)
            : [struct_ptr] "m" (*(volatile struct ComplexData**)&cd_ptr),
              [offset] "r" (i * 48)
            : "rax", "r14", "r15", "memory"
        );
        
        /* Complex inline assembly 4: Multiple constraints on same operand */
        /* Forces RELOAD_FOR_OUTADDR_ADDRESS */
        volatile int64_t *addr_calc = (volatile int64_t*)(reg_c + reg_d);
        asm volatile (
            "/* Complex op 4 */\n\t"
            "movq %[addr], %%r15\n\t"
            "movq (%%r15), %%r14\n\t"
            "imulq $3, %%r14\n\t"
            "movq %%r14, %[out]"
            : [out] "=rm" (reg_d)
            : [addr] "X" (addr_calc),
              "[out]" (reg_d)
            : "r14", "r15", "memory"
        );
        
        /* Complex inline assembly 5: Forcing RELOAD_FOR_OTHER_ADDRESS specifically */
        /* This pattern often triggers RELOAD_FOR_OTHER_ADDRESS */
        volatile int64_t *other_addr = (volatile int64_t*)((char*)&global_data + reg_e);
        asm volatile (
            "/* Complex op 5 - targeting OTHER_ADDRESS */\n\t"
            "movq %[ptr], %%r15\n\t"
            "movq (%%r15, %[idx], 8), %%r14\n\t"
            "addq %%r14, %[accum]"
            : [accum] "+r" (reg_e)
            : [ptr] "m" (*(volatile int64_t**)&other_addr),
              [idx] "r" (reg_a & 0xF)
            : "r14", "r15", "memory"
        );
        
        /* Mix in some C code with complex array indexing */
        /* This creates additional reload pressure */
        int64_t idx1 = reg_a & 0xFF;
        int64_t idx2 = reg_b & 0x7;
        int64_t idx3 = reg_c & 0x3;
        
        volatile int64_t val = global_data[idx3].inner[idx2].a;
        global_data[idx1 % 4].counters[idx2] += val;
        
        /* More inline assembly with overlapping clobbers */
        asm volatile (
            "/* Complex op 6 */\n\t"
            "movq %[val], %%r15\n\t"
            "addq %%r15, %[sum]\n\t"
            "movq %[sum], (%[mem])"
            : [sum] "+r" (reg_c)
            : [val] "r" (val),
              [mem] "m" (*(volatile int64_t*)(reg_d + idx1 * 8))
            : "r15", "memory"
        );
    }
}

/* Second function with different addressing patterns */
void mixed_operand_types(void) {
    /* Immediate, register, and memory mixed */
    int64_t imm_val = 42;
    register int64_t fixed_reg asm("r9") = 100;
    
    /* Pattern that forces multiple reload types */
    for (int i = 0; i < 32; i++) {
        /* Complex constraint: 'irm' for one operand, fixed register for another */
        asm volatile (
            "/* Mixed operand pattern */\n\t"
            "addq %[src], %[dst]\n\t"
            "movq %[dst], (%[mem])"
            : [dst] "+r" (fixed_reg)
            : [src] "irm" (global_index_array[i]),
              [mem] "r" (reg_a + i * 8)
            : "memory"
        );
        
        /* Another complex pattern */
        volatile int64_t *volatile ptr = (volatile int64_t*)(reg_b + i * 64);
        asm volatile (
            "/* Another mixed pattern */\n\t"
            "movq (%[addr]), %%r15\n\t"
            "subq %[imm], %%r15\n\t"
            "movq %%r15, %[out]"
            : [out] "=rm" (reg_d)
            : [addr] "m" (*(volatile int64_t**)&ptr),
              [imm] "i" (imm_val)
            : "r15", "memory"
        );
    }
}

/* Main function that sets up data and calls complex functions */
int main(void) {
    /* Initialize some data */
    for (int i = 0; i < 256; i++) {
        global_index_array[i] = i * 2;
    }
    
    for (int i = 0; i < 128; i++) {
        global_ptr_array[i] = (int32_t*)&global_index_array[i % 256];
    }
    
    /* Call functions with complex addressing */
    complex_addressing_loop(16);
    mixed_operand_types();
    
    /* Final complex operation */
    asm volatile (
        "/* Final operation */\n\t"
        "movq %[a], %%rax\n\t"
        "addq %[b], %%rax\n\t"
        "movq %%rax, %[c]"
        : [c] "=m" (global_index_array[0])
        : [a] "r" (reg_a),
          [b] "m" (*(volatile int64_t*)(reg_c))
        : "rax", "memory"
    );
    
    return 0;
}
