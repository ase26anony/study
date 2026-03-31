/* test_resource_coverage.c
 * 
 * This program is designed to generate RTL patterns that will exercise
 * the uncovered lines in GCC's resource.cc (lines 282-290) during
 * compilation with optimization enabled.
 * 
 * Compile with: gcc -O2 -fno-omit-frame-pointer -fschedule-insns -c test_resource_coverage.c
 * For coverage: gcc -O2 -fprofile-arcs -ftest-coverage test_resource_coverage.c -o test
 */

#include <stdint.h>
#include <stdio.h>

/* Ensure optimization is enabled at compile time */
#ifdef __OPTIMIZE__
#define OPTIMIZATION_ENABLED 1
#else
#define OPTIMIZATION_ENABLED 0
#endif

/* Prevent inlining to ensure separate RTL generation */
#define NOINLINE __attribute__((noinline))

/* Function 1: Generate ZERO_EXTRACT patterns using bit-field operations */
NOINLINE static int bitfield_operations(volatile int *data) {
    /* Complex bit-field extraction that may generate ZERO_EXTRACT */
    volatile unsigned int x = *data;
    
    /* Multiple bit-field extractions with shifting and masking */
    int result = 0;
    result |= (x >> 3) & 0xF;    /* 4-bit field at position 3 */
    result |= (x >> 8) & 0xFF;   /* 8-bit field at position 8 */
    result |= (x >> 16) & 0x7;   /* 3-bit field at position 16 */
    
    /* Nested bit-field operations */
    int y = ((x & 0xFF00) >> 8) | ((x & 0xFF) << 8);
    result ^= (y >> 4) & 0xF;
    
    return result;
}

/* Function 2: Generate STRICT_LOW_PART patterns using inline assembly (x86) */
NOINLINE static int strict_low_part_ops(void) {
    int result = 0;
    
#ifdef __x86_64__ || __i386__
    /* Byte operations that may generate STRICT_LOW_PART */
    unsigned char byte_val = 0xAB;
    unsigned short word_val = 0x1234;
    
    /* Inline assembly that modifies partial registers */
    asm volatile (
        "movb %1, %0\n\t"
        : "=q"(byte_val)
        : "r"((unsigned char)0xCD)
        : "cc"
    );
    
    asm volatile (
        "movw %1, %0\n\t"
        : "=r"(word_val)
        : "r"((unsigned short)0x5678)
        : "cc"
    );
    
    result = byte_val + word_val;
#else
    /* Fallback: Use bit-field structures which may also generate partial reg ops */
    struct {
        unsigned low : 8;
        unsigned high : 8;
    } parts = {0x12, 0x34};
    
    /* Access partial bits */
    result = parts.low | (parts.high << 8);
#endif
    
    return result;
}

/* Function 3: Generate SUBREG patterns through type conversions */
NOINLINE static int subreg_conversions(int a, long long b) {
    int result = 0;
    
    /* Type conversions that generate SUBREG */
    short s1 = a;               /* int -> short */
    char c1 = a;                /* int -> char */
    unsigned short us = b;      /* long long -> unsigned short */
    
    /* Access different parts of larger types */
    union {
        long long ll;
        int i[2];
        short s[4];
    } u;
    
    u.ll = b;
    
    /* Mix different-sized accesses */
    result = s1 + c1 + us + u.i[0] + u.s[2];
    
    /* Pointer casting for sub-register access */
    int *ptr = &a;
    short *sptr = (short *)ptr;
    result += *sptr;
    
    return result;
}

/* Function 4: Generate complex MEM_P patterns with addressing modes */
NOINLINE static int memory_addressing(int idx1, int idx2) {
    /* Multi-dimensional array with variable indices */
    int arr[10][10];
    static int static_arr[100];
    
    /* Initialize arrays to prevent optimization */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            arr[i][j] = i * 10 + j;
        }
    }
    
    for (int i = 0; i < 100; i++) {
        static_arr[i] = i;
    }
    
    /* Complex memory addressing patterns */
    int *ptr1 = &arr[idx1 % 10][idx2 % 10];
    int *ptr2 = static_arr + (idx1 * 7 + idx2 * 3) % 100;
    
    /* Structure with nested arrays */
    struct {
        int data[5][5];
        int more_data[10];
    } s;
    
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            s.data[i][j] = i * j;
        }
    }
    
    /* Multiple memory references with different addressing modes */
    int sum = *ptr1 + *ptr2;
    sum += s.data[idx1 % 5][idx2 % 5];
    sum += *(ptr1 + 1);
    sum += *(ptr2 - 1);
    
    /* Pointer arithmetic in loop */
    int *walk = static_arr;
    for (int i = 0; i < 10; i++) {
        sum += walk[i];
    }
    
    return sum;
}

/* Function 5: Combined operations to increase RTL pattern diversity */
NOINLINE static int combined_operations(volatile int *mem, int idx) {
    int result = 0;
    
    /* Mix all patterns together */
    result += bitfield_operations(mem);
    result += strict_low_part_ops();
    result += subreg_conversions(idx, idx * 2LL);
    result += memory_addressing(idx, idx + 1);
    
    /* Additional bit-field in memory context */
    struct {
        volatile unsigned int field1 : 4;
        volatile unsigned int field2 : 12;
        volatile unsigned int field3 : 8;
    } bit_struct = {0};
    
    bit_struct.field1 = (*mem >> 4) & 0xF;
    bit_struct.field2 = (*mem >> 8) & 0xFFF;
    result += bit_struct.field1 + bit_struct.field2;
    
    return result;
}

/* Main driver that calls all pattern generators in a loop */
int main(void) {
    /* Compile-time check for optimization */
    _Static_assert(OPTIMIZATION_ENABLED, 
                   "Compile with optimization (-O2 or -O3) for coverage");
    
    volatile int data = 0x12345678;
    int sum = 0;
    
    /* Loop to increase chances of RTL generation and resource marking */
    for (int i = 0; i < 100; i++) {
        data ^= i;  /* Prevent complete optimization */
        
        /* Call each pattern generator */
        sum += bitfield_operations(&data);
        sum += strict_low_part_ops();
        sum += subreg_conversions(i, data);
        sum += memory_addressing(i, i * 2);
        sum += combined_operations(&data, i);
        
        /* Conditional to prevent loop unrolling from eliminating patterns */
        if (sum > 1000000) {
            sum %= 1000;
        }
    }
    
    /* Use the result to prevent dead code elimination */
    printf("Result: %d\n", sum);
    return sum != 0 ? 0 : 1;
}
