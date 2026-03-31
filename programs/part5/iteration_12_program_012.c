/* reload_trigger.c
 * Designed to trigger specific reload types in GCC's reload pass
 * Compile with: gcc -O2 -fno-omit-frame-pointer -fno-schedule-insns -fno-schedule-insns2 -c reload_trigger.c
 */

#include <stdint.h>

/* Complex structure to force non-trivial addressing */
struct ComplexData {
    int64_t a[8];
    int32_t b[16];
    int16_t c[32];
    int8_t d[64];
};

/* Volatile structure to prevent optimization */
volatile struct ComplexData global_data;

/* Explicit register variables - using x86-64 registers */
register int64_t reg_a asm("r10");
register int64_t reg_b asm("r11");
register int64_t reg_c asm("r12");
register int64_t reg_d asm("r13");
register int64_t reg_e asm("r14");
register int64_t reg_f asm("r15");

/* Function with complex addressing modes */
void trigger_reloads(void) {
    /* Local arrays with different alignments */
    int64_t array1[128] __attribute__((aligned(64)));
    int32_t array2[256] __attribute__((aligned(32)));
    int16_t array3[512] __attribute__((aligned(16)));
    
    /* Pointers that will need address reloads */
    volatile int64_t *volatile ptr1 = array1;
    volatile int32_t *volatile ptr2 = array2;
    volatile int16_t *volatile ptr3 = array3;
    
    /* Initialize explicit register variables */
    reg_a = (int64_t)array1;
    reg_b = (int64_t)array2;
    reg_c = (int64_t)array3;
    reg_d = 0;
    reg_e = 0;
    reg_f = 0;
    
    /* Loop with complex addressing computations */
    for (int i = 0; i < 100; i++) {
        /* Complex address computation involving multiple registers */
        int64_t idx1 = i * 3 + reg_d;
        int64_t idx2 = i * 5 + reg_e;
        int64_t idx3 = i * 7 + reg_f;
        
        /* Pattern 1: Mixed operand types with memory constraint */
        /* Forces RELOAD_FOR_INPUT_ADDRESS and RELOAD_FOR_OTHER_ADDRESS */
        asm volatile (
            "lea (%[base], %[idx], 8), %[temp]\n\t"
            "mov (%[temp]), %[out1]\n\t"
            "add %[imm], %[out1]"
            : [out1] "=r" (reg_d)
            : [base] "r" (reg_a), 
              [idx] "r" (idx1),
              [imm] "i" (42),
              [temp] "r" (reg_f)  /* Explicit temp register */
            : "memory"
        );
        
        /* Pattern 2: Nested address computation with multiple memory accesses */
        /* Forces RELOAD_FOR_INPADDR_ADDRESS and RELOAD_FOR_OPADDR_ADDR */
        asm volatile (
            "mov %[ptr], %[addr]\n\t"
            "mov (%[addr], %[offset], 4), %[val]\n\t"
            "imul %[scale], %[val]"
            : [val] "=r" (reg_e),
              [addr] "=&r" (reg_f)  /* Early clobber */
            : [ptr] "m" (*(volatile int64_t *)ptr2),  /* Memory constraint forces address reload */
              [offset] "r" (idx2),
              [scale] "r" (reg_d)
            : "memory"
        );
        
        /* Pattern 3: Complex addressing with immediate displacement */
        /* Forces RELOAD_FOR_OUTPUT_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
        int64_t complex_offset = idx3 * 2 + 16;
        asm volatile (
            "mov %[src], %%r8\n\t"
            "lea (%%r8, %[off]), %%r9\n\t"
            "mov (%%r9), %[result]\n\t"
            "add %[addend], %[result]"
            : [result] "=r" (reg_f)
            : [src] "r" (reg_c),
              [off] "r" (complex_offset),
              [addend] "r" (reg_e)
            : "r8", "r9", "memory"
        );
        
        /* Pattern 4: Multiple memory operands with register constraints */
        /* Forces RELOAD_FOR_OPERAND_ADDRESS */
        asm volatile (
            "mov (%[addr1]), %%r8\n\t"
            "mov (%[addr2], %[idx], 2), %%r9\n\t"
            "add %%r8, %%r9\n\t"
            "mov %%r9, %[out]"
            : [out] "=r" (reg_d)
            : [addr1] "r" (&array1[i & 127]),
              [addr2] "r" (&array3[i & 511]),
              [idx] "r" (reg_b)
            : "r8", "r9", "memory"
        );
        
        /* Pattern 5: Forcing RELOAD_FOR_OTHER_ADDRESS specifically */
        /* Complex addressing mode that doesn't fit standard patterns */
        struct ComplexData local_data;
        volatile struct ComplexData *volatile complex_ptr = &local_data;
        
        asm volatile (
            /* Compute address with multiple components */
            "mov %[base], %%r8\n\t"
            "lea (%[offset], %%r8, 1), %%r9\n\t"
            "mov %[index], %%r10\n\t"
            "shl $3, %%r10\n\t"
            "add %%r10, %%r9\n\t"
            /* Access memory at computed address */
            "mov (%%r9), %[result]"
            : [result] "=r" (reg_e)
            : [base] "r" (complex_ptr),
              [offset] "r" (offsetof(struct ComplexData, b)),
              [index] "r" (i & 15)
            : "r8", "r9", "r10", "memory"
        );
        
        /* Mix in global volatile accesses to prevent optimization */
        global_data.a[i & 7] = reg_d;
        global_data.b[i & 15] = reg_e;
        global_data.c[i & 31] = reg_f;
        
        /* Update indices for next iteration */
        reg_d = (reg_d * 1103515245 + 12345) & 0x7fffffff;
        reg_e = (reg_e * 1664525 + 1013904223) & 0x7fffffff;
        reg_f = (reg_f * 214013 + 2531011) & 0x7fffffff;
    }
    
    /* Final pattern: Inline assembly with many constraints */
    /* Forces multiple reload types simultaneously */
    int64_t final_result;
    asm volatile (
        "mov %[in1], %%r8\n\t"
        "mov %[in2], %%r9\n\t"
        "mov %[in3], %%r10\n\t"
        "lea (%%r8, %%r9, 4), %%r11\n\t"
        "add %%r10, %%r11\n\t"
        "mov (%%r11), %[out]"
        : [out] "=r" (final_result)
        : [in1] "r" (reg_a),
          [in2] "r" (reg_b),
          [in3] "m" (*(volatile int64_t *)&array1[64]),  /* Memory constraint */
          "[out]" "0" (reg_c)  /* Matching constraint */
        : "r8", "r9", "r10", "r11", "memory"
    );
}

/* Main function to set up and call the trigger */
int main(void) {
    /* Initialize global data */
    for (int i = 0; i < 8; i++) {
        global_data.a[i] = i * 100;
    }
    for (int i = 0; i < 16; i++) {
        global_data.b[i] = i * 10;
    }
    
    /* Call the function multiple times to increase reload pressure */
    for (int j = 0; j < 10; j++) {
        trigger_reloads();
    }
    
    return 0;
}
