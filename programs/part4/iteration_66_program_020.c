/* test_reload_coverage.c
 * Designed to trigger various reload types in GCC's reload pass
 * Specifically targets the switch cases in chain_reload_insns()
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Prevent optimizations that might simplify addressing */
#define NOOPT __attribute__((optimize("O0")))
#define VOLATILE_DEREF(ptr) (*(volatile int*)(ptr))

/* Complex data structures to force address computations */
struct Inner {
    int data[8];
    int* ptr_array[4];
};

struct Outer {
    struct Inner inner[4];
    int matrix[4][8];
    volatile int* volatile base_ptr;
};

/* Global to prevent optimization */
volatile int global_index = 0;
struct Outer global_struct;

/* Helper to force address computation before call */
NOOPT void use_address(int* addr) {
    asm volatile("" : : "r"(addr) : "memory");
}

/* Test RELOAD_FOR_INPUT_ADDRESS */
NOOPT void test_input_address(void) {
    struct Outer local;
    volatile int idx1 = global_index;
    volatile int idx2 = idx1 + 1;
    
    /* Complex addressing that requires input address reload */
    asm volatile(
        "movl (%[addr]), %%eax\n\t"
        : 
        : [addr] "m" (local.inner[idx1 & 3].data[(idx2 * 2) & 7])
        : "eax", "memory"
    );
    
    /* Another complex input address */
    int sum = 0;
    for (int i = 0; i < 4; i++) {
        asm volatile(
            "addl (%[base], %[index], 4), %%eax\n\t"
            : "+a"(sum)
            : [base] "r" (local.matrix[0]),
              [index] "r" (i * 2 + (idx1 & 1))
            : "memory"
        );
    }
}

/* Test RELOAD_FOR_OUTPUT_ADDRESS */
NOOPT void test_output_address(void) {
    struct Outer local;
    volatile int offset = global_index * 3;
    
    /* Complex output address computation */
    asm volatile(
        "movl $42, (%[addr])\n\t"
        : 
        : [addr] "m" (local.matrix[(offset >> 2) & 3][offset & 7])
        : "memory"
    );
    
    /* Output with register-based index */
    volatile int* dynamic_ptr = &local.inner[0].data[0];
    for (int i = 0; i < 4; i++) {
        int idx = (i + offset) & 7;
        asm volatile(
            "movl %%ebx, (%[base], %[idx], 4)\n\t"
            : 
            : [base] "r" (dynamic_ptr),
              [idx] "r" (idx),
              "b" (i * 10)
            : "memory"
        );
    }
}

/* Test RELOAD_FOR_INPADDR_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
NOOPT void test_inpaddr_outaddr(void) {
    struct Outer local;
    volatile int idx = global_index;
    
    /* Mixed input/output with complex addressing */
    int temp;
    asm volatile(
        "movl (%[in]), %%eax\n\t"
        "addl $100, %%eax\n\t"
        "movl %%eax, (%[out])\n\t"
        : "=a"(temp)
        : [in] "m" (local.inner[(idx >> 1) & 3].ptr_array[idx & 3]),
          [out] "m" (local.matrix[idx & 3][(idx * 2) & 7])
        : "memory"
    );
    
    /* Chain of address computations */
    int* ptr1 = local.inner[0].data;
    int* ptr2 = local.inner[1].data;
    for (int i = 0; i < 4; i++) {
        asm volatile(
            "movl (%[src], %[i1], 4), %%ecx\n\t"
            "movl %%ecx, (%[dst], %[i2], 4)\n\t"
            : 
            : [src] "r" (ptr1),
              [i1] "r" ((i + idx) & 7),
              [dst] "r" (ptr2),
              [i2] "r" ((i * 2) & 7),
              "c" (0)
            : "memory"
        );
    }
}

/* Test RELOAD_FOR_OPERAND_ADDRESS */
NOOPT void test_operand_address(void) {
    struct Outer local;
    volatile int idx = global_index;
    
    /* Force operand address computation before function call */
    use_address(&local.inner[idx & 3].data[(idx * 3) & 7]);
    
    /* Multiple complex operand addresses */
    use_address(&local.matrix[(idx >> 1) & 3][idx & 7]);
    use_address(local.inner[(idx >> 2) & 3].ptr_array[idx & 3]);
}

/* Test RELOAD_FOR_OPADDR_ADDR */
NOOPT void test_opaddr_addr(void) {
    struct Outer local;
    volatile int base_idx = global_index;
    
    /* Complex address of an address */
    int** addr_of_ptr = &local.inner[base_idx & 3].ptr_array[(base_idx >> 1) & 3];
    asm volatile(
        "movl (%[addr]), %%edx\n\t"
        "movl $99, (%%edx)\n\t"
        : 
        : [addr] "r" (addr_of_ptr)
        : "edx", "memory"
    );
    
    /* Nested addressing */
    int* volatile* volatile pp = &local.inner[0].ptr_array[0];
    for (int i = 0; i < 4; i++) {
        int* p = pp[i];
        asm volatile(
            "movl $55, (%[p], %[off], 4)\n\t"
            : 
            : [p] "r" (p),
              [off] "r" ((i + base_idx) & 3)
            : "memory"
        );
    }
}

/* Test RELOAD_FOR_OTHER_ADDRESS */
NOOPT void test_other_address(void) {
    struct Outer local;
    volatile int idx1 = global_index;
    volatile int idx2 = idx1 * 2 + 1;
    
    /* Unusual address computation pattern */
    asm volatile(
        "leal (%[a], %[b], 2), %%esi\n\t"
        "movl (%%esi), %%edi\n\t"
        : 
        : [a] "r" (&local.inner[0].data[0]),
          [b] "r" (idx1 + idx2)
        : "esi", "edi", "memory"
    );
    
    /* Multiple base registers in address */
    int* base1 = &local.matrix[0][0];
    int* base2 = &local.inner[0].data[0];
    asm volatile(
        "movl (%[b1], %[i1], 4), %%eax\n\t"
        "addl (%[b2], %[i2], 4), %%eax\n\t"
        "movl %%eax, (%[b1], %[i3], 4)\n\t"
        : 
        : [b1] "r" (base1),
          [i1] "r" (idx1 & 7),
          [b2] "r" (base2),
          [i2] "r" (idx2 & 7),
          [i3] "r" ((idx1 + idx2) & 7),
          "a" (0)
        : "memory"
    );
}

/* Test RELOAD_OTHER */
NOOPT void test_reload_other(void) {
    struct Outer local;
    volatile int idx = global_index;
    
    /* Multiple memory operands with register pressure */
    register int r1 asm("ebx") = idx * 2;
    register int r2 asm("ecx") = idx * 3;
    register int r3 asm("edx") = idx * 4;
    
    asm volatile(
        "imull %[r1], %[r2]\n\t"
        "addl %[r3], %[r2]\n\t"
        "movl %[r2], (%[mem])\n\t"
        : [r2] "+r" (r2)
        : [r1] "r" (r1),
          [r3] "r" (r3),
          [mem] "m" (local.inner[(idx >> 2) & 3].data[(idx * 5) & 7])
        : "memory"
    );
}

/* Comprehensive test mixing all types */
NOOPT void test_mixed_reloads(void) {
    struct Outer local;
    volatile int idx = global_index;
    
    /* Create register pressure */
    int a = idx * 1;
    int b = idx * 2;
    int c = idx * 3;
    int d = idx * 4;
    int e = idx * 5;
    int f = idx * 6;
    
    /* Force all into registers */
    asm volatile("" : "+r"(a), "+r"(b), "+r"(c), "+r"(d), "+r"(e), "+r"(f));
    
    /* Complex computation with multiple memory accesses */
    for (int i = 0; i < 8; i++) {
        /* Input address reload */
        int val;
        asm volatile(
            "movl (%[addr]), %%eax\n\t"
            : "=a"(val)
            : [addr] "m" (local.matrix[a & 3][(b + i) & 7])
            : "memory"
        );
        
        /* Output address reload */
        asm volatile(
            "movl %%eax, (%[addr])\n\t"
            : 
            : [addr] "m" (local.inner[c & 3].data[(d + i) & 7]),
              "a" (val + e + f)
            : "memory"
        );
        
        /* Operand address for function call */
        use_address(&local.inner[(i >> 1) & 3].ptr_array[i & 3]);
    }
}

/* Main driver */
int main(void) {
    /* Initialize global struct */
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 8; j++) {
            global_struct.inner[i].data[j] = i * 100 + j;
            global_struct.matrix[i][j] = i * 200 + j;
        }
        for (int j = 0; j < 4; j++) {
            global_struct.inner[i].ptr_array[j] = &global_struct.inner[(i + j) & 3].data[0];
        }
    }
    
    /* Run tests with different indices to trigger various reload patterns */
    int checksum = 0;
    
    for (int iter = 0; iter < 100; iter++) {
        global_index = iter;
        
        test_input_address();
        test_output_address();
        test_inpaddr_outaddr();
        test_operand_address();
        test_opaddr_addr();
        test_other_address();
        test_reload_other();
        
        if (iter % 10 == 0) {
            test_mixed_reloads();
        }
        
        /* Simple checksum to ensure code runs */
        checksum += global_struct.inner[iter & 3].data[iter & 7];
    }
    
    printf("Checksum: %d\n", checksum);
    printf("Test completed - reload patterns should have been triggered during compilation\n");
    
    return 0;
}
