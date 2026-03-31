/* reload_test.c - Complex addressing mode test for GCC reload pass */
#include <stdint.h>

/* Volatile struct to prevent optimization */
struct DataBlock {
    volatile int32_t values[16];
    volatile int64_t pointers[8];
    volatile uint32_t flags;
};

/* Explicit register variables for x86-64 */
register uint64_t reg_a asm("r10");
register uint64_t reg_b asm("r11");
register uint64_t reg_c asm("r12");
register uint64_t reg_d asm("r13");
register uint64_t reg_e asm("r14");

/* Global data to ensure addressing complexity */
struct DataBlock global_block[4];
volatile int32_t* volatile ptr_array[32];
int64_t accumulator = 0;

/* Function with complex inline assembly patterns */
void complex_addressing_test(int iterations) {
    /* Local variables that will need various reload types */
    int32_t index1 = 0, index2 = 1, index3 = 2;
    volatile int32_t* volatile dyn_ptr;
    struct DataBlock* block_ptr = &global_block[0];
    
    /* Initialize explicit register variables */
    reg_a = (uint64_t)&global_block[0];
    reg_b = (uint64_t)&global_block[1];
    reg_c = (uint64_t)ptr_array;
    reg_d = 0;
    reg_e = 0;
    
    /* Loop with nested address computations */
    for (int i = 0; i < iterations; i++) {
        /* Pattern 1: RELOAD_FOR_INPUT_ADDRESS and RELOAD_FOR_OTHER_ADDRESS */
        /* Complex memory addressing with multiple index registers */
        asm volatile (
            "movq (%[base], %[idx1], 8), %[temp]\n\t"
            "addq %[temp], %[accum]\n\t"
            "movl (%[block], %[idx2], 4), %%eax\n\t"
            "addl %%eax, %[sum]\n\t"
            : [accum] "+r" (accumulator), [sum] "+rm" (index1)
            : [base] "r" (reg_c), [idx1] "r" (reg_d), 
              [temp] "r" (reg_e), [block] "r" (reg_a), [idx2] "r" (reg_b)
            : "rax", "memory", "cc"
        );
        
        /* Pattern 2: RELOAD_FOR_INPADDR_ADDRESS and RELOAD_FOR_OUTPUT_ADDRESS */
        /* Memory operand with address that needs reloading */
        int32_t temp_val;
        dyn_ptr = &block_ptr->values[index2];
        
        asm volatile (
            "movl (%[mem]), %%ebx\n\t"
            "imull %%ebx, %[out]\n\t"
            "leaq (%[base], %[idx], 4), %[addr]\n\t"
            : [out] "=r" (temp_val), [addr] "=r" (reg_e)
            : [mem] "m" (*(volatile int32_t*)dyn_ptr),
              [base] "r" (reg_a), [idx] "r" (reg_d)
            : "rbx", "memory", "cc"
        );
        
        /* Pattern 3: RELOAD_FOR_OPERAND_ADDRESS and RELOAD_FOR_OPADDR_ADDR */
        /* Mixed immediate, register, and memory constraints */
        uint64_t offset = i * 16;
        
        asm volatile (
            "movq %[off], %%rcx\n\t"
            "addq %%rcx, %[base]\n\t"
            "movq (%[base], %[idx], 1), %[result]\n\t"
            : [base] "+r" (reg_b), [result] "=r" (reg_c)
            : [off] "irm" (offset), [idx] "r" (reg_d)
            : "rcx", "memory", "cc"
        );
        
        /* Pattern 4: RELOAD_FOR_OUTADDR_ADDRESS */
        /* Output memory address that needs reloading */
        int64_t* output_addr = &block_ptr->pointers[index3];
        
        asm volatile (
            "movq %[val], (%[addr])\n\t"
            "addq $8, %[addr]\n\t"
            : [addr] "+r" (output_addr)
            : [val] "r" (reg_c)
            : "memory"
        );
        
        /* Pattern 5: RELOAD_FOR_INPUT with complex addressing */
        /* Multiple input operands with conflicting constraints */
        asm volatile (
            "movq %[in1], %%rdx\n\t"
            "addq %%rdx, %[in2]\n\t"
            "movq %[in2], %[out]\n\t"
            : [out] "=r" (reg_d)
            : [in1] "r" (reg_a), [in2] "r" (reg_b)
            : "rdx", "cc"
        );
        
        /* Update indices with complex expressions */
        index1 = (index1 * 3 + 1) & 0xF;
        index2 = (index2 * 5 + i) & 0xF;
        index3 = (index3 * 7 + 2) & 0x7;
        
        /* Modify register variables to force spills/reloads */
        reg_a += 4;
        reg_b += 8;
        reg_d = (reg_d << 1) | 1;
        
        /* Volatile memory access to prevent optimization */
        *(volatile int32_t*)(reg_a) = i;
        *(volatile int64_t*)(reg_b) = accumulator;
    }
    
    /* Final pattern: RELOAD_OTHER case */
    /* Complex asm with many clobbered registers */
    asm volatile (
        "movq %[a], %%r8\n\t"
        "movq %[b], %%r9\n\t"
        "addq %%r8, %%r9\n\t"
        "movq %%r9, %[c]\n\t"
        "movq %[c], %%r10\n\t"
        "subq %%r10, %[d]\n\t"
        : [c] "=r" (reg_c), [d] "=r" (reg_d)
        : [a] "r" (reg_a), [b] "r" (reg_b)
        : "r8", "r9", "r10", "cc"
    );
}

/* Secondary function with different addressing patterns */
void nested_addressing(int depth) {
    volatile int32_t matrix[8][8];
    register int32_t* row_ptr asm("r15");
    
    for (int i = 0; i < 8; i++) {
        row_ptr = &matrix[i][0];
        
        /* Complex address computation in loop */
        for (int j = 0; j < 8; j++) {
            int32_t* elem_ptr = row_ptr + j;
            
            /* Force address reload for memory operand */
            asm volatile (
                "movl (%[ptr], %[off], 4), %%esi\n\t"
                "addl %%esi, %[acc]\n\t"
                : [acc] "+r" (accumulator)
                : [ptr] "r" (row_ptr), [off] "r" (reg_d)
                : "rsi", "memory", "cc"
            );
            
            /* Another pattern with immediate offset */
            asm volatile (
                "movl %[imm], (%[base], %[idx], 4)\n\t"
                : 
                : [imm] "ir" (i * j), [base] "r" (row_ptr), [idx] "r" (reg_d)
                : "memory"
            );
        }
        
        /* Update register with complex expression */
        asm volatile (
            "leaq (%[base], %[idx], 8), %[out]\n\t"
            : [out] "=r" (reg_a)
            : [base] "r" (reg_b), [idx] "r" (reg_c)
        );
    }
}

int main() {
    /* Initialize global data */
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 16; j++) {
            global_block[i].values[j] = i * 100 + j;
        }
        for (int j = 0; j < 8; j++) {
            global_block[i].pointers[j] = (int64_t)&global_block[(i + j) % 4];
        }
        global_block[i].flags = 0xABCD1234;
    }
    
    for (int i = 0; i < 32; i++) {
        ptr_array[i] = &global_block[i % 4].values[0];
    }
    
    /* Run tests with different iteration counts */
    complex_addressing_test(16);
    nested_addressing(2);
    
    /* Final barrier */
    asm volatile ("" : : : "memory");
    
    return (int)(accumulator & 0x7FFFFFFF);
}
