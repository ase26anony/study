/* reload_coverage.c - Program to trigger various reload types in GCC reload1.cc */
#include <stdint.h>
#include <stdlib.h>

/* Force specific architecture for register scarcity */
#ifdef __x86_64__
#undef __x86_64__
#endif

/* Disable optimizations that might reduce register pressure */
#pragma GCC optimize ("O0")
#pragma GCC push_options

/* ========== TEST FUNCTION 1: Complex Array Addressing ========== */
/* Targets: RELOAD_FOR_INPUT, RELOAD_FOR_INPUT_ADDRESS, RELOAD_FOR_INPADDR_ADDRESS */
static __attribute__((noinline)) 
int test_array_addressing(int seed) {
    /* Many local variables to consume registers */
    volatile int a1, a2, a3, a4, a5, a6, a7, a8, a9, a10;
    volatile int b1, b2, b3, b4, b5, b6, b7, b8, b9, b10;
    int c1, c2, c3, c4, c5, c6, c7, c8, c9, c10;
    
    /* Multi-dimensional arrays with complex indexing */
    int arr3d[3][4][5];
    int arr2d[8][8];
    int *ptr_arr[16];
    
    /* Initialize with volatile writes to force spills */
    a1 = seed; a2 = seed + 1; a3 = seed + 2; a4 = seed + 3; a5 = seed + 4;
    a6 = seed + 5; a7 = seed + 6; a8 = seed + 7; a9 = seed + 8; a10 = seed + 9;
    
    /* Complex array addressing - forces address reloads */
    /* RELOAD_FOR_INPUT_ADDRESS: computing arr3d[i][j][k] address */
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 4; j++) {
            for (int k = 0; k < 5; k++) {
                /* Nested addressing with multiple computations */
                arr3d[i][j][k] = a1 * i + a2 * j + a3 * k + a4;
            }
        }
    }
    
    /* More complex: pointer arithmetic with multiple bases */
    /* RELOAD_FOR_INPADDR_ADDRESS: address of address computation */
    int *base_ptr = &arr3d[0][0][0];
    int offset1 = a5;
    int offset2 = a6;
    int stride = a7;
    
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            /* Complex address computation that needs reloading */
            arr2d[i][j] = *(base_ptr + offset1 * i + offset2 * j + stride);
            /* Use result in another computation */
            arr2d[i][j] += *(base_ptr + offset2 * i + offset1 * j + stride + 1);
        }
    }
    
    /* Chain computations to create data dependencies */
    c1 = arr3d[a1 % 3][a2 % 4][a3 % 5];
    c2 = arr2d[a4 % 8][a5 % 8];
    c3 = c1 + c2 + arr3d[a6 % 3][a7 % 4][a8 % 5];
    c4 = c2 * c3 - arr2d[a9 % 8][a10 % 8];
    
    /* Force all values to be used */
    return c1 + c2 + c3 + c4;
}

/* ========== TEST FUNCTION 2: Structure and Inline Assembly ========== */
/* Targets: RELOAD_FOR_OUTPUT_ADDRESS, RELOAD_FOR_OUTADDR_ADDRESS, RELOAD_FOR_OPERAND_ADDRESS */
static __attribute__((noinline))
int test_struct_asm(int seed) {
    /* Complex structure with many members */
    struct BigStruct {
        int data[16];
        int *ptr;
        volatile int counter;
        int more_data[8];
    };
    
    /* Multiple structure instances */
    struct BigStruct s1, s2, s3, s4;
    volatile struct BigStruct *vs1 = &s1;
    volatile struct BigStruct *vs2 = &s2;
    
    /* Initialize structures */
    for (int i = 0; i < 16; i++) {
        s1.data[i] = seed + i;
        s2.data[i] = seed * i;
        s3.data[i] = seed - i;
        s4.data[i] = seed / (i + 1);
    }
    
    /* Pointer chasing with complex addressing */
    int *ptr_chain[10];
    ptr_chain[0] = &s1.data[0];
    for (int i = 1; i < 10; i++) {
        ptr_chain[i] = ptr_chain[i-1] + (seed % (i + 3));
    }
    
    /* Inline assembly with multiple outputs and memory operands */
    /* This forces output address reloads */
    int result1, result2, result3;
    int addr1 = (int)(&s1.data[5]);
    int addr2 = (int)(&s2.data[7]);
    int addr3 = (int)(&s3.data[3]);
    
    /* Complex asm with multiple memory outputs */
    __asm__ volatile (
        "movl %[a1], %%eax\n\t"
        "movl %[a2], %%ebx\n\t"
        "movl %[a3], %%ecx\n\t"
        "addl %%ebx, %%eax\n\t"
        "subl %%ecx, %%eax\n\t"
        "movl %%eax, %[r1]\n\t"
        "imull %%ebx, %%ecx\n\t"
        "movl %%ecx, %[r2]\n\t"
        "leal (%%eax, %%ebx, 2), %%edx\n\t"
        "movl %%edx, %[r3]\n\t"
        : [r1] "=m" (result1), [r2] "=m" (result2), [r3] "=m" (result3)
        : [a1] "mr" (addr1), [a2] "mr" (addr2), [a3] "mr" (addr3)
        : "eax", "ebx", "ecx", "edx", "memory"
    );
    
    /* More complex: asm with computed memory addresses */
    int index1 = seed % 16;
    int index2 = (seed * 3) % 16;
    
    __asm__ volatile (
        "movl %[idx1], %%eax\n\t"
        "movl %[idx2], %%ebx\n\t"
        "movl %[base], %%ecx\n\t"
        "movl (%%ecx, %%eax, 4), %%edx\n\t"
        "addl (%%ecx, %%ebx, 4), %%edx\n\t"
        "movl %%edx, %[out]\n\t"
        : [out] "=m" (s1.counter)
        : [base] "r" (s1.data), [idx1] "r" (index1), [idx2] "r" (index2)
        : "eax", "ebx", "ecx", "edx", "memory"
    );
    
    /* Structure member access with pointer arithmetic */
    int sum = 0;
    for (int i = 0; i < 8; i++) {
        /* Complex addressing: base + index * scale + struct_offset */
        sum += vs1->data[i] + vs2->data[7 - i];
        sum += ptr_chain[i % 10][0];
    }
    
    return result1 + result2 + result3 + sum + s1.counter;
}

/* ========== TEST FUNCTION 3: Pointer Chains and Mixed Types ========== */
/* Targets: RELOAD_OTHER, RELOAD_FOR_OTHER_ADDRESS, RELOAD_FOR_OPADDR_ADDR */
static __attribute__((noinline))
int test_pointer_chains(int seed) {
    /* Create complex pointer network */
    int data_pool[64];
    int *ptr_array[32];
    int **ptr_to_ptr[16];
    volatile int *volatile_ptr;
    
    /* Initialize data */
    for (int i = 0; i < 64; i++) {
        data_pool[i] = seed ^ i;
    }
    
    /* Create pointer chains */
    for (int i = 0; i < 32; i++) {
        ptr_array[i] = &data_pool[(i * 3) % 64];
    }
    
    for (int i = 0; i < 16; i++) {
        ptr_to_ptr[i] = &ptr_array[i * 2];
    }
    
    volatile_ptr = &data_pool[31];
    
    /* Complex pointer dereferencing with multiple levels */
    int total = 0;
    #pragma GCC unroll 4
    for (int i = 0; i < 8; i++) {
        /* Multiple indirections requiring address reloads */
        int val1 = **ptr_to_ptr[i];
        int val2 = *ptr_array[i * 2 + 1];
        int val3 = *volatile_ptr;
        
        /* Mix with array accesses */
        val1 += data_pool[**ptr_to_ptr[i] % 64];
        val2 += ptr_array[i][0];  /* Array access through pointer */
        
        /* Address of pointer itself needs reloading */
        int *temp_ptr = ptr_array[i];
        val3 += *(temp_ptr + (val1 % 8));
        
        total += val1 + val2 + val3;
        
        /* Modify pointers to force more reloads */
        volatile_ptr = &data_pool[(i + 5) % 64];
    }
    
    /* Use __builtin_expect to create data dependencies */
    int branch_var = seed;
    for (int i = 0; i < 4; i++) {
        if (__builtin_expect((branch_var & (1 << i)) != 0, 0)) {
            total += **ptr_to_ptr[i];
        } else {
            total += *ptr_array[i * 2];
        }
        branch_var >>= 1;
    }
    
    return total;
}

/* ========== TEST FUNCTION 4: Mixed Everything ========== */
/* Attempts to trigger any remaining reload types */
static __attribute__((noinline))
int test_mixed_reloads(int seed) {
    /* Local variables that will compete for registers */
    register int r1 asm("eax") = seed;
    register int r2 asm("ebx") = seed * 2;
    register int r3 asm("ecx") = seed * 3;
    int r4, r5, r6, r7, r8, r9, r10;
    volatile int v1, v2, v3, v4, v5;
    
    /* Small arrays for address computation */
    int arr1[7] = {seed, seed+1, seed+2, seed+3, seed+4, seed+5, seed+6};
    int arr2[5] = {seed*2, seed*3, seed*4, seed*5, seed*6};
    int arr3[3] = {seed/2, seed/3, seed/4};
    
    /* Complex expression mixing everything */
    v1 = r1;
    v2 = r2;
    v3 = r3;
    
    /* Force spills with many intermediate computations */
    r4 = arr1[v1 % 7] + arr2[v2 % 5] * arr3[v3 % 3];
    r5 = arr2[(v1 + v2) % 5] - arr3[(v2 + v3) % 3];
    r6 = arr1[(v1 * v3) % 7] / (arr2[v2 % 5] + 1);
    r7 = (arr1[0] << 2) | (arr2[1] << 1) | arr3[2];
    r8 = r4 ^ r5 ^ r6 ^ r7;
    r9 = (r4 * r5) + (r6 * r7) - r8;
    r10 = r9 % (seed + 1);
    
    /* Memory barrier to prevent optimization */
    __asm__ volatile ("" : : : "memory");
    
    /* Use all results */
    return r4 + r5 + r6 + r7 + r8 + r9 + r10;
}

/* ========== MAIN DRIVER ========== */
int main(void) {
    int total = 0;
    int seed = 42;  /* Arbitrary seed */
    
    /* Call all test functions multiple times with different seeds */
    for (int i = 0; i < 3; i++) {
        total += test_array_addressing(seed + i * 100);
        total += test_struct_asm(seed + i * 200);
        total += test_pointer_chains(seed + i * 300);
        total += test_mixed_reloads(seed + i * 400);
    }
    
    /* Use the result to prevent dead code elimination */
    volatile int *output = (volatile int*)malloc(sizeof(int));
    *output = total;
    
    return (*output > 0) ? 0 : 1;
}

#pragma GCC pop_options
