/* reload1_trigger.c
 * Designed to trigger specific reload types in GCC's reload pass
 * Compile with: gcc -O2 -fno-omit-frame-pointer -fno-schedule-insns -fno-schedule-insns2 -c reload1_trigger.c
 */

#include <stdint.h>

/* Complex structure to force address computations */
struct ComplexData {
    int64_t a;
    int64_t b;
    int64_t c[4];
    volatile int64_t d[3];
    struct {
        int64_t x;
        volatile int64_t y;
    } nested;
};

/* Global arrays to create addressing pressure */
static struct ComplexData global_array[8];
static volatile int64_t volatile_buffer[256];

/* Explicit register variables - force specific register allocation */
register int64_t reg_a asm("r10");
register int64_t reg_b asm("r11");
register int64_t reg_c asm("r12");
register int64_t reg_d asm("r13");
register int64_t reg_e asm("r14");
register int64_t reg_f asm("r15");

/* Function with complex addressing patterns */
void trigger_reloads(int iterations) {
    /* Local variables that will conflict with register variables */
    int64_t local_index = 0;
    int64_t local_offset = 8;
    volatile int64_t* volatile_ptr = volatile_buffer;
    struct ComplexData* data_ptr = global_array;
    
    /* Mixed immediate and memory operands */
    const int64_t immediate_const = 0x12345678;
    int64_t dynamic_values[4] = {1, 2, 3, 4};
    
    /* Loop with nested address computations */
    for (int i = 0; i < iterations; i++) {
        /* Force RELOAD_FOR_INPUT and RELOAD_FOR_INPUT_ADDRESS */
        asm volatile (
            "/* Complex memory operation with register pressure */\n\t"
            "add %[dst], %[src1], %[src2]\n\t"
            "mov %[tmp], %[imm]\n\t"
            : [dst] "=r" (reg_a), [tmp] "=r" (reg_b)
            : [src1] "r" (reg_c), 
              [src2] "m" (*(volatile int64_t*)(volatile_ptr + local_index)),
              [imm] "i" (immediate_const)
            : "memory"
        );
        
        /* Force RELOAD_FOR_OTHER_ADDRESS - complex address computation */
        /* This creates a situation where we need an address reload for 
           an operand that's not a simple input/output */
        int64_t complex_offset = local_index * 16 + reg_d;
        asm volatile (
            "/* Operation requiring other address reload */\n\t"
            "lea (%[base], %[idx], 8), %[addr]\n\t"
            "mov (%[addr]), %[val]\n\t"
            : [addr] "=r" (reg_e), [val] "=r" (reg_f)
            : [base] "r" (data_ptr), 
              [idx] "r" (complex_offset),
              "m" (*(struct ComplexData*)data_ptr)
            : "memory"
        );
        
        /* Force RELOAD_FOR_INPADDR_ADDRESS and RELOAD_FOR_OUTPUT_ADDRESS */
        /* Multiple memory references with different addressing modes */
        int64_t* array_ptr = dynamic_values;
        asm volatile (
            "/* Multiple memory operations with address computations */\n\t"
            "mov %[out1], (%[in1], %[in2], 4)\n\t"
            "mov (%[in3], %[in4]), %[out2]\n\t"
            : [out1] "=r" (reg_a), [out2] "=r" (reg_b)
            : [in1] "r" (array_ptr),
              [in2] "r" (local_index),
              [in3] "r" (volatile_ptr),
              [in4] "r" (reg_c),
              "m" (*(int64_t*)array_ptr),
              "m" (*(volatile int64_t*)volatile_ptr)
            : "memory"
        );
        
        /* Force RELOAD_FOR_OPERAND_ADDRESS and RELOAD_FOR_OPADDR_ADDR */
        /* Complex operand addressing with structure fields */
        struct ComplexData local_struct;
        int64_t struct_offset = offsetof(struct ComplexData, nested.y);
        
        asm volatile (
            "/* Operand address reload for structure field */\n\t"
            "mov %[offset](%[struct]), %[result]\n\t"
            "add %[result], %[inc], %[result]\n\t"
            : [result] "=r" (reg_d)
            : [struct] "r" (&local_struct),
              [offset] "r" (struct_offset),
              [inc] "r" (reg_e),
              "m" (local_struct.nested.y)
            : "memory"
        );
        
        /* Force RELOAD_FOR_OUTADDR_ADDRESS */
        /* Output address computation */
        int64_t* output_buffer = volatile_buffer + 128;
        asm volatile (
            "/* Output address computation */\n\t"
            "mov %[val], (%[base], %[index], 2)\n\t"
            : "=m" (*(int64_t*)(output_buffer + local_index))
            : [val] "r" (reg_f),
              [base] "r" (output_buffer),
              [index] "r" (local_index)
            : "memory"
        );
        
        /* Mix all register variables to create pressure */
        asm volatile (
            "/* Register shuffling to force spills/reloads */\n\t"
            "add %0, %1, %2\n\t"
            "sub %3, %4, %5\n\t"
            : "+r" (reg_a), "+r" (reg_b), "+r" (reg_c)
            : "r" (reg_d), "r" (reg_e), "r" (reg_f)
        );
        
        /* Update indices with complex computations */
        local_index = (local_index * 3 + 1) & 0xF;
        local_offset += reg_a & 0x7;
        
        /* Access volatile memory with complex addressing */
        volatile_ptr = volatile_buffer + ((reg_b + local_offset) & 0xFF);
        
        /* Structure pointer arithmetic */
        data_ptr = global_array + ((reg_c >> 3) & 0x7);
    }
}

/* Secondary function with different patterns */
void more_complex_addressing(int count) {
    /* Multi-dimensional array access */
    int64_t matrix[8][8];
    register int64_t idx1 asm("r10");
    register int64_t idx2 asm("r11");
    
    for (int i = 0; i < count; i++) {
        idx1 = (i * 7) & 0x7;
        idx2 = (i * 3) & 0x7;
        
        /* Complex addressing with multiple index registers */
        asm volatile (
            "/* Multi-dimensional array access */\n\t"
            "mov (%[base], %[idx1], 8), %[row]\n\t"
            "mov (%[row], %[idx2], 8), %[val]\n\t"
            : [row] "=r" (reg_a), [val] "=r" (reg_b)
            : [base] "r" (matrix),
              [idx1] "r" (idx1),
              [idx2] "r" (idx2),
              "m" (matrix[0][0])
            : "memory"
        );
        
        /* Immediate, register, and memory mix */
        asm volatile (
            "/* Mixed operand types */\n\t"
            "imul %[dest], %[src], %[imm]\n\t"
            : [dest] "=r" (reg_c)
            : [src] "irm" (reg_b),  /* Can be immediate, register, or memory */
              [imm] "i" (0x1234)
        );
    }
}

int main() {
    /* Initialize some data */
    for (int i = 0; i < 256; i++) {
        volatile_buffer[i] = i;
    }
    
    /* Call functions with different complexities */
    trigger_reloads(16);
    more_complex_addressing(8);
    
    /* Final barrier */
    asm volatile ("" : : : "memory");
    
    return 0;
}
