/* test_reload_coverage.c
 * Designed to trigger multiple reload types in GCC's reload pass
 * Specifically targets the switch cases in chain_reload_insns()
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
volatile int g_index1 = 5;
volatile int g_index2 = 10;
volatile int g_base_offset = 100;

/* Complex data structures to force complex addressing */
struct InnerStruct {
    int data[16];
    int offset;
};

struct OuterStruct {
    struct InnerStruct inner[8];
    int base[32];
    struct InnerStruct* ptr;
};

/* Global test data */
struct OuterStruct g_nested_array[4];
int g_large_array[1024];
int* g_dynamic_ptr;

/* Function prototypes */
void init_test_data(void);
int test_input_address(void);
int test_output_address(void);
int test_operand_address(struct OuterStruct* s, int idx1, int idx2);
int test_mixed_reloads(void);
int test_inpaddr_address(void);
int test_outaddr_address(void);
int test_other_address(void);

/* Helper to force address computation before call */
__attribute__((noinline))
void use_address(void* addr) {
    asm volatile("" : : "r"(addr) : "memory");
}

/* Helper to force value usage */
__attribute__((noinline))
int use_value(int val) {
    asm volatile("" : : "r"(val) : "memory");
    return val;
}

/* Initialize test data structures */
void init_test_data(void) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 8; j++) {
            for (int k = 0; k < 16; k++) {
                g_nested_array[i].inner[j].data[k] = i * 1000 + j * 100 + k;
            }
            g_nested_array[i].inner[j].offset = j * 10;
        }
        g_nested_array[i].ptr = &g_nested_array[i].inner[0];
    }
    
    for (int i = 0; i < 1024; i++) {
        g_large_array[i] = i * 3;
    }
    
    g_dynamic_ptr = &g_large_array[500];
}

/* Test RELOAD_FOR_INPUT_ADDRESS */
int test_input_address(void) {
    int sum = 0;
    volatile int idx1 = g_index1;
    volatile int idx2 = g_index2;
    
    /* Complex addressing that requires input address reload */
    for (int i = 0; i < 4; i++) {
        /* Force input address computation with multiple registers */
        int val;
        asm volatile(
            "movl %[array], %[val]\n\t"
            : [val] "=r" (val)
            : [array] "m" (g_nested_array[i].inner[idx1].data[idx2])
            : "memory"
        );
        sum += val;
        
        /* Another complex input address */
        asm volatile(
            "addl %[offset], %[sum]\n\t"
            : [sum] "+r" (sum)
            : [offset] "m" (g_nested_array[(i + idx1) % 4].inner[idx2].offset)
            : "cc", "memory"
        );
    }
    
    return sum;
}

/* Test RELOAD_FOR_OUTPUT_ADDRESS */
int test_output_address(void) {
    int result = 0;
    volatile int offset = g_base_offset;
    
    /* Force output address reloads */
    for (int i = 0; i < 8; i++) {
        /* Complex output address computation */
        int temp = i * 7;
        asm volatile(
            "movl %[temp], %[dest]\n\t"
            : [dest] "=m" (g_large_array[offset + (i << 2)])
            : [temp] "r" (temp)
            : "memory"
        );
        
        /* Nested output address */
        asm volatile(
            "movl $0xABCD, %[dest]\n\t"
            : [dest] "=m" (g_nested_array[i % 4].inner[i / 2].data[i * 3 % 16])
            :
            : "memory"
        );
        
        result += temp;
    }
    
    return result;
}

/* Test RELOAD_FOR_OPERAND_ADDRESS and RELOAD_FOR_OPADDR_ADDR */
int test_operand_address(struct OuterStruct* s, int idx1, int idx2) {
    int sum = 0;
    
    /* Force operand address computation before function call */
    for (int i = 0; i < 3; i++) {
        /* Complex address passed to function */
        use_address(&s->inner[idx1 + i].data[idx2 + i * 2]);
        
        /* Another complex operand address */
        int* addr = &s->base[(idx1 << 2) + idx2 + i * 3];
        use_address(addr);
        
        /* Use the value at computed address */
        sum += *addr;
    }
    
    /* Mixed addressing in expression */
    sum += s->inner[idx1].data[idx2] + s->ptr->data[idx1];
    
    return sum;
}

/* Test RELOAD_FOR_INPADDR_ADDRESS */
int test_inpaddr_address(void) {
    int sum = 0;
    volatile int idx = g_index1;
    
    /* Force input address of address reloads */
    for (int i = 0; i < 6; i++) {
        struct InnerStruct* inner_ptr;
        
        /* Complex address computation for pointer */
        asm volatile(
            "leaq %[array], %[ptr]\n\t"
            : [ptr] "=r" (inner_ptr)
            : [array] "m" (g_nested_array[(idx + i) % 4].inner[i])
            : "memory"
        );
        
        /* Use the pointer with offset */
        sum += inner_ptr->data[i] + inner_ptr->offset;
    }
    
    return sum;
}

/* Test RELOAD_FOR_OUTADDR_ADDRESS */
int test_outaddr_address(void) {
    volatile int idx = g_index2;
    
    /* Force output address of address reloads */
    struct InnerStruct** ptr_ptr = &g_nested_array[0].ptr;
    
    for (int i = 0; i < 4; i++) {
        /* Complex address for pointer storage */
        asm volatile(
            "movq %[src], %[dest]\n\t"
            : [dest] "=m" (g_nested_array[i].ptr)
            : [src] "r" (&g_nested_array[(i + idx) % 4].inner[i % 2])
            : "memory"
        );
    }
    
    return idx;
}

/* Test RELOAD_FOR_OTHER_ADDRESS */
int test_other_address(void) {
    int sum = 0;
    
    /* Force other address reloads through complex control flow */
    int* volatile ptr_array[4];
    
    for (int i = 0; i < 4; i++) {
        /* Complex address computation in conditional */
        if (i & 1) {
            ptr_array[i] = &g_nested_array[i].inner[0].data[g_index1];
        } else {
            ptr_array[i] = &g_large_array[g_index2 + i * 8];
        }
        
        /* Use with additional offset */
        sum += ptr_array[i][i];
    }
    
    return sum;
}

/* Test mixed reload types in single function */
int test_mixed_reloads(void) {
    int total = 0;
    volatile int idx1 = g_index1;
    volatile int idx2 = g_index2;
    
    /* Mix input and output addressing */
    for (int i = 0; i < 5; i++) {
        /* Input address reload */
        int input_val;
        asm volatile(
            "movl %[src], %[val]\n\t"
            : [val] "=r" (input_val)
            : [src] "m" (g_nested_array[i % 3].inner[idx1].data[idx2 + i])
            : "memory"
        );
        
        /* Output address reload */
        int output_val = input_val * 2;
        asm volatile(
            "movl %[val], %[dest]\n\t"
            : [dest] "=m" (g_large_array[(idx1 << 3) + idx2 + i * 4])
            : [val] "r" (output_val)
            : "memory"
        );
        
        /* Operand address reload */
        total += test_operand_address(&g_nested_array[i % 3], idx1 + i, idx2 - i);
        
        /* Update indices to create dependencies */
        idx1 = (idx1 + 1) % 3;
        idx2 = (idx2 + 2) % 5;
    }
    
    return total;
}

/* Main driver */
int main(void) {
    int checksum = 0;
    
    init_test_data();
    
    printf("Testing various reload types...\n");
    
    /* Test each reload type */
    checksum += test_input_address();
    printf("test_input_address completed\n");
    
    checksum += test_output_address();
    printf("test_output_address completed\n");
    
    checksum += test_operand_address(&g_nested_array[0], g_index1, g_index2);
    printf("test_operand_address completed\n");
    
    checksum += test_inpaddr_address();
    printf("test_inpaddr_address completed\n");
    
    checksum += test_outaddr_address();
    printf("test_outaddr_address completed\n");
    
    checksum += test_other_address();
    printf("test_other_address completed\n");
    
    checksum += test_mixed_reloads();
    printf("test_mixed_reloads completed\n");
    
    printf("Final checksum: %d\n", checksum);
    
    return 0;
}
