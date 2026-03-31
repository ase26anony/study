/* Test program to cover lines 282-290 in resource.cc (mark_referenced_resources) */
#include <stdint.h>
#include <assert.h>

/* Ensure optimization is enabled for RTL generation */
#ifndef __OPTIMIZE__
#error "Compile with -O2 or -O3 for proper RTL pattern generation"
#endif

/* Prevent inlining to ensure separate RTL generation */
#define NOINLINE __attribute__((noinline))

/* Bit-field structure for ZERO_EXTRACT */
struct bitfield {
    volatile unsigned int field1 : 4;
    volatile unsigned int field2 : 8;
    volatile unsigned int field3 : 20;
};

/* Function 1: Generate ZERO_EXTRACT RTL through bit-field operations */
NOINLINE static unsigned int test_zero_extract(struct bitfield *bf) {
    unsigned int result = 0;
    /* Multiple bit-field operations to increase chances */
    result |= bf->field1;
    result |= (bf->field2 << 4);
    result |= (bf->field3 << 12);
    
    /* Additional bit extraction pattern */
    volatile unsigned int mask = 0xF0;
    result ^= (result >> 8) & 0xFF;  /* May generate ZERO_EXTRACT */
    
    return result;
}

/* Function 2: Generate STRICT_LOW_PART RTL using inline assembly (x86) */
NOINLINE static uint32_t test_strict_low_part(uint32_t val) {
    uint32_t result = 0;
    
#ifdef __x86_64__ || __i386__
    /* Byte operation that may generate STRICT_LOW_PART */
    asm volatile (
        "movb %b1, %b0\n\t"
        : "=q"(result)      /* =q constraint for byte-addressable register */
        : "r"(val)
        : "cc"
    );
    
    /* Half-word operation */
    uint16_t half;
    asm volatile (
        "movw %w1, %w0\n\t"
        : "=r"(half)
        : "r"(val)
        : "cc"
    );
    result += half;
#else
    /* Fallback: type punning that may generate similar patterns */
    union {
        uint32_t full;
        struct {
            uint16_t low;
            uint16_t high;
        } parts;
    } u = {.full = val};
    
    result = u.parts.low + u.parts.high;
#endif
    
    return result;
}

/* Function 3: Generate SUBREG RTL through type conversions */
NOINLINE static int test_subreg(int a, short b, char c) {
    int result = 0;
    
    /* Various type conversions that may generate SUBREG */
    short s = a;                    /* int to short */
    result += s;
    
    char ch = b;                    /* short to char */
    result += ch;
    
    int from_char = c;              /* char to int */
    result += from_char;
    
    /* 64-bit to 32-bit on 64-bit systems */
    long long big = 0x123456789ABCDEF0LL;
    int small = (int)big;           /* May generate SUBREG */
    result ^= small;
    
    return result;
}

/* Function 4: Generate complex MEM_P RTL with addressing modes */
NOINLINE static int test_mem_operands(int *arr, int idx1, int idx2) {
    int result = 0;
    
    /* Multi-dimensional array access with variable indices */
    int matrix[10][10];
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            matrix[i][j] = i * j;
        }
    }
    
    /* Complex addressing: arr[idx1] + *(arr + idx2) */
    result += arr[idx1];
    result += *(arr + idx2);
    
    /* More complex: arr[idx1 + idx2] + matrix[idx1][idx2] */
    result += arr[idx1 + idx2];
    result += matrix[idx1 % 10][idx2 % 10];
    
    /* Pointer arithmetic with different scales */
    char *cptr = (char *)arr;
    result += cptr[idx1 * sizeof(int)];  /* Byte-granular access */
    
    return result;
}

/* Function 5: Combined patterns in loop for scheduling passes */
NOINLINE static int test_combined(void) {
    struct bitfield bf = {1, 2, 3};
    int arr[100];
    int sum = 0;
    
    /* Initialize array with pattern */
    for (int i = 0; i < 100; i++) {
        arr[i] = i * 3;
    }
    
    /* Loop with multiple pattern-generating operations */
    for (int i = 0; i < 50; i++) {
        /* Mix all patterns */
        sum += test_zero_extract(&bf);
        sum += test_strict_low_part(i);
        sum += test_subreg(i, i % 100, i % 256);
        sum += test_mem_operands(arr, i % 50, (i * 2) % 50);
        
        /* Modify bitfield to prevent optimization */
        bf.field1 = (bf.field1 + 1) & 0xF;
        bf.field2 = (bf.field2 * 3) & 0xFF;
    }
    
    return sum;
}

/* Main function that ensures all code paths are executed */
int main(void) {
    /* Compile-time check for optimization */
    _Static_assert(__OPTIMIZE__, "Optimization must be enabled");
    
    int result = 0;
    
    /* Execute all test functions */
    result += test_combined();
    
    /* Additional calls with different parameters */
    struct bitfield bf = {5, 10, 15};
    result += test_zero_extract(&bf);
    
    result += test_strict_low_part(0x12345678);
    result += test_subreg(1000, 500, 100);
    
    int arr[20];
    for (int i = 0; i < 20; i++) arr[i] = i;
    result += test_mem_operands(arr, 5, 10);
    
    /* Return non-zero result to indicate execution */
    return (result != 0) ? 0 : 1;
}
