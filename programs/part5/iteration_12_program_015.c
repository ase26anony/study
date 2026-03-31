/* reload1_trigger.c
 * Designed to trigger specific reload types in GCC's reload pass:
 * - RELOAD_FOR_OTHER_ADDRESS
 * - RELOAD_FOR_INPUT_ADDRESS
 * - RELOAD_FOR_INPADDR_ADDRESS
 * - RELOAD_FOR_OPADDR_ADDR
 * - And other address reload types
 */

#include <stdint.h>
#include <stdlib.h>

/* Complex structure to force non-trivial addressing */
struct ComplexStruct {
    int64_t data[8];
    int32_t indices[4];
    volatile int16_t flags;
    char padding[7];
};

/* Multi-dimensional array with volatile elements */
volatile struct ComplexStruct global_array[4][3];

/* Explicit register variables - will force specific register allocation */
register uint64_t reg_a asm("r10");
register uint64_t reg_b asm("r11");
register uint64_t reg_c asm("r12");
register uint64_t reg_d asm("r13");
register uint64_t reg_e asm("r14");

/* Function with complex addressing patterns */
void complex_addressing_loop(int iterations) {
    /* Local arrays to create addressing pressure */
    int32_t index_array[16];
    volatile int64_t *volatile ptr_array[8];
    
    /* Initialize arrays */
    for (int i = 0; i < 16; i++) {
        index_array[i] = i * 3;
    }
    
    for (int i = 0; i < 8; i++) {
        ptr_array[i] = (volatile int64_t*)&global_array[i % 4][i % 3];
    }
    
    /* Use explicit register variables in computations */
    reg_a = (uint64_t)&global_array[0][0];
    reg_b = (uint64_t)index_array;
    reg_c = 0;
    
    /* Main loop with complex addressing */
    for (int i = 0; i < iterations; i++) {
        /* Pattern 1: Complex memory addressing with multiple registers */
        /* This should trigger RELOAD_FOR_INPUT_ADDRESS and RELOAD_FOR_OTHER_ADDRESS */
        asm volatile (
            /* Load from memory using complex address calculation */
            "movq (%[base], %[idx], 8), %[temp1]\n\t"
            /* Use the value in another operation */
            "addq %[temp1], %[sum]\n\t"
            /* Store with different addressing mode */
            "movq %[sum], (%[ptr], %[offset], 1)"
            : [sum] "+r" (reg_c), [temp1] "=&r" (reg_d)
            : [base] "r" (reg_a), 
              [idx] "r" (reg_b + i * 4),  /* Complex index calculation */
              [ptr] "r" ((uint64_t)ptr_array[i % 8]),
              [offset] "i" (16)           /* Immediate offset */
            : "memory", "cc"
        );
        
        /* Pattern 2: Nested address computation with fixed registers */
        /* Should trigger RELOAD_FOR_INPADDR_ADDRESS */
        volatile int64_t *volatile complex_ptr = 
            (volatile int64_t *volatile)((char*)&global_array[0][0] + i * 32);
        
        asm volatile (
            /* Compute address using multiple registers */
            "lea (%[base], %[scale], %[index]), %[addr]\n\t"
            /* Access memory at computed address */
            "movq (%[addr]), %[val]\n\t"
            /* Modify and store back */
            "addq $1, %[val]\n\t"
            "movq %[val], (%[addr])"
            : [addr] "=&r" (reg_e), [val] "=&r" (reg_d)
            : [base] "r" (reg_a),
              [scale] "r" (i * 8),
              [index] "r" (reg_b)
            : "memory", "cc"
        );
        
        /* Pattern 3: Mixed operand types with immediate and memory */
        /* Should trigger RELOAD_FOR_OPADDR_ADDR */
        struct ComplexStruct *cs_ptr = (struct ComplexStruct*)&global_array[i % 4][i % 3];
        
        asm volatile (
            /* Operation with mixed constraints */
            "imulq $0x%[imm], (%[mem]), %%r15\n\t"
            "addq %%r15, %[out]"
            : [out] "+r" (reg_c)
            : [mem] "r" (&cs_ptr->data[i % 8]),
              [imm] "i" (0x1234)  /* Immediate */
            : "r15", "memory", "cc"
        );
        
        /* Pattern 4: Forcing address reload for output */
        /* Should trigger RELOAD_FOR_OUTPUT_ADDRESS or RELOAD_FOR_OUTADDR_ADDRESS */
        int64_t output_buffer[4];
        
        asm volatile (
            /* Complex store operation */
            "movq %[in], (%[out_base], %[out_idx], 8)\n\t"
            /* Also store with offset */
            "movq %[in2], 32(%[out_base])"
            : 
            : [in] "r" (reg_c),
              [in2] "r" (reg_d),
              [out_base] "r" (output_buffer),
              [out_idx] "r" (i & 3)
            : "memory"
        );
        
        /* Pattern 5: Multiple memory operands with different addressing */
        /* Should trigger various address reload types */
        volatile int32_t *idx_ptr = &cs_ptr->indices[0];
        
        asm volatile (
            /* Multiple memory accesses with complex addressing */
            "movl (%[idx_ptr], %[offset], 4), %%eax\n\t"
            "addl %%eax, (%[flags_ptr])\n\t"
            "movl %[const], (%[idx_ptr], %[offset2], 4)"
            : 
            : [idx_ptr] "r" (idx_ptr),
              [flags_ptr] "r" (&cs_ptr->flags),
              [offset] "r" (i % 4),
              [offset2] "r" ((i + 1) % 4),
              [const] "i" (0xABCD)
            : "rax", "memory", "cc"
        );
    }
}

/* Another function with different addressing patterns */
void nested_addressing(int depth) {
    /* Multi-level pointer indirection */
    volatile struct ComplexStruct ***ptr3 = 
        (volatile struct ComplexStruct ***)malloc(sizeof(void*) * 4);
    
    for (int i = 0; i < 4; i++) {
        ptr3[i] = (volatile struct ComplexStruct **)malloc(sizeof(void*) * 3);
        for (int j = 0; j < 3; j++) {
            ptr3[i][j] = &global_array[i][j];
        }
    }
    
    /* Complex addressing through multiple pointer levels */
    for (int i = 0; i < depth; i++) {
        /* This triple indirection should require address reloads */
        asm volatile (
            /* Load through multiple pointer levels */
            "movq (%[ptr]), %%rax\n\t"
            "movq (%%rax, %[idx1], 8), %%rbx\n\t"
            "movq (%%rbx, %[idx2], 8), %%rcx\n\t"
            "movq (%%rcx), %[result]"
            : [result] "=r" (reg_a)
            : [ptr] "r" (ptr3),
              [idx1] "r" (i % 4),
              [idx2] "r" ((i * 2) % 3)
            : "rax", "rbx", "rcx", "memory"
        );
        
        /* Use the result in another complex operation */
        asm volatile (
            "lea (%[base], %[index], %[scale]), %[addr]\n\t"
            "movq %[val], (%[addr])"
            : [addr] "=&r" (reg_b)
            : [base] "r" (reg_a),
              [index] "r" (i * 16),
              [scale] "i" (1),
              [val] "r" (reg_c)
            : "memory"
        );
    }
    
    /* Cleanup */
    for (int i = 0; i < 4; i++) {
        free((void*)ptr3[i]);
    }
    free(ptr3);
}

/* Function with inline assembly that uses operand addresses */
void operand_address_reloads(void) {
    /* Array of different types to force various addressing modes */
    char byte_array[64];
    int32_t word_array[16];
    int64_t dword_array[8];
    
    /* Initialize */
    for (int i = 0; i < 64; i++) byte_array[i] = i;
    for (int i = 0; i < 16; i++) word_array[i] = i * 100;
    for (int i = 0; i < 8; i++) dword_array[i] = i * 1000;
    
    /* Complex pattern mixing different array types */
    for (int i = 0; i < 8; i++) {
        /* Should trigger RELOAD_FOR_OPERAND_ADDRESS */
        asm volatile (
            /* Mix byte and word accesses */
            "movzbl (%[bytes], %[idx]), %%eax\n\t"
            "addl (%[words], %[idx2], 4), %%eax\n\t"
            "movl %%eax, (%[dwords], %[idx3], 8)"
            : 
            : [bytes] "r" (byte_array),
              [words] "r" (word_array),
              [dwords] "r" (dword_array),
              [idx] "r" (i * 2),
              [idx2] "r" (i),
              [idx3] "r" (i)
            : "rax", "memory", "cc"
        );
    }
}

int main(void) {
    /* Initialize global array */
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 3; j++) {
            for (int k = 0; k < 8; k++) {
                global_array[i][j].data[k] = i * 100 + j * 10 + k;
            }
            for (int k = 0; k < 4; k++) {
                global_array[i][j].indices[k] = k;
            }
            global_array[i][j].flags = 0;
        }
    }
    
    /* Execute functions with different addressing patterns */
    complex_addressing_loop(16);
    nested_addressing(8);
    operand_address_reloads();
    
    /* Final operation to use all register variables */
    asm volatile (
        "addq %[a], %[b]\n\t"
        "addq %[c], %[b]\n\t"
        "movq %[b], %[result]"
        : [result] "=r" (reg_a)
        : [a] "r" (reg_a),
          [b] "r" (reg_b),
          [c] "r" (reg_c)
        : "cc"
    );
    
    return (int)reg_a;
}
