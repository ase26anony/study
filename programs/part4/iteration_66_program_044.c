/* test_reload_coverage.c
 * Designed to trigger multiple reload types in GCC's reload pass
 * Compile with: gcc -O2 -fno-omit-frame-pointer -fno-strict-aliasing test_reload_coverage.c -o test
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Complex data structures to force address computations */
typedef struct {
    int member_array[8];
    int padding[4];
} inner_struct_t;

typedef struct {
    inner_struct_t inner_struct;
    int outer_data[4];
    int* dynamic_ptr;
} outer_struct_t;

typedef struct {
    outer_struct_t nested_array[16];
    int meta_data[32];
} container_t;

/* Volatile variables to prevent optimization */
volatile int global_index = 0;
volatile int* volatile global_base = NULL;

/* Test function for RELOAD_FOR_INPUT_ADDRESS */
void test_input_address(container_t* container, int idx1, int idx2, int idx3) {
    /* Complex addressing that requires input address reload */
    int result;
    
    /* Force register pressure */
    register int r1 asm("r12") = idx1;
    register int r2 asm("r13") = idx2;
    register int r3 asm("r14") = idx3;
    
    /* Complex address computation that can't fit in single instruction */
    asm volatile (
        "movl (%[addr]), %[res]\n\t"
        : [res] "=r" (result)
        : [addr] "m" (container->nested_array[r1 + r2].inner_struct.member_array[r3 * 2]),
          "r" (r1), "r" (r2), "r" (r3)
        : "memory"
    );
    
    /* Use result to prevent dead code elimination */
    global_index += result;
}

/* Test function for RELOAD_FOR_OUTPUT_ADDRESS */
void test_output_address(container_t* container, int base_idx, int offset, int value) {
    /* Complex output addressing */
    register int b asm("r12") = base_idx;
    register int o asm("r13") = offset;
    register int v asm("r14") = value;
    
    /* Output to memory with complex address */
    asm volatile (
        "movl %[val], (%[addr])\n\t"
        : [addr] "=m" (container->nested_array[b].outer_data[o & 3])
        : [val] "r" (v),
          "r" (b), "r" (o)
        : "memory"
    );
}

/* Test function for RELOAD_FOR_INPADDR_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
void test_inpaddr_outaddr(container_t* container, int idx) {
    /* Mixed input/output with address computations */
    int temp1, temp2;
    register int i asm("r12") = idx;
    
    /* Input address reload for reading */
    asm volatile (
        "leal (%[base], %[idx], 8), %%eax\n\t"
        "movl (%%eax), %[t1]\n\t"
        : [t1] "=r" (temp1)
        : [base] "r" (container->nested_array),
          [idx] "r" (i)
        : "eax", "memory"
    );
    
    /* Output address reload for writing */
    asm volatile (
        "leal 64(%[base], %[idx], 4), %%ebx\n\t"
        "movl %[t1], (%%ebx)\n\t"
        : "=m" (container->meta_data[i])
        : [base] "r" (container),
          [idx] "r" (i),
          [t1] "r" (temp1)
        : "ebx", "memory"
    );
}

/* Test function for RELOAD_FOR_OPERAND_ADDRESS */
void helper_func(int* addr) {
    /* Force address to be computed before call */
    *addr += 1;
}

void test_operand_address(container_t* container, int idx1, int idx2) {
    /* Complex address passed as function argument */
    register int i1 asm("r12") = idx1;
    register int i2 asm("r13") = idx2;
    
    /* Address computation that needs reload before function call */
    helper_func(&container->nested_array[i1].inner_struct.member_array[i2 * 3]);
    
    /* Another complex address computation */
    helper_func(&container->meta_data[(i1 << 2) + i2]);
}

/* Test function for RELOAD_FOR_OPADDR_ADDR */
void test_opaddr_addr(container_t* container, int* indices, int count) {
    /* Multiple complex address computations in loop */
    for (int i = 0; i < count; i++) {
        int idx = indices[i];
        int* addr1 = &container->nested_array[idx & 0xF].outer_data[idx & 0x3];
        int* addr2 = &container->meta_data[(idx * 7) & 0x1F];
        
        /* Use both addresses in inline asm */
        asm volatile (
            "movl (%[a1]), %%eax\n\t"
            "addl %%eax, (%[a2])\n\t"
            : 
            : [a1] "r" (addr1),
              [a2] "r" (addr2)
            : "eax", "memory"
        );
    }
}

/* Test function for RELOAD_FOR_OTHER_ADDRESS */
void test_other_address(container_t* container, int idx) {
    /* Unusual addressing pattern */
    int* volatile ptr = container->nested_array[idx].dynamic_ptr;
    
    /* Chain of address computations */
    if (ptr) {
        int offset = (idx * 13) & 0x7F;
        asm volatile (
            "movl (%[base], %[off], 4), %%eax\n\t"
            "addl $1, %%eax\n\t"
            "movl %%eax, (%[ptr], %[off], 2)\n\t"
            : 
            : [base] "r" (container->meta_data),
              [off] "r" (offset),
              [ptr] "r" (ptr)
            : "eax", "memory"
        );
    }
}

/* Test function for RELOAD_OTHER */
void test_other_reload(container_t* container) {
    /* Multiple register constraints causing spill/reload */
    int a, b, c, d;
    
    asm volatile (
        "movl $1, %0\n\t"
        "movl $2, %1\n\t"
        "movl $3, %2\n\t"
        "movl $4, %3\n\t"
        : "=r" (a), "=r" (b), "=r" (c), "=r" (d)
        :
        : "memory"
    );
    
    /* Use all registers in complex computation */
    asm volatile (
        "imull %[a], %[b]\n\t"
        "addl %[c], %[b]\n\t"
        "subl %[d], %[b]\n\t"
        "movl %[b], (%[mem])\n\t"
        : 
        : [a] "r" (a),
          [b] "r" (b),
          [c] "r" (c),
          [d] "r" (d),
          [mem] "r" (&container->meta_data[0])
        : "memory"
    );
}

/* Main driver function */
int main() {
    /* Allocate and initialize test data */
    container_t* container = (container_t*)malloc(sizeof(container_t));
    if (!container) return 1;
    
    /* Initialize data */
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 8; j++) {
            container->nested_array[i].inner_struct.member_array[j] = i * 100 + j;
        }
        for (int j = 0; j < 4; j++) {
            container->nested_array[i].outer_data[j] = i * 10 + j;
        }
        container->nested_array[i].dynamic_ptr = &container->meta_data[i * 2];
    }
    
    for (int i = 0; i < 32; i++) {
        container->meta_data[i] = i;
    }
    
    global_base = container->meta_data;
    
    /* Call test functions with various parameters to trigger different reload types */
    
    /* Trigger input address reloads */
    for (int i = 0; i < 8; i++) {
        test_input_address(container, i, i+1, i+2);
    }
    
    /* Trigger output address reloads */
    for (int i = 0; i < 8; i++) {
        test_output_address(container, i, i*2, i*100);
    }
    
    /* Trigger inpaddr/outaddr reloads */
    for (int i = 0; i < 4; i++) {
        test_inpaddr_outaddr(container, i*3);
    }
    
    /* Trigger operand address reloads */
    int indices[] = {1, 3, 5, 7, 9, 11};
    test_operand_address(container, 2, 4);
    test_opaddr_addr(container, indices, 6);
    
    /* Trigger other address reloads */
    test_other_address(container, 5);
    
    /* Trigger other reloads */
    test_other_reload(container);
    
    /* Compute checksum to ensure code isn't optimized away */
    int checksum = 0;
    for (int i = 0; i < 32; i++) {
        checksum += container->meta_data[i];
    }
    
    printf("Checksum: %d\n", checksum);
    printf("Global index: %d\n", global_index);
    
    free(container);
    return 0;
}
