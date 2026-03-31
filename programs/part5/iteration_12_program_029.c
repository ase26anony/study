/* reload1_trigger.c
 * Designed to trigger specific reload types in GCC's reload pass:
 * - RELOAD_FOR_OTHER_ADDRESS
 * - RELOAD_FOR_INPUT_ADDRESS
 * - RELOAD_FOR_INPADDR_ADDRESS
 * - RELOAD_FOR_OPERAND_ADDRESS
 * - RELOAD_FOR_OPADDR_ADDR
 */

#include <stdint.h>
#include <stdlib.h>

/* Complex structure to force non-trivial addressing */
struct DataBlock {
    int64_t values[8];
    volatile int32_t flags[4];
    char padding[32];
};

/* Multi-dimensional array with padding */
struct Matrix {
    struct DataBlock rows[16];
    int64_t metadata[4];
};

/* Explicit register variables - force specific register allocation */
register uint64_t reg_a asm("r10");
register uint64_t reg_b asm("r11");
register uint64_t reg_c asm("r12");
register uint64_t reg_d asm("r13");
register uint64_t reg_e asm("r14");

/* Volatile pointers to prevent optimization */
volatile struct Matrix* volatile matrix_ptr;
volatile int64_t* volatile data_ptr;

/* Function with complex addressing patterns */
void process_matrix(volatile struct Matrix* mat, int iterations) {
    /* Local variables that will conflict with explicit registers */
    uint64_t idx1, idx2, idx3;
    volatile int64_t* row_ptr;
    volatile int32_t* flag_ptr;
    
    /* Initialize explicit register variables */
    reg_a = (uint64_t)mat;
    reg_b = 0;
    reg_c = 8;  /* Offset for values array */
    reg_d = 16; /* Offset for flags array */
    reg_e = iterations;
    
    /* Loop with complex addressing computations */
    for (idx1 = 0; idx1 < iterations; ++idx1) {
        /* Force reloads by using reg_a as base with multiple offsets */
        row_ptr = (volatile int64_t*)(reg_a + idx1 * sizeof(struct DataBlock));
        flag_ptr = (volatile int32_t*)(reg_a + idx1 * sizeof(struct DataBlock) + 
                                      offsetof(struct DataBlock, flags));
        
        /* Pattern 1: Complex inline asm with memory constraint and register clobbering */
        asm volatile (
            /* Use reg_b as temporary, force address reload for memory operand */
            "movq %[rowptr], %%r11\n\t"
            "addq %%r12, %%r11\n\t"
            "movq (%%r11), %[out1]\n\t"
            : [out1] "=r" (idx2)
            : [rowptr] "m" (*(volatile int64_t**)&row_ptr),
              "r" (reg_c)
            : "r11", "memory"
        );
        
        /* Pattern 2: Multiple constraints forcing address reload */
        asm volatile (
            /* Complex addressing with immediate offset */
            "leaq 32(%[base], %[idx], 4), %[temp]\n\t"
            "movq (%[temp]), %[result]\n\t"
            : [result] "=r" (idx3),
              [temp] "=&r" (reg_b)
            : [base] "r" (reg_a),
              [idx] "r" (idx2),
              "m" (*(volatile struct Matrix*)mat)
            : "cc", "memory"
        );
        
        /* Pattern 3: Force RELOAD_FOR_OTHER_ADDRESS */
        /* Mixed constraints: immediate, register, and memory */
        asm volatile (
            "imulq $8, %[idx], %%r15\n\t"
            "addq %[base], %%r15\n\t"
            "movq 16(%%r15), %[out]\n\t"
            : [out] "=r" (reg_d)
            : [base] "r" (reg_a),
              [idx] "irm" (idx3),  /* Immediate, register, or memory - forces choices */
              "m" (mat->rows[0].values[0])  /* Memory constraint to force address reload */
            : "r15", "memory"
        );
        
        /* Pattern 4: Nested address computation with explicit register clobber */
        {
            uint64_t temp_addr = (uint64_t)flag_ptr + reg_d;
            
            asm volatile (
                /* Access through computed address with offset */
                "movq %[addr], %%rbx\n\t"
                "movl 4(%%rbx), %%eax\n\t"
                "addl %%eax, %%r12d\n\t"
                : "+r" (reg_c)  /* Read-write operand */
                : [addr] "m" (*(volatile uint64_t*)&temp_addr),
                  "r" (reg_b)
                : "rax", "rbx", "cc", "memory"
            );
        }
        
        /* Pattern 5: Force RELOAD_FOR_OPERAND_ADDRESS and RELOAD_FOR_OPADDR_ADDR */
        /* Multiple memory operands with different addressing requirements */
        {
            volatile int64_t* ptr1 = &mat->rows[idx1 % 16].values[idx2 % 8];
            volatile int32_t* ptr2 = &mat->rows[idx3 % 16].flags[idx1 % 4];
            
            asm volatile (
                /* Two memory accesses with different base registers */
                "movq %[ptr1], %%r8\n\t"
                "movq %[ptr2], %%r9\n\t"
                "movq (%%r8), %%r10\n\t"
                "addq (%%r9), %%r10\n\t"
                "movq %%r10, %[result]\n\t"
                : [result] "=m" (*(volatile int64_t*)ptr1)
                : [ptr1] "m" (*(volatile int64_t**)&ptr1),
                  [ptr2] "m" (*(volatile int32_t**)&ptr2),
                  "r" (reg_a), "r" (reg_b)
                : "r8", "r9", "r10", "memory"
            );
        }
        
        /* Pattern 6: Inline asm with output address reload */
        {
            int64_t output_val;
            volatile int64_t* output_ptr = &mat->metadata[idx1 % 4];
            
            asm volatile (
                /* Operation requiring output address in register */
                "movq %[outptr], %%rdi\n\t"
                "movq %[in1], %%rsi\n\t"
                "addq %[in2], %%rsi\n\t"
                "movq %%rsi, (%%rdi)\n\t"
                : "=m" (*(volatile int64_t*)output_ptr)
                : [outptr] "m" (*(volatile int64_t**)&output_ptr),
                  [in1] "r" (reg_c),
                  [in2] "r" (reg_d)
                : "rdi", "rsi", "memory"
            );
        }
        
        /* Mix in some C operations to create more register pressure */
        reg_a += idx1 * 16;
        reg_b ^= idx2;
        reg_c += idx3;
        
        /* Volatile access to force memory barrier */
        asm volatile("" ::: "memory");
    }
    
    /* Final pattern: Complex addressing across multiple structs */
    {
        struct DataBlock local_block;
        volatile struct DataBlock* volatile blocks[4];
        
        for (int i = 0; i < 4; i++) {
            blocks[i] = &mat->rows[i * 4];
        }
        
        /* Force address reloads for array of pointers */
        asm volatile (
            "movq %[blocks], %%rcx\n\t"
            "movq 8(%%rcx), %%rdx\n\t"
            "movq 16(%%rdx), %%rax\n\t"
            "addq %%rax, %[sum]\n\t"
            : [sum] "+r" (reg_e)
            : [blocks] "m" (blocks),
              "r" (reg_a)
            : "rcx", "rdx", "rax", "cc", "memory"
        );
    }
}

/* Helper to create complex data structure */
volatile struct Matrix* create_matrix(void) {
    volatile struct Matrix* mat = (volatile struct Matrix*)
        aligned_alloc(64, sizeof(struct Matrix));
    
    if (!mat) return NULL;
    
    /* Initialize with pattern */
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 8; j++) {
            mat->rows[i].values[j] = i * 100 + j;
        }
        for (int j = 0; j < 4; j++) {
            mat->rows[i].flags[j] = i + j;
        }
    }
    
    for (int i = 0; i < 4; i++) {
        mat->metadata[i] = i * 1000;
    }
    
    return mat;
}

int main(void) {
    /* Create complex data structure */
    volatile struct Matrix* mat = create_matrix();
    if (!mat) return 1;
    
    /* Set global pointers */
    matrix_ptr = mat;
    data_ptr = &mat->rows[0].values[0];
    
    /* Process with complex addressing patterns */
    process_matrix(mat, 8);
    
    /* Second pass with different parameters */
    reg_a = 0;
    reg_b = 1;
    reg_c = 2;
    reg_d = 3;
    reg_e = 4;
    
    process_matrix(mat, 4);
    
    /* Cleanup */
    free((void*)mat);
    
    return 0;
}
