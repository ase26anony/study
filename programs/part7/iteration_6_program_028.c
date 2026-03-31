/* Test program to cover GCC resource.cc lines 282-290 */
#include <stdint.h>
#include <assert.h>

/* Ensure optimization is enabled for RTL generation */
#ifndef __OPTIMIZE__
#error "Compile with -O2 or -O3 for proper RTL generation"
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
    /* Multiple bit-field operations to increase chances */
    unsigned int val1 = bf->field1;
    unsigned int val2 = bf->field2;
    unsigned int val3 = bf->field3;
    
    /* Operations that may generate ZERO_EXTRACT */
    unsigned int result = (val1 << 4) | (val2 & 0x0F);
    result ^= (val3 >> 8) & 0xFF;
    
    /* Additional bit-field extraction */
    result |= ((bf->field2 >> 2) & 0x3F) << 10;
    
    return result;
}

/* Function 2: Generate STRICT_LOW_PART RTL using inline assembly (x86) */
NOINLINE static uint32_t test_strict_low_part(uint32_t x) {
    uint32_t result = 0;
    
#ifdef __x86_64__ || __i386__
    /* Byte operation that may generate STRICT_LOW_PART */
    asm volatile (
        "movb %b1, %b0\n\t"
        : "=q"(result)
        : "r"(x)
        : "cc"
    );
    
    /* Half-word operation */
    uint16_t hw;
    asm volatile (
        "movw %w1, %w0\n\t"
        : "=r"(hw)
        : "r"(x)
        : "cc"
    );
    result += hw;
#else
    /* Fallback: operations that might generate partial register accesses */
    result = (uint8_t)x;
    result += (uint16_t)(x >> 8);
#endif
    
    return result;
}

/* Function 3: Generate SUBREG RTL through type conversions */
NOINLINE static int test_subreg(int a, short b, char c) {
    /* Various type conversions that generate SUBREG */
    short s1 = a;               /* int to short */
    char c1 = b;               /* short to char */
    int i1 = s1;               /* short to int */
    int i2 = c1;               /* char to int */
    
    /* Operations on different-sized types */
    long long ll = (long long)a * (long long)b;
    int ll_low = (int)ll;      /* Extract low part */
    int ll_high = (int)(ll >> 32); /* Extract high part */
    
    /* Union for type punning */
    union {
        uint32_t full;
        uint16_t halves[2];
    } u;
    u.full = a;
    u.halves[0] = b;
    
    return i1 + i2 + ll_low + ll_high + u.full;
}

/* Function 4: Generate complex MEM_P RTL with addressing modes */
NOINLINE static int test_mem_operands(int *arr, int idx1, int idx2) {
    /* Multi-dimensional array access */
    int matrix[10][10];
    
    /* Complex addressing with variable indices */
    int val1 = matrix[idx1][idx2];
    int val2 = matrix[idx2][idx1];
    
    /* Pointer arithmetic with variable offsets */
    int *ptr1 = arr + idx1;
    int *ptr2 = arr + idx2 * 2;
    
    /* Structure pointer access */
    struct {
        int a;
        int b;
        int c[5];
    } s;
    
    s.a = idx1;
    s.b = idx2;
    int val3 = s.c[idx1 % 5];
    
    /* Combined addressing modes */
    return val1 + *ptr1 + *ptr2 + val2 + val3 + arr[idx1 + idx2];
}

/* Function 5: Mixed operations to increase RTL pattern variety */
NOINLINE static int test_mixed_operations(void) {
    volatile int vars[4] = {1, 2, 3, 4};
    struct bitfield bf = {1, 2, 3};
    
    int sum = 0;
    
    /* Call all test functions in a loop to increase coverage chances */
    for (int i = 0; i < 4; i++) {
        sum += test_zero_extract(&bf);
        sum += test_strict_low_part(vars[i]);
        sum += test_subreg(vars[i], (short)vars[(i+1)%4], (char)vars[(i+2)%4]);
        sum += test_mem_operands((int*)vars, i, (i+1)%4);
    }
    
    return sum;
}

/* Main function with compile-time assertion */
int main(void) {
    /* Ensure optimization is enabled */
    _Static_assert(__OPTIMIZE__, "Optimization must be enabled for RTL coverage");
    
    /* Initialize test data */
    int data[10];
    for (int i = 0; i < 10; i++) {
        data[i] = i * 2;
    }
    
    /* Execute tests */
    int result = test_mixed_operations();
    
    /* Use result to prevent dead code elimination */
    volatile int sink = result;
    
    /* Simple validation */
    assert(sink != 0);
    
    return 0;
}
