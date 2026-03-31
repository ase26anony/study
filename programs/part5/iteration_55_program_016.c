/* reload_coverage.c - Program to trigger various reload types in GCC reload1.cc */

#include <stdint.h>
#include <stdlib.h>

/* Force specific architecture for register scarcity */
#ifdef __x86_64__
#undef __x86_64__
#endif

/* Volatile variables to prevent optimization */
static volatile int global_seed = 42;

/* Complex structure for address computations */
struct NestedData {
    int values[8];
    struct NestedData* next;
    short offsets[16];
    volatile long counter;
};

/* Large arrays to force spills */
static int large_array[256];
static struct NestedData data_pool[32];

/* ========== TEST 1: Complex Array Addressing ========== */
/* Targets: RELOAD_FOR_INPUT, RELOAD_FOR_INPUT_ADDRESS, RELOAD_FOR_INPADDR_ADDRESS */
__attribute__((noinline))
static int test_complex_addressing(int idx1, int idx2, int idx3) {
    /* Many local variables to consume registers */
    volatile int a1 = idx1 + global_seed;
    volatile int a2 = idx2 * 3;
    volatile int a3 = idx3 / 2;
    volatile int a4 = a1 ^ a2;
    volatile int a5 = a2 | a3;
    volatile int a6 = a3 & a4;
    volatile int a7 = a4 + a5;
    volatile int a8 = a5 - a6;
    volatile int a9 = a6 * a7;
    volatile int a10 = a7 / (a8 ? a8 : 1);
    
    /* Multi-dimensional addressing - forces address reloads */
    int md_array[4][8][16];
    
    /* Complex address computation requiring multiple registers */
    int result = 0;
    
    /* Nested addressing with multiple index calculations */
    for (int i = 0; i < 4; i++) {
        /* Force address computation into register */
        int* base = &md_array[i][(a1 + i) % 8][0];
        
        /* RELOAD_FOR_INPUT_ADDRESS: base needs to be in register for addressing */
        for (int j = 0; j < 8; j++) {
            /* Complex offset calculation */
            int offset = (a2 + j * a3) % 16;
            
            /* This forces address reloads:
               base + offset*sizeof(int) needs address computation */
            base[offset] = a4 + i * j;
            
            /* More computations to clobber registers */
            a1 = a2 + a3;
            a2 = a3 ^ a4;
            a3 = a4 | a5;
        }
        
        /* Even more complex: address of address computation */
        int** addr_ptr = (int**)&base;
        
        /* RELOAD_FOR_INPADDR_ADDRESS: address of the input address */
        result += **addr_ptr;
    }
    
    return result + a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10;
}

/* ========== TEST 2: Structure Access with Pointer Chains ========== */
/* Targets: RELOAD_FOR_OPERAND_ADDRESS, RELOAD_FOR_OPADDR_ADDR, RELOAD_FOR_OTHER_ADDRESS */
__attribute__((noinline))
static int test_structure_chains(int iterations) {
    /* Initialize data pool with chain */
    for (int i = 0; i < 31; i++) {
        data_pool[i].next = &data_pool[i + 1];
        for (int j = 0; j < 8; j++) {
            data_pool[i].values[j] = i * 100 + j;
        }
    }
    data_pool[31].next = &data_pool[0];
    
    /* Many local variables for register pressure */
    volatile long l1 = iterations;
    volatile long l2 = global_seed;
    volatile long l3 = l1 * l2;
    volatile long l4 = l2 / 7;
    volatile long l5 = l3 ^ l4;
    volatile long l6 = l4 | l5;
    volatile long l7 = l5 & l6;
    volatile long l8 = l6 + l7;
    volatile long l9 = l7 - l8;
    volatile long l10 = l8 * l9;
    
    struct NestedData* current = &data_pool[0];
    int sum = 0;
    
    /* Pointer chasing with complex addressing */
    for (int i = 0; i < iterations; i++) {
        /* RELOAD_FOR_OPERAND_ADDRESS: current->values needs address computation */
        int* values_ptr = current->values;
        
        /* Complex index calculation */
        int idx = (l1 + i * l2) % 8;
        
        /* Access with offset - forces operand address reload */
        sum += values_ptr[idx];
        
        /* More complex: address of structure member pointer */
        int** values_addr = &values_ptr;
        
        /* RELOAD_FOR_OPADDR_ADDR: address of the operand address */
        if (*values_addr) {
            sum += (*values_addr)[idx ^ 1];
        }
        
        /* Chain traversal with computation */
        int offset_idx = (l3 + i) % 16;
        
        /* RELOAD_FOR_OTHER_ADDRESS: obscure address type */
        sum += current->offsets[offset_idx];
        
        /* Update many variables to clobber registers */
        l1 = l2 + l3;
        l2 = l3 * l4;
        l3 = l4 ^ l5;
        l4 = l5 | l6;
        l5 = l6 & l7;
        
        current = current->next;
    }
    
    return sum + l1 + l2 + l3 + l4 + l5 + l6 + l7 + l8 + l9 + l10;
}

/* ========== TEST 3: Inline Assembly with Multiple Outputs ========== */
/* Targets: RELOAD_FOR_OUTPUT_ADDRESS, RELOAD_FOR_OUTADDR_ADDRESS, RELOAD_OTHER */
__attribute__((noinline))
static int test_inline_asm(int x, int y, int z) {
    /* Many volatile variables to force memory traffic */
    volatile int v1 = x;
    volatile int v2 = y;
    volatile int v3 = z;
    volatile int v4 = v1 + v2;
    volatile int v5 = v2 * v3;
    volatile int v6 = v3 ^ v4;
    volatile int v7 = v4 | v5;
    volatile int v8 = v5 & v6;
    volatile int v9 = v6 + v7;
    volatile int v10 = v7 - v8;
    
    int result1, result2, result3;
    int* output_addr;
    
    /* Complex address for output */
    int output_buffer[32];
    int complex_index = (v1 * v2 + v3) % 32;
    
    /* Inline asm with multiple outputs and clobbers */
    /* This forces output address reloads */
    __asm__ volatile (
        /* Multiple output operands */
        "movl %[in1], %%eax\n\t"
        "addl %[in2], %%eax\n\t"
        "movl %%eax, %[out1]\n\t"
        
        "imull %[in3], %%eax\n\t"
        "movl %%eax, %[out2]\n\t"
        
        /* Complex output to memory address */
        "leal (%[buf], %[idx], 4), %%ebx\n\t"
        "movl %%eax, (%%ebx)\n\t"
        "movl %%ebx, %[out_addr]\n\t"
        
        /* Clobber many registers */
        :
        [out1] "=r" (result1),
        [out2] "=r" (result2),
        [out_addr] "=r" (output_addr)
        :
        [in1] "r" (v1),
        [in2] "r" (v2),
        [in3] "r" (v3),
        [buf] "r" (output_buffer),
        [idx] "r" (complex_index)
        : "eax", "ebx", "ecx", "edx", "memory", "cc"
    );
    
    /* Use the results to prevent dead code elimination */
    v1 = result1 + result2;
    v2 = (int)output_addr - (int)output_buffer;
    
    /* More inline asm with alternative constraints */
    int final_result;
    __asm__ volatile (
        /* Alternative constraints force different reload types */
        "addl %%eax, %%ebx\n\t"
        "movl %%ebx, %0\n\t"
        : "=r" (final_result)
        : "a" (v1), "b" (v2)
        : "cc"
    );
    
    return final_result + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
}

/* ========== TEST 4: Mixed Patterns with Loop Unrolling ========== */
/* Targets all reload types through unrolled patterns */
#pragma GCC unroll 4
__attribute__((noinline))
static int test_mixed_unrolled(int base) {
    /* Extreme register pressure with many variables */
    volatile int r0 = base;
    volatile int r1 = r0 + 1;
    volatile int r2 = r1 * 2;
    volatile int r3 = r2 / 3;
    volatile int r4 = r3 ^ r0;
    volatile int r5 = r4 | r1;
    volatile int r6 = r5 & r2;
    volatile int r7 = r6 + r3;
    volatile int r8 = r7 - r4;
    volatile int r9 = r8 * r5;
    volatile int r10 = r9 / (r6 ? r6 : 1);
    volatile int r11 = r10 ^ r7;
    volatile int r12 = r11 | r8;
    volatile int r13 = r12 & r9;
    volatile int r14 = r13 + r10;
    volatile int r15 = r14 - r11;
    
    /* Multi-dimensional local array */
    int local_md[3][5][7];
    
    int total = 0;
    
    /* Manually unrolled loop with different patterns */
    /* Iteration 1: Complex array addressing */
    {
        int* ptr1 = &local_md[(r0 + 0) % 3][(r1 + 1) % 5][0];
        int idx1 = (r2 * r3) % 7;
        ptr1[idx1] = r4;
        total += ptr1[idx1];
    }
    
    /* Iteration 2: Structure-like access */
    {
        struct { int a; int b[4]; volatile int c; } s;
        s.a = r5;
        s.b[(r6 + 2) % 4] = r7;
        s.c = r8;
        total += s.a + s.b[0] + s.c;
    }
    
    /* Iteration 3: Pointer arithmetic */
    {
        char* byte_ptr = (char*)local_md;
        byte_ptr += (r9 * 4 + r10 * 8) % sizeof(local_md);
        total += *byte_ptr;
    }
    
    /* Iteration 4: Address of address */
    {
        int** ptr_to_ptr = (int**)&r11;
        total += **ptr_to_ptr;
    }
    
    return total + r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 + r10 + r11 + r12 + r13 + r14 + r15;
}

/* ========== MAIN DRIVER ========== */
int main(int argc, char** argv) {
    /* Initialize global arrays */
    for (int i = 0; i < 256; i++) {
        large_array[i] = i * 3;
    }
    
    int result = 0;
    
    /* Call all test functions with different arguments */
    result += test_complex_addressing(argc, 7, 13);
    result += test_structure_chains(argc < 2 ? 8 : argc);
    result += test_inline_asm(result, argc * 2, global_seed);
    result += test_mixed_unrolled(result);
    
    /* Call multiple times with different patterns */
    for (int i = 0; i < 4; i++) {
        result += test_complex_addressing(i, i*2, i*3);
        result += test_structure_chains(4);
    }
    
    /* Use result to prevent optimization */
    return result > 1000 ? 0 : 1;
}
