/* test_resource_coverage.c
 * Designed to trigger uncovered lines 282-290 in resource.cc
 * Compile with: gcc -O2 -m32 -fno-strict-aliasing -fdump-rtl-all -c test_resource_coverage.c
 */

#include <stddef.h>

/* Force functions to not be inlined to ensure RTL generation */
#define NOINLINE __attribute__((noinline))

/* Function A: Focus on ZERO_EXTRACT and MEM patterns */
NOINLINE static void func_a(volatile int *counter) {
    /* Struct with volatile bit-field for ZERO_EXTRACT */
    struct S {
        volatile unsigned int f1:5;
        volatile unsigned int f2:3;
        volatile unsigned int f3:8;
    } s;
    
    /* Array for MEM with complex addressing */
    volatile int arr[16][16];
    
    /* Force multiple ZERO_EXTRACT operations */
    s.f1 = (*counter) & 0x1F;
    s.f2 = ((*counter) >> 5) & 0x7;
    s.f3 = ((*counter) >> 8) & 0xFF;
    
    /* Complex MEM access with pointer arithmetic */
    int idx1 = (*counter) & 0xF;
    int idx2 = ((*counter) >> 4) & 0xF;
    
    /* This should generate MEM RTL with addressing calculations */
    volatile int val = arr[idx1][idx2];
    
    /* Use result to prevent elimination */
    *counter += val & 1;
}

/* Function B: Focus on STRICT_LOW_PART and SUBREG patterns */
NOINLINE static void func_b(volatile int *counter) {
    volatile short hs = *counter & 0xFFFF;
    volatile char hc = *counter & 0xFF;
    
    /* Type punning for SUBREG generation */
    int combined = 0;
    short *ps = (short*)&combined;
    char *pc = (char*)&combined;
    
    /* Mixed-size accesses for SUBREG */
    *ps = hs;
    *(pc + 2) = hc;
    
    /* Inline assembly for STRICT_LOW_PART */
    /* Modifying only low byte of a register */
    asm volatile (
        "addb $1, %0\n\t"
        : "=q" (hc)
        : "0" (hc)
        : "cc"
    );
    
    /* More type punning */
    long long big = 0x123456789ABCDEF0LL;
    int *pint = (int*)&big;
    volatile int half = pint[0] + pint[1];
    
    /* Use results */
    *counter += combined + hc + half;
}

/* Function C: Complex expression mixing patterns */
NOINLINE static void func_c(volatile int *counter, volatile int selector) {
    /* Struct with bit-fields at different positions */
    struct BitFieldStruct {
        volatile unsigned int a:3;
        volatile unsigned int b:5;
        volatile unsigned int c:12;
        volatile unsigned int d:8;
    } bfs;
    
    /* Array with multi-dimensional access */
    volatile int matrix[8][8][8];
    
    /* Complex addressing calculation */
    int i = (selector >> 0) & 0x7;
    int j = (selector >> 3) & 0x7;
    int k = (selector >> 6) & 0x7;
    
    /* MEM access with complex addressing */
    volatile int base = matrix[i][j][k];
    
    /* Conditional bit-field operation */
    if (base & 1) {
        bfs.a = (selector & 0x7);
        bfs.b = ((selector >> 3) & 0x1F);
    } else {
        bfs.c = (selector & 0xFFF);
        bfs.d = ((selector >> 12) & 0xFF);
    }
    
    /* Pointer arithmetic with type conversion */
    char *byte_ptr = (char*)&base;
    int *int_ptr = (int*)(byte_ptr + 1);  /* Misaligned access - may generate interesting RTL */
    
    /* Use volatile to force generation */
    volatile int temp = *int_ptr;
    
    /* Update counter */
    *counter += bfs.a + bfs.b + bfs.c + bfs.d + temp;
}

/* Helper with mixed operations */
NOINLINE static void mixed_operations(volatile int *counter) {
    /* Create SUBREG through union */
    union {
        int i;
        short s[2];
        char c[4];
    } u;
    
    u.i = *counter;
    
    /* Modify parts through different views */
    u.s[0] += 1;           /* SUBREG access */
    u.c[2] = u.c[1] ^ 0x55; /* Another SUBREG */
    
    /* Bit-field in local struct */
    struct {
        volatile unsigned int low:4;
        volatile unsigned int high:4;
    } bits;
    
    bits.low = u.i & 0xF;
    bits.high = (u.i >> 4) & 0xF;
    
    /* Array with index calculation */
    volatile int arr[32];
    int idx = (u.i * 1103515245 + 12345) & 0x1F;  /* Simple PRNG */
    
    /* MEM access */
    volatile int val = arr[idx];
    
    /* Update */
    *counter = u.i + bits.low + bits.high + val;
}

int main(int argc, char **argv) {
    volatile int counter = 0;
    volatile int selector = 0;
    
    /* Use argc to bound loops for compilation analysis */
    int iterations = (argc > 1) ? 10 : 5;
    
    /* Initialize some arrays to avoid undefined behavior in compilation */
    volatile int init_arr[16][16];
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) {
            init_arr[i][j] = i * j;
        }
    }
    
    volatile int init_matrix[8][8][8];
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            for (int k = 0; k < 8; k++) {
                init_matrix[i][j][k] = i + j + k;
            }
        }
    }
    
    /* Main loop to generate repeated RTL patterns */
    for (volatile int iter = 0; iter < iterations; iter++) {
        /* Call pattern functions with volatile arguments */
        func_a(&counter);
        func_b(&counter);
        func_c(&counter, selector);
        mixed_operations(&counter);
        
        /* Update selector with simple PRNG-like operation */
        selector = (selector * 1664525 + 1013904223) & 0x7FFFFFFF;
        
        /* Prevent loop elimination */
        asm volatile("" : "+r" (counter) : : "memory");
    }
    
    /* Final dummy use to prevent dead code elimination */
    volatile int result = counter;
    
    /* The program doesn't need correct runtime semantics,
     * but we return something to make compilation happy */
    return result & 1;
}
