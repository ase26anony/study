/* test_reload_coverage.c
 * Designed to trigger various reload types in GCC's reload pass
 * Specifically targets the switch cases in chain_reload_insns()
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Complex data structures to force complex addressing */
typedef struct {
    int data[8];
    struct {
        int x;
        int y;
        int z;
    } coords;
} InnerStruct;

typedef struct {
    InnerStruct inner[4];
    int matrix[3][3];
    volatile int* volatile_ptr;
} OuterStruct;

/* Global arrays to create register pressure */
volatile int global_index = 0;
volatile int global_offset = 0;
OuterStruct global_array[16];
int global_buffer[256];

/* Function to prevent optimization */
static inline void memory_barrier(void) {
    asm volatile("" ::: "memory");
}

/* Test RELOAD_FOR_INPUT_ADDRESS */
void test_input_address(void) {
    int i, j, k;
    volatile int* volatile vi = &global_index;
    
    /* Complex addressing that requires input address reload */
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            for (k = 0; k < 4; k++) {
                /* Complex array indexing that will need address computation */
                int idx = (*vi + i) * 16 + (j << 2) + k;
                
                /* Inline asm with complex memory input operand */
                asm volatile(
                    "movl %[input], %%eax\n\t"
                    "addl $1, %%eax\n\t"
                    : /* no outputs */
                    : [input] "m" (global_array[idx % 16].inner[k].data[j * 2 + i])
                    : "eax", "memory"
                );
            }
        }
    }
}

/* Test RELOAD_FOR_OUTPUT_ADDRESS */
void test_output_address(int base) {
    int i;
    volatile int offset = global_offset;
    
    /* Complex output addressing */
    for (i = 0; i < 8; i++) {
        /* Compute address with multiple operations */
        int addr_idx = (base + i) * 3 + (offset % 4);
        
        /* Inline asm with complex memory output operand */
        asm volatile(
            "movl $0x1234, %%eax\n\t"
            "movl %%eax, %[output]\n\t"
            : [output] "=m" (global_buffer[addr_idx * 2 + (i & 3)])
            : /* no inputs */
            : "eax", "memory"
        );
    }
}

/* Test RELOAD_FOR_INPADDR_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
void test_inpaddr_outaddr(void) {
    int i, j;
    volatile int* ptr = &global_index;
    
    /* Mixed input/output with address computations */
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            /* Complex addressing for both input and output */
            int in_idx = (*ptr + i) * 8 + j;
            int out_idx = (*ptr + j) * 8 + i;
            
            /* Inline asm with both memory input and output */
            asm volatile(
                "movl %[in], %%eax\n\t"
                "addl $42, %%eax\n\t"
                "movl %%eax, %[out]\n\t"
                : [out] "=m" (global_array[out_idx % 16].matrix[i][j])
                : [in] "m" (global_array[in_idx % 16].inner[j].coords.x),
                  "m" (global_buffer[in_idx]) /* Extra memory input */
                : "eax", "memory"
            );
        }
    }
}

/* Test RELOAD_FOR_OPERAND_ADDRESS */
void helper_function(InnerStruct* complex_ptr) {
    /* Force address computation before call */
    asm volatile(
        "movl (%0), %%eax\n\t"
        "addl $1, %%eax\n\t"
        : /* no outputs */
        : "r" (&complex_ptr->data[complex_ptr->coords.x])
        : "eax", "memory"
    );
}

void test_operand_address(void) {
    int i;
    
    for (i = 0; i < 8; i++) {
        /* Complex address expression as function argument */
        int idx = (global_index + i) & 7;
        helper_function(&global_array[idx].inner[i % 4]);
    }
}

/* Test RELOAD_FOR_OTHER_ADDRESS and RELOAD_OTHER */
void test_other_address(void) {
    int i, j;
    volatile int temp;
    
    /* Multiple complex memory operations in sequence */
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            /* Nested structure access with computation */
            int base = global_index;
            int offset1 = (base + i) << 2;
            int offset2 = (base + j) << 1;
            
            /* Multiple memory operations forcing various reloads */
            asm volatile(
                /* First operation with complex address */
                "movl %[addr1], %%eax\n\t"
                "movl %%eax, %[temp]\n\t"
                
                /* Second operation with different complex address */
                "movl %[addr2], %%ebx\n\t"
                "addl %%eax, %%ebx\n\t"
                "movl %%ebx, %[addr3]\n\t"
                
                : [temp] "=m" (temp),
                  [addr3] "=m" (global_buffer[offset1 + offset2])
                : [addr1] "m" (global_array[offset1 % 16].inner[i].data[j]),
                  [addr2] "m" (global_array[offset2 % 16].matrix[i][j])
                : "eax", "ebx", "memory"
            );
        }
    }
}

/* Test RELOAD_FOR_OPADDR_ADDR */
void test_opaddr_addr(void) {
    int i;
    volatile int* volatile vptr = &global_index;
    
    /* Address of address computation */
    for (i = 0; i < 8; i++) {
        /* Complex pointer arithmetic */
        int** complex_ptr = (int**)&global_array[*vptr + i].volatile_ptr;
        
        asm volatile(
            "movl %[ptr], %%eax\n\t"
            "movl (%%eax), %%ebx\n\t"
            "addl $8, %%ebx\n\t"
            "movl %%ebx, (%%eax)\n\t"
            : /* no outputs */
            : [ptr] "r" (complex_ptr)
            : "eax", "ebx", "memory"
        );
    }
}

/* Mixed test combining multiple reload types */
void test_mixed_reloads(void) {
    int i, j, k;
    volatile int seed = global_index;
    
    /* Triple nested loop to increase register pressure */
    for (i = 0; i < 3; i++) {
        for (j = 0; j < 3; j++) {
            for (k = 0; k < 3; k++) {
                /* Multiple complex addressing modes */
                int idx1 = (seed + i) * 9 + j * 3 + k;
                int idx2 = (seed + k) * 9 + i * 3 + j;
                int idx3 = (seed + j) * 9 + k * 3 + i;
                
                /* Inline asm with multiple memory operands */
                asm volatile(
                    /* Input address reload */
                    "movl %[in1], %%eax\n\t"
                    
                    /* Output address reload */
                    "movl %%eax, %[out1]\n\t"
                    
                    /* Another input with different addressing */
                    "movl %[in2], %%ebx\n\t"
                    "addl %%eax, %%ebx\n\t"
                    
                    /* Complex output addressing */
                    "movl %%ebx, %[out2]\n\t"
                    
                    : [out1] "=m" (global_array[idx1 % 16].matrix[i][j]),
                      [out2] "=m" (global_buffer[idx2 * 2])
                    : [in1] "m" (global_array[idx3 % 16].inner[k].data[j]),
                      [in2] "m" (global_array[(idx1 + idx2) % 16].inner[i].coords.y)
                    : "eax", "ebx", "memory"
                );
            }
        }
    }
}

/* Main driver function */
int main(void) {
    int i, checksum = 0;
    
    /* Initialize test data */
    for (i = 0; i < 16; i++) {
        int j, k;
        for (j = 0; j < 4; j++) {
            for (k = 0; k < 8; k++) {
                global_array[i].inner[j].data[k] = i * 100 + j * 10 + k;
            }
            global_array[i].inner[j].coords.x = i + j;
            global_array[i].inner[j].coords.y = i * 2 + j;
            global_array[i].inner[j].coords.z = i * 3 + j;
        }
        
        for (j = 0; j < 3; j++) {
            for (k = 0; k < 3; k++) {
                global_array[i].matrix[j][k] = i * 10 + j * 3 + k;
            }
        }
        
        global_array[i].volatile_ptr = &global_index;
    }
    
    for (i = 0; i < 256; i++) {
        global_buffer[i] = i;
    }
    
    /* Run all tests to trigger different reload types */
    test_input_address();
    memory_barrier();
    
    test_output_address(global_index);
    memory_barrier();
    
    test_inpaddr_outaddr();
    memory_barrier();
    
    test_operand_address();
    memory_barrier();
    
    test_other_address();
    memory_barrier();
    
    test_opaddr_addr();
    memory_barrier();
    
    test_mixed_reloads();
    memory_barrier();
    
    /* Compute checksum to ensure code isn't optimized away */
    for (i = 0; i < 16; i++) {
        checksum += global_array[i].inner[0].data[0];
        checksum += global_array[i].matrix[0][0];
    }
    
    for (i = 0; i < 256; i++) {
        checksum += global_buffer[i];
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
