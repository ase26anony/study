/* Test program to cover specific RTL patterns in GCC's resource.cc */
#include <stdint.h>
#include <assert.h>

/* Ensure optimization is enabled for RTL generation */
#ifndef __OPTIMIZE__
#error "Compile with optimization (-O2 or -O3) to generate target RTL patterns"
#endif

/* Prevent inlining to ensure separate RTL generation */
#define NOINLINE __attribute__((noinline))

/* Bit-field structure for ZERO_EXTRACT */
struct bitfield {
    volatile unsigned int field1 : 4;
    volatile unsigned int field2 : 8;
    volatile unsigned int field3 : 12;
};

/* Function 1: Generate ZERO_EXTRACT pattern */
NOINLINE static unsigned int test_zero_extract(struct bitfield *bf) {
    /* Multiple bit-field operations to encourage ZERO_EXTRACT */
    unsigned int val1 = bf->field1;
    unsigned int val2 = bf->field2;
    unsigned int val3 = bf->field3;
    
    /* Operations that may generate ZERO_EXTRACT in RTL */
    unsigned int result = (val1 << 4) | (val2 >> 2);
    result = (result & 0xFF) | ((val3 & 0xF) << 8);
    
    /* Additional bit-field extraction */
    result = (bf->field2 >> 1) & 0x7F;
    result |= (bf->field1 << 7);
    
    return result;
}

/* Function 2: Generate STRICT_LOW_PART pattern (x86-specific) */
NOINLINE static uint32_t test_strict_low_part(uint32_t x) {
    uint32_t result = 0;
    
#ifdef __x86_64__ || __i386__
    /* Inline assembly that modifies only part of a register */
    asm volatile (
        /* Byte operation - may generate STRICT_LOW_PART */
        "movb %b1, %b0\n\t"
        /* Half-word operation */
        "movw %w1, %w0\n\t"
        : "=r"(result)
        : "r"(x)
        : "cc"
    );
    
    /* Additional byte operations */
    uint8_t byte_val;
    asm volatile (
        "movb %%al, %0"
        : "=q"(byte_val)
        : "a"(x)
    );
    result |= byte_val;
#else
    /* Fallback: type punning that might generate similar patterns */
    union {
        uint32_t full;
        struct {
            uint16_t low;
            uint16_t high;
        } parts;
    } u;
    u.full = x;
    result = u.parts.low | (u.parts.high << 16);
#endif
    
    return result;
}

/* Function 3: Generate SUBREG patterns */
NOINLINE static int test_subreg(int a, short b, char c) {
    /* Type conversions that generate SUBREG */
    short s1 = a;           /* int to short */
    char c1 = b;            /* short to char */
    int i1 = s1;            /* short to int */
    int i2 = c1;            /* char to int */
    
    /* Operations on different-sized types */
    long long ll = (long long)a * (long long)b;
    int ll_low = (int)ll;    /* Extract low part */
    int ll_high = (int)(ll >> 32); /* Extract high part */
    
    /* Structure with mixed types */
    struct mixed {
        int x;
        short y;
        char z;
    } m;
    m.x = a;
    m.y = b;
    m.z = c;
    
    /* Access different parts */
    short y_part = m.y;     /* May involve SUBREG */
    char z_part = m.z;      /* May involve SUBREG */
    
    return i1 + i2 + ll_low + ll_high + y_part + z_part;
}

/* Function 4: Generate complex MEM_P patterns */
NOINLINE static int test_mem_access(int *arr, int n, int idx1, int idx2) {
    int sum = 0;
    
    /* Complex array addressing with variable indices */
    sum += arr[idx1 * 2 + 3];
    sum += arr[idx2 * 3 - 1];
    
    /* Pointer arithmetic with non-constant offsets */
    int *ptr = arr + idx1;
    sum += *(ptr + idx2);
    sum += *(ptr - 1);
    
    /* Multi-dimensional array access simulation */
    int matrix[10][10];
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            matrix[i][j] = i * j;
        }
    }
    
    /* Variable indexing into 2D array */
    sum += matrix[idx1 % 10][idx2 % 10];
    
    /* Structure with array member */
    struct with_array {
        int data[20];
        int count;
    } wa;
    
    wa.count = n;
    for (int i = 0; i < (n % 20); i++) {
        wa.data[i] = i * 2;
        sum += wa.data[i * 3 % 20];  /* Complex array indexing */
    }
    
    return sum;
}

/* Function 5: Mixed patterns in loop to force resource marking */
NOINLINE static int test_mixed_patterns(int iterations) {
    struct bitfield bf = {1, 2, 3};
    int arr[100];
    int total = 0;
    
    /* Initialize array with values */
    for (int i = 0; i < 100; i++) {
        arr[i] = i * 3;
    }
    
    /* Loop with multiple pattern-generating operations */
    for (int i = 0; i < iterations; i++) {
        /* Alternate between different patterns */
        switch (i % 4) {
            case 0:
                total += test_zero_extract(&bf);
                break;
            case 1:
                total += test_strict_low_part(i * 7);
                break;
            case 2:
                total += test_subreg(i, i * 2, i * 3);
                break;
            case 3:
                total += test_mem_access(arr, i, i % 10, (i * 2) % 10);
                break;
        }
        
        /* Modify bitfields to prevent optimization */
        bf.field1 = (bf.field1 + 1) & 0xF;
        bf.field2 = (bf.field2 * 3) & 0xFF;
        bf.field3 = (bf.field3 - 1) & 0xFFF;
    }
    
    return total;
}

/* Main function that drives the tests */
int main(void) {
    /* Compile-time check for optimization */
    _Static_assert(__OPTIMIZE__, "Optimization must be enabled");
    
    int result = 0;
    
    /* Test each pattern individually */
    struct bitfield bf = {5, 10, 15};
    result += test_zero_extract(&bf);
    
    result += test_strict_low_part(0x12345678);
    
    result += test_subreg(1000, 500, 100);
    
    int test_arr[50];
    for (int i = 0; i < 50; i++) {
        test_arr[i] = i * i;
    }
    result += test_mem_access(test_arr, 25, 3, 7);
    
    /* Test mixed patterns with loop */
    result += test_mixed_patterns(100);
    
    /* Return non-zero result to ensure all code executes */
    return result != 0 ? 0 : 1;
}
