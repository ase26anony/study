/* test_reload_coverage.c
 * Designed to trigger multiple reload types in GCC's reload pass
 * Specifically targets the switch cases in chain_reload_insns()
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Complex nested structure to force address computations */
struct inner {
    int data[8];
    volatile int* volatile_ptr;
};

struct outer {
    struct inner arrays[4];
    volatile int index_mask;
    int padding[3];
};

/* Global volatile variables to prevent optimization */
volatile int g_index1 = 1;
volatile int g_index2 = 2;
volatile int g_base_offset = 100;

/* Test RELOAD_FOR_INPUT_ADDRESS */
void test_input_address(struct outer* ctx, int idx1, int idx2) {
    /* Complex addressing: array[idx1].arrays[idx2].data[idx1+idx2] */
    int val;
    
    /* Force address computation with multiple register values */
    asm volatile (
        "movl (%[addr]), %[val]\n\t"
        : [val] "=r" (val)
        : [addr] "m" (ctx->arrays[idx1].arrays[idx2].data[idx1 + idx2])
        : "memory"
    );
    
    /* Use the value to prevent dead code elimination */
    ctx->arrays[0].arrays[0].data[0] += val;
}

/* Test RELOAD_FOR_OUTPUT_ADDRESS */
void test_output_address(struct outer* ctx, int offset1, int offset2) {
    /* Complex output address with shift operation */
    int temp = offset1 * 4 + offset2;
    
    /* Force output address reload */
    asm volatile (
        "movl %[tmp], (%[addr])\n\t"
        : 
        : [tmp] "r" (temp),
          [addr] "m" (ctx->arrays[offset1].data[offset2 << 1])
        : "memory"
    );
}

/* Test RELOAD_FOR_INPUT and mixed types */
int test_mixed_reloads(struct outer* ctx, int loop_count) {
    int sum = 0;
    volatile int* volatile_index = &g_index1;
    
    for (int i = 0; i < loop_count; i++) {
        int idx1 = (*volatile_index + i) & 3;
        int idx2 = (i * 2) & 7;
        
        /* Mixed input/output with complex addressing */
        int input_val;
        asm volatile (
            "movl (%[in_addr]), %[in_val]\n\t"
            "addl $1, %[in_val]\n\t"
            "movl %[in_val], (%[out_addr])\n\t"
            : [in_val] "=&r" (input_val)
            : [in_addr] "m" (ctx->arrays[idx1].data[idx2]),
              [out_addr] "m" (ctx->arrays[(idx1 + 1) & 3].data[(idx2 + 1) & 7])
            : "memory"
        );
        
        sum += input_val;
        
        /* Force another address computation */
        test_input_address(ctx, idx1, idx2);
    }
    
    return sum;
}

/* Test RELOAD_FOR_OPERAND_ADDRESS and RELOAD_FOR_OPADDR_ADDR */
void test_operand_address(struct outer* ctx, int idx) {
    /* Take address of complex expression */
    int* addr1 = &ctx->arrays[idx].data[idx * 2];
    int* addr2 = &ctx->arrays[idx + 1].data[0];
    
    /* Use in inline asm with memory clobber */
    asm volatile (
        "movl (%1), %%eax\n\t"
        "addl %%eax, (%0)\n\t"
        : 
        : "r" (addr1), "r" (addr2)
        : "eax", "memory"
    );
    
    /* Nested addressing in function call */
    helper_function(&ctx->arrays[idx].data[ctx->index_mask & 7]);
}

/* Helper function to force address computation before call */
void helper_function(int* ptr) {
    *ptr += 1;
}

/* Test RELOAD_FOR_INPADDR_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
void test_addr_of_addr(struct outer* ctx) {
    int** addr_ptr;
    int temp;
    
    /* Address of an address computation */
    addr_ptr = &ctx->arrays[0].volatile_ptr;
    
    /* Complex chain: compute address, then use it */
    asm volatile (
        "movl (%[addr_ptr]), %%eax\n\t"
        "movl (%%eax), %[temp]\n\t"
        : [temp] "=r" (temp)
        : [addr_ptr] "m" (addr_ptr)
        : "eax", "memory"
    );
    
    /* Output to address that itself needs address computation */
    int* out_addr = &ctx->arrays[1].data[g_index2];
    asm volatile (
        "movl %[val], (%%eax)\n\t"
        : 
        : [val] "r" (temp),
          "a" (out_addr)
        : "memory"
    );
}

/* Test RELOAD_FOR_OTHER_ADDRESS */
void test_other_address(void) {
    /* Create a situation with multiple indirect references */
    struct outer* volatile volatile_ctx = (struct outer*)malloc(sizeof(struct outer));
    
    if (volatile_ctx) {
        /* Complex addressing with volatile pointer */
        int idx = g_index1;
        asm volatile (
            "movl (%[base], %[idx], 8), %%eax\n\t"
            "movl 4(%%eax, %[idx], 4), %%ebx\n\t"
            : 
            : [base] "r" (volatile_ctx->arrays),
              [idx] "r" (idx)
            : "eax", "ebx", "memory"
        );
        
        free((void*)volatile_ctx);
    }
}

/* Main driver */
int main(void) {
    /* Initialize test structure */
    struct outer ctx;
    
    /* Initialize with pattern */
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            for (int k = 0; k < 8; k++) {
                ctx.arrays[i].arrays[j].data[k] = i * 100 + j * 10 + k;
            }
            ctx.arrays[i].arrays[j].volatile_ptr = &ctx.arrays[i].arrays[0].data[0];
        }
    }
    ctx.index_mask = 0x7;
    
    int checksum = 0;
    
    /* Test various reload patterns */
    test_input_address(&ctx, g_index1, g_index2);
    checksum += ctx.arrays[0].arrays[0].data[0];
    
    test_output_address(&ctx, 2, 3);
    checksum += ctx.arrays[2].data[6];
    
    checksum += test_mixed_reloads(&ctx, 4);
    
    test_operand_address(&ctx, 1);
    checksum += ctx.arrays[1].data[2];
    
    test_addr_of_addr(&ctx);
    checksum += ctx.arrays[1].data[g_index2];
    
    test_other_address();
    
    printf("Final checksum: %d\n", checksum);
    return checksum != 0 ? 0 : 1;
}
