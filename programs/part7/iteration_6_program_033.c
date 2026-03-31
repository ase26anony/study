/* Test program to cover specific RTL patterns in GCC's resource.cc */
#include <stdint.h>
#include <assert.h>

/* Force functions to not be inlined to ensure RTL generation */
#define NOINLINE __attribute__((noinline))

/* Ensure optimization is enabled for RTL generation */
#ifdef __OPTIMIZE__
#define OPTIMIZED 1
#else
#define OPTIMIZED 0
#endif

_Static_assert(OPTIMIZED, "Compile with optimization (-O2 or -O3) for RTL generation");

/* Global volatile variables to prevent optimization */
volatile int global_counter = 0;
volatile int global_mask = 0xFF;

/* ========== Function 1: Generate ZERO_EXTRACT RTL ========== */
NOINLINE static int test_zero_extract(void) {
    /* Bit-field extraction operations that may generate ZERO_EXTRACT */
    volatile unsigned int x = 0xABCD1234;
    unsigned int result = 0;
    
    /* Multiple bit-field extractions */
    result |= (x >> 4) & 0xF;      /* Extract 4 bits */
    result |= (x >> 12) & 0xFF;    /* Extract 8 bits */
    result |= (x >> 20) & 0x7;     /* Extract 3 bits */
    
    /* Using bit-field structure */
    struct {
        unsigned int low : 8;
        unsigned int mid : 12;
        unsigned int high : 4;
    } bits = {0};
    
    bits.low = (x >> 0) & 0xFF;
    bits.mid = (x >> 8) & 0xFFF;
    bits.high = (x >> 20) & 0xF;
    
    return result + bits.low + bits.mid + bits.high;
}

/* ========== Function 2: Generate STRICT_LOW_PART RTL ========== */
NOINLINE static int test_strict_low_part(void) {
    int result = 0;
    
    /* Use inline assembly for byte operations (x86/x86_64 specific) */
    #if defined(__x86_64__) || defined(__i386__)
    unsigned char byte_val = 0x42;
    unsigned int dword_val = 0;
    
    /* Assembly that modifies only part of a register */
    asm volatile (
        "movb %1, %b0\n\t"          /* Move byte to low part */
        : "=r"(dword_val)
        : "r"(byte_val)
        : "cc"
    );
    
    result += dword_val;
    
    /* Another byte operation */
    unsigned short word_val = 0x1234;
    asm volatile (
        "movw %1, %w0\n\t"          /* Move word to low part */
        : "=r"(dword_val)
        : "r"(word_val)
        : "cc"
    );
    
    result += dword_val;
    #else
    /* Fallback for non-x86: use bit operations that might generate similar patterns */
    volatile unsigned int val = 0x87654321;
    result = (val & 0xFF) | ((val >> 8) & 0xFF00);
    #endif
    
    return result;
}

/* ========== Function 3: Generate SUBREG RTL ========== */
NOINLINE static int test_subreg(void) {
    int result = 0;
    
    /* Type conversions that generate SUBREG */
    long long big_val = 0x1122334455667788LL;
    
    /* Access different parts of the long long */
    int low_part = (int)big_val;           /* Truncation */
    int high_part = (int)(big_val >> 32);  /* High part */
    
    /* Mix different sized types */
    short s1 = low_part & 0xFFFF;
    short s2 = (low_part >> 16) & 0xFFFF;
    
    /* Promote back to int with SUBREG */
    int combined = (int)s1 + ((int)s2 << 16);
    
    /* Union for type punning */
    union {
        long long ll;
        int i[2];
        short s[4];
        char c[8];
    } u;
    
    u.ll = big_val;
    result = u.i[0] + u.i[1] + u.s[2] + u.c[3];
    
    return result + combined + high_part;
}

/* ========== Function 4: Generate complex MEM_P RTL ========== */
NOINLINE static int test_mem_operands(void) {
    /* Complex memory addressing modes */
    int array[256];
    int *ptr = array;
    volatile int idx = global_counter;
    
    /* Initialize array */
    for (int i = 0; i < 256; i++) {
        array[i] = i * 3;
    }
    
    int result = 0;
    
    /* Various memory addressing patterns */
    result += ptr[idx];                    /* Base + index */
    result += ptr[idx + 1];                /* Base + index + offset */
    result += *(ptr + idx * 2);            /* Base + scaled index */
    result += array[idx * 3 + 5];          /* Array with complex index */
    
    /* Pointer arithmetic with different types */
    char *char_ptr = (char *)ptr;
    result += char_ptr[idx * sizeof(int) + 1];  /* Byte access */
    
    /* Structure field access */
    struct {
        int a;
        int b;
        int c[4];
    } s = {0};
    
    s.a = idx;
    s.b = idx * 2;
    for (int i = 0; i < 4; i++) {
        s.c[i] = idx + i;
    }
    
    result += s.a + s.b + s.c[idx % 4];
    
    /* Multi-dimensional array */
    int matrix[8][8];
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            matrix[i][j] = i * 8 + j;
        }
    }
    
    int i = idx % 8;
    int j = (idx * 3) % 8;
    result += matrix[i][j] + matrix[j][i];
    
    return result;
}

/* ========== Main function with loop ========== */
int main(void) {
    int total = 0;
    
    /* Loop to increase chance of RTL generation and resource marking */
    for (int i = 0; i < 100; i++) {
        global_counter = i;
        
        /* Call all pattern functions */
        total += test_zero_extract();
        total += test_strict_low_part();
        total += test_subreg();
        total += test_mem_operands();
        
        /* Conditional to prevent loop optimization */
        if (total > 1000000) {
            total = total % 1000;
        }
    }
    
    /* Return predictable result */
    return total % 256;
}
