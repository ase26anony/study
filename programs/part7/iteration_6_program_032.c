/* Test program to cover specific RTL patterns in GCC's resource.cc */
#include <stdint.h>
#include <assert.h>

/* Force functions to not be inlined to ensure RTL generation */
#define NOINLINE __attribute__((noinline))

/* Ensure optimization is enabled for RTL pattern generation */
#ifndef __OPTIMIZE__
#error "Compile with optimization (-O2 or -O3) for RTL pattern generation"
#endif

/* Global volatile variables to prevent optimization */
volatile int global_counter = 0;
volatile int global_mask = 0xFF;

/* ==================== ZERO_EXTRACT patterns ==================== */

/* Bit-field structure for ZERO_EXTRACT */
struct bitfield_struct {
    volatile unsigned int field1 : 4;
    volatile unsigned int field2 : 8;
    volatile unsigned int field3 : 12;
};

NOINLINE static int test_zero_extract(void) {
    struct bitfield_struct bf;
    int result = 0;
    
    /* Multiple bit-field operations that may generate ZERO_EXTRACT */
    bf.field1 = (global_counter >> 0) & 0xF;
    bf.field2 = (global_counter >> 4) & 0xFF;
    bf.field3 = (global_counter >> 12) & 0xFFF;
    
    /* Extract bits using shift-and-mask (classic ZERO_EXTRACT pattern) */
    result = (bf.field2 >> 2) & 0x3F;  /* Extract 6 bits from middle */
    result += (bf.field3 << 1) & 0x1FFF; /* Extract with shift */
    
    /* Compound extraction */
    volatile unsigned int x = global_counter;
    result += (x >> 3) & 0x1F;  /* Another ZERO_EXTRACT candidate */
    
    return result;
}

/* ==================== STRICT_LOW_PART patterns ==================== */

NOINLINE static int test_strict_low_part(void) {
    int result = 0;
    
#ifdef __x86_64__ || __i386__
    /* x86-specific inline assembly for byte operations */
    unsigned char byte_val;
    unsigned short word_val;
    unsigned int dword_val = global_counter;
    
    /* Byte operation that may generate STRICT_LOW_PART */
    asm volatile ("movb %1, %0" 
                  : "=q"(byte_val) 
                  : "r"((unsigned char)dword_val)
                  : "cc");
    
    /* Word operation */
    asm volatile ("movw %1, %0" 
                  : "=r"(word_val) 
                  : "r"((unsigned short)dword_val)
                  : "cc");
    
    result = byte_val + word_val;
#else
    /* Generic fallback: operations on partial types */
    volatile long long big_val = global_counter;
    volatile int small_val;
    
    /* Casting between different-sized types may generate SUBREG/STRICT_LOW_PART */
    small_val = (int)big_val;  /* Potential SUBREG */
    
    /* Byte access through pointer */
    unsigned char *ptr = (unsigned char *)&big_val;
    result = ptr[0] + ptr[1] + small_val;
#endif
    
    return result;
}

/* ==================== SUBREG patterns ==================== */

NOINLINE static int test_subreg(void) {
    int result = 0;
    
    /* Type conversions that generate SUBREG */
    volatile long long ll_val = global_counter * 100LL;
    volatile int i_val = (int)ll_val;  /* SUBREG from 64-bit to 32-bit */
    
    volatile double d_val = (double)global_counter;
    volatile float f_val = (float)d_val;  /* SUBREG for floating point */
    
    /* Access halves of larger types */
    union {
        long long whole;
        struct {
            int low;
            int high;
        } parts;
    } converter;
    
    converter.whole = ll_val;
    result = converter.parts.low + converter.parts.high + i_val;
    
    /* Pointer casting for sub-register access */
    volatile short *short_ptr = (volatile short *)&i_val;
    result += short_ptr[0] + short_ptr[1];
    
    return result;
}

/* ==================== MEM_P with complex addressing ==================== */

NOINLINE static int test_mem_operands(void) {
    int result = 0;
    
    /* Multi-dimensional array with variable indices */
    volatile int arr[10][10];
    volatile int *ptr_arr = (volatile int *)arr;
    
    /* Complex addressing modes */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            /* Variable index array access - generates MEM with complex address */
            arr[i][j] = global_counter + i * 10 + j;
            
            /* Pointer arithmetic with variable offset */
            *(ptr_arr + i * 10 + j) += i + j;
            
            /* Memory access with scaled index */
            result += arr[j][i];  /* Swapped indices for different pattern */
        }
    }
    
    /* Structure with multiple fields */
    struct complex_struct {
        int a[5];
        int b[5];
        int c;
    } cs;
    
    volatile int idx = global_counter % 5;
    cs.a[idx] = result;
    cs.b[idx] = result * 2;
    cs.c = cs.a[idx] + cs.b[idx];
    
    /* More complex addressing: array of pointers */
    volatile int *ptr_array[5];
    for (int i = 0; i < 5; i++) {
        ptr_array[i] = &cs.a[i];
        result += *ptr_array[i];  /* MEM through pointer array */
    }
    
    return result + cs.c;
}

/* ==================== Main driver ==================== */

NOINLINE static int driver_loop(int iterations) {
    int total = 0;
    
    for (int i = 0; i < iterations; i++) {
        global_counter = i;
        
        /* Call all pattern generators in sequence */
        total += test_zero_extract();
        total += test_strict_low_part();
        total += test_subreg();
        total += test_mem_operands();
        
        /* Conditional to prevent loop optimization */
        if (total > 1000000) {
            total %= 1000000;  /* Prevent overflow */
        }
    }
    
    return total;
}

int main(void) {
    /* Compile-time check for optimization */
    _Static_assert(__OPTIMIZE__, "Optimization required for RTL pattern generation");
    
    /* Run the test driver */
    int result = driver_loop(100);
    
    /* Use result to prevent dead code elimination */
    volatile int sink = result;
    
    /* Return something based on computation */
    return (result > 0) ? 0 : 1;
}
