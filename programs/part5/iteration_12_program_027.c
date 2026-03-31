/* reload1_test.c - Complex addressing mode test for GCC reload pass coverage */
#include <stdint.h>

/* Volatile structure to prevent optimization */
struct DataBlock {
    volatile int64_t values[8];
    volatile int32_t indices[16];
    volatile void* pointers[4];
};

/* Explicit register variables to force register pressure */
register int64_t reg_a asm("r10");
register int64_t reg_b asm("r11");
register int64_t reg_c asm("r12");
register int64_t reg_d asm("r13");
register int64_t reg_e asm("r14");
register int64_t reg_f asm("r15");

/* Complex addressing computation function */
static inline int64_t compute_index(int64_t base, int64_t offset, int64_t scale) {
    int64_t result;
    /* Force address computation with multiple constraints */
    asm volatile (
        "lea (%1,%2,%3), %0\n\t"
        : "=r"(result)
        : "r"(base), "r"(offset), "i"(scale)
        : /* no clobbers */
    );
    return result;
}

/* Function with complex inline assembly requiring various reload types */
void process_data(struct DataBlock* block1, 
                  struct DataBlock* block2,
                  volatile int64_t* output,
                  int iterations) {
    
    /* Initialize register variables */
    reg_a = (int64_t)block1;
    reg_b = (int64_t)block2;
    reg_c = 0;
    reg_d = 8;
    reg_e = 16;
    reg_f = 32;
    
    /* Loop with nested address computations */
    for (int i = 0; i < iterations; i++) {
        int64_t temp1, temp2, temp3;
        volatile int64_t* addr1, *addr2;
        
        /* RELOAD_FOR_INPUT_ADDRESS and RELOAD_FOR_OTHER_ADDRESS pattern */
        /* Complex memory access with multiple register constraints */
        asm volatile (
            "movq (%[base1], %[idx1], 8), %[out1]\n\t"
            "addq %[imm1], %[out1]\n\t"
            "movq %[out1], (%[base2], %[idx2], 4)\n\t"
            : [out1] "=&r"(temp1)
            : [base1] "r"(reg_a), 
              [idx1] "r"(reg_c),
              [imm1] "i"(256),
              [base2] "r"(reg_b),
              [idx2] "r"(reg_d)
            : "memory"
        );
        
        /* RELOAD_FOR_INPADDR_ADDRESS pattern */
        /* Memory operand with address that needs reloading */
        addr1 = &block1->values[reg_c % 8];
        asm volatile (
            "movq (%[addr]), %%rax\n\t"
            "imulq %[scale], %%rax\n\t"
            "movq %%rax, %[result]\n\t"
            : [result] "=r"(temp2)
            : [addr] "m"(*addr1),
              [scale] "r"(reg_f)
            : "rax", "memory"
        );
        
        /* RELOAD_FOR_OPERAND_ADDRESS and RELOAD_FOR_OPADDR_ADDR pattern */
        /* Complex address computation in output operand */
        int64_t computed_addr;
        asm volatile (
            "lea (%[ptr], %[off], 8), %[addr]\n\t"
            : [addr] "=r"(computed_addr)
            : [ptr] "r"(block2),
              [off] "r"(reg_e)
        );
        
        /* Use computed address with volatile access */
        *(volatile int64_t*)computed_addr = temp1 + temp2;
        
        /* RELOAD_FOR_OUTPUT_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS pattern */
        /* Output with complex addressing mode */
        addr2 = &block2->indices[reg_d % 16];
        asm volatile (
            "movq %[val], (%[dest])\n\t"
            "addq $1, %[dest]\n\t"
            : [dest] "+r"(addr2)
            : [val] "r"(temp2)
            : "memory"
        );
        
        /* Update register variables - causes spilling/reloading */
        reg_c = compute_index(reg_c, 1, 2);
        reg_d = compute_index(reg_d, reg_e, 1);
        reg_e = (reg_e * 3) % 32;
        
        /* Mixed operand types with immediate and memory */
        asm volatile (
            "cmpq %[imm], (%[mem])\n\t"
            "setg %b[flag]\n\t"
            : [flag] "=r"(temp3)
            : [mem] "r"(addr1),
              [imm] "i"(1000)
            : "cc", "memory"
        );
        
        /* Store result through volatile pointer */
        *output++ = temp3 ? temp1 : temp2;
        
        /* Force register variable usage in address computation */
        asm volatile (
            "movq %%r10, %%rax\n\t"
            "addq %%r11, %%rax\n\t"
            "movq %%rax, %%r12\n\t"
            : 
            : 
            : "rax", "r10", "r11", "r12", "memory"
        );
    }
}

/* Main function with data setup */
int main() {
    /* Static data to ensure addressing complexity */
    static struct DataBlock block1;
    static struct DataBlock block2;
    static volatile int64_t results[100];
    
    /* Initialize data */
    for (int i = 0; i < 8; i++) {
        block1.values[i] = i * 100;
        block2.values[i] = i * 200;
    }
    
    for (int i = 0; i < 16; i++) {
        block1.indices[i] = i;
        block2.indices[i] = i * 2;
    }
    
    /* Call processing function with loop */
    process_data(&block1, &block2, results, 50);
    
    /* Additional complex inline assembly in main */
    register void* ptr1 asm("r8") = &block1;
    register void* ptr2 asm("r9") = &block2;
    
    /* RELOAD_FOR_OTHER_ADDRESS - complex address reload */
    int64_t final_result;
    asm volatile (
        "movq (%[p1], %[idx], 8), %[out]\n\t"
        "addq (%[p2], %[idx], 4), %[out]\n\t"
        "movq %[out], (%[dest])\n\t"
        : [out] "=&r"(final_result)
        : [p1] "r"(ptr1),
          [p2] "r"(ptr2),
          [idx] "r"(reg_c),
          [dest] "m"(results[99])
        : "memory"
    );
    
    /* Prevent dead code elimination */
    asm volatile ("" : : "r"(final_result) : "memory");
    
    return (int)final_result;
}
