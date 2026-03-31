/* test_reload_coverage.c
 * Designed to trigger various reload types in GCC's reload pass
 * Specifically targets the switch cases in chain_reload_insns()
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Complex data structures to force address computations */
typedef struct {
    int data[8];
    int* ptr;
    int offset;
} InnerStruct;

typedef struct {
    InnerStruct inner[4];
    int matrix[4][4];
    volatile int* volatile_ptr;
} OuterStruct;

/* Global arrays to increase register pressure */
volatile int global_array[256];
OuterStruct global_structs[8];

/* Function to prevent optimization */
static int use_result(int val) {
    volatile int sink = val;
    return sink;
}

/* Test RELOAD_FOR_INPUT_ADDRESS */
void test_input_address(void) {
    int i, j, k;
    volatile int idx1 = 1, idx2 = 2, idx3 = 3;
    
    /* Complex addressing that requires input address reload */
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            for (k = 0; k < 4; k++) {
                /* Multiple index computations in address */
                int val = global_structs[i].inner[j].data[k + (idx1 * idx2)];
                
                /* Inline asm with complex memory input */
                asm volatile (
                    "/* Input address computation */\n\t"
                    : /* no outputs */
                    : "m" (global_structs[(i + j) & 7].matrix[k][(idx1 + idx2) & 3]),
                      "m" (global_array[(i << 2) + j + k + idx3])
                    : "memory"
                );
                
                use_result(val);
            }
        }
    }
}

/* Test RELOAD_FOR_OUTPUT_ADDRESS */
void test_output_address(void) {
    int i, j;
    volatile int offset = 4;
    
    /* Complex output addressing */
    for (i = 0; i < 8; i++) {
        for (j = 0; j < 4; j++) {
            /* Inline asm with complex memory output */
            asm volatile (
                "/* Output address computation */\n\t"
                : "=m" (global_structs[i].inner[j].data[(i * j + offset) & 7]),
                  "=m" (global_array[(i << 3) + (j << 1) + offset])
                : /* no inputs */
                : "memory"
            );
            
            /* Chain output addresses */
            global_structs[i].inner[j].ptr = 
                &global_array[(i << 4) + (j << 2) + offset];
        }
    }
}

/* Test RELOAD_FOR_OPERAND_ADDRESS */
void test_operand_address(OuterStruct* s, int idx1, int idx2) {
    /* Complex address passed as function argument */
    int* addr = &s->inner[idx1].data[idx2 + (idx1 << 1)];
    
    /* Force address computation before use */
    asm volatile (
        "/* Operand address */\n\t"
        : /* no outputs */
        : "r" (addr), "m" (*addr)
        : "memory"
    );
    
    use_result((intptr_t)addr);
}

/* Test RELOAD_FOR_INPADDR_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
void test_inpaddr_outaddr(void) {
    int i;
    volatile int base = 16;
    
    for (i = 0; i < 8; i++) {
        /* Mixed input/output with address-of operators */
        int* input_addr = &global_structs[i].inner[i & 3].data[base + i];
        int* output_addr = &global_array[(i << 2) + base];
        
        /* Complex addressing in both input and output */
        asm volatile (
            "/* Mixed inpaddr/outaddr */\n\t"
            : "=m" (*output_addr), "=m" (global_structs[i].matrix[i & 3][(i + base) & 3])
            : "m" (*input_addr), "m" (global_structs[(i + 1) & 7].inner[0].data[base]),
              "r" (input_addr), "r" (output_addr)
            : "memory"
        );
    }
}

/* Test RELOAD_FOR_OTHER_ADDRESS */
void test_other_address(void) {
    int i, j;
    volatile int offset1 = 2, offset2 = 3;
    
    /* Nested addressing with multiple computations */
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            /* Very complex addressing expression */
            int val = global_structs[i].inner[j].data[
                (i * offset1 + j * offset2 + 
                 global_array[(i << 2) + j]) & 7
            ];
            
            /* Multiple memory operands with different address computations */
            asm volatile (
                "/* Other address computations */\n\t"
                : /* no outputs */
                : "m" (global_structs[i].matrix[j][(offset1 + offset2) & 3]),
                  "m" (global_structs[j].inner[i].data[(i + j + offset1) & 7]),
                  "m" (global_array[(i << 3) + (j << 1) + offset2]),
                  "r" (offset1), "r" (offset2)
                : "memory"
            );
            
            use_result(val);
        }
    }
}

/* Test RELOAD_OTHER and mixed types */
void test_mixed_reloads(void) {
    int i;
    volatile int idx = 0;
    
    for (i = 0; i < 16; i++) {
        /* Constantly changing addressing mode */
        idx = (idx + 1) & 7;
        
        /* Mixed constraints to trigger various reload types */
        asm volatile (
            "/* Mixed reload types */\n\t"
            : "=m" (global_structs[idx].inner[idx & 3].data[i & 7]),
              "=r" (idx)
            : "m" (global_array[(i << 1) + idx]),
              "m" (global_structs[(i + 1) & 7].matrix[idx][idx & 3]),
              "0" (idx),
              "r" (i)
            : "memory", "cc"
        );
        
        /* Function call with complex address argument */
        test_operand_address(&global_structs[idx], idx, i & 3);
    }
}

/* Initialize test data */
void init_data(void) {
    int i, j, k;
    
    for (i = 0; i < 256; i++) {
        global_array[i] = i;
    }
    
    for (i = 0; i < 8; i++) {
        global_structs[i].volatile_ptr = &global_array[i * 16];
        
        for (j = 0; j < 4; j++) {
            global_structs[i].inner[j].offset = j;
            global_structs[i].inner[j].ptr = &global_array[i * 16 + j * 4];
            
            for (k = 0; k < 8; k++) {
                global_structs[i].inner[j].data[k] = i * 100 + j * 10 + k;
            }
            
            for (k = 0; k < 4; k++) {
                global_structs[i].matrix[j][k] = i * 16 + j * 4 + k;
            }
        }
    }
}

/* Main driver */
int main(void) {
    int checksum = 0;
    int i;
    
    init_data();
    
    printf("Starting reload coverage tests...\n");
    
    /* Run all tests multiple times to increase coverage chance */
    for (i = 0; i < 3; i++) {
        test_input_address();
        test_output_address();
        test_inpaddr_outaddr();
        test_other_address();
        test_mixed_reloads();
        
        /* Simple computation to use results */
        checksum += global_array[i * 8] + 
                   global_structs[i & 7].inner[0].data[0];
    }
    
    /* Call with complex addresses */
    for (i = 0; i < 4; i++) {
        test_operand_address(&global_structs[i], i, (i * 2) & 3);
        checksum += (intptr_t)&global_structs[i].inner[i & 3];
    }
    
    printf("Checksum: %d\n", checksum);
    printf("Tests completed.\n");
    
    return checksum != 0 ? 0 : 1;
}
