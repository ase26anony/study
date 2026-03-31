/* test_resource_coverage.c
 * 
 * This program is designed to trigger specific RTL patterns in GCC's
 * resource.cc mark_referenced_resources function (lines 282-290).
 * Compile with: gcc -O2 -fno-omit-frame-pointer -fschedule-insns test_resource_coverage.c -o test
 */

#include <stdio.h>
#include <stdint.h>

/* Prevent inlining to ensure separate RTL generation */
#define NOINLINE __attribute__((noinline))

/* Ensure optimization is enabled for RTL pattern generation */
#ifdef __OPTIMIZE__
#define OPTIMIZED 1
#else
#error "Compile with optimization enabled (-O1, -O2, or -O3)"
#endif

/* ========== Function 1: Generate ZERO_EXTRACT RTL ========== */
NOINLINE static int bitfield_operations(void) {
    /* Use volatile to prevent constant propagation */
    volatile unsigned int source = 0xABCD1234;
    volatile unsigned int shift = 8;
    volatile unsigned int mask = 0xFF;
    
    /* Bit-field extraction that may generate ZERO_EXTRACT */
    unsigned int result = (source >> shift) & mask;
    
    /* Another bit-field pattern */
    struct bitfield {
        unsigned int low : 4;
        unsigned int mid : 8;
        unsigned int high : 4;
    } bf;
    
    volatile struct bitfield *bf_ptr = (volatile struct bitfield *)&source;
    result += bf_ptr->mid;  /* Bit-field access */
    
    return (int)result;
}

/* ========== Function 2: Generate STRICT_LOW_PART RTL ========== */
NOINLINE static int strict_low_part_ops(void) {
    int result = 0;
    
    /* STRICT_LOW_PART often appears with byte/halfword operations */
    volatile short s_val = 1000;
    volatile char c_val = 100;
    
    /* Type conversions that may generate SUBREG/STRICT_LOW_PART */
    int from_short = s_val;          /* SUBREG may appear here */
    int from_char = c_val;           /* Another SUBREG candidate */
    
    result = from_short + from_char;
    
    /* Inline assembly for x86 that may generate STRICT_LOW_PART */
    #if defined(__i386__) || defined(__x86_64__)
    int asm_result;
    int input = 0x12345678;
    
    /* Byte operation that may use STRICT_LOW_PART */
    asm volatile (
        "movb %1, %0\n\t"
        : "=q" (asm_result)  /* "q" constraint for byte-addressable register */
        : "r" (input)
        : "cc"
    );
    
    result += asm_result;
    #endif
    
    return result;
}

/* ========== Function 3: Generate SUBREG RTL ========== */
NOINLINE static int subreg_operations(void) {
    volatile long long big_val = 0x123456789ABCDEF0LL;
    volatile int *ptr;
    
    /* Access different parts of larger types - may generate SUBREG */
    int low_part = (int)big_val;           /* Truncation */
    int high_part = (int)(big_val >> 32);  /* Another truncation */
    
    /* Union for type punning - often generates SUBREG */
    union {
        long long ll;
        int i[2];
    } converter;
    
    converter.ll = big_val;
    int via_union = converter.i[0] + converter.i[1];
    
    /* Pointer casting between different sizes */
    ptr = (int *)&big_val;
    int via_cast = ptr[0] + ptr[1];
    
    return low_part + high_part + via_union + via_cast;
}

/* ========== Function 4: Generate complex MEM_P RTL ========== */
NOINLINE static int memory_operations(void) {
    volatile int array[64];
    volatile int *ptr_array = array;
    volatile int indices[4] = {1, 3, 5, 7};
    
    /* Initialize array */
    for (int i = 0; i < 64; i++) {
        array[i] = i * 3;
    }
    
    int sum = 0;
    
    /* Complex memory addressing modes */
    for (int i = 0; i < 4; i++) {
        /* Variable index array access */
        sum += array[indices[i]];
        
        /* Pointer arithmetic with variable offset */
        sum += *(ptr_array + indices[i] * 2);
        
        /* Multi-dimensional-like access */
        sum += array[i * 8 + indices[i % 2]];
    }
    
    /* Structure with multiple fields */
    struct data {
        int a;
        int b;
        int c[4];
    };
    
    volatile struct data d;
    d.a = 10;
    d.b = 20;
    for (int i = 0; i < 4; i++) {
        d.c[i] = i * 5;
    }
    
    /* Structure field accesses */
    sum += d.a + d.b;
    sum += d.c[2];
    
    return sum;
}

/* ========== Function 5: Mixed operations in loop ========== */
NOINLINE static int mixed_operations_loop(void) {
    int total = 0;
    
    /* Loop to increase RTL generation opportunities */
    for (volatile int i = 0; i < 10; i++) {
        /* Mix different operations */
        total += bitfield_operations() % 16;
        total += strict_low_part_ops() % 16;
        total += subreg_operations() % 16;
        total += memory_operations() % 16;
    }
    
    return total;
}

/* ========== Main function ========== */
int main(void) {
    /* Compile-time check for optimization */
    #ifndef __OPTIMIZE__
    printf("Warning: Compile with optimization for proper RTL generation\n");
    #endif
    
    int result = 0;
    
    /* Execute all pattern generators */
    result += bitfield_operations();
    result += strict_low_part_ops();
    result += subreg_operations();
    result += memory_operations();
    result += mixed_operations_loop();
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    /* Return non-zero to indicate success */
    return result != 0 ? 0 : 1;
}
