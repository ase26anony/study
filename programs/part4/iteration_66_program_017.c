/* test_reload_coverage.c
 * Designed to trigger multiple reload types in GCC's reload pass
 * Specifically targets the switch cases in chain_reload_insns()
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Volatile variables to prevent optimization */
volatile int g_index1 = 1;
volatile int g_index2 = 2;
volatile int g_index3 = 3;

/* Complex data structures to force address computations */
struct Inner {
    int data[8];
    volatile int* ptr;
};

struct Outer {
    struct Inner arrays[4];
    int offsets[4];
    volatile long base;
};

/* Global test structures */
struct Outer g_outer;
int g_global_array[256];
volatile int* g_volatile_ptr = g_global_array;

/* Function to force RELOAD_FOR_INPUT_ADDRESS */
void test_input_address(struct Outer* outer, int idx1, int idx2) {
    /* Complex addressing: outer->arrays[idx1].data[idx2] */
    /* This forces address computation before memory access */
    asm volatile (
        "/* Input address computation */\n\t"
        : /* no outputs */
        : "m" (outer->arrays[idx1].data[idx2]),
          "r" (outer),
          "r" (idx1),
          "r" (idx2)
        : "memory"
    );
    
    /* Nested structure access with volatile */
    int val = outer->arrays[(idx1 << 1) + g_index1].data[idx2 + g_index2];
    asm volatile ("" : "+r" (val) : : "memory");
}

/* Function to force RELOAD_FOR_OUTPUT_ADDRESS */
void test_output_address(struct Outer* outer, int idx1, int idx2, int value) {
    /* Complex output addressing */
    asm volatile (
        "/* Output address computation */\n\t"
        : "=m" (outer->arrays[idx1].data[idx2])
        : "r" (value),
          "r" (outer),
          "r" (idx1),
          "r" (idx2)
        : "memory"
    );
    
    /* Multiple output addresses in sequence */
    outer->offsets[idx1] = value;
    outer->arrays[idx2].ptr = &outer->offsets[idx1];
}

/* Function to force RELOAD_FOR_OPERAND_ADDRESS */
void test_operand_address(struct Inner* inner, int* base, int offset) {
    /* Passing complex address as function argument */
    asm volatile (
        "/* Operand address for call */\n\t"
        : 
        : "r" (&inner->data[offset]),
          "r" (base),
          "r" (offset)
        : "memory"
    );
    
    /* Force address computation before use */
    int* complex_addr = &inner->data[(offset << 2) + *base];
    asm volatile ("" : "+r" (complex_addr) : : "memory");
}

/* Function to force RELOAD_FOR_INPADDR_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
void test_inpaddr_outaddr(struct Outer* outer, int idx) {
    /* Mixed input/output with address-of operations */
    volatile int* input_ptr = &outer->arrays[idx].data[0];
    volatile int** output_ptr_ptr = &outer->arrays[idx].ptr;
    
    asm volatile (
        "/* Mixed inpaddr/outaddr */\n\t"
        : "=m" (*output_ptr_ptr)
        : "m" (*input_ptr),
          "r" (input_ptr),
          "r" (output_ptr_ptr)
        : "memory"
    );
    
    /* Chain of address computations */
    int** ptr_to_ptr = (int**)&outer->arrays[idx].ptr;
    *ptr_to_ptr = &outer->offsets[idx];
}

/* Function to force RELOAD_FOR_OTHER_ADDRESS */
void test_other_address(int* array, int size) {
    /* Complex loop with addressing that doesn't fit other categories */
    for (volatile int i = 0; i < size; i++) {
        /* Unusual addressing pattern */
        int* addr = &array[(i * g_index1) + (i >> g_index2)];
        asm volatile (
            "/* Other address computation */\n\t"
            : 
            : "r" (addr),
              "r" (i),
              "m" (*addr)
            : "memory"
        );
    }
}

/* Function to force RELOAD_OTHER */
void test_reload_other(void) {
    /* Multiple volatile operations that force various reloads */
    volatile int tmp1 = g_index1;
    volatile int tmp2 = g_index2;
    volatile int tmp3 = g_index3;
    
    /* Complex expression with multiple volatile accesses */
    int result = (tmp1 * tmp2) + (tmp3 << 2);
    
    asm volatile (
        "/* Mixed reloads for OTHER */\n\t"
        : "=r" (result)
        : "m" (g_global_array[result]),
          "m" (g_global_array[result + 1]),
          "0" (result)
        : "memory"
    );
}

/* Function to force RELOAD_FOR_OPADDR_ADDR */
void test_opaddr_addr(struct Outer* outer) {
    /* Address of address computation */
    int (*addr_func)(struct Outer*, int) = NULL;
    
    /* Force address of a computed address */
    asm volatile (
        "/* OPADDR_ADDR computation */\n\t"
        : "=m" (addr_func)
        : "r" (&outer->base),
          "r" (outer)
        : "memory"
    );
    
    /* Complex pointer chain */
    volatile int*** triple_ptr = (volatile int***)&outer->arrays[0].ptr;
    asm volatile ("" : "+r" (triple_ptr) : : "memory");
}

/* Main test driver */
int main(void) {
    int checksum = 0;
    
    /* Initialize test data */
    for (int i = 0; i < 4; i++) {
        g_outer.offsets[i] = i * 10;
        for (int j = 0; j < 4; j++) {
            for (int k = 0; k < 8; k++) {
                g_outer.arrays[j].data[k] = (j * 100) + (k * 10);
            }
            g_outer.arrays[j].ptr = &g_outer.offsets[j];
        }
    }
    
    g_outer.base = (long)&g_global_array[0];
    
    /* Test 1: Input address reloads */
    printf("Testing input address reloads...\n");
    for (int i = 0; i < 3; i++) {
        test_input_address(&g_outer, i, i * 2);
        checksum += g_outer.arrays[i].data[i * 2];
    }
    
    /* Test 2: Output address reloads */
    printf("Testing output address reloads...\n");
    for (int i = 0; i < 3; i++) {
        test_output_address(&g_outer, i, i + 1, i * 100);
        checksum += g_outer.arrays[i].data[i + 1];
    }
    
    /* Test 3: Mixed inpaddr/outaddr */
    printf("Testing inpaddr/outaddr reloads...\n");
    for (int i = 0; i < 2; i++) {
        test_inpaddr_outaddr(&g_outer, i);
        checksum += (int)g_outer.arrays[i].ptr;
    }
    
    /* Test 4: Operand address reloads */
    printf("Testing operand address reloads...\n");
    for (int i = 0; i < 3; i++) {
        test_operand_address(&g_outer.arrays[i], &g_outer.offsets[i], i);
        checksum += g_outer.arrays[i].data[i];
    }
    
    /* Test 5: Other address reloads */
    printf("Testing other address reloads...\n");
    test_other_address(g_global_array, 16);
    checksum += g_global_array[0];
    
    /* Test 6: RELOAD_OTHER */
    printf("Testing RELOAD_OTHER...\n");
    test_reload_other();
    checksum += g_index1 + g_index2 + g_index3;
    
    /* Test 7: OPADDR_ADDR */
    printf("Testing OPADDR_ADDR reloads...\n");
    test_opaddr_addr(&g_outer);
    checksum += (int)g_outer.base;
    
    /* Final checksum to ensure computations aren't optimized away */
    printf("Final checksum: %d\n", checksum);
    
    return checksum != 0 ? 0 : 1;
}
