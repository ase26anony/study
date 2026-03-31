/* reload1_test.c - Complex addressing mode test for GCC reload pass */
#include <stdint.h>
#include <stdlib.h>

/* Volatile structure to prevent optimization */
struct ComplexData {
    volatile int64_t a;
    volatile int32_t b[4];
    volatile char c[32];
};

/* Global arrays to create addressing complexity */
struct ComplexData global_data[16];
volatile int64_t global_array[256];
volatile int32_t *volatile global_ptr;

/* Explicit register variables - using x86-64 registers */
register int64_t reg_a asm("r10");
register int64_t reg_b asm("r11");
register int64_t reg_c asm("r12");
register int64_t reg_d asm("r13");
register int64_t reg_e asm("r14");

/* Function with complex addressing patterns */
void test_reload_patterns(int iterations) {
    /* Local variables with different types */
    volatile int64_t local_base = 0x1000;
    volatile int32_t *addr_ptr = (int32_t*)&local_base;
    struct ComplexData *struct_ptr = &global_data[0];
    
    /* Multiple index calculations */
    int idx1 = 0, idx2 = 0, idx3 = 0;
    
    /* Loop with complex addressing */
    for (int i = 0; i < iterations; i++) {
        /* Force RELOAD_FOR_INPUT and RELOAD_FOR_INPUT_ADDRESS */
        asm volatile (
            "movq %[ptr], %%r15\n\t"
            "addl (%%r15, %[idx1], 4), %[out]\n\t"
            : [out] "+r" (reg_a)
            : [ptr] "irm" (addr_ptr), 
              [idx1] "r" (idx1)
            : "r15", "memory", "cc"
        );
        
        /* Force RELOAD_FOR_OTHER_ADDRESS */
        /* Complex memory access with multiple index registers */
        asm volatile (
            "leaq (%[base], %[idx2], 8), %%r15\n\t"
            "movq (%%r15, %[idx3], 4), %%rax\n\t"
            "addq %%rax, %[out]\n\t"
            : [out] "+r" (reg_b)
            : [base] "r" (struct_ptr),
              [idx2] "r" (idx2),
              [idx3] "r" (idx3),
              "m" (*(struct ComplexData(*)[16])global_data)
            : "rax", "r15", "memory", "cc"
        );
        
        /* Force RELOAD_FOR_INPADDR_ADDRESS and RELOAD_FOR_OUTPUT_ADDRESS */
        /* Mixed input/output with memory constraints */
        int64_t temp;
        asm volatile (
            "movq %[in1], %%r15\n\t"
            "imulq %[in2], %%r15\n\t"
            "movq %%r15, %[out]\n\t"
            "addq %%r15, (%[mem])\n\t"
            : [out] "=r" (temp),
              "+m" (global_array[idx1])
            : [in1] "r" (reg_c),
              [in2] "r" (reg_d),
              [mem] "r" (&global_array[idx1])
            : "r15", "memory", "cc"
        );
        reg_e = temp;
        
        /* Force RELOAD_FOR_OUTADDR_ADDRESS */
        /* Output with address computation */
        int64_t* output_addr = &global_array[idx2];
        asm volatile (
            "movq %[val], (%[addr])\n\t"
            : 
            : [val] "r" (reg_e),
              [addr] "r" (output_addr)
            : "memory"
        );
        
        /* Force RELOAD_FOR_OPERAND_ADDRESS and RELOAD_FOR_OPADDR_ADDR */
        /* Nested address computation */
        struct ComplexData* nested_ptr = &global_data[idx3];
        asm volatile (
            "movq 8(%[ptr]), %%r15\n\t"
            "addq %%r15, %[sum]\n\t"
            "movq %[sum], 16(%[ptr])\n\t"
            : [sum] "+r" (reg_a)
            : [ptr] "r" (nested_ptr),
              "m" (*nested_ptr)
            : "r15", "memory", "cc"
        );
        
        /* Force RELOAD_FOR_OTHER_ADDRESS with multiple constraints */
        /* Complex constraint mixing */
        int64_t offset = idx1 * 16 + idx2 * 8 + idx3 * 4;
        asm volatile (
            "movq (%[base], %[off], 1), %%r15\n\t"
            "addq %%r15, %[acc]\n\t"
            : [acc] "+r" (reg_b)
            : [base] "r" (global_array),
              [off] "r" (offset),
              "m" (global_array[offset])
            : "r15", "memory", "cc"
        );
        
        /* Update indices with complex patterns */
        idx1 = (idx1 + 1) & 15;
        idx2 = (idx2 * 3 + 1) & 7;
        idx3 = (idx3 + idx1) & 3;
        
        /* Use explicit register variables in computations */
        reg_c = reg_a + reg_b;
        reg_d = reg_c * 2 - reg_e;
    }
    
    /* Final barrier */
    asm volatile ("" : : : "memory");
}

/* Second function with different patterns */
void test_mixed_operands(void) {
    volatile int64_t buffer[32];
    volatile int32_t* aliased_ptr = (int32_t*)buffer;
    
    /* Force various reload types with immediate operands */
    for (int i = 0; i < 32; i++) {
        /* Mixed immediate, register, and memory */
        asm volatile (
            "addq $0x%c[imm], (%[mem], %[idx], 8)\n\t"
            : 
            : [mem] "r" (buffer),
              [idx] "r" (i),
              [imm] "i" (i * 2)
            : "memory", "cc"
        );
        
        /* RELOAD_FOR_OTHER_ADDRESS with struct member */
        struct ComplexData* sptr = &global_data[i & 3];
        int member_idx = i & 1;
        asm volatile (
            "movl %[val], (%[ptr], %[idx], 4)\n\t"
            : 
            : [ptr] "r" (&sptr->b[0]),
              [idx] "r" (member_idx),
              [val] "r" (i),
              "m" (sptr->b[member_idx])
            : "memory"
        );
    }
}

/* Main function */
int main(int argc, char** argv) {
    /* Initialize global data */
    for (int i = 0; i < 16; i++) {
        global_data[i].a = i * 100;
        for (int j = 0; j < 4; j++) {
            global_data[i].b[j] = i + j;
        }
    }
    
    for (int i = 0; i < 256; i++) {
        global_array[i] = i;
    }
    
    /* Initialize register variables */
    reg_a = 1;
    reg_b = 2;
    reg_c = 3;
    reg_d = 4;
    reg_e = 5;
    
    /* Set global pointer */
    global_ptr = (int32_t*)global_array;
    
    /* Run tests with different iteration counts */
    test_reload_patterns(100);
    test_mixed_operands();
    
    /* Additional complex pattern in main */
    volatile int64_t* volatile ptr_array[8];
    for (int i = 0; i < 8; i++) {
        ptr_array[i] = &global_array[i * 16];
    }
    
    /* Complex loop with pointer chasing */
    for (int i = 0; i < 50; i++) {
        volatile int64_t** current = &ptr_array[i & 7];
        asm volatile (
            "movq (%[cur]), %%r15\n\t"
            "movq (%%r15), %%rax\n\t"
            "addq %%rax, %[sum]\n\t"
            : [sum] "+r" (reg_a)
            : [cur] "r" (current),
              "m" (**current)
            : "rax", "r15", "memory", "cc"
        );
        
        /* Force address reload with offset */
        int64_t offset = i * 8;
        asm volatile (
            "movq %[base], %%r15\n\t"
            "addq %[off], %%r15\n\t"
            "movq (%%r15), %%rax\n\t"
            "subq %%rax, %[diff]\n\t"
            : [diff] "+r" (reg_b)
            : [base] "r" (global_array),
              [off] "r" (offset),
              "m" (global_array[i])
            : "rax", "r15", "memory", "cc"
        );
    }
    
    /* Prevent dead code elimination */
    asm volatile ("" : : "r"(reg_a), "r"(reg_b), "r"(reg_c), "r"(reg_d), "r"(reg_e));
    
    return (int)(reg_a + reg_b + reg_c + reg_d + reg_e) & 0xFF;
}
