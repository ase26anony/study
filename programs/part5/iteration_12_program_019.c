/* reload_test.c - Complex addressing mode test for GCC reload pass */
#include <stdint.h>

/* Volatile struct to prevent optimization */
struct DataBlock {
    volatile int64_t values[16];
    volatile int32_t indices[32];
    volatile uint8_t flags[64];
};

/* Explicit register variables for x86-64 */
register uint64_t r10_var asm("r10");
register uint64_t r11_var asm("r11");
register uint64_t r12_var asm("r12");
register uint64_t r13_var asm("r13");
register uint64_t r14_var asm("r14");
register uint64_t r15_var asm("r15");

/* Complex addressing computation function */
void complex_address_test(struct DataBlock *block, int iterations) {
    /* Force these into specific registers */
    register uint64_t base_addr asm("rbx") = (uint64_t)block;
    register uint64_t index_reg asm("rcx") = 0;
    register uint64_t offset_reg asm("rdx") = 8;
    
    /* Array with complex indexing */
    volatile int64_t *dynamic_array = (volatile int64_t *)block->values;
    
    for (int i = 0; i < iterations; i++) {
        /* Pattern 1: RELOAD_FOR_OTHER_ADDRESS with multiple constraints */
        asm volatile (
            /* Complex memory access with register-indirect addressing */
            "movq (%[base], %[index], 8), %[temp1]\n\t"
            "addq %[offset], %[temp1]\n\t"
            "movq %[temp1], (%[base], %[index], 8)"
            : [temp1] "=&r" (r10_var)
            : [base] "r" (base_addr),
              [index] "r" (index_reg),
              [offset] "irm" (offset_reg + i * 4)  /* Mixed constraint */
            : "memory"
        );
        
        /* Pattern 2: RELOAD_FOR_INPUT_ADDRESS with explicit clobbers */
        asm volatile (
            /* Force address computation reload */
            "leaq 64(%[base], %[idx], 4), %[addr]\n\t"
            "movl (%[addr]), %[val]\n\t"
            "addl %[imm], %[val]\n\t"
            "movl %[val], (%[addr])"
            : [addr] "=&r" (r11_var),
              [val] "=&r" (r12_var)
            : [base] "r" (base_addr),
              [idx] "r" (index_reg),
              [imm] "i" (0x1000)  /* Large immediate */
            : "memory", "cc"
        );
        
        /* Pattern 3: RELOAD_FOR_OPADDR_ADDR with nested addressing */
        {
            volatile uint8_t *flag_ptr = &block->flags[index_reg % 64];
            int64_t multiplier = (i & 3) + 1;
            
            asm volatile (
                /* Complex address chain */
                "movq %[ptr], %[tmp]\n\t"
                "imulq %[mul], %[tmp]\n\t"
                "addq %[base2], %[tmp]\n\t"
                "movb $1, (%[tmp])"
                : [tmp] "=&r" (r13_var)
                : [ptr] "m" (*flag_ptr),  /* Memory constraint forces address reload */
                  [mul] "r" (multiplier),
                  [base2] "r" (base_addr)
                : "memory", "cc"
            );
        }
        
        /* Pattern 4: Multiple reload types in one asm */
        {
            int32_t temp_idx = block->indices[i % 32];
            int64_t complex_offset = (temp_idx * 8) + 0x100;
            
            asm volatile (
                /* Mix of address reload types */
                "movq %[complex], %%rax\n\t"
                "addq %[base3], %%rax\n\t"
                "movq (%%rax), %[out1]\n\t"
                "addq %[in1], %[out1]\n\t"
                "movq %[out1], (%[base3], %[idx2], 1)"
                : [out1] "=r" (r14_var)
                : [base3] "r" (base_addr),
                  [idx2] "r" (index_reg),
                  [complex] "irm" (complex_offset),  /* Forces RELOAD_FOR_OTHER_ADDRESS */
                  [in1] "r" (r15_var)
                : "rax", "memory", "cc"
            );
        }
        
        /* Pattern 5: RELOAD_FOR_OUTPUT_ADDRESS with volatile */
        {
            volatile int64_t *output_loc = 
                (volatile int64_t *)((char *)block + i * 8 + 256);
            
            asm volatile (
                /* Output address needs reload */
                "movq %[in2], (%[out_addr])\n\t"
                "lock xaddq %[inc], (%[out_addr])"
                : 
                : [out_addr] "m" (*output_loc),  /* Output address reload */
                  [in2] "r" (r14_var),
                  [inc] "r" (r13_var)
                : "memory", "cc"
            );
        }
        
        /* Update index with complex computation */
        asm volatile (
            "movq %[idx], %%rax\n\t"
            "imulq $299, %%rax, %%rax\n\t"
            "addq $1, %%rax\n\t"
            "movq %%rax, %[new_idx]"
            : [new_idx] "=r" (index_reg)
            : [idx] "r" (index_reg)
            : "rax", "cc"
        );
        
        /* Rotate register values to create pressure */
        asm volatile (
            "xchgq %[a], %[b]\n\t"
            "xchgq %[b], %[c]\n\t"
            "xchgq %[c], %[d]"
            : [a] "+r" (r10_var),
              [b] "+r" (r11_var),
              [c] "+r" (r12_var),
              [d] "+r" (r13_var)
            :
            : "cc"
        );
    }
}

/* Multi-dimensional array access with complex indexing */
void nested_array_test(int size) {
    /* Force stack allocation with alignment requirements */
    struct {
        int64_t matrix[32][32];
        int32_t offsets[1024];
        volatile uint8_t control[256];
    } __attribute__((aligned(64))) data;
    
    register int64_t *row_ptr asm("r8");
    register int32_t idx1 asm("r9d") = 0;
    register int32_t idx2 asm("r10d") = 0;
    
    /* Initialize with volatile writes */
    for (int i = 0; i < 1024; i++) {
        data.offsets[i] = i * 3;
    }
    
    for (int i = 0; i < size; i++) {
        /* Complex address computation that may need RELOAD_FOR_INPADDR_ADDRESS */
        row_ptr = &data.matrix[idx1 % 32][0];
        
        asm volatile (
            /* Nested addressing with multiple reloads */
            "movslq %[off_idx], %%rax\n\t"
            "movl %[offsets](%%rax, 4), %[off_val]\n\t"
            "movslq %[off_val], %%rbx\n\t"
            "movq %[row], %%rcx\n\t"
            "movq (%%rcx, %%rbx, 8), %[result]\n\t"
            "addq $1, %[result]\n\t"
            "movq %[result], (%%rcx, %%rbx, 8)"
            : [off_val] "=&r" (idx2),
              [result] "=&r" (r10_var)
            : [off_idx] "r" (idx1),
              [offsets] "m" (data.offsets),  /* Base address needs reload */
              [row] "r" (row_ptr)
            : "rax", "rbx", "rcx", "memory", "cc"
        );
        
        /* RELOAD_FOR_OPERAND_ADDRESS pattern */
        {
            volatile uint8_t *ctrl = &data.control[(idx1 + idx2) % 256];
            
            asm volatile (
                "movb $1, (%[ctrl_addr])\n\t"
                "mfence"
                : 
                : [ctrl_addr] "m" (*ctrl)  /* Operand address reload */
                : "memory"
            );
        }
        
        idx1 = (idx1 * 17 + 1) % 1024;
    }
}

int main() {
    /* Allocate with unusual alignment to force complex addressing */
    struct DataBlock *block = (struct DataBlock *)
        __builtin_aligned_alloc(128, sizeof(struct DataBlock));
    
    if (!block) return 1;
    
    /* Initialize register variables */
    r10_var = 0xDEADBEEF;
    r11_var = 0xCAFEBABE;
    r12_var = 0x12345678;
    r13_var = 0x9ABCDEF0;
    r14_var = 0x55555555;
    r15_var = 0xAAAAAAAA;
    
    /* Run tests with different complexities */
    complex_address_test(block, 100);
    nested_array_test(50);
    
    /* Final barrier to prevent optimization */
    asm volatile ("" : : : "memory");
    
    __builtin_free(block);
    return 0;
}
