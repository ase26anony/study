/* test_resource_coverage.c
 * 
 * This program is designed to trigger specific RTL patterns in GCC's
 * resource management code (resource.cc lines 282-290).
 * 
 * Compile with: gcc -O2 -fno-omit-frame-pointer -fschedule-insns -c test_resource_coverage.c
 */

#include <stdint.h>
#include <assert.h>

/* Ensure optimization is enabled for coverage */
#ifndef __OPTIMIZE__
#error "Compile with optimization enabled (-O1, -O2, or -O3)"
#endif

/* Prevent inlining to ensure separate RTL generation */
#define NOINLINE __attribute__((noinline))

/* Function 1: Generate ZERO_EXTRACT RTL pattern using bit-field operations */
NOINLINE static int bitfield_extract(void) {
    volatile uint32_t source = 0x12345678;
    /* This should generate ZERO_EXTRACT for bit-field extraction */
    uint32_t extracted = (source >> 8) & 0xFFF;  /* Extract bits 8-19 */
    return extracted;
}

/* Function 2: Generate STRICT_LOW_PART RTL pattern using inline assembly (x86) */
NOINLINE static int strict_low_part_asm(void) {
    int result = 0;
    
#ifdef __x86_64__ || __i386__
    /* Inline assembly that modifies only part of a register */
    int input = 0x12345678;
    asm volatile (
        "movb %1, %0\n\t"
        : "=q"(result)    /* =q constraint for byte-addressable register */
        : "r"(input)
        : "cc"
    );
#else
    /* Fallback for non-x86: use bit-field structure */
    struct {
        int full : 32;
        int low_part : 8;
    } s;
    s.full = 0x12345678;
    result = s.low_part;
#endif
    
    return result;
}

/* Function 3: Generate SUBREG RTL patterns through type conversions */
NOINLINE static int subreg_conversions(void) {
    int32_t a = 0x12345678;
    
    /* These conversions often generate SUBREG */
    int16_t b = a;                     /* Truncation */
    int64_t c = b;                     /* Extension */
    int32_t d = (int32_t)((int16_t)c); /* Multiple conversions */
    
    /* Use union for type punning - may generate SUBREG */
    union {
        int32_t i;
        int16_t s[2];
    } u;
    u.i = a;
    
    return u.s[0] + u.s[1] + b + d;
}

/* Function 4: Generate complex memory operands (MEM_P with addressing) */
NOINLINE static int complex_memory_access(int index1, int index2) {
    /* Multi-dimensional array with variable indices */
    int arr[10][10];
    
    /* Initialize array */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            arr[i][j] = i * 10 + j;
        }
    }
    
    /* Complex addressing mode with variable indices */
    int result = arr[index1][index2] 
               + *(*(arr + index1) + index2)  /* Pointer arithmetic */
               + arr[index2 % 5][index1 % 3]; /* More complex indexing */
    
    /* Structure with nested arrays */
    struct {
        int data[5][5];
        int extra;
    } s;
    
    /* Initialize structure */
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            s.data[i][j] = i * j;
        }
    }
    s.extra = 42;
    
    /* More complex memory accesses */
    result += s.data[index1 % 5][index2 % 5]
            + *(&s.extra + (index1 & 1))  /* May create interesting addressing */
            + *(int*)((char*)&s + sizeof(int) * (index1 % 3)); /* Byte offset */
    
    return result;
}

/* Function 5: Mixed operations to increase RTL pattern diversity */
NOINLINE static int mixed_operations(int param) {
    volatile int x = param;
    
    /* Bit-field extraction (ZERO_EXTRACT potential) */
    int y = (x >> 4) & 0x0F;
    
    /* Type conversions (SUBREG potential) */
    short z = y;
    int w = z;
    
    /* Memory operations */
    int array[4] = {1, 2, 3, 4};
    int mem_result = array[w & 3] + *(array + (y & 3));
    
    return y + z + w + mem_result;
}

/* Main function that exercises all patterns */
int main(void) {
    int total = 0;
    
    /* Loop to increase chance of RTL generation and resource marking */
    for (int i = 0; i < 10; i++) {
        total += bitfield_extract();
        total += strict_low_part_asm();
        total += subreg_conversions();
        total += complex_memory_access(i % 5, (i * 3) % 5);
        total += mixed_operations(i);
    }
    
    /* Use result to prevent dead code elimination */
    volatile int sink = total;
    
    /* Simple validation */
    if (total != 0) {
        return 0;  /* Success */
    }
    
    return 1;  /* Should never reach here with the loop */
}

/* Additional test case specifically for ZERO_EXTRACT with bit-fields */
NOINLINE static int explicit_bitfield_test(void) {
    struct {
        unsigned int a : 4;
        unsigned int b : 8;
        unsigned int c : 4;
    } bitfield;
    
    volatile unsigned int source = 0xDEADBEEF;
    
    /* Direct bit-field assignment - may generate ZERO_EXTRACT */
    bitfield.a = (source >> 0) & 0xF;
    bitfield.b = (source >> 4) & 0xFF;
    bitfield.c = (source >> 12) & 0xF;
    
    return bitfield.a + bitfield.b + bitfield.c;
}

/* Force inclusion of all functions */
void force_references(void) {
    /* Call all functions to ensure they're not optimized away */
    volatile int dummy = 0;
    dummy += explicit_bitfield_test();
    (void)dummy;
}
