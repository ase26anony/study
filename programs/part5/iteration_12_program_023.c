/* reload1_coverage.c
 * Designed to trigger specific reload types in GCC's reload pass
 * Target: lines 7146-7174 in reload1.cc
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Complex structure to force memory addressing */
struct ComplexData {
    int64_t a;
    int64_t b;
    int64_t c[8];
    volatile int64_t d[4];
    struct {
        int32_t x;
        int32_t y;
    } nested;
};

/* Explicit register variables - force specific register allocation */
register int64_t reg_a asm("r10");
register int64_t reg_b asm("r11");
register int64_t reg_c asm("r12");
register int64_t reg_d asm("r13");
register int64_t reg_e asm("r14");
register int64_t reg_f asm("r15");

/* Volatile pointers to prevent optimization */
volatile struct ComplexData* volatile ptr1;
volatile struct ComplexData* volatile ptr2;

/* Multi-dimensional array with complex access pattern */
int64_t multi_array[4][8][16];

/* Function with complex addressing modes */
void complex_addressing_loop(int iterations) {
    /* Local variables that will conflict with explicit registers */
    int64_t local_index = 0;
    int64_t local_offset = 8;
    int64_t local_stride = 16;
    
    /* Force these into registers that might conflict */
    register int64_t idx_reg asm("r8") = local_index;
    register int64_t off_reg asm("r9") = local_offset;
    
    /* Loop with nested address computations */
    for (int i = 0; i < iterations; i++) {
        /* Complex memory access pattern 1: RELOAD_FOR_INPUT_ADDRESS */
        asm volatile (
            "movq (%[base], %[idx], 8), %[temp]\n\t"
            "addq %[off], %[temp]\n\t"
            "movq %[temp], (%[dest])"
            : [temp] "=&r" (local_index)
            : [base] "r" (&multi_array[0][0][0]),
              [idx] "r" (idx_reg),
              [off] "r" (off_reg),
              [dest] "m" (*(volatile int64_t*)&reg_a)
            : "memory"
        );
        
        /* Pattern 2: RELOAD_FOR_OTHER_ADDRESS with explicit register conflict */
        asm volatile (
            "leaq (%[ptr], %[idx], 4), %[addr]\n\t"
            "movq (%[addr]), %[val]\n\t"
            "imulq %[scale], %[val]"
            : [addr] "=&r" (local_offset), [val] "=&r" (local_stride)
            : [ptr] "r" (ptr1),
              [idx] "r" (reg_b),  /* Uses explicit register variable */
              [scale] "i" (sizeof(struct ComplexData))  /* Immediate */
            : "r10", "r11", "memory"  /* Clobbers explicit registers */
        );
        
        /* Pattern 3: RELOAD_FOR_INPADDR_ADDRESS with mixed constraints */
        struct ComplexData local_struct;
        asm volatile (
            "movq %[in1], %%r10\n\t"
            "movq %[in2], %%r11\n\t"
            "addq %%r10, %%r11\n\t"
            "movq %%r11, %[out]"
            : [out] "=m" (local_struct.d[0])
            : [in1] "irm" (reg_c),  /* Immediate, register, or memory */
              [in2] "m" (*(struct ComplexData*)ptr2)  /* Memory constraint */
            : "r10", "r11", "memory"
        );
        
        /* Pattern 4: RELOAD_FOR_OPERAND_ADDRESS with volatile */
        volatile int64_t* volatile volatile_ptr = &local_struct.d[1];
        asm volatile (
            "movq (%[addr]), %%rax\n\t"
            "addq $1, %%rax\n\t"
            "movq %%rax, (%[addr])"
            :
            : [addr] "r" (volatile_ptr)
            : "rax", "memory"
        );
        
        /* Pattern 5: RELOAD_FOR_OPADDR_ADDR with complex index */
        int64_t complex_index = idx_reg * 2 + off_reg;
        asm volatile (
            "movq (%[base], %[idx], 8), %%r15\n\t"
            "addq %%r15, %[sum]"
            : [sum] "+r" (reg_f)
            : [base] "r" (&multi_array),
              [idx] "r" (complex_index)
            : "r15", "memory"
        );
        
        /* Update indices with dependencies to prevent optimization */
        idx_reg = (idx_reg + 1) & 0x7;
        off_reg = (off_reg * 3 + i) & 0xF;
        
        /* Force spill/reload of explicit registers */
        asm volatile (
            "xchgq %%r12, %%r13\n\t"
            "xchgq %%r14, %%r15"
            :
            :
            : "r12", "r13", "r14", "r15", "memory"
        );
    }
}

/* Function with output address reloads */
void output_address_patterns(int count) {
    int64_t output_buffer[256];
    volatile int64_t* out_ptr = output_buffer;
    
    for (int i = 0; i < count; i++) {
        /* RELOAD_FOR_OUTPUT_ADDRESS pattern */
        asm volatile (
            "movq %[data], (%[dest], %[idx], 8)"
            :
            : [data] "r" (reg_d),
              [dest] "r" (out_ptr),
              [idx] "r" (reg_e)  /* Index in register */
            : "memory"
        );
        
        /* RELOAD_FOR_OUTADDR_ADDRESS pattern with offset */
        int64_t offset = i * sizeof(int64_t);
        asm volatile (
            "movq %[val], %[off](%[base])"
            :
            : [val] "r" (reg_a),
              [off] "r" (offset),  /* Offset in register */
              [base] "r" (&output_buffer[128])  /* Base address */
            : "memory"
        );
        
        /* Mix in some arithmetic to create register pressure */
        reg_d = reg_d * 1103515245 + 12345;
        reg_e = (reg_e << 13) ^ reg_e;
    }
}

/* Main function that sets up the scenario */
int main(int argc, char** argv) {
    int iterations = 100;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    /* Initialize data structures */
    struct ComplexData data1, data2;
    ptr1 = &data1;
    ptr2 = &data2;
    
    /* Initialize explicit register variables */
    reg_a = 0x123456789ABCDEF0LL;
    reg_b = 0xFEDCBA9876543210LL;
    reg_c = 0x5555555555555555LL;
    reg_d = 0xAAAAAAAAAAAAAAAALL;
    reg_e = 0x3333333333333333LL;
    reg_f = 0xCCCCCCCCCCCCCCCCLL;
    
    /* Initialize multi-dimensional array */
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 8; j++) {
            for (int k = 0; k < 16; k++) {
                multi_array[i][j][k] = i * 100 + j * 10 + k;
            }
        }
    }
    
    /* Execute patterns to generate complex RTL */
    complex_addressing_loop(iterations);
    output_address_patterns(iterations / 2);
    
    /* Final volatile access to prevent dead code elimination */
    asm volatile ("" : : "r"(reg_a), "r"(reg_b), "r"(reg_c), 
                       "r"(reg_d), "r"(reg_e), "r"(reg_f) : "memory");
    
    printf("Completed %d iterations\n", iterations);
    return 0;
}
