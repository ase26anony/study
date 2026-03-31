/* test_resource_coverage.c
 * 
 * This program is designed to trigger specific RTL patterns in GCC's
 * resource.cc during compilation with optimization enabled.
 * It aims to cover lines 282-290 in mark_referenced_resources().
 */

#include <stdint.h>
#include <stdio.h>

/* Prevent inlining to ensure separate RTL generation */
#define NOINLINE __attribute__((noinline))

/* Compile-time check for optimization */
#ifdef __OPTIMIZE__
#define OPTIMIZED 1
#else
#error "Compile with optimization enabled (-O2 or -O3)"
#endif

/* Ensure we're compiling with GCC */
#ifndef __GNUC__
#error "This test requires GCC compiler"
#endif

/* ========== Pattern 1: ZERO_EXTRACT (bit-field operations) ========== */

NOINLINE static int bitfield_extract(void) {
    /* Use volatile to prevent constant propagation */
    volatile unsigned int source = 0xDEADBEEF;
    volatile unsigned int shift = 8;
    volatile unsigned int mask = 0xFF;
    
    /* This should generate ZERO_EXTRACT RTL */
    unsigned int result = (source >> shift) & mask;
    
    /* Additional bit-field operations */
    struct bitfield {
        unsigned int field1 : 4;
        unsigned int field2 : 8;
        unsigned int field3 : 12;
    } bf;
    
    volatile struct bitfield *bf_ptr = (volatile struct bitfield *)&source;
    result += bf_ptr->field2;  /* Another potential ZERO_EXTRACT */
    
    return (int)result;
}

/* ========== Pattern 2: STRICT_LOW_PART (inline assembly) ========== */

NOINLINE static int strict_low_part_ops(void) {
    int result = 0;
    
#if defined(__i386__) || defined(__x86_64__)
    /* x86-specific assembly that may generate STRICT_LOW_PART */
    int var1 = 0x12345678;
    short var2;
    
    /* Byte operation that modifies only part of register */
    asm volatile (
        "movb %1, %0\n\t"
        : "=q"(var2)          /* =q constraint for byte-addressable reg */
        : "r"(var1)
        : "cc"
    );
    
    result += var2;
    
    /* Another potential STRICT_LOW_PART */
    char byte_val;
    asm volatile (
        "movb %%al, %0\n\t"
        : "=r"(byte_val)
        :: "cc"
    );
    
    result += byte_val;
#else
    /* Generic fallback - use bit operations that might generate similar RTL */
    volatile long long large = 0x123456789ABCDEF0LL;
    volatile int small = (large & 0xFFFF);  /* Low part extraction */
    result = small;
#endif
    
    return result;
}

/* ========== Pattern 3: SUBREG (type conversions) ========== */

NOINLINE static int subreg_conversions(void) {
    volatile int int_val = 0x12345678;
    volatile short short_val;
    volatile char char_val;
    
    /* These conversions should generate SUBREG RTL */
    short_val = int_val;           /* int -> short */
    char_val = short_val;          /* short -> char */
    
    /* Mixed-size operations */
    volatile long long ll_val = 0x1122334455667788LL;
    volatile int half1 = ll_val;           /* Low 32 bits */
    volatile int half2 = (ll_val >> 32);   /* High 32 bits */
    
    /* Union for type punning - may generate SUBREG */
    union pun {
        int i;
        short s[2];
        char c[4];
    } u;
    
    u.i = int_val;
    result += u.s[0] + u.s[1];  /* Access sub-parts */
    
    return (int)(short_val + char_val + half1 + half2);
}

/* ========== Pattern 4: MEM_P with complex addressing ========== */

NOINLINE static int complex_memory_access(void) {
    volatile int array[256][16];
    volatile int *ptr_array[128];
    volatile int result = 0;
    
    /* Initialize to prevent optimization */
    for (int i = 0; i < 256; i++) {
        for (int j = 0; j < 16; j++) {
            array[i][j] = i * 16 + j;
        }
    }
    
    for (int i = 0; i < 128; i++) {
        ptr_array[i] = &array[i*2][0];
    }
    
    /* Complex addressing modes that should generate MEM_P with XEXP */
    volatile int index1 = 10;
    volatile int index2 = 5;
    volatile int offset = 3;
    
    /* Multi-dimensional array access with variable indices */
    result += array[index1][index2];
    
    /* Pointer arithmetic with variable offset */
    result += *(ptr_array[index1] + offset);
    
    /* Nested pointer dereference */
    volatile int **pptr = (volatile int **)&ptr_array[20];
    result += **pptr;
    
    /* Structure with array member */
    struct with_array {
        int data[32];
        int count;
    } sa;
    
    sa.count = 15;
    for (int i = 0; i < 32; i++) {
        sa.data[i] = i * 2;
    }
    
    result += sa.data[sa.count];  /* Variable index array access */
    
    return result;
}

/* ========== Main driver with loops ========== */

int main(void) {
    int total = 0;
    
    /* Loop to increase chance of RTL processing */
    for (int i = 0; i < 100; i++) {
        total += bitfield_extract();
        total += strict_low_part_ops();
        total += subreg_conversions();
        total += complex_memory_access();
        
        /* Conditional to prevent loop unrolling from eliminating all RTL */
        if (total > 1000000) {
            total = 0;  /* Reset to avoid overflow */
        }
    }
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d (Compiled with optimization: %s)\n", 
           total, 
           OPTIMIZED ? "YES" : "NO");
    
    return total != 0 ? 0 : 1;
}

/* Additional global variables to increase RTL complexity */
volatile int global_counter = 0;
volatile struct {
    int a : 10;
    int b : 10;
    int c : 12;
} global_bitfield = {0, 0, 0};

/* Function using global bitfield */
NOINLINE static int use_global_bitfield(void) {
    global_bitfield.a = (global_counter >> 2) & 0x3FF;
    global_bitfield.b = (global_counter >> 12) & 0x3FF;
    return global_bitfield.a + global_bitfield.b;
}
