/* reload_test.c - Complex addressing modes to trigger reload types */
#include <stdint.h>
#include <stdlib.h>

/* Force specific register allocations */
register uint64_t reg_a asm("r10");
register uint64_t reg_b asm("r11");
register uint64_t reg_c asm("r12");
register uint64_t reg_d asm("r13");
register uint64_t reg_e asm("r14");

/* Complex data structure to force address computations */
struct MultiDim {
    uint64_t data[8][8];
    uint64_t extra[16];
    volatile uint64_t flags;
};

/* Volatile pointer to prevent optimization */
volatile struct MultiDim* volatile global_ptr;

/* Function with complex addressing patterns */
void complex_addressing_test(uint64_t iter) {
    /* Local array with complex access pattern */
    uint64_t local_array[256] __attribute__((aligned(64)));
    
    /* Initialize some values */
    for (int i = 0; i < 256; i++) {
        local_array[i] = i * 3;
    }
    
    /* Force register variables to hold specific values */
    reg_a = (uint64_t)&local_array[0];
    reg_b = iter * 8;
    reg_c = 0x12345678;
    
    /* Loop with nested address computations */
    for (uint64_t i = 0; i < iter; i++) {
        /* Complex address computation involving multiple registers */
        uint64_t idx1 = (reg_b + i) % 256;
        uint64_t idx2 = (reg_c + i * 7) % 256;
        
        /* Pattern 1: RELOAD_FOR_INPUT_ADDRESS and RELOAD_FOR_OTHER_ADDRESS */
        /* Multiple memory operands with register constraints */
        asm volatile (
            "movq (%[base], %[idx1], 8), %%r15\n\t"
            "addq %%r15, %[sum]\n\t"
            "movq %[sum], (%[base], %[idx2], 8)"
            : [sum] "+r" (reg_d)
            : [base] "r" (reg_a), 
              [idx1] "r" (idx1),
              [idx2] "r" (idx2),
              "m" (*(struct { uint64_t x[256]; } *)reg_a)  /* Force memory constraint */
            : "r15", "memory", "cc"
        );
        
        /* Pattern 2: RELOAD_FOR_INPADDR_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
        /* Explicit register variable usage with clobbers */
        uint64_t* volatile ptr1 = (uint64_t*)(reg_a + idx1 * 8);
        uint64_t* volatile ptr2 = (uint64_t*)(reg_a + idx2 * 8);
        
        asm volatile (
            "movq (%[ptr1]), %%r14\n\t"
            "imulq $0x1F, %%r14, %%r14\n\t"
            "movq %%r14, (%[ptr2])"
            :
            : [ptr1] "r" (ptr1),
              [ptr2] "r" (ptr2),
              "m" (*ptr1), "m" (*ptr2)  /* Multiple memory constraints */
            : "r14", "memory", "cc"
        );
        
        /* Pattern 3: RELOAD_FOR_OPERAND_ADDRESS and RELOAD_FOR_OPADDR_ADDR */
        /* Mixed immediate, register, and memory constraints */
        uint64_t temp;
        asm volatile (
            "leaq (%[base], %[idx1], 8), %[temp]\n\t"
            "movq (%[temp]), %[temp]\n\t"
            "addq $0xDEADBEEF, %[temp]"
            : [temp] "=&r" (temp)
            : [base] "r" (reg_a),
              [idx1] "r" (idx1),
              "i" (0xDEADBEEF),          /* Immediate constraint */
              "m" (*(uint64_t*)(reg_a + idx1 * 8))  /* Memory constraint */
            : "cc"
        );
        
        /* Pattern 4: RELOAD_FOR_OUTPUT_ADDRESS */
        /* Output operand with memory constraint */
        uint64_t result;
        uint64_t* output_addr = &local_array[(i * 3) % 256];
        
        asm volatile (
            "movq %%r12, %[result]\n\t"
            "xorq $0xABCD, %[result]"
            : [result] "=m" (*output_addr)  /* Output to memory address */
            : "r" (reg_c)
            : "cc"
        );
        
        /* Use global volatile pointer to force more reloads */
        if (global_ptr) {
            /* Complex struct member access with volatile */
            uint64_t volatile* flag_ptr = &global_ptr->flags;
            uint64_t row = idx1 % 8;
            uint64_t col = idx2 % 8;
            
            /* Pattern triggering RELOAD_FOR_OTHER_ADDRESS */
            asm volatile (
                "movq (%[struct], %[row], 64), %%r15\n\t"  /* 64 = 8 * 8 bytes */
                "addq (%[struct], %[row], 64), %%r15\n\t"
                "movq %%r15, (%[struct], %[col], 8)"
                :
                : [struct] "r" (global_ptr),
                  [row] "r" (row),
                  [col] "r" (col),
                  "m" (global_ptr->data[row][0]),  /* Complex memory constraint */
                  "m" (global_ptr->data[0][col])
                : "r15", "memory", "cc"
            );
        }
        
        /* Clobber explicit registers to force spills/reloads */
        asm volatile ("" : : : "r10", "r11", "r12", "r13", "r14");
    }
}

/* Secondary function with different patterns */
void mixed_operand_test(void) {
    /* Large struct on stack */
    struct {
        uint64_t a[32];
        uint64_t b[32];
        volatile uint64_t c[32];
    } big_struct __attribute__((aligned(128)));
    
    /* Initialize */
    for (int i = 0; i < 32; i++) {
        big_struct.a[i] = i;
        big_struct.b[i] = i * 2;
        big_struct.c[i] = i * 3;
    }
    
    /* Multiple explicit register variables */
    register uint64_t r1 asm("r10") = (uint64_t)&big_struct;
    register uint64_t r2 asm("r11") = 16;
    register uint64_t r3 asm("r12") = 24;
    
    /* Pattern with "irm" constraint and fixed register */
    for (int i = 0; i < 8; i++) {
        uint64_t offset = i * 8;
        
        asm volatile (
            "movq (%[base], %[offset]), %%rax\n\t"
            "addq %[imm], %%rax\n\t"
            "movq %%rax, (%[base], %[idx2])"
            :
            : [base] "r" (r1),
              [offset] "r" (offset),
              [idx2] "r" (r2),
              [imm] "irm" (0x1234),      /* Immediate, register, or memory */
              "m" (big_struct.a[i]),     /* Force memory address reload */
              "m" (big_struct.b[16])     /* Another memory constraint */
            : "rax", "memory", "cc"
        );
        
        /* Switch between different index registers */
        asm volatile (
            "xchgq %%r11, %%r12\n\t"
            "addq $8, %%r10"
            : "+r" (r1), "+r" (r2), "+r" (r3)
            :
            : "cc"
        );
    }
}

/* Main function setting up test */
int main(int argc, char** argv) {
    /* Allocate and initialize global struct */
    struct MultiDim* data = malloc(sizeof(struct MultiDim));
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            data->data[i][j] = i * 8 + j;
        }
    }
    for (int i = 0; i < 16; i++) {
        data->extra[i] = i * 5;
    }
    data->flags = 0;
    
    global_ptr = data;
    
    /* Run tests with different iteration counts */
    complex_addressing_test(100);
    mixed_operand_test();
    
    /* Cleanup */
    free(data);
    
    return 0;
}
