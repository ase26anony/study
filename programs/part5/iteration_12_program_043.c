/* reload_test.c - Complex addressing mode test for GCC reload pass */
#include <stdint.h>
#include <stdlib.h>

/* Force specific registers to be used */
register uint64_t reg_a asm("r10");
register uint64_t reg_b asm("r11");
register uint64_t reg_c asm("r12");
register uint64_t reg_d asm("r13");
register uint64_t reg_e asm("r14");

/* Complex structure to force memory addressing */
struct ComplexData {
    uint64_t data[8];
    uint64_t *ptr_array[4];
    struct ComplexData *next;
};

/* Volatile pointers to prevent optimization */
volatile struct ComplexData *volatile_data;
volatile uint64_t *volatile_base;

/* Multi-dimensional array with complex access pattern */
uint64_t multi_array[4][8][16];

/* Function with complex addressing that should trigger various reload types */
void test_complex_addressing(void) {
    /* Initialize register variables with non-trivial values */
    reg_a = (uint64_t)&multi_array[0][0][0];
    reg_b = 8;
    reg_c = 16;
    reg_d = 32;
    reg_e = 64;
    
    /* Allocate and initialize volatile data */
    volatile struct ComplexData *data1 = (volatile struct ComplexData*)malloc(sizeof(struct ComplexData));
    volatile struct ComplexData *data2 = (volatile struct ComplexData*)malloc(sizeof(struct ComplexData));
    
    for (int i = 0; i < 8; i++) {
        data1->data[i] = i * 2;
        data2->data[i] = i * 3;
    }
    
    data1->ptr_array[0] = (uint64_t*)&data1->data[0];
    data1->ptr_array[1] = (uint64_t*)&data1->data[4];
    data1->next = data2;
    
    volatile_data = data1;
    volatile_base = &data1->data[0];
    
    /* Loop with complex addressing modes - should trigger RELOAD_FOR_INPUT_ADDRESS */
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 8; j++) {
            /* Complex address computation involving multiple registers */
            uint64_t offset = (reg_b * i + reg_c * j) & 0xFF;
            
            /* Inline assembly with memory constraint that needs address reload */
            asm volatile (
                "addq %[mem1], %[reg1]\n\t"
                "movq %[reg1], %[mem2]\n\t"
                : [reg1] "+r" (reg_a)
                : [mem1] "m" (volatile_data->data[offset % 8]),
                  [mem2] "m" (multi_array[i][j][offset % 16])
                : "memory", "cc"
            );
            
            /* Another asm with explicit register constraints causing conflicts */
            uint64_t temp = reg_d + reg_e;
            asm volatile (
                "leaq (%[base], %[idx], 8), %[addr]\n\t"
                "movq (%[addr]), %[val]\n\t"
                : [addr] "=r" (reg_b), [val] "=r" (reg_c)
                : [base] "r" (volatile_base), 
                  [idx] "r" (temp),
                  "m" (*(volatile uint64_t*)(volatile_base + temp))
                : "memory"
            );
        }
    }
    
    /* Test RELOAD_FOR_OTHER_ADDRESS with complex pointer chains */
    for (int iter = 0; iter < 100; iter++) {
        volatile struct ComplexData *current = volatile_data;
        uint64_t accum = 0;
        
        while (current != NULL) {
            /* Complex addressing through pointer array - may need RELOAD_FOR_OTHER_ADDRESS */
            for (int k = 0; k < 4; k++) {
                uint64_t *ptr = current->ptr_array[k];
                if (ptr) {
                    /* Mixed constraints: immediate, register, and memory */
                    asm volatile (
                        "imulq $0x%a[imm], %%r15, %%r15\n\t"
                        "addq (%%r15), %[sum]\n\t"
                        : [sum] "+r" (accum)
                        : [imm] "i" (k + 1),
                          "r" (ptr),
                          "m" (*ptr)
                        : "r15", "memory", "cc"
                    );
                }
            }
            
            /* Access through next pointer with offset */
            if (current->next) {
                asm volatile (
                    "movq 0x20(%[ptr]), %[val]\n\t"
                    "addq %[val], %[acc]\n\t"
                    : [acc] "+r" (accum), [val] "=r" (reg_d)
                    : [ptr] "r" (current),
                      "m" (*(volatile uint64_t*)((char*)current + 0x20))
                    : "cc"
                );
            }
            
            current = current->next;
        }
        
        /* Store result back through complex address */
        uint64_t store_addr = (uint64_t)volatile_base + reg_a + reg_b;
        asm volatile (
            "movq %[val], (%[addr])\n\t"
            : 
            : [val] "r" (accum),
              [addr] "r" (store_addr),
              "m" (*(volatile uint64_t*)store_addr)
            : "memory"
        );
    }
    
    /* Test RELOAD_FOR_OPADDR_ADDR with nested address computations */
    {
        /* Explicit register variables that conflict with addressing */
        register uint64_t idx1 asm("r8") = reg_a;
        register uint64_t idx2 asm("r9") = reg_b;
        
        for (int i = 0; i < 50; i++) {
            /* Complex index calculation */
            uint64_t complex_idx = (idx1 * i + idx2 * (i * 2)) % 128;
            
            /* Inline asm that uses the index for multiple purposes */
            asm volatile (
                "movq (%[base], %[idx], 8), %[tmp1]\n\t"
                "leaq (%[base], %[tmp1], 4), %[tmp2]\n\t"
                "movq %[tmp2], (%[dest], %[idx], 8)\n\t"
                : [tmp1] "=&r" (reg_e), [tmp2] "=&r" (reg_d)
                : [base] "r" (volatile_base),
                  [idx] "r" (complex_idx),
                  [dest] "r" (&multi_array[0][0][0]),
                  "m" (*(volatile uint64_t*)(volatile_base + complex_idx * 8)),
                  "m" (multi_array[0][0][complex_idx % 16])
                : "memory"
            );
            
            /* Update indices with complex operations */
            asm volatile (
                "addq %%r12, %%r8\n\t"
                "subq %%r13, %%r9\n\t"
                : "+r" (idx1), "+r" (idx2)
                : 
                : "cc"
            );
        }
    }
    
    free((void*)data1);
    free((void*)data2);
}

/* Main function that sets up the test */
int main(void) {
    /* Initialize multi-dimensional array */
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 8; j++) {
            for (int k = 0; k < 16; k++) {
                multi_array[i][j][k] = i + j * 2 + k * 3;
            }
        }
    }
    
    /* Run the complex addressing test multiple times */
    for (int run = 0; run < 10; run++) {
        test_complex_addressing();
    }
    
    return 0;
}
