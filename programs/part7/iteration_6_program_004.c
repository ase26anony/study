/* Test program to cover GCC resource.cc lines 282-290 */
/* Compile with: gcc -O2 -fno-omit-frame-pointer -fschedule-insns -fprofile-arcs -ftest-coverage test.c -o test */

#include <stdint.h>
#include <assert.h>

/* Ensure optimization is enabled for better RTL generation */
#ifndef __OPTIMIZE__
#error "Compile with optimization enabled (-O1, -O2, or -O3)"
#endif

/* Prevent inlining to ensure separate functions are analyzed */
#define NOINLINE __attribute__((noinline))

/* Function 1: Generate ZERO_EXTRACT patterns using bit-field operations */
NOINLINE static int test_zero_extract(void) {
    /* Use volatile to prevent constant propagation */
    volatile unsigned int value = 0xABCD1234;
    volatile int shift = 8;
    volatile int width = 12;
    
    /* These operations often generate ZERO_EXTRACT in RTL */
    unsigned int result = 0;
    
    /* Bit-field extraction pattern */
    result = (value >> shift) & ((1U << width) - 1);
    
    /* Another pattern using bit-field structure (may also generate ZERO_EXTRACT) */
    struct bitfield {
        unsigned int low : 8;
        unsigned int mid : 12;
        unsigned int high : 12;
    };
    
    volatile struct bitfield bf;
    bf.low = 0xFF;
    bf.mid = 0xABC;
    bf.high = 0xDEF;
    
    /* Accessing bit-fields can generate ZERO_EXTRACT */
    result += bf.mid;
    result += bf.high;
    
    return (int)result;
}

/* Function 2: Generate STRICT_LOW_PART patterns using inline assembly (x86 specific) */
NOINLINE static int test_strict_low_part(void) {
    int result = 0;
    
    /* x86-specific inline assembly that may generate STRICT_LOW_PART */
    #if defined(__i386__) || defined(__x86_64__)
    int var1 = 0x12345678;
    int var2 = 0x9ABCDEF0;
    
    /* Byte operations that might use STRICT_LOW_PART for partial register updates */
    asm volatile (
        "movb %1, %0\n\t"
        : "=q"(var1)
        : "r"((unsigned char)var2)
        : "cc"
    );
    
    /* Half-word operation */
    asm volatile (
        "movw %1, %0\n\t"
        : "=r"(var1)
        : "r"((unsigned short)var2)
        : "cc"
    );
    
    result = var1 + var2;
    #else
    /* Fallback for non-x86: use bit operations that might still generate interesting RTL */
    volatile int x = 0x1234;
    volatile short y = 0x5678;
    
    /* Casting between different sizes might generate SUBREG or similar */
    x = (x & 0xFFFF0000) | (y & 0xFFFF);
    result = x;
    #endif
    
    return result;
}

/* Function 3: Generate SUBREG patterns using type conversions and unions */
NOINLINE static int test_subreg(void) {
    volatile long long big_val = 0x123456789ABCDEF0LL;
    volatile int int_val;
    volatile short short_val;
    volatile char char_val;
    
    /* Type conversions that generate SUBREG */
    int_val = (int)big_val;           /* truncation */
    short_val = (short)int_val;       /* narrowing */
    char_val = (char)short_val;       /* further narrowing */
    
    /* Access different parts of larger types */
    union {
        long long ll;
        int i[2];
        short s[4];
        char c[8];
    } u;
    
    u.ll = big_val;
    
    /* Accessing array elements of different sizes generates SUBREG */
    result += u.i[0] + u.i[1];
    result += u.s[0] + u.s[1] + u.s[2] + u.s[3];
    result += u.c[0] + u.c[7];
    
    /* Pointer casting between types */
    int *int_ptr = (int*)&big_val;
    result += int_ptr[0] + int_ptr[1];
    
    return result;
}

/* Function 4: Generate MEM_P with complex addressing patterns */
NOINLINE static int test_mem_addressing(void) {
    volatile int array[256];
    volatile int matrix[16][16];
    volatile int *ptr = array;
    volatile int index1, index2, index3;
    
    /* Initialize with non-constant values */
    index1 = 10;
    index2 = 20;
    index3 = 30;
    
    int result = 0;
    
    /* Complex addressing modes */
    result += array[index1];                     /* Base + index */
    result += array[index1 + index2];            /* Base + computed index */
    result += *(ptr + index1 * 4);               /* Pointer arithmetic */
    result += matrix[index1][index2];            /* 2D array access */
    
    /* Structure with multiple fields */
    struct data {
        int a;
        int b;
        int c[4];
        int d;
    };
    
    volatile struct data s;
    s.a = 100;
    s.b = 200;
    s.c[0] = 300;
    s.d = 400;
    
    /* Structure field accesses generate MEM with addressing */
    result += s.a + s.b + s.c[index1 & 3] + s.d;
    
    /* More complex pointer arithmetic */
    int *p1 = &array[0];
    int *p2 = &array[128];
    result += *(p1 + index1) + *(p2 - index2);
    
    return result;
}

/* Function 5: Mixed patterns in loops to increase RTL generation */
NOINLINE static int test_mixed_patterns(void) {
    volatile int arr[100];
    volatile long long big_arr[50];
    int sum = 0;
    
    /* Initialize arrays */
    for (int i = 0; i < 100; i++) {
        arr[i] = i * 3;
    }
    
    for (int i = 0; i < 50; i++) {
        big_arr[i] = i * 5LL;
    }
    
    /* Loop with mixed operations that should generate various RTL patterns */
    for (volatile int i = 0; i < 10; i++) {
        /* ZERO_EXTRACT pattern */
        int extracted = (arr[i] >> 3) & 0xF;
        
        /* SUBREG pattern through type conversion */
        short half = (short)arr[i];
        
        /* MEM_P with complex addressing */
        sum += arr[i * 2 + 1];
        sum += arr[i * 3 % 100];
        
        /* Access parts of long long (may generate SUBREG) */
        int low_part = (int)big_arr[i % 50];
        int high_part = (int)(big_arr[i % 50] >> 32);
        
        sum += extracted + half + low_part + high_part;
    }
    
    return sum;
}

/* Main function that calls all test patterns */
int main(void) {
    int total = 0;
    
    /* Static assertion to ensure optimization */
    _Static_assert(__OPTIMIZE__, "Optimization must be enabled");
    
    /* Call each test function multiple times to increase coverage chance */
    for (int i = 0; i < 3; i++) {
        total += test_zero_extract();
        total += test_strict_low_part();
        total += test_subreg();
        total += test_mem_addressing();
        total += test_mixed_patterns();
    }
    
    /* Return a non-zero value to indicate successful execution */
    return (total != 0) ? 0 : 1;
}
