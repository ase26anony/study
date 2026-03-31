/* reload1_trigger.c
 * Designed to trigger specific reload types in GCC's reload pass:
 * - RELOAD_FOR_OTHER_ADDRESS
 * - RELOAD_FOR_INPUT_ADDRESS
 * - RELOAD_FOR_OPADDR_ADDR
 * - RELOAD_FOR_OPERAND_ADDRESS
 */

#include <stdint.h>
#include <stdlib.h>

/* Complex data structure to force non-trivial addressing */
struct DataBlock {
    int64_t values[8];
    struct DataBlock* next;
    volatile int32_t flags[4];
};

/* Multi-dimensional array with padding */
struct Matrix {
    double data[16][8];
    int32_t indices[16];
    volatile int64_t counters[4];
};

/* Explicit register variables - force specific register allocation */
register int64_t reg_a asm("r10");
register int64_t reg_b asm("r11");
register int64_t reg_c asm("r12");
register int64_t reg_d asm("r13");
register int64_t reg_e asm("r14");
register int64_t reg_f asm("r15");

/* Volatile pointers to prevent optimization */
volatile struct DataBlock* volatile_data;
volatile struct Matrix* volatile_matrix;

/* Function with complex addressing patterns */
void complex_addressing_loop(int iterations) {
    /* Local arrays with different alignments */
    int64_t array1[128] __attribute__((aligned(64)));
    int64_t array2[256] __attribute__((aligned(32)));
    struct DataBlock blocks[8];
    struct Matrix matrices[4];
    
    /* Initialize data */
    for (int i = 0; i < 128; i++) {
        array1[i] = i * 3;
    }
    for (int i = 0; i < 256; i++) {
        array2[i] = i * 5;
    }
    
    volatile_data = blocks;
    volatile_matrix = matrices;
    
    /* Complex loop with multiple addressing modes */
    for (int outer = 0; outer < iterations; outer++) {
        /* Force register pressure by using all explicit registers */
        reg_a = outer * 7;
        reg_b = outer * 11;
        reg_c = outer * 13;
        reg_d = outer * 17;
        reg_e = outer * 19;
        reg_f = outer * 23;
        
        /* Pattern 1: Complex memory addressing with multiple registers */
        /* This should trigger RELOAD_FOR_INPUT_ADDRESS and RELOAD_FOR_OTHER_ADDRESS */
        asm volatile (
            /* Load from memory using complex address calculation */
            "movq (%[base], %[idx], 8), %[temp1]\n\t"
            "addq %[offset], %[temp1]\n\t"
            "movq %[temp1], (%[dest], %[idx2], 4)"
            : [temp1] "=&r" (reg_a)  /* Early clobber to force separate reg */
            : [base] "r" (array1), 
              [idx] "r" (reg_b), 
              [offset] "irm" (256),  /* Mixed: immediate, register, or memory */
              [dest] "r" (array2),
              [idx2] "r" (reg_c)
            : "memory"
        );
        
        /* Pattern 2: Nested address computation with volatile access */
        /* Should trigger RELOAD_FOR_OPADDR_ADDR */
        int64_t* volatile ptr = (int64_t* volatile)&blocks[outer & 7];
        asm volatile (
            "leaq (%[ptr], %[scale], 8), %[addr]\n\t"
            "movq (%[addr]), %[val]\n\t"
            "imulq %[mul], %[val]"
            : [addr] "=&r" (reg_d),  /* Address register - early clobber */
              [val] "=r" (reg_e)
            : [ptr] "m" (*ptr),      /* Memory constraint forces address reload */
              [scale] "r" (reg_f),
              [mul] "irm" (37)       /* Mixed constraint */
            : "cc"
        );
        
        /* Pattern 3: Multiple memory operands with conflicting constraints */
        /* Should trigger RELOAD_FOR_OPERAND_ADDRESS */
        volatile int64_t* vptr1 = &volatile_matrix->counters[outer & 3];
        volatile int64_t* vptr2 = &volatile_matrix->counters[(outer + 1) & 3];
        
        asm volatile (
            "movq (%[src1]), %[tmp]\n\t"
            "addq (%[src2]), %[tmp]\n\t"
            "movq %[tmp], (%[dst])"
            : [tmp] "=&r" (reg_a)
            : [src1] "r" (vptr1),    /* Register constraint for address */
              [src2] "r" (vptr2),    /* Another address in register */
              [dst] "m" (array1[reg_b & 127])  /* Memory constraint with index */
            : "memory", "cc"
        );
        
        /* Pattern 4: Complex struct addressing with multiple levels */
        /* Should trigger various address reload types */
        struct DataBlock* current = &blocks[outer & 7];
        for (int inner = 0; inner < 4; inner++) {
            /* Force address computation involving multiple registers */
            int64_t index = (reg_c + inner * reg_d) & 7;
            
            asm volatile (
                /* Compute address with multiple components */
                "leaq (%[base], %[idx], 8), %[addr]\n\t"
                /* Access volatile member through computed address */
                "movl (%[addr], %[offset]), %[flag]\n\t"
                /* Use in computation */
                "addl %[flag], %[sum]"
                : [addr] "=&r" (reg_f),    /* Address register */
                  [flag] "=r" (reg_e),     /* Flag value */
                  [sum] "+r" (reg_a)       /* Accumulator */
                : [base] "r" (current->values),  /* Base address */
                  [idx] "r" (index),       /* Index register */
                  [offset] "irm" (offsetof(struct DataBlock, flags)) /* Mixed */
                : "cc", "memory"
            );
            
            /* Additional pressure with explicit register clobbering */
            asm volatile (
                "movq %%r10, %%rax\n\t"
                "movq %%r11, %%rbx\n\t"
                "xchgq %%rax, %%rbx"
                : 
                : 
                : "rax", "rbx", "r10", "r11", "cc"
            );
        }
        
        /* Pattern 5: Immediate + memory + register in single asm */
        /* Mixed constraints to force difficult reload decisions */
        int64_t immediate_val = 0x123456789ABCDEF;
        
        asm volatile (
            "movq %[imm], %%rax\n\t"
            "addq (%[mem]), %%rax\n\t"
            "addq %[reg], %%rax\n\t"
            "movq %%rax, (%[out])"
            : 
            : [imm] "irm" (immediate_val),  /* Could be immediate or register */
              [mem] "r" (&array2[reg_b & 255]),  /* Address in register */
              [reg] "r" (reg_c),            /* Value in register */
              [out] "m" (blocks[outer & 7].values[reg_d & 7]) /* Complex memory dest */
            : "rax", "rbx", "rcx", "memory", "cc"
        );
    }
}

/* Secondary function with different pattern */
void nested_address_computation(int n) {
    /* Multi-dimensional array access pattern */
    int64_t matrix[8][8][8];
    
    /* Initialize */
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            for (int k = 0; k < 8; k++) {
                matrix[i][j][k] = i * 64 + j * 8 + k;
            }
        }
    }
    
    /* Complex nested loop with address computations */
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < 8; j++) {
            /* Use explicit registers in address computation */
            reg_a = i;
            reg_b = j;
            reg_c = (i * j) & 7;
            
            /* Complex addressing: matrix[i][j][k] with k in register */
            int64_t* base = &matrix[0][0][0];
            int64_t index = ((i * 64) + (j * 8) + reg_c);
            
            asm volatile (
                /* Compute element address */
                "leaq (%[base], %[idx], 8), %[addr]\n\t"
                /* Load and modify */
                "movq (%[addr]), %[val]\n\t"
                "addq %[inc], %[val]\n\t"
                /* Store back through different addressing */
                "movq %[val], (%[base2], %[idx2], 8)"
                : [addr] "=&r" (reg_d),
                  [val] "=&r" (reg_e)
                : [base] "r" (base),
                  [idx] "r" (index),
                  [inc] "irm" (42),
                  [base2] "r" (base),
                  [idx2] "r" (((j * 64) + (i * 8) + reg_c) & 511) /* Different index */
                : "memory", "cc"
            );
            
            /* Force address reload for output operand */
            volatile int64_t* vptr = &matrix[i][j][reg_c];
            asm volatile (
                "incq (%[ptr])"
                : 
                : [ptr] "m" (*vptr)  /* Memory constraint forces address reload */
                : "memory"
            );
        }
    }
}

int main(int argc, char** argv) {
    int iterations = 100;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
        if (iterations > 1000) iterations = 1000; /* Bound for safety */
    }
    
    /* Execute both patterns to increase reload opportunities */
    complex_addressing_loop(iterations);
    nested_address_computation(iterations / 4);
    
    /* Final barrier */
    asm volatile ("" : : : "memory");
    
    return 0;
}
