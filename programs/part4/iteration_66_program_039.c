/* test_reload_coverage.c
 * Designed to trigger various reload types in GCC's reload pass
 * Specifically targets the switch cases in chain_reload_insns()
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Complex data structures to force complex addressing */
typedef struct {
    int data[4];
    struct {
        int inner[3];
        long offset;
    } nested;
} ComplexStruct;

typedef struct {
    ComplexStruct* array[8];
    volatile int indices[2];
    long base_offset;
} Container;

/* Global volatile variables to prevent optimization */
volatile int g_index1 = 1;
volatile int g_index2 = 2;
volatile long g_offset = 100;

/* Function 1: Trigger RELOAD_FOR_INPUT_ADDRESS */
void test_input_address(Container* cont, int idx1, int idx2) {
    /* Complex addressing with multiple components */
    int val;
    
    /* Force input address reload: array[idx1]->nested.inner[idx2] */
    asm volatile(
        "movl %[input], %[output]\n\t"
        : [output] "=r" (val)
        : [input] "m" (cont->array[idx1]->nested.inner[idx2]),
          "r" (cont), "r" (idx1), "r" (idx2)
        : "memory"
    );
    
    /* Additional complexity with shifting */
    asm volatile(
        ""
        :
        : "m" (cont->array[(idx1 << 1) + idx2]->data[g_index1]),
          "r" (cont), "r" (idx1), "r" (idx2), "r" (g_index1)
        : "memory"
    );
}

/* Function 2: Trigger RELOAD_FOR_OUTPUT_ADDRESS */
void test_output_address(Container* cont, int idx1, int idx2, int value) {
    /* Complex output addressing */
    asm volatile(
        "movl %[val], %[output]\n\t"
        : [output] "=m" (cont->array[idx1]->data[idx2 + g_index2])
        : [val] "r" (value),
          "r" (cont), "r" (idx1), "r" (idx2), "r" (g_index2)
        : "memory"
    );
    
    /* Nested output with structure member */
    asm volatile(
        "movq %[val], %[output]\n\t"
        : [output] "=m" (cont->array[g_index1]->nested.offset)
        : [val] "r" ((long)value * 2),
          "r" (cont), "r" (g_index1)
        : "memory"
    );
}

/* Function 3: Trigger RELOAD_FOR_INPADDR_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
void test_inpaddr_outaddr(Container* cont, int idx) {
    int temp;
    volatile int* volatile_ptr = &cont->indices[0];
    
    /* Mixed input/output with address computations */
    asm volatile(
        "movl (%[in_addr]), %[temp]\n\t"
        "addl $1, %[temp]\n\t"
        "movl %[temp], (%[out_addr])\n\t"
        : [temp] "=&r" (temp),
          [out_addr] "=r" (volatile_ptr)
        : [in_addr] "r" (&cont->array[idx]->data[g_index1]),
          "1" (volatile_ptr),
          "r" (cont), "r" (idx), "r" (g_index1)
        : "memory"
    );
}

/* Function 4: Trigger RELOAD_FOR_OPERAND_ADDRESS */
void helper_func(int* addr1, long* addr2, volatile int* addr3) {
    /* Force address computation before call */
    asm volatile(
        ""
        :
        : "r" (addr1), "r" (addr2), "r" (addr3)
        : "memory"
    );
}

void test_operand_address(Container* cont, int idx1, int idx2) {
    /* Complex address expressions as function arguments */
    helper_func(
        &cont->array[idx1]->data[idx2 * 2],
        &cont->array[idx2]->nested.offset,
        &cont->indices[(idx1 + idx2) & 1]
    );
}

/* Function 5: Trigger RELOAD_FOR_OTHER_ADDRESS and RELOAD_OTHER */
void test_other_address(Container* cont, int idx) {
    volatile long* addr_array[4];
    int i;
    
    /* Create address computation in loop */
    for (i = 0; i < 4; i++) {
        addr_array[i] = &cont->array[(idx + i) % 4]->nested.offset;
        
        /* Force other address reload */
        asm volatile(
            "movq (%[addr]), %%rax\n\t"
            "addq $8, %%rax\n\t"
            "movq %%rax, (%[addr])\n\t"
            :
            : [addr] "r" (addr_array[i]),
              "r" (cont), "r" (idx), "r" (i)
            : "rax", "memory"
        );
    }
    
    /* Additional complexity with multiple constraints */
    asm volatile(
        ""
        :
        : "m" (cont->array[0]->data[0]),
          "m" (cont->array[1]->data[1]),
          "m" (cont->array[2]->data[2]),
          "r" (cont)
        : "memory"
    );
}

/* Function 6: Mixed reload types in complex loop */
void test_mixed_reloads(Container* cont, int iterations) {
    int i, j;
    volatile int sum = 0;
    
    for (i = 0; i < iterations; i++) {
        for (j = 0; j < 4; j++) {
            /* Mix input and output addressing */
            int input_val;
            
            /* Input address reload */
            asm volatile(
                "movl %[in], %[out]\n\t"
                : [out] "=r" (input_val)
                : [in] "m" (cont->array[i % 4]->data[j + g_index1]),
                  "r" (cont), "r" (i), "r" (j), "r" (g_index1)
                : "memory"
            );
            
            /* Output address reload */
            asm volatile(
                "movl %[val], %[out]\n\t"
                : [out] "=m" (cont->array[j % 4]->nested.inner[i % 3])
                : [val] "r" (input_val + i + j),
                  "r" (cont), "r" (i), "r" (j)
                : "memory"
            );
            
            sum += input_val;
        }
    }
    
    /* Use sum to prevent dead code elimination */
    asm volatile("" : : "r" (sum) : "memory");
}

/* Main driver function */
int main() {
    int i;
    Container* cont = malloc(sizeof(Container));
    
    /* Initialize container */
    for (i = 0; i < 8; i++) {
        cont->array[i] = malloc(sizeof(ComplexStruct));
        for (int j = 0; j < 4; j++) {
            cont->array[i]->data[j] = i * 10 + j;
        }
        for (int j = 0; j < 3; j++) {
            cont->array[i]->nested.inner[j] = i * 5 + j;
        }
        cont->array[i]->nested.offset = i * 100;
    }
    
    cont->indices[0] = 0;
    cont->indices[1] = 1;
    cont->base_offset = 1000;
    
    /* Call test functions to trigger different reload types */
    test_input_address(cont, 1, 2);
    test_output_address(cont, 2, 1, 42);
    test_inpaddr_outaddr(cont, 3);
    test_operand_address(cont, 0, 2);
    test_other_address(cont, 1);
    test_mixed_reloads(cont, 3);
    
    /* Compute checksum to ensure all code executes */
    int checksum = 0;
    for (i = 0; i < 8; i++) {
        for (int j = 0; j < 4; j++) {
            checksum += cont->array[i]->data[j];
        }
        checksum += (int)cont->array[i]->nested.offset;
    }
    
    printf("Checksum: %d\n", checksum);
    
    /* Cleanup */
    for (i = 0; i < 8; i++) {
        free(cont->array[i]);
    }
    free(cont);
    
    return 0;
}
