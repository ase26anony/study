/* test_resource_coverage.c
 * 
 * This program generates RTL patterns that should trigger the uncovered
 * block in GCC's resource.cc (lines 282-290) during optimization passes.
 * Compile with: gcc -O2 -fno-omit-frame-pointer -fschedule-insns -c test_resource_coverage.c
 */

#include <stdint.h>
#include <assert.h>

/* Force functions to not be inlined to ensure RTL generation */
#define NOINLINE __attribute__((noinline))

/* Compile-time check for optimization level */
#ifdef __OPTIMIZE__
#define OPTIMIZED 1
#else
#error "Compile with optimization enabled (-O1, -O2, or -O3)"
#endif

/* ========== Pattern 1: ZERO_EXTRACT (bit-field operations) ========== */
NOINLINE static int bitfield_extract(void) {
    /* Use volatile to prevent constant folding */
    volatile unsigned int source = 0x12345678;
    unsigned int mask = 0x00000FF0;  /* Extract bits 4-11 */
    
    /* This should generate ZERO_EXTRACT RTL */
    unsigned int extracted = (source >> 4) & 0xFF;
    
    /* Complex bitfield operation with variable shift */
    volatile unsigned int shift_amount = 8;
    unsigned int dynamic_extract = (source >> shift_amount) & mask;
    
    return extracted + dynamic_extract;
}

/* ========== Pattern 2: STRICT_LOW_PART (partial register access) ========== */
NOINLINE static int partial_register_ops(void) {
    int result = 0;
    
    /* x86-specific inline assembly for byte register operations */
#if defined(__i386__) || defined(__x86_64__)
    volatile uint8_t byte_val = 0x42;
    uint32_t dword_reg;
    
    /* This assembly should generate STRICT_LOW_PART for byte operation */
    asm volatile (
        "movb %1, %b0\n\t"          /* Move byte to low part of register */
        "andl $0xFF, %0"            /* Ensure only low byte is modified */
        : "=r"(dword_reg)
        : "r"(byte_val)
        : "cc"
    );
    result += dword_reg;
    
    /* Another example with 16-bit operation */
    volatile uint16_t word_val = 0xABCD;
    uint32_t another_reg;
    
    asm volatile (
        "movw %1, %w0\n\t"          /* Move word to low part of register */
        : "=r"(another_reg)
        : "r"(word_val)
    );
    result += another_reg;
#endif
    
    /* Non-x86 fallback: Use union for type punning (may also generate SUBREG) */
    union {
        uint32_t full;
        struct {
            uint16_t low;
            uint16_t high;
        } parts;
    } pun;
    
    pun.full = 0xDEADBEEF;
    pun.parts.low = 0x1234;  /* Modifying low part only */
    result += pun.full;
    
    return result;
}

/* ========== Pattern 3: SUBREG (subregister operations) ========== */
NOINLINE static int subregister_operations(void) {
    int total = 0;
    
    /* Type conversions that often generate SUBREG */
    volatile long long big_val = 0x1122334455667788LL;
    
    /* Truncation to smaller type */
    int truncated = (int)big_val;           /* SUBREG for truncation */
    total += truncated;
    
    /* Access halves of 64-bit value */
    int low_half = (int)(big_val & 0xFFFFFFFF);
    int high_half = (int)(big_val >> 32);
    total += low_half + high_half;
    
    /* Pointer casting between different sized types */
    volatile float float_val = 3.14159f;
    uint32_t int_view = *(uint32_t*)&float_val;  /* May involve MEM + SUBREG */
    total += int_view;
    
    return total;
}

/* ========== Pattern 4: Complex Memory Operands (MEM_P) ========== */
NOINLINE static int complex_memory_access(void) {
    volatile int array[256];
    volatile int matrix[16][16];
    volatile int *ptr = &array[0];
    
    /* Initialize to prevent optimization */
    for (int i = 0; i < 256; i++) {
        array[i] = i * 3;
    }
    
    int sum = 0;
    volatile int index1 = 10;
    volatile int index2 = 20;
    
    /* Complex addressing modes with variable indices */
    sum += array[index1 * 2 + 5];           /* MEM with scaled index */
    sum += array[index1 + index2];          /* MEM with two variables */
    sum += *(ptr + index1 * 3 - 7);         /* MEM with complex pointer arithmetic */
    
    /* Multi-dimensional array access */
    sum += matrix[index1][index2];          /* MEM with two indices */
    
    /* Structure access (simulated with array of structs) */
    struct pair {
        int first;
        int second;
    } pairs[10];
    
    volatile int idx = 3;
    sum += pairs[idx].second;               /* MEM with structure field */
    
    return sum;
}

/* ========== Combined Test Function ========== */
NOINLINE static int combined_operations(void) {
    /* Mix all patterns in one function to increase interaction */
    int result = 0;
    
    /* ZERO_EXTRACT pattern */
    volatile unsigned int bits = 0x89ABCDEF;
    result += (bits >> 12) & 0xFFF;
    
    /* SUBREG pattern */
    volatile double d = 2.71828;
    uint64_t bits2 = *(uint64_t*)&d;
    result += (int)(bits2 >> 32);  /* High 32 bits */
    
    /* Complex memory */
    volatile int buf[100];
    volatile int i = 25;
    result += buf[i * 2 + 1];
    
    return result;
}

/* ========== Main Driver ========== */
int main(void) {
    int total = 0;
    
    /* Call each pattern function multiple times in a loop
     * to increase chance of RTL generation and resource marking */
    for (int i = 0; i < 10; i++) {
        total += bitfield_extract();
        total += partial_register_ops();
        total += subregister_operations();
        total += complex_memory_access();
        total += combined_operations();
    }
    
    /* Use result to prevent dead code elimination */
    volatile int sink = total;
    
    /* Simple self-check */
    if (total != 0) {
        return 0;  /* Success */
    }
    
    return 1;  /* Should never reach here with proper initialization */
}

/* Additional global variables to increase complexity */
volatile unsigned long global_bitfield = 0xF0F0F0F0;
volatile int global_array[128];
volatile short global_shorts[256];

/* Another function that mixes patterns */
NOINLINE static int mixed_pattern(int iterations) {
    int acc = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* ZERO_EXTRACT with volatile */
        acc += (global_bitfield >> (i % 16)) & 0xFF;
        
        /* SUBREG through pointer casting */
        short *sptr = (short*)&global_array[i % 64];
        acc += *sptr;
        
        /* Complex memory addressing */
        acc += global_shorts[i * 2 + 1];
    }
    
    return acc;
}
