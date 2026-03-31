/* reload_test.c - Complex addressing mode test for GCC reload pass */
#include <stdint.h>

/* Volatile struct to prevent optimization */
struct ComplexData {
    volatile int64_t a;
    volatile int32_t b[4];
    volatile int16_t c[8];
    volatile int8_t d[16];
};

/* Global data to force memory addressing */
struct ComplexData global_data[4];
volatile int64_t *volatile global_ptr = (int64_t*)&global_data[0];

/* Explicit register variables - tie to specific registers */
register int64_t reg_a asm("r10");
register int64_t reg_b asm("r11");
register int64_t reg_c asm("r12");
register int64_t reg_d asm("r13");
register int64_t reg_e asm("r14");
register int64_t reg_f asm("r15");

/* Function with complex addressing patterns */
void complex_addressing_test(int iterations) {
    /* Local arrays with different alignments */
    int64_t array1[256] __attribute__((aligned(64)));
    int32_t array2[512] __attribute__((aligned(32)));
    int16_t array3[1024] __attribute__((aligned(16)));
    
    /* Volatile pointers to force memory accesses */
    volatile int64_t *volatile ptr1 = array1;
    volatile int32_t *volatile ptr2 = array2;
    volatile int16_t *volatile ptr3 = array3;
    
    /* Initialize some data */
    for (int i = 0; i < 256; i++) {
        array1[i] = i * 3;
    }
    
    /* Loop with complex address computations */
    for (int i = 0; i < iterations; i++) {
        /* Force RELOAD_FOR_OTHER_ADDRESS and RELOAD_FOR_INPUT_ADDRESS */
        /* Complex addressing with multiple register components */
        asm volatile (
            /* Operation using explicit register variables for address computation */
            "lea (%[base], %[idx], 8), %[temp1]\n\t"
            "mov (%[temp1]), %[temp2]\n\t"
            "add %[temp2], %[reg_a]\n\t"
            : [reg_a] "+r" (reg_a), [temp1] "=&r" (reg_b), [temp2] "=&r" (reg_c)
            : [base] "r" (ptr1), [idx] "r" (i)
            : "memory", "cc"
        );
        
        /* Force RELOAD_FOR_INPADDR_ADDRESS and RELOAD_FOR_OPERAND_ADDRESS */
        /* Memory operand with complex addressing mode */
        int64_t offset = i * 16 + 8;
        asm volatile (
            "movq %[offset], %%rax\n\t"
            "addq %[ptr], %%rax\n\t"
            "movq (%%rax), %%rbx\n\t"
            "addq %%rbx, %[result]\n\t"
            : [result] "+r" (reg_d)
            : [ptr] "r" (ptr2), [offset] "irm" (offset)
            : "rax", "rbx", "memory", "cc"
        );
        
        /* Force RELOAD_FOR_OUTPUT_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
        /* Store operation with complex address */
        int64_t store_val = i * 7;
        int64_t store_idx = (i * 3) & 0xFF;
        asm volatile (
            "mov %[val], (%[base], %[idx], 4)\n\t"
            : 
            : [base] "r" (ptr3), [idx] "r" (store_idx), [val] "r" (store_val)
            : "memory"
        );
        
        /* Force RELOAD_FOR_OPADDR_ADDR */
        /* Nested address computation */
        struct ComplexData *cptr = &global_data[i & 3];
        int32_t complex_offset = (i * 11) & 0xF;
        
        asm volatile (
            /* Compute address of array element within struct */
            "mov %[cptr], %%rax\n\t"
            "lea 0x10(%%rax), %%rbx\n\t"      /* Start of b array */
            "mov %[off], %%rcx\n\t"
            "lea (%%rbx, %%rcx, 4), %%rdx\n\t" /* b[off] address */
            "mov (%%rdx), %%rsi\n\t"          /* Load b[off] */
            "add %%rsi, %[sum]\n\t"           /* Add to sum */
            : [sum] "+r" (reg_e)
            : [cptr] "r" (cptr), [off] "r" (complex_offset)
            : "rax", "rbx", "rcx", "rdx", "rsi", "memory", "cc"
        );
        
        /* Force RELOAD_FOR_OTHER_ADDRESS with mixed constraints */
        /* Multiple memory operands with different constraints */
        int64_t *mem_ptr1 = &array1[(i * 5) & 0xFF];
        int64_t *mem_ptr2 = &array1[(i * 7) & 0xFF];
        
        asm volatile (
            "mov (%[src]), %%rax\n\t"
            "add %%rax, (%[dst])\n\t"
            : 
            : [src] "r" (mem_ptr1), [dst] "r" (mem_ptr2)
            : "rax", "memory", "cc"
        );
        
        /* Complex loop-carried dependency chain */
        /* Using all explicit register variables in computation */
        asm volatile (
            "imul %[reg_b], %[reg_a]\n\t"
            "add %[reg_c], %[reg_a]\n\t"
            "sub %[reg_d], %[reg_a]\n\t"
            "add %[reg_e], %[reg_a]\n\t"
            "mov %[reg_a], %[reg_f]\n\t"
            : [reg_a] "+r" (reg_a), [reg_f] "=r" (reg_f)
            : [reg_b] "r" (reg_b), [reg_c] "r" (reg_c), 
              [reg_d] "r" (reg_d), [reg_e] "r" (reg_e)
            : "cc"
        );
    }
    
    /* Final barrier */
    asm volatile ("" : : : "memory");
}

/* Secondary function with different addressing patterns */
void mixed_operand_test(void) {
    /* Immediate, register, and memory mix */
    int64_t local_array[128];
    volatile int64_t accum = 0;
    
    /* Initialize */
    for (int i = 0; i < 128; i++) {
        local_array[i] = i * i;
    }
    
    /* Force various reload types with immediate constraints */
    for (int i = 0; i < 32; i++) {
        int64_t idx1 = i * 2;
        int64_t idx2 = i * 3;
        int64_t imm_val = 0x12345678;
        
        /* Mixed "irm" constraint with fixed register */
        asm volatile (
            "mov %[idx1], %%r8\n\t"
            "mov %[idx2], %%r9\n\t"
            "lea (%[array], %%r8, 8), %%r10\n\t"
            "mov (%%r10), %%r11\n\t"
            "add %[imm], %%r11\n\t"
            "lea (%[array], %%r9, 8), %%r12\n\t"
            "mov %%r11, (%%r12)\n\t"
            : 
            : [array] "r" (local_array), [idx1] "r" (idx1), 
              [idx2] "r" (idx2), [imm] "irm" (imm_val)
            : "r8", "r9", "r10", "r11", "r12", "memory", "cc"
        );
        
        /* Complex struct member access */
        struct {
            int64_t header;
            int32_t data[32];
            int64_t footer;
        } __attribute__((packed)) packed_struct;
        
        volatile int32_t *data_ptr = packed_struct.data;
        int32_t offset = i * sizeof(int32_t);
        
        /* This should trigger RELOAD_FOR_OTHER_ADDRESS */
        asm volatile (
            "mov %[ptr], %%rax\n\t"
            "add %[off], %%rax\n\t"
            "movl $0x55AA, (%%rax)\n\t"
            : 
            : [ptr] "r" (data_ptr), [off] "r" (offset)
            : "rax", "memory"
        );
    }
}

/* Main function that sets up and calls tests */
int main(void) {
    /* Initialize global data */
    for (int i = 0; i < 4; i++) {
        global_data[i].a = i * 100;
        for (int j = 0; j < 4; j++) {
            global_data[i].b[j] = i * 10 + j;
        }
    }
    
    /* Initialize explicit register variables */
    reg_a = 1;
    reg_b = 2;
    reg_c = 3;
    reg_d = 4;
    reg_e = 5;
    reg_f = 6;
    
    /* Run tests with different iteration counts */
    complex_addressing_test(100);
    mixed_operand_test();
    
    /* Use results to prevent dead code elimination */
    asm volatile (
        "add %[a], %[b]\n\t"
        "add %[c], %[d]\n\t"
        "add %[e], %[f]\n\t"
        : 
        : [a] "r" (reg_a), [b] "r" (reg_b),
          [c] "r" (reg_c), [d] "r" (reg_d),
          [e] "r" (reg_e), [f] "r" (reg_f)
        : "cc"
    );
    
    return 0;
}
