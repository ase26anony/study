/* reload1_trigger.c
 * Designed to trigger specific reload types in GCC's reload pass:
 * - RELOAD_FOR_OTHER_ADDRESS
 * - RELOAD_FOR_INPUT_ADDRESS
 * - RELOAD_FOR_INPADDR_ADDRESS
 * - RELOAD_FOR_OPADDR_ADDR
 * - And others from the uncovered switch cases
 */

#include <stdint.h>
#include <stdio.h>

/* Complex structure to force non-trivial addressing */
struct ComplexStruct {
    int64_t data[8];
    int32_t indices[4];
    volatile int16_t flags;
    char padding[7];
};

/* Global arrays to create addressing pressure */
static struct ComplexStruct global_array[16];
static volatile int64_t volatile_buffer[32];

/* Explicit register variables - force specific register allocation */
register int64_t reg_a asm("r10");
register int64_t reg_b asm("r11");
register int64_t reg_c asm("r12");
register int64_t reg_d asm("r13");
register int64_t reg_e asm("r14");
register int64_t reg_f asm("r15");

/* Function with complex addressing patterns */
void process_with_reloads(int iterations, int offset) {
    /* Local variables that will conflict with register variables */
    int64_t local_base = (int64_t)&global_array[0];
    volatile int64_t* volatile_ptr = &volatile_buffer[0];
    
    /* Mixed types: immediate, memory, register */
    const int32_t immediate_const = 0x1234;
    int64_t memory_operand = 0xABCD;
    
    /* Multi-dimensional index calculation */
    int64_t idx1, idx2, idx3;
    
    /* Loop with nested address computations */
    for (int i = 0; i < iterations; i++) {
        /* Complex index calculation using multiple registers */
        idx1 = (reg_a + i) & 0xF;
        idx2 = (reg_b + offset) & 0x3;
        idx3 = (reg_c * 2) & 0x7;
        
        /* Pattern 1: RELOAD_FOR_OTHER_ADDRESS
         * Complex addressing with multiple register components
         */
        asm volatile (
            "/* Complex address computation */\n\t"
            "lea (%[base], %[idx1], 8), %[temp]\n\t"
            "add %[idx2], %[temp]\n\t"
            "mov (%[temp], %[idx3], 4), %[out1]\n\t"
            : [out1] "=r" (memory_operand),
              [temp] "=&r" (reg_f)
            : [base] "r" (local_base),
              [idx1] "r" (idx1),
              [idx2] "r" (idx2),
              [idx3] "r" (idx3)
            : "memory", "cc"
        );
        
        /* Pattern 2: RELOAD_FOR_INPUT_ADDRESS and RELOAD_FOR_INPADDR_ADDRESS
         * Memory operand with register-indirect addressing that needs reloading
         */
        int64_t* addr_calc = &global_array[idx1].data[idx2];
        asm volatile (
            "mov (%[addr]), %[out]\n\t"
            "add %[imm], %[out]\n\t"
            : [out] "=r" (reg_e)
            : [addr] "m" (*addr_calc),  /* Forces address reload */
              [imm] "i" (immediate_const)  /* Immediate constraint */
            : "cc"
        );
        
        /* Pattern 3: RELOAD_FOR_OPERAND_ADDRESS and RELOAD_FOR_OPADDR_ADDR
         * Multiple memory operands with different addressing requirements
         */
        struct ComplexStruct* struct_ptr = &global_array[i % 8];
        asm volatile (
            "/* Multiple memory accesses with address computations */\n\t"
            "mov %[ptr], %[temp1]\n\t"
            "add $16, %[temp1]\n\t"  /* Offset to indices array */
            "mov (%[temp1], %[idx], 4), %[out2]\n\t"
            "mov %[out2], (%[vol_ptr], %[idx], 8)\n\t"
            : [out2] "=&r" (reg_d),
              [temp1] "=&r" (reg_f)
            : [ptr] "rm" (struct_ptr),      /* Register or memory */
              [idx] "r" (idx3),
              [vol_ptr] "r" (volatile_ptr)  /* Fixed register constraint */
            : "memory"
        );
        
        /* Pattern 4: Mixed constraints creating conflicts
         * One operand must be in register, another needs address reload
         */
        int64_t* complex_addr = &struct_ptr->data[idx2] + idx3;
        asm volatile (
            "mov %[val1], %[out3]\n\t"
            "imul (%[complex]), %[out3]\n\t"
            : [out3] "=r" (reg_a)  /* Output tied to input register */
            : [val1] "0" (reg_b),  /* Same as output register */
              [complex] "m" (*complex_addr)  /* Forces address reload */
            : "cc"
        );
        
        /* Pattern 5: RELOAD_FOR_OUTPUT_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS
         * Output memory operand with complex addressing
         */
        volatile int64_t* output_addr = &volatile_buffer[(i + offset) & 0x1F];
        asm volatile (
            "mov %[in], (%[out_addr])\n\t"
            : 
            : [in] "r" (reg_a),
              [out_addr] "m" (*output_addr)  /* Output address needs reload */
            : "memory"
        );
        
        /* Use the results to prevent dead code elimination */
        reg_b = reg_a + reg_e;
        reg_c = reg_d ^ memory_operand;
        
        /* Compiler barrier */
        asm volatile ("" : : : "memory");
    }
}

/* Secondary function with different register pressure */
void nested_addressing(int depth, int64_t* results) {
    if (depth <= 0) return;
    
    /* Array of pointers creating alias issues */
    int64_t* ptr_array[4];
    for (int i = 0; i < 4; i++) {
        ptr_array[i] = &global_array[i].data[depth & 0x7];
    }
    
    /* Complex addressing across multiple pointer levels */
    register int64_t* reg_ptr asm("r10") = ptr_array[0];
    volatile int64_t* volatile reg_vol_ptr asm("r11") = &volatile_buffer[0];
    
    for (int j = 0; j < depth; j++) {
        /* Pattern triggering RELOAD_FOR_OTHER_ADDRESS */
        int64_t offset = (j * 7) & 0x3F;
        asm volatile (
            "mov (%[base], %[off], 8), %[tmp]\n\t"
            "add %[tmp], (%[vol], %[off], 8)\n\t"
            : [tmp] "=&r" (reg_a)
            : [base] "r" (reg_ptr),
              [off] "r" (offset),
              [vol] "r" (reg_vol_ptr)
            : "memory", "cc"
        );
        
        /* Switch register bindings to force spills/reloads */
        asm volatile (
            "mov %[old], %[new]\n\t"
            : [new] "=r" (reg_ptr)
            : [old] "r" (reg_vol_ptr)
            : 
        );
        
        /* Alternate between pointer types */
        reg_vol_ptr = (volatile int64_t*)reg_ptr;
        results[j] = reg_a;
    }
    
    /* Recursive call with different parameters */
    nested_addressing(depth - 1, results + depth);
}

int main() {
    /* Initialize data */
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 8; j++) {
            global_array[i].data[j] = i * 100 + j;
        }
        global_array[i].flags = i;
    }
    
    /* Initialize register variables */
    reg_a = 0x1000;
    reg_b = 0x2000;
    reg_c = 0x3000;
    reg_d = 0x4000;
    reg_e = 0x5000;
    reg_f = 0x6000;
    
    int64_t results[32];
    
    /* First pass: complex addressing in loops */
    process_with_reloads(8, 3);
    
    /* Second pass: nested addressing with recursion */
    nested_addressing(5, results);
    
    /* Third pass: mixed immediate/memory/register constraints */
    for (int i = 0; i < 4; i++) {
        /* Force RELOAD_FOR_OPERAND_ADDRESS */
        struct ComplexStruct* s = &global_array[i];
        const int imm = 0x100 + i * 0x40;
        
        asm volatile (
            "mov %[idx], %%rax\n\t"
            "shl $3, %%rax\n\t"
            "add %[struct], %%rax\n\t"
            "mov (%%rax), %[out]\n\t"
            "add %[imm], %[out]\n\t"
            : [out] "=r" (results[i])
            : [struct] "r" (s),
              [idx] "r" (i),
              [imm] "i" (imm)  /* Immediate that may need reload */
            : "rax", "cc", "memory"
        );
    }
    
    /* Use results to prevent optimization */
    int64_t sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += results[i];
    }
    
    return (int)(sum & 0x7FFFFFFF);
}
