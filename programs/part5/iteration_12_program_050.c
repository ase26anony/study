/* reload_test.c - Complex addressing mode test for GCC reload pass */
#include <stdint.h>
#include <stdlib.h>

/* Volatile structure to prevent optimization */
struct ComplexData {
    volatile int64_t a;
    volatile int32_t b[4];
    volatile int16_t c[8];
    volatile int8_t d[16];
};

/* Global arrays to create addressing complexity */
struct ComplexData global_array[32];
volatile int64_t global_buffer[256];

/* Explicit register variables - force specific register allocation */
register int64_t reg_a asm("r10");
register int64_t reg_b asm("r11");
register int64_t reg_c asm("r12");
register int64_t reg_d asm("r13");
register int64_t reg_e asm("r14");
register int64_t reg_f asm("r15");

/* Function with complex addressing patterns */
void complex_addressing_test(int iterations) {
    /* Local variables with mixed types */
    int32_t local_index = 0;
    volatile int64_t* volatile_ptr = (volatile int64_t*)global_buffer;
    struct ComplexData* struct_ptr = &global_array[0];
    
    /* Initialize register variables */
    reg_a = (int64_t)&global_array[0];
    reg_b = (int64_t)&global_buffer[0];
    reg_c = 8;  /* stride */
    reg_d = 16; /* offset */
    
    for (int i = 0; i < iterations; i++) {
        /* Pattern 1: RELOAD_FOR_OTHER_ADDRESS and RELOAD_FOR_INPUT_ADDRESS */
        /* Complex memory addressing with multiple register components */
        asm volatile (
            "movq (%[base], %[idx], 8), %[temp]\n\t"
            "addq %[offset], %[temp]\n\t"
            "movq %[temp], (%[dest], %[stride], 4)"
            : [temp] "=&r" (reg_f)
            : [base] "r" (reg_a), 
              [idx] "r" (reg_c),
              [offset] "irm" (reg_d),  /* Mixed constraint: immediate, register, or memory */
              [dest] "r" (reg_b),
              [stride] "r" (reg_e)
            : "memory"
        );
        
        /* Pattern 2: RELOAD_FOR_INPADDR_ADDRESS and RELOAD_FOR_OPERAND_ADDRESS */
        /* Nested address computation with memory constraint */
        int64_t computed_addr;
        asm volatile (
            "lea (%[ptr], %[scale], 4), %[addr]\n\t"
            "movq (%[addr]), %[val]"
            : [addr] "=r" (computed_addr),
              [val] "=r" (reg_e)
            : [ptr] "r" (struct_ptr),
              [scale] "r" (local_index),
              "m" (*(struct ComplexData*)struct_ptr)  /* Force address reload */
            : "cc"
        );
        
        /* Pattern 3: RELOAD_FOR_OUTPUT_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
        /* Output address reload with complex index */
        volatile int64_t* output_addr;
        asm volatile (
            "movq %[data], (%[out], %[idx], 2)"
            : "=m" (*(volatile int64_t*)output_addr)  /* Memory output */
            : [out] "r" (reg_b),
              [idx] "r" (reg_c),
              [data] "r" (reg_a),
              "m" (*output_addr)  /* Input memory */
            : "memory"
        );
        
        /* Pattern 4: RELOAD_FOR_OPADDR_ADDR with mixed operand types */
        /* Immediate, register, and memory constraints in one asm */
        int64_t immediate_val = 0x12345678;
        asm volatile (
            "imulq %[imm], %[reg], %[res]\n\t"
            "addq (%[mem]), %[res]"
            : [res] "=r" (reg_f)
            : [reg] "r" (reg_d),
              [imm] "irm" (immediate_val),  /* Can be immediate or register */
              [mem] "r" (&global_buffer[i % 256])  /* Memory address that needs reload */
            : "cc", "memory"
        );
        
        /* Pattern 5: Multiple overlapping register usage forcing spills/reloads */
        /* Use all explicit registers in different combinations */
        asm volatile (
            "movq %%r10, %%rax\n\t"
            "addq %%r11, %%rax\n\t"
            "subq %%r12, %%rax\n\t"
            "movq %%rax, %%r13\n\t"
            "xorq %%r14, %%r15"
            : 
            : 
            : "rax", "r10", "r11", "r12", "r13", "r14", "r15", "cc"
        );
        
        /* Complex array indexing with multiple dimensions */
        /* This creates addressing modes that may need various reload types */
        int64_t* multi_array[4][8];
        for (int x = 0; x < 4; x++) {
            for (int y = 0; y < 8; y++) {
                /* Address computation involving multiple registers */
                int64_t index = (reg_c * x + reg_d * y) % 256;
                volatile_ptr = &global_buffer[index];
                
                /* Inline asm that uses the computed address */
                asm volatile (
                    "movq (%[addr]), %[val]\n\t"
                    "addq %[inc], %[val]\n\t"
                    "movq %[val], (%[addr])"
                    : [val] "=&r" (reg_f)
                    : [addr] "r" (volatile_ptr),
                      [inc] "r" (reg_e),
                      "m" (*volatile_ptr)
                    : "memory"
                );
            }
        }
        
        /* Update indices and pointers for next iteration */
        local_index = (local_index + 1) % 32;
        struct_ptr = &global_array[local_index];
        reg_c = (reg_c + 1) % 16;
        reg_d = (reg_d + 3) % 32;
        reg_e = (reg_e + 5) % 64;
        
        /* Compiler barrier to prevent optimization of addressing */
        asm volatile ("" : : : "memory");
    }
}

/* Secondary function with different addressing patterns */
void nested_address_computation(int depth) {
    if (depth <= 0) return;
    
    /* Stack-based array with complex indexing */
    int64_t stack_array[64];
    volatile int64_t* ptr_array[16];
    
    /* Initialize pointer array with complex addresses */
    for (int i = 0; i < 16; i++) {
        ptr_array[i] = &stack_array[(i * reg_c) % 64];
    }
    
    /* Loop with nested addressing */
    for (int i = 0; i < depth; i++) {
        /* RELOAD_FOR_OTHER_ADDRESS pattern */
        int64_t* base_ptr = (int64_t*)&ptr_array[0];
        int64_t offset = i * sizeof(int64_t*);
        
        asm volatile (
            "movq (%[base], %[off], 1), %[ptr]\n\t"
            "movq (%[ptr]), %[val]"
            : [ptr] "=r" (reg_a),
              [val] "=r" (reg_b)
            : [base] "r" (base_ptr),
              [off] "r" (offset),
              "m" (*(int64_t**)base_ptr)  /* Force address reload */
            : "memory"
        );
        
        /* Use the value in another asm with different constraints */
        asm volatile (
            "addq %[val], %[acc]"
            : [acc] "+r" (reg_c)
            : [val] "irm" (reg_b)  /* Mixed constraint */
            : "cc"
        );
    }
    
    /* Recursive call with reduced depth */
    nested_address_computation(depth - 1);
}

int main() {
    /* Initialize global data */
    for (int i = 0; i < 32; i++) {
        global_array[i].a = i;
        for (int j = 0; j < 4; j++) {
            global_array[i].b[j] = i * j;
        }
    }
    
    for (int i = 0; i < 256; i++) {
        global_buffer[i] = i * 2;
    }
    
    /* Run tests with different complexities */
    complex_addressing_test(8);  /* Small iteration count but complex patterns */
    nested_address_computation(4);
    
    /* Final barrier */
    asm volatile ("" : : : "memory");
    
    return 0;
}
