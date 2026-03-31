/* reload1_trigger.c - Program to trigger specific reload types in GCC's reload pass */

#include <stdint.h>
#include <stdlib.h>

/* Complex structure to force various addressing modes */
struct ComplexData {
    uint64_t a[8];
    uint32_t b[16];
    uint16_t c[32];
    uint8_t d[64];
    uint64_t e[4];
};

/* Volatile structure to prevent optimization */
volatile struct ComplexData global_data;

/* Function with complex addressing patterns */
void trigger_reloads(void) {
    /* Explicit register variables - will force specific register allocation */
    register uint64_t r10_var asm("r10") = 0x12345678;
    register uint64_t r11_var asm("r11") = 0x87654321;
    register uint64_t r12_var asm("r12") = (uint64_t)&global_data;
    register uint64_t r13_var asm("r13") = 0;
    register uint64_t r14_var asm("r14") = 0;
    register uint64_t r15_var asm("r15") = 0;
    
    /* Local arrays with different alignments */
    uint64_t array1[32] __attribute__((aligned(64)));
    uint32_t array2[64] __attribute__((aligned(32)));
    uint16_t array3[128] __attribute__((aligned(16)));
    
    /* Volatile pointers to prevent optimization */
    volatile uint64_t *volatile ptr1 = array1;
    volatile uint32_t *volatile ptr2 = array2;
    volatile uint16_t *volatile ptr3 = array3;
    
    /* Initialize arrays */
    for (int i = 0; i < 32; i++) array1[i] = i * 2;
    for (int i = 0; i < 64; i++) array2[i] = i * 3;
    for (int i = 0; i < 128; i++) array3[i] = i * 5;
    
    /* Complex loop with multiple addressing modes */
    for (int outer = 0; outer < 4; outer++) {
        /* Force RELOAD_FOR_INPUT_ADDRESS and RELOAD_FOR_OTHER_ADDRESS */
        for (int i = 0; i < 8; i++) {
            /* Pattern 1: Complex memory addressing with multiple registers */
            asm volatile (
                /* This forces address reloads due to complex constraints */
                "movq (%[base], %[idx], 8), %[temp1]\n\t"
                "addq %[offset], %[temp1]\n\t"
                "movq %[temp1], (%[dest], %[idx2], 4)"
                : [temp1] "=&r" (r13_var)
                : [base] "r" (ptr1), 
                  [idx] "r" (r10_var + i), 
                  [offset] "irm" (0x1000),  /* Mixed: immediate, register, or memory */
                  [dest] "r" (ptr2), 
                  [idx2] "r" (r11_var + i * 2)
                : "memory"
            );
            
            /* Pattern 2: Forces RELOAD_FOR_OPADDR_ADDR */
            /* Complex address computation involving multiple registers */
            uint64_t complex_addr = (uint64_t)(ptr3 + (r12_var & 0xFF) + (r10_var >> 8));
            
            asm volatile (
                /* Multiple memory operands with register constraints */
                "movw (%[addr1]), %%ax\n\t"
                "addw (%[addr2]), %%ax\n\t"
                "movw %%ax, (%[addr3])"
                : 
                : [addr1] "r" (complex_addr),
                  [addr2] "r" ((uint64_t)(ptr2 + (r11_var & 0x3F))),
                  [addr3] "r" ((uint64_t)(&global_data.d[i]))
                : "ax", "memory"
            );
            
            /* Pattern 3: Forces RELOAD_FOR_INPADDR_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
            /* Nested address computation */
            volatile uint64_t *nested_ptr = (volatile uint64_t *)((uint64_t)ptr1 + 
                                                                  (r10_var << 2) + 
                                                                  (r11_var >> 4));
            
            asm volatile (
                /* Input and output addresses both need reloads */
                "movq (%[in_addr]), %[out]\n\t"
                "imulq $3, %[out], %[out]\n\t"
                "movq %[out], (%[out_addr])"
                : [out] "=r" (r14_var)
                : [in_addr] "r" (nested_ptr),
                  [out_addr] "r" (&global_data.e[i % 4]),
                  "m" (*(struct ComplexData*)&global_data)  /* Force memory constraint */
                : "memory"
            );
            
            /* Pattern 4: Forces RELOAD_FOR_OPERAND_ADDRESS and RELOAD_FOR_OTHER_ADDRESS */
            /* Multiple explicit register variables in complex expressions */
            asm volatile (
                /* Complex operation with many register constraints */
                "leaq (%[r10], %[r11], 4), %[temp]\n\t"
                "addq %[r12], %[temp]\n\t"
                "movq %[temp], (%[mem])"
                : [temp] "=&r" (r15_var)
                : [r10] "r" (r10_var),
                  [r11] "r" (r11_var),
                  [r12] "r" (r12_var),
                  [mem] "r" (&array1[(i + outer) % 32])
                : "memory"
            );
            
            /* Modify register variables to change addressing patterns */
            r10_var = (r10_var * 1103515245 + 12345) & 0x7FFFFFFF;
            r11_var = (r11_var * 1664525 + 1013904223) & 0x7FFFFFFF;
        }
        
        /* Pattern 5: Forces RELOAD_FOR_OUTPUT_ADDRESS */
        /* Complex output addressing with immediate offset */
        uint64_t output_base = (uint64_t)&global_data.a[0];
        
        asm volatile (
            /* Output address needs reloading due to complex computation */
            "movq $0xDEADBEEF, (%[out_base], %[offset], 8)"
            : 
            : [out_base] "r" (output_base),
              [offset] "r" (r10_var & 0x7),
              "m" (global_data.a[0])  /* Memory constraint forces address reload */
            : "memory"
        );
        
        /* Access volatile structure through complex pointer chain */
        volatile struct ComplexData *volatile ptr_chain = &global_data;
        for (int j = 0; j < 2; j++) {
            /* Forces RELOAD_FOR_INPUT_ADDRESS with volatile */
            asm volatile (
                "movq (%[ptr]), %%rax\n\t"
                "addq $1, %%rax\n\t"
                "movq %%rax, (%[ptr])"
                : 
                : [ptr] "r" (&ptr_chain->a[(r11_var + j) % 8]),
                  "m" (*ptr_chain)  /* Whole structure memory clobber */
                : "rax", "memory"
            );
            
            ptr_chain = (volatile struct ComplexData *volatile)((uint64_t)ptr_chain + 64);
        }
    }
    
    /* Final complex pattern mixing all addressing modes */
    {
        /* Multi-dimensional array access simulation */
        uint64_t md_array[4][8][16];
        volatile uint64_t (*volatile md_ptr)[8][16] = md_array;
        
        /* Triple nested loop with complex indexing */
        for (int i = 0; i < 2; i++) {
            for (int j = 0; j < 4; j++) {
                for (int k = 0; k < 8; k++) {
                    /* Complex index computation using multiple registers */
                    uint64_t idx = (r10_var * i + r11_var * j + r12_var * k) % 16;
                    
                    asm volatile (
                        /* Forces multiple reload types simultaneously */
                        "movq (%[base], %[idx], 8), %[temp]\n\t"
                        "addq %[imm], %[temp]\n\t"
                        "movq %[temp], (%[dest])"
                        : [temp] "=&r" (r13_var)
                        : [base] "r" (&(*md_ptr)[j][0]),
                          [idx] "r" (idx),
                          [imm] "irm" (0x100),  /* Mixed constraint */
                          [dest] "r" (&global_data.a[k % 8]),
                          "m" (*(uint64_t(*)[8][16])md_ptr)  /* Force memory constraint */
                        : "memory"
                    );
                }
            }
        }
    }
}

/* Main function that sets up and calls the trigger function */
int main(void) {
    /* Initialize global data */
    for (int i = 0; i < 8; i++) global_data.a[i] = i;
    for (int i = 0; i < 16; i++) global_data.b[i] = i * 2;
    for (int i = 0; i < 32; i++) global_data.c[i] = i * 3;
    for (int i = 0; i < 64; i++) global_data.d[i] = i * 5;
    for (int i = 0; i < 4; i++) global_data.e[i] = i * 7;
    
    /* Call the function that triggers reloads multiple times */
    for (int i = 0; i < 3; i++) {
        trigger_reloads();
    }
    
    /* Use the results to prevent dead code elimination */
    asm volatile ("" : : "r"(global_data.a[0]), "r"(global_data.b[0]), 
                           "r"(global_data.c[0]), "r"(global_data.d[0]) : "memory");
    
    return 0;
}
