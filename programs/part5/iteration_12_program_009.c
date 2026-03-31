/* reload_test.c - Complex addressing mode test for GCC reload pass coverage */
#include <stdint.h>
#include <stdlib.h>

/* Force specific registers to be used for certain variables */
register uint64_t reg_a asm("r10");
register uint64_t reg_b asm("r11");
register uint64_t reg_c asm("r12");
register uint64_t reg_d asm("r13");
register uint64_t reg_e asm("r14");

/* Complex data structures to force various addressing modes */
struct Nested {
    volatile uint64_t data[8];
    volatile uint64_t* ptr;
    volatile uint64_t index;
};

struct Outer {
    struct Nested arrays[4];
    volatile uint64_t base;
    volatile uint64_t offset;
};

/* Volatile globals to prevent optimization */
volatile struct Outer global_struct;
volatile uint64_t global_array[256];
volatile uint64_t* volatile global_ptr;

/* Function with complex addressing patterns */
void complex_addressing_test(uint64_t iter_count) {
    /* Local variables bound to registers */
    register uint64_t idx1 asm("r15") = 0;
    register uint64_t idx2 asm("rbx") = 0;
    uint64_t temp_results[8];
    
    /* Initialize global structures */
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 8; j++) {
            global_struct.arrays[i].data[j] = i * 100 + j;
        }
        global_struct.arrays[i].ptr = &global_array[i * 64];
        global_struct.arrays[i].index = i * 16;
    }
    global_struct.base = (uint64_t)&global_array[0];
    global_struct.offset = 64;
    
    global_ptr = &global_array[128];
    
    /* Main loop with complex addressing */
    for (uint64_t iter = 0; iter < iter_count; iter++) {
        /* Pattern 1: RELOAD_FOR_OTHER_ADDRESS with multiple constraints */
        /* Complex memory addressing with register constraints that conflict */
        asm volatile (
            /* Operation using reg_a as both input and address base */
            "lea (%[base], %[idx1], 8), %[temp1]\n\t"
            "mov (%[temp1]), %[temp2]\n\t"
            "add %[temp2], %[reg_a]\n\t"
            /* Force address reload for another operand */
            "mov %[reg_b], (%[reg_c], %[idx2], 4)\n\t"
            : [reg_a] "+r" (reg_a), [temp1] "=&r" (temp_results[0]),
              [temp2] "=&r" (temp_results[1])
            : [base] "r" (global_struct.base), [idx1] "r" (idx1),
              [reg_b] "r" (reg_b), [reg_c] "r" (reg_c), [idx2] "r" (idx2),
              "m" (global_struct), "m" (*global_ptr)
            : "memory", "cc"
        );
        
        /* Pattern 2: RELOAD_FOR_INPUT_ADDRESS and RELOAD_FOR_INPADDR_ADDRESS */
        /* Multiple memory operands with different addressing requirements */
        uint64_t* volatile ptr1 = (uint64_t*)&global_struct.arrays[0];
        uint64_t* volatile ptr2 = (uint64_t*)&global_struct.arrays[2];
        
        asm volatile (
            /* Complex addressing with structure member access */
            "mov %[offset], %%rax\n\t"
            "mov (%[ptr1], %%rax, 8), %[temp3]\n\t"
            /* Address computation for second memory operand */
            "lea (%[ptr2], %[reg_d], 8), %%rbx\n\t"
            "add (%[mem]), %%rbx\n\t"
            "mov %[temp3], (%%rbx)\n\t"
            : [temp3] "=&r" (temp_results[2])
            : [ptr1] "r" (ptr1), [ptr2] "r" (ptr2),
              [offset] "r" (global_struct.offset),
              [reg_d] "r" (reg_d), [mem] "m" (global_struct.arrays[1].data[0])
            : "rax", "rbx", "memory", "cc"
        );
        
        /* Pattern 3: RELOAD_FOR_OUTPUT_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
        /* Output operands with memory constraints requiring address reloads */
        volatile uint64_t output_buffer[16];
        uint64_t computed_addr;
        
        asm volatile (
            /* Compute address for output */
            "lea (%[base2], %[idx1], 2), %[out_addr]\n\t"
            /* Store using computed address */
            "mov %[reg_e], (%[out_addr])\n\t"
            /* Another output with different addressing */
            "mov %[reg_a], %[offset2](%[base3])\n\t"
            : [out_addr] "=&r" (computed_addr),
              [offset2] "+m" (output_buffer[4])
            : [base2] "r" (&output_buffer[0]), [idx1] "r" (idx1),
              [reg_e] "r" (reg_e), [reg_a] "r" (reg_a),
              [base3] "r" (&output_buffer[8])
            : "memory", "cc"
        );
        
        /* Pattern 4: RELOAD_FOR_OPERAND_ADDRESS and RELOAD_FOR_OPADDR_ADDR */
        /* Mixed immediate, register, and memory constraints */
        const uint64_t immediate_const = 0x12345678;
        volatile uint64_t* addr_array[4];
        
        for (int i = 0; i < 4; i++) {
            addr_array[i] = &global_struct.arrays[i].data[0];
        }
        
        asm volatile (
            /* Multiple constraints forcing different reload types */
            "imul %[imm], %[reg_b], %[temp4]\n\t"
            "add (%[addr_ptr], %[idx2], 8), %[temp4]\n\t"
            "mov %[temp4], (%[mem_dest])\n\t"
            : [temp4] "=&r" (temp_results[3]),
              [mem_dest] "+m" (*(volatile uint64_t*)addr_array[idx2 % 4])
            : [imm] "i" (immediate_const), [reg_b] "r" (reg_b),
              [idx2] "r" (idx2), [addr_ptr] "r" (addr_array),
              "m" (addr_array[0][0]), "m" (addr_array[1][0])
            : "memory", "cc"
        );
        
        /* Pattern 5: RELOAD_FOR_INPUT with complex constraints */
        /* Multiple input operands requiring different handling */
        struct Nested* volatile nested_ptr = &global_struct.arrays[idx1 % 4];
        
        asm volatile (
            /* Operation with multiple constrained inputs */
            "mov %[nested_idx], %%rax\n\t"
            "mov (%[nested_ptr], %%rax, 8), %[temp5]\n\t"
            "add %[reg_c], %[temp5]\n\t"
            "mov %[temp5], %[reg_d]\n\t"
            : [reg_d] "=r" (reg_d), [temp5] "=&r" (temp_results[4])
            : [nested_ptr] "r" (nested_ptr),
              [nested_idx] "r" (nested_ptr->index),
              [reg_c] "r" (reg_c), "m" (*nested_ptr)
            : "rax", "memory", "cc"
        );
        
        /* Update indices for next iteration */
        idx1 = (idx1 + 1) % 16;
        idx2 = (idx2 + 3) % 16;
        reg_b = reg_a + idx1;
        reg_c = reg_d + idx2;
        reg_e = reg_b * reg_c;
        
        /* Compiler barrier to prevent optimization */
        asm volatile ("" : : : "memory");
    }
}

/* Secondary function with different patterns */
void secondary_test(uint64_t count) {
    volatile uint64_t matrix[8][8];
    register uint64_t i asm("r10") = 0;
    register uint64_t j asm("r11") = 0;
    
    /* Initialize matrix */
    for (i = 0; i < 8; i++) {
        for (j = 0; j < 8; j++) {
            matrix[i][j] = i * 8 + j;
        }
    }
    
    /* Complex addressing in nested loops */
    for (uint64_t iter = 0; iter < count; iter++) {
        for (i = 0; i < 7; i++) {
            for (j = 0; j < 7; j++) {
                /* Pattern causing RELOAD_FOR_OTHER_ADDRESS */
                uint64_t* row_ptr = (uint64_t*)&matrix[i][0];
                uint64_t* next_row_ptr = (uint64_t*)&matrix[i+1][0];
                
                asm volatile (
                    /* Cross-row addressing requiring address reloads */
                    "mov (%[row], %[j], 8), %%rax\n\t"
                    "add (%[next_row], %[j], 8), %%rax\n\t"
                    "mov %%rax, %[j](%[row], 8)\n\t"
                    : 
                    : [row] "r" (row_ptr), [next_row] "r" (next_row_ptr),
                      [j] "r" (j)
                    : "rax", "memory", "cc"
                );
                
                /* Another complex pattern */
                asm volatile (
                    /* Multiple memory operands with offset calculations */
                    "lea (%[row], %[j], 8), %%rbx\n\t"
                    "mov 8(%%rbx), %%rcx\n\t"
                    "sub -8(%%rbx), %%rcx\n\t"
                    "mov %%rcx, (%[dest])\n\t"
                    : 
                    : [row] "r" (row_ptr), [j] "r" (j),
                      [dest] "m" (matrix[i][j])
                    : "rbx", "rcx", "memory", "cc"
                );
            }
        }
    }
}

int main() {
    /* Initialize register variables */
    reg_a = 0x1000;
    reg_b = 0x2000;
    reg_c = 0x3000;
    reg_d = 0x4000;
    reg_e = 0x5000;
    
    /* Run tests with different iteration counts */
    complex_addressing_test(100);
    secondary_test(50);
    
    /* Final compiler barrier */
    asm volatile ("" : : : "memory");
    
    /* Return something based on register values (prevents dead code elimination) */
    return (reg_a + reg_b + reg_c + reg_d + reg_e) % 256;
}
