/* test_reload_coverage.c
 * Designed to trigger specific reload types in GCC's reload pass
 * Compile with: gcc -O2 -fno-omit-frame-pointer -fno-strict-aliasing test_reload_coverage.c -o test
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Complex data structures to force complex addressing */
typedef struct {
    int data[8];
    int *ptr;
    int offset;
} InnerStruct;

typedef struct {
    InnerStruct inner[4];
    int matrix[4][4];
    volatile int index;
} OuterStruct;

/* Global variables to increase register pressure */
volatile int g_index1, g_index2, g_index3;
volatile int *g_ptr1, *g_ptr2;
OuterStruct g_outer;

/* Function to force RELOAD_FOR_INPUT_ADDRESS */
void test_input_address(OuterStruct *outer, int idx1, int idx2, int idx3) {
    /* Complex addressing that likely needs input address reload */
    int val;
    
    /* Multiple index computations in address */
    asm volatile (
        "movl %[array], %[val]\n\t"
        : [val] "=r" (val)
        : [array] "m" (outer->inner[idx1].data[idx2 + idx3 * 2])
        : "memory"
    );
    
    /* More complex addressing with shifting */
    asm volatile (
        ""
        :
        : "m" (outer->matrix[(idx1 << 1) + idx2][idx3]),
          "r" (idx1), "r" (idx2), "r" (idx3)
        : "memory"
    );
    
    g_ptr1 = &outer->inner[idx1].data[idx2];
}

/* Function to force RELOAD_FOR_OUTPUT_ADDRESS */
void test_output_address(OuterStruct *outer, int idx1, int idx2, int idx3) {
    /* Complex output addressing */
    int temp = idx1 + idx2;
    
    /* Output to memory with complex address computation */
    asm volatile (
        "movl %[temp], %[dest]\n\t"
        : [dest] "=m" (outer->inner[idx1].data[idx2 * 2 + idx3])
        : [temp] "r" (temp)
        : "memory"
    );
    
    /* Multiple output addresses with register pressure */
    asm volatile (
        ""
        : "=m" (outer->matrix[idx1][idx2]),
          "=m" (outer->inner[idx3].offset)
        : "r" (idx1), "r" (idx2), "r" (idx3)
        : "memory"
    );
}

/* Function to force RELOAD_FOR_OPERAND_ADDRESS */
void test_operand_address(InnerStruct *inner, int idx) {
    /* Taking address of complex expression */
    int *addr1 = &inner->data[idx * 2 + 3];
    int *addr2 = &inner->data[(idx << 2) & 7];
    
    /* Using addresses in inline asm */
    asm volatile (
        "addl $1, %0\n\t"
        "addl $2, %1\n\t"
        : "+m" (*addr1), "+m" (*addr2)
        :
        : "memory"
    );
}

/* Function to force RELOAD_FOR_INPADDR_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
void test_inpaddr_outaddr(OuterStruct *outer, int idx1, int idx2) {
    /* Mixed input/output with address computations */
    int temp1, temp2;
    
    /* Complex input address with separate address reload */
    asm volatile (
        "movl (%[addr1]), %[temp1]\n\t"
        "movl %[temp1], (%[addr2])\n\t"
        : [temp1] "=&r" (temp1), [temp2] "=r" (temp2)
        : [addr1] "r" (&outer->inner[idx1].data[idx2]),
          [addr2] "r" (&outer->matrix[idx1][idx2]),
          "m" (outer->inner[idx1].data[idx2]),
          "m" (outer->matrix[idx1][idx2])
        : "memory"
    );
    
    /* Chain of address computations */
    int *ptr1 = &outer->inner[idx1].data[0];
    int *ptr2 = ptr1 + idx2;
    
    asm volatile (
        ""
        : "+m" (*ptr2)
        : "r" (ptr1), "r" (idx2)
        : "memory"
    );
}

/* Function to force RELOAD_FOR_OTHER_ADDRESS */
void test_other_address(OuterStruct *outer, int idx) {
    /* Unusual addressing pattern */
    volatile int *volatile_ptr = &outer->index;
    
    /* Address computation with volatile */
    asm volatile (
        "movl %%eax, %0\n\t"
        : "=m" (outer->inner[(idx + *volatile_ptr) & 3].data[0])
        : "a" (idx)
        : "memory"
    );
    
    /* Multiple volatile accesses */
    int temp = *volatile_ptr;
    asm volatile (
        ""
        : "=m" (outer->matrix[temp][idx])
        : "r" (temp), "r" (idx)
        : "memory"
    );
}

/* Function to force RELOAD_OTHER type */
void test_other_reload(OuterStruct *outer) {
    /* Complex pattern that doesn't fit other categories */
    int i, j, k;
    
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            for (k = 0; k < 4; k++) {
                /* Triple nested addressing with computation */
                asm volatile (
                    "addl $1, %0\n\t"
                    : "+m" (outer->inner[i].data[j + k])
                    : "r" (i), "r" (j), "r" (k)
                    : "memory"
                );
            }
        }
    }
}

/* Mixed test combining multiple reload types */
int test_mixed(OuterStruct *outer, int idx1, int idx2, int idx3) {
    int result = 0;
    
    /* Force input address reload */
    test_input_address(outer, idx1, idx2, idx3);
    
    /* Force output address reload */
    test_output_address(outer, idx2, idx3, idx1);
    
    /* Create register pressure */
    volatile int a = idx1 * 2;
    volatile int b = idx2 * 3;
    volatile int c = idx3 * 4;
    
    /* Complex expression with multiple uses */
    result = outer->inner[a & 3].data[b & 7] + 
             outer->matrix[c & 3][a & 3];
    
    /* Force operand address reload */
    test_operand_address(&outer->inner[idx1], idx2);
    
    /* Force inpaddr/outaddr reloads */
    test_inpaddr_outaddr(outer, idx2, idx3);
    
    /* Force other address reload */
    test_other_address(outer, idx1);
    
    return result;
}

/* Main driver function */
int main() {
    OuterStruct outer;
    int i, j, k;
    int checksum = 0;
    
    /* Initialize data */
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 8; j++) {
            outer.inner[i].data[j] = i * 10 + j;
        }
        outer.inner[i].offset = i * 100;
        for (j = 0; j < 4; j++) {
            for (k = 0; k < 4; k++) {
                outer.matrix[j][k] = j * 4 + k;
            }
        }
    }
    outer.index = 42;
    
    /* Set global pointers to increase register pressure */
    g_ptr1 = &outer.inner[0].data[0];
    g_ptr2 = &outer.matrix[0][0];
    
    /* Run tests with various indices to trigger different reload patterns */
    for (i = 0; i < 3; i++) {
        for (j = 0; j < 3; j++) {
            for (k = 0; k < 3; k++) {
                g_index1 = i;
                g_index2 = j;
                g_index3 = k;
                
                /* Call mixed test */
                checksum += test_mixed(&outer, i, j, k);
                
                /* Call individual tests */
                test_input_address(&outer, i, j, k);
                test_output_address(&outer, j, k, i);
                test_operand_address(&outer.inner[i], j);
                test_inpaddr_outaddr(&outer, k, i);
                test_other_address(&outer, j);
            }
        }
    }
    
    /* Force OTHER reload type */
    test_other_reload(&outer);
    
    /* Final computation to prevent optimization */
    checksum += outer.index;
    for (i = 0; i < 4; i++) {
        checksum += outer.inner[i].offset;
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
