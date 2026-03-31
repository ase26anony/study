/* test_resource_coverage.c
 * 
 * This program generates RTL patterns that should trigger the uncovered
 * lines in GCC's resource.cc (lines 282-290) during compilation with
 * optimization enabled. The patterns include:
 * - ZERO_EXTRACT for bit-field operations
 * - STRICT_LOW_PART for partial register accesses (x86-specific)
 * - SUBREG for type conversions and partial accesses
 * - MEM_P with complex addressing modes
 */

#include <stdint.h>
#include <assert.h>

/* Ensure optimization is enabled for RTL generation */
#ifndef __OPTIMIZE__
#error "Compile with optimization (-O2 or -O3) to generate target RTL patterns"
#endif

/* Prevent inlining to ensure separate RTL generation */
#define NOINLINE __attribute__((noinline))

/* Global volatile variables to prevent constant folding */
volatile int g_volatile_int = 12345;
volatile long long g_volatile_ll = 0x123456789ABCDEF0LL;

/* ==================== ZERO_EXTRACT Patterns ==================== */

/* Bit-field structure for ZERO_EXTRACT operations */
struct bitfield_struct {
    volatile unsigned int field1 : 4;
    volatile unsigned int field2 : 8;
    volatile unsigned int field3 : 12;
};

NOINLINE static unsigned int test_zero_extract(void) {
    struct bitfield_struct bf;
    unsigned int result = 0;
    
    /* Multiple bit-field operations that may generate ZERO_EXTRACT */
    bf.field1 = (g_volatile_int >> 0) & 0xF;
    bf.field2 = (g_volatile_int >> 4) & 0xFF;
    bf.field3 = (g_volatile_int >> 12) & 0xFFF;
    
    /* Combine with shifts and masks */
    result |= (bf.field1 << 0);
    result |= (bf.field2 << 4);
    result |= (bf.field3 << 12);
    
    /* Additional ZERO_EXTRACT patterns with volatile */
    volatile unsigned int v = g_volatile_int;
    result ^= (v >> 3) & 0x1F;      /* 5-bit extract */
    result ^= (v >> 8) & 0x3F;      /* 6-bit extract */
    result ^= (v >> 16) & 0xFF;     /* 8-bit extract */
    
    return result;
}

/* ==================== STRICT_LOW_PART Patterns ==================== */

#ifdef __x86_64__ || __i386__
/* x86-specific inline assembly for STRICT_LOW_PART */
NOINLINE static int test_strict_low_part(void) {
    int result = 0;
    unsigned char byte_val;
    unsigned short word_val;
    
    /* Byte operations that may generate STRICT_LOW_PART */
    asm volatile (
        "movb %1, %0\n\t"
        : "=q"(byte_val)        /* "q" = a, b, c, or d register (byte accessible) */
        : "r"((unsigned char)g_volatile_int)
        : "cc"
    );
    
    /* Word operations */
    asm volatile (
        "movw %1, %0\n\t"
        : "=r"(word_val)        /* Word register */
        : "r"((unsigned short)g_volatile_int)
        : "cc"
    );
    
    /* Mixed-size operations */
    int temp = g_volatile_int;
    asm volatile (
        "addb %1, %b0\n\t"      /* %b0 = low byte of operand 0 */
        : "+r"(temp)
        : "ri"((unsigned char)byte_val)
        : "cc"
    );
    
    result = byte_val + word_val + temp;
    return result;
}
#else
/* Fallback for non-x86: use bit-field operations that may still generate partial reg ops */
NOINLINE static int test_strict_low_part(void) {
    volatile int x = g_volatile_int;
    volatile short s;
    
    /* Type punning through union for partial access */
    union {
        int i;
        short s[2];
    } u;
    u.i = x;
    s = u.s[0];  /* Low part access */
    
    /* Multiple partial accesses */
    unsigned char c1 = (x >> 0) & 0xFF;
    unsigned char c2 = (x >> 8) & 0xFF;
    unsigned char c3 = (x >> 16) & 0xFF;
    
    return s + c1 + c2 + c3;
}
#endif

/* ==================== SUBREG Patterns ==================== */

NOINLINE static long long test_subreg(void) {
    long long result = 0;
    
    /* Type conversions that generate SUBREG */
    int a = g_volatile_int;
    short b = a;                    /* int -> short may use SUBREG */
    char c = a;                     /* int -> char may use SUBREG */
    
    /* Access halves of 64-bit value */
    long long ll = g_volatile_ll;
    int low_part = (int)ll;         /* Low 32 bits */
    int high_part = (int)(ll >> 32); /* High 32 bits */
    
    /* Pointer casting for SUBREG */
    unsigned int ui = 0xDEADBEEF;
    unsigned short us = *(unsigned short*)&ui;  /* Access low half */
    
    /* Structure field access with different sizes */
    struct mixed_sizes {
        char c;
        short s;
        int i;
        long long ll;
    } ms;
    
    ms.c = c;
    ms.s = b;
    ms.i = a;
    ms.ll = ll;
    
    result = b + c + low_part + high_part + us + ms.i;
    return result;
}

/* ==================== MEM_P with Complex Addressing ==================== */

NOINLINE static int test_mem_complex_addressing(void) {
    volatile int array[64][64];  /* Multi-dimensional for complex addressing */
    volatile int *ptr = (int*)array;
    int indices[4] = {1, 3, 7, 15};
    int result = 0;
    
    /* Initialize with volatile to prevent optimization */
    for (int i = 0; i < 64; i++) {
        for (int j = 0; j < 64; j++) {
            array[i][j] = i * 64 + j;
        }
    }
    
    /* Complex addressing modes with variable indices */
    for (int k = 0; k < 4; k++) {
        int i = indices[k] * 2;
        int j = indices[k] * 3;
        
        /* Multi-dimensional array access with non-constant offsets */
        result += array[i][j];
        result += array[j][i];
        
        /* Pointer arithmetic with variable offsets */
        result += *(ptr + i * 64 + j);
        result += *(ptr + j * 64 + i);
        
        /* Structure-like access pattern */
        struct point {
            int x, y, z;
        } points[16];
        
        volatile int idx = indices[k] & 0xF;
        points[idx].x = i;
        points[idx].y = j;
        points[idx].z = i + j;
        
        result += points[idx].x + points[idx].y + points[idx].z;
    }
    
    /* Additional memory patterns with scaled indices */
    volatile int *arr2 = (int*)array;
    for (int i = 0; i < 16; i++) {
        /* Various addressing modes */
        result += arr2[i * 4];          /* Scaled index */
        result += arr2[i + 16];         /* Base + offset */
        result += arr2[i * 2 + 8];      /* Scaled index + offset */
    }
    
    return result;
}

/* ==================== Main Driver ==================== */

int main(void) {
    unsigned int total = 0;
    
    /* Loop to increase chance of RTL processing */
    for (int i = 0; i < 10; i++) {
        total += test_zero_extract();
        total += test_strict_low_part();
        total += test_subreg();
        total += test_mem_complex_addressing();
        
        /* Modify global to prevent loop elimination */
        g_volatile_int ^= total;
        g_volatile_ll += i;
    }
    
    /* Use result to prevent dead code elimination */
    volatile int sink = total;
    
    /* Simple validation */
    assert(sink != 0 || g_volatile_int != 0);
    
    return (sink > 0) ? 0 : 1;
}
