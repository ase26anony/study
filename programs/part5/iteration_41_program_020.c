/* test_resource_patterns.c
 * 
 * This program generates RTL patterns to test GCC's resource.cc:
 * - ZERO_EXTRACT for bit-field operations
 * - STRICT_LOW_PART for partial register updates
 * - SUBREG for register sub-parts
 * - Complex memory addressing modes
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Prevent constant folding and dead code elimination */
volatile int g_seed = 42;
#define UNPREDICTABLE(x) ((x) ^ (g_seed & 1))

/* ========== ZERO_EXTRACT PATTERNS ========== */

/* Bit-field structure for ZERO_EXTRACT operations */
struct bitfield_struct {
    unsigned int field1 : 5;
    unsigned int field2 : 7;
    unsigned int field3 : 3;
    unsigned int field4 : 17;
};

/* Explicit bit-field extraction */
unsigned int test_zero_extract_explicit(unsigned int x) {
    unsigned int result = 0;
    
    /* Multiple bit-field extractions that should generate ZERO_EXTRACT */
    result |= (x >> 3) & 0x1F;      /* Extract bits 3-7 */
    result |= (x >> 8) & 0x7F;      /* Extract bits 8-14 */
    result |= (x >> 15) & 0x7;      /* Extract bits 15-17 */
    
    /* Prevent optimization */
    return UNPREDICTABLE(result);
}

/* Bit-field structure operations */
unsigned int test_zero_extract_struct(struct bitfield_struct *s, unsigned int val) {
    unsigned int result = 0;
    
    /* Bit-field assignments */
    s->field1 = val & 0x1F;
    s->field2 = (val >> 5) & 0x7F;
    s->field3 = (val >> 12) & 0x7;
    
    /* Bit-field comparisons */
    if (s->field1 == 0x10) result |= 0x1;
    if (s->field2 > 0x20) result |= 0x2;
    if (s->field3 != 0x3) result |= 0x4;
    
    /* Combine bit-fields */
    result |= (s->field1 << 16) | (s->field2 << 8) | s->field3;
    
    return UNPREDICTABLE(result);
}

/* Complex bit-field arithmetic */
unsigned int test_zero_extract_arithmetic(unsigned int *arr, int n) {
    unsigned int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Multiple extract operations in sequence */
        unsigned int val = arr[i];
        
        /* These should generate ZERO_EXTRACT RTL */
        unsigned int low = (val & 0xFF) >> 0;
        unsigned int mid = (val & 0xFF00) >> 8;
        unsigned int high = (val & 0xFF0000) >> 16;
        unsigned int top = (val & 0xFF000000) >> 24;
        
        /* Use in computation to prevent elimination */
        sum += (low * mid) + (high ^ top);
        
        /* Nested extractions */
        unsigned int nested = ((val >> 4) & 0xF) | ((val >> 16) & 0xF0);
        sum += nested;
    }
    
    return UNPREDICTABLE(sum);
}

/* ========== STRICT_LOW_PART PATTERNS ========== */

/* Partial register updates */
unsigned int test_strict_low_part(void) {
    volatile unsigned int result = 0;
    
    /* char and short operations that may generate STRICT_LOW_PART */
    unsigned int accumulator = 0x12345678;
    
    /* Partial writes through pointers - may generate partial register stores */
    unsigned char *byte_ptr = (unsigned char *)&accumulator;
    unsigned short *short_ptr = (unsigned short *)&accumulator;
    
    /* These stores should preserve upper bits */
    byte_ptr[1] = 0xAA;          /* Modify only byte 1 */
    short_ptr[1] = 0xBBBB;       /* Modify only high 16 bits */
    
    /* Volatile pointer to force memory operations */
    volatile unsigned short *vol_short = (volatile unsigned short *)&accumulator;
    *vol_short = 0xCCCC;         /* This may generate STRICT_LOW_PART */
    
    /* Inline assembly for explicit partial register access */
    unsigned int asm_result;
    __asm__ volatile (
        "movl %1, %0\n\t"
        "movb %2, %b0\n\t"       /* Modify only low byte */
        : "=r"(asm_result)
        : "r"(accumulator), "r"(0xDD)
        : "cc"
    );
    
    result = accumulator ^ asm_result;
    
    /* Function with small integer parameters */
    auto short process_short(short a, short b) -> short {
        /* Local modification of parameter in register */
        a = (short)(a + b);
        return a;
    }
    
    result += process_short(100, 200);
    
    return UNPREDICTABLE(result);
}

/* Array of small types for partial updates */
unsigned int test_strict_low_part_array(short *arr, int n) {
    unsigned int checksum = 0;
    
    for (int i = 0; i < n; i++) {
        /* These assignments may generate STRICT_LOW_PART */
        short temp = arr[i];
        temp = (short)(temp * 3 + 7);
        
        /* Volatile store to force partial register write */
        volatile short *vptr = &arr[i];
        *vptr = temp;
        
        checksum += temp;
    }
    
    return UNPREDICTABLE(checksum);
}

/* ========== SUBREG PATTERNS ========== */

/* Union for type-punning */
union type_pun {
    uint32_t i;
    uint16_t s[2];
    uint8_t b[4];
    float f;
};

/* SUBREG through union access */
unsigned int test_subreg_union(union type_pun *u, int count) {
    unsigned int sum = 0;
    
    for (int i = 0; i < count; i++) {
        /* Access different views of the same register */
        u[i].i = i * 0x01010101;
        
        /* These accesses should generate SUBREG */
        sum += u[i].s[0];        /* Low 16 bits */
        sum += u[i].s[1];        /* High 16 bits */
        sum += u[i].b[2];        /* Third byte */
        
        /* Cast between types */
        uint16_t low_half = (uint16_t)(u[i].i & 0xFFFF);
        uint16_t high_half = (uint16_t)((u[i].i >> 16) & 0xFFFF);
        
        sum += low_half * high_half;
    }
    
    return UNPREDICTABLE(sum);
}

/* SIMD-like operations using unions */
unsigned int test_subreg_simd(void) {
    union {
        uint64_t dword;
        uint32_t words[2];
        uint16_t halves[4];
    } data;
    
    data.dword = 0x0123456789ABCDEFULL;
    
    /* Extract and manipulate sub-parts */
    uint32_t sum = 0;
    sum += data.words[0];                /* Low 32 bits */
    sum += data.words[1];                /* High 32 bits */
    
    /* Process individual 16-bit halves */
    for (int i = 0; i < 4; i++) {
        data.halves[i] = (uint16_t)(data.halves[i] + i);
    }
    
    sum += data.words[0] ^ data.words[1];
    
    return UNPREDICTABLE(sum);
}

/* ========== COMPLEX MEMORY ADDRESSING ========== */

/* Structure with mixed types for complex addressing */
struct mixed_data {
    int a;
    short b;
    char c;
    int d;
    short e[4];
};

/* Complex memory references with SUBREG/ZERO_EXTRACT */
unsigned int test_complex_memory(struct mixed_data *data, int index) {
    unsigned int result = 0;
    
    /* Array access with index calculation */
    result += data[index].a;
    result += data[index].b;
    result += data[index].c;
    
    /* Pointer arithmetic with type conversion */
    short *ptr = &data[index].b;
    for (int i = 0; i < 3; i++) {
        /* Complex addressing mode */
        result += *(ptr + i);
    }
    
    /* Bit-field in memory with complex address */
    struct bitfield_struct *bf_ptr = (struct bitfield_struct *)&data[index];
    bf_ptr->field1 = (index * 7) & 0x1F;
    result += bf_ptr->field1;
    
    /* Memory access through computed pointer */
    int *int_ptr = &data[index].a + (index & 1);
    result += *int_ptr;
    
    return UNPREDICTABLE(result);
}

/* ========== MAIN TEST DRIVER ========== */

int main(int argc, char **argv) {
    unsigned int final_result = 0;
    
    /* Initialize test data */
    unsigned int test_array[16];
    for (int i = 0; i < 16; i++) {
        test_array[i] = i * 0x11111111;
    }
    
    struct bitfield_struct bf_data[4];
    memset(bf_data, 0, sizeof(bf_data));
    
    short short_array[8];
    for (int i = 0; i < 8; i++) {
        short_array[i] = (short)(i * 100);
    }
    
    union type_pun unions[4];
    
    struct mixed_data mixed[4];
    for (int i = 0; i < 4; i++) {
        mixed[i].a = i * 10;
        mixed[i].b = (short)(i * 20);
        mixed[i].c = (char)(i * 30);
        mixed[i].d = i * 40;
        for (int j = 0; j < 4; j++) {
            mixed[i].e[j] = (short)(i * 10 + j);
        }
    }
    
    /* Run all tests to generate various RTL patterns */
    
    /* ZERO_EXTRACT tests */
    final_result ^= test_zero_extract_explicit(0x89ABCDEF);
    
    for (int i = 0; i < 4; i++) {
        final_result += test_zero_extract_struct(&bf_data[i], test_array[i]);
    }
    
    final_result += test_zero_extract_arithmetic(test_array, 8);
    
    /* STRICT_LOW_PART tests */
    final_result ^= test_strict_low_part();
    final_result += test_strict_low_part_array(short_array, 8);
    
    /* SUBREG tests */
    final_result += test_subreg_union(unions, 4);
    final_result ^= test_subreg_simd();
    
    /* Complex memory tests */
    for (int i = 0; i < 4; i++) {
        final_result += test_complex_memory(mixed, i);
    }
    
    /* Use volatile to prevent dead code elimination */
    volatile unsigned int output = final_result;
    
    /* Print result to ensure code isn't eliminated */
    printf("Test result: 0x%08X\n", output);
    
    return (int)(output & 0x7FFFFFFF);
}
