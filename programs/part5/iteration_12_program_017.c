/* reload_test.c - Complex addressing mode test for GCC reload pass */
#include <stdint.h>

/* Volatile struct to prevent optimization */
struct Data {
    volatile int32_t a;
    volatile int64_t b;
    volatile void* ptr;
};

/* Explicit register variables for x86-64 */
register uint64_t reg_a asm("r10");
register uint64_t reg_b asm("r11");
register uint64_t reg_c asm("r12");
register uint64_t reg_d asm("r13");
register uint64_t reg_e asm("r14");

/* Complex multi-dimensional array */
static volatile struct Data data_array[8][16];
static volatile int64_t large_buffer[256];

/* Function with complex addressing patterns */
void __attribute__((noinline)) 
complex_address_computation(volatile struct Data* base, int idx1, int idx2) {
    /* Force register usage with explicit variables */
    reg_a = (uint64_t)base;
    reg_b = idx1 * sizeof(struct Data) * 16;
    reg_c = idx2 * sizeof(struct Data);
    
    /* Complex addressing with multiple reload types */
    for (int i = 0; i < 8; i++) {
        /* RELOAD_FOR_INPUT_ADDRESS pattern */
        asm volatile (
            "movq (%[base], %[idx1], 8), %[temp]\n\t"
            "addq %[idx2], %[temp]"
            : [temp] "=r" (reg_d)
            : [base] "r" (reg_a),
              [idx1] "r" (reg_b),
              [idx2] "r" (reg_c)
            : "memory"
        );
        
        /* RELOAD_FOR_OTHER_ADDRESS pattern - complex memory operand */
        asm volatile (
            "leaq (%[addr], %[offset], 1), %[result]\n\t"
            "movq (%[result]), %[result]"
            : [result] "=&r" (reg_e)
            : [addr] "r" (reg_a),
              [offset] "irm" (i * 32 + 8)
            : "memory"
        );
        
        /* Mixed constraints causing RELOAD_FOR_INPADDR_ADDRESS */
        volatile int64_t* ptr = &large_buffer[i * 16];
        asm volatile (
            "imulq %[mul], (%[mem]), %[out]"
            : [out] "=r" (reg_d)
            : [mem] "m" (*(volatile int64_t*)ptr),
              [mul] "r" (reg_e)
            : "memory"
        );
        
        /* Force spill/reload of fixed registers */
        asm volatile (
            "movq %%r10, %%rax\n\t"
            "movq %%r11, %%rbx\n\t"
            "movq %%r12, %%rcx"
            :
            :
            : "rax", "rbx", "rcx", "r10", "r11", "r12"
        );
    }
}

/* Function requiring RELOAD_FOR_OPERAND_ADDRESS */
void __attribute__((noinline, optimize("O0")))
operand_address_reloads(void) {
    volatile uint64_t values[4] = {1, 2, 3, 4};
    register uint64_t r1 asm("r15");
    
    /* Complex operand addressing */
    for (int i = 0; i < 100; i++) {
        /* RELOAD_FOR_OPERAND_ADDRESS pattern */
        asm volatile (
            "movq (%[base], %[index], 8), %[dest]\n\t"
            "addq $0x12345678, %[dest]"
            : [dest] "=r" (r1)
            : [base] "r" (values),
              [index] "r" (i & 3),
              "m" (*(volatile uint64_t*)(values + (i & 3)))
            : "memory"
        );
        
        /* RELOAD_FOR_OPADDR_ADDR pattern */
        asm volatile (
            "lea (%[addr], %[scale], 4), %[out]\n\t"
            "movq (%[out]), %[out]"
            : [out] "=&r" (reg_a)
            : [addr] "r" (values),
              [scale] "r" (r1)
            : "memory"
        );
    }
}

/* Function with output address reloads */
void __attribute__((noinline))
output_address_reloads(volatile int64_t* out) {
    /* RELOAD_FOR_OUTPUT_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
    for (int i = 0; i < 32; i++) {
        volatile int64_t* current = out + i * 4;
        
        asm volatile (
            "movq %[in], (%[out])\n\t"
            "addq $1, (%[out])"
            :
            : [in] "r" (reg_b),
              [out] "r" (current),
              "m" (*current)
            : "memory"
        );
        
        /* Nested address computation */
        asm volatile (
            "movq 8(%[base], %[idx], 8), %[temp]\n\t"
            "movq %[temp], 16(%[base], %[idx], 8)"
            : [temp] "=&r" (reg_c)
            : [base] "r" (out),
              [idx] "r" (i)
            : "memory"
        );
    }
}

/* Main function creating complex reload scenarios */
int main(void) {
    /* Initialize data */
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 16; j++) {
            data_array[i][j].a = i * 100 + j;
            data_array[i][j].b = (int64_t)&data_array[i][j];
        }
    }
    
    for (int i = 0; i < 256; i++) {
        large_buffer[i] = i * 2;
    }
    
    /* Test different reload patterns */
    complex_address_computation(&data_array[0][0], 2, 3);
    operand_address_reloads();
    output_address_reloads(large_buffer);
    
    /* Additional complex pattern mixing all types */
    volatile uint64_t* volatile ptr_array[8];
    for (int i = 0; i < 8; i++) {
        ptr_array[i] = (volatile uint64_t*)&data_array[i][0];
    }
    
    /* Loop with mixed addressing modes */
    for (int outer = 0; outer < 4; outer++) {
        for (int inner = 0; inner < 8; inner++) {
            /* Force RELOAD_FOR_OTHER_ADDRESS with complex constraint */
            asm volatile (
                "movq (%[base], %[idx1], 8), %[temp1]\n\t"
                "addq (%[base], %[idx2], 8), %[temp1]\n\t"
                "movq %[temp1], (%[dest], %[idx3], 4)"
                : [temp1] "=&r" (reg_a)
                : [base] "r" (ptr_array),
                  [idx1] "r" (outer),
                  [idx2] "r" (inner),
                  [dest] "r" (large_buffer),
                  [idx3] "r" ((outer * 8 + inner) & 0xFF),
                  "m" (*(volatile uint64_t*)ptr_array[outer]),
                  "m" (*(volatile uint64_t*)ptr_array[inner]),
                  "m" (*(volatile uint64_t*)(large_buffer + ((outer * 8 + inner) & 0xFF)))
                : "memory"
            );
            
            /* Clobber multiple explicit registers */
            asm volatile ("" : : : "r10", "r11", "r12", "r13", "r14", "r15");
        }
    }
    
    return 0;
}
