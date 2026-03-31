/* test_resource_coverage.c
 * 
 * This program is designed to trigger specific RTL patterns in GCC's
 * resource management subsystem (resource.cc lines 282-290):
 * - ZERO_EXTRACT: bit-field operations
 * - STRICT_LOW_PART: partial register updates
 * - SUBREG: register sub-parts access
 * - Complex memory references
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent optimizations that might eliminate our test patterns */
volatile int g_volatile_seed = 42;

/* ==================== ZERO_EXTRACT PATTERNS ==================== */

/* Bit-field structure for ZERO_EXTRACT operations */
struct bitfield_struct {
    unsigned int field1 : 5;
    unsigned int field2 : 8;
    unsigned int field3 : 3;
    unsigned int field4 : 16;
};

/* Function 1: Extract bit-fields from integers */
unsigned int test_zero_extract_int(unsigned int x, unsigned int y) {
    unsigned int sum = 0;
    
    /* Various bit-field extraction patterns */
    sum += (x >> 3) & 0x1F;        /* Extract 5 bits starting at bit 3 */
    sum += (y & 0xFF00) >> 8;      /* Extract middle 8 bits */
    sum += (x >> 10) & 0x7;        /* Extract 3 bits */
    sum += (y & 0xFFFF);           /* Extract lower 16 bits */
    
    /* Chain extractions to prevent optimization */
    unsigned int temp = (sum >> 2) & 0x3;
    sum += (temp << 4) & 0x30;
    
    return sum;
}

/* Function 2: Bit-field structure operations */
unsigned int test_zero_extract_struct(struct bitfield_struct *s, int iterations) {
    unsigned int sum = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Bit-field reads (generate ZERO_EXTRACT) */
        sum += s->field1;
        sum += s->field2;
        sum += s->field3;
        sum += s->field4;
        
        /* Bit-field writes (may also generate ZERO_EXTRACT) */
        s->field1 = (sum + i) & 0x1F;
        s->field2 = (sum >> 5) & 0xFF;
        s->field3 = (sum >> 13) & 0x7;
        
        /* Complex bit-field expression */
        if ((s->field1 & 0x0F) == (i & 0x0F)) {
            s->field4 = (s->field4 << 1) | 1;
        }
    }
    
    return sum;
}

/* Function 3: Mixed bit-field operations with memory */
unsigned int test_zero_extract_mixed(unsigned int *array, int size) {
    unsigned int sum = 0;
    
    for (int i = 0; i < size; i++) {
        /* Extract various bit ranges */
        unsigned int val = array[i] ^ g_volatile_seed;
        
        /* Multiple ZERO_EXTRACT patterns */
        sum += (val & 0x000000FF);        /* Lower 8 bits */
        sum += (val & 0x0000FF00) >> 8;   /* Next 8 bits */
        sum += (val & 0x00FF0000) >> 16;  /* Next 8 bits */
        sum += (val & 0xFF000000) >> 24;  /* Upper 8 bits */
        
        /* Conditional extraction */
        if (i & 1) {
            sum += (val >> 4) & 0x0FFF;   /* 12-bit extract */
        } else {
            sum += (val >> 8) & 0xFFFF;   /* 16-bit extract */
        }
        
        /* Write back modified value with bit-field insert */
        array[i] = (array[i] & 0xFFFF0000) | (sum & 0xFFFF);
    }
    
    return sum;
}

/* ==================== STRICT_LOW_PART PATTERNS ==================== */

/* Function 4: Partial register updates with char/short */
unsigned int test_strict_low_part_chars(char *char_array, short *short_array, int size) {
    unsigned int sum = 0;
    
    for (int i = 0; i < size; i++) {
        /* char operations that may use partial registers */
        char c = char_array[i];
        short s = short_array[i];
        
        /* Arithmetic that promotes then truncates */
        int temp_int = c + s + i;
        
        /* Partial register writes (STRICT_LOW_PART) */
        char_array[i] = (char)(temp_int & 0xFF);
        short_array[i] = (short)(temp_int & 0xFFFF);
        
        /* Volatile pointer to force memory access */
        volatile short *vs = &short_array[i];
        *vs = (short)(*vs + 1);  /* Partial update through volatile */
        
        sum += temp_int;
    }
    
    return sum;
}

/* Function 5: Inline assembly for partial register access */
unsigned int test_strict_low_part_asm(short *data, int count) {
    unsigned int sum = 0;
    
    for (int i = 0; i < count; i++) {
        short val = data[i];
        short result;
        
        /* Inline assembly that operates on byte/word registers */
        #if defined(__i386__) || defined(__x86_64__)
        asm volatile (
            "movw %1, %%ax\n\t"
            "addw $1, %%ax\n\t"
            "movw %%ax, %0"
            : "=r" (result)
            : "r" (val)
            : "ax"
        );
        #else
        /* Generic fallback */
        result = val + 1;
        #endif
        
        data[i] = result;
        sum += result;
    }
    
    return sum;
}

/* Function 6: Mixed-size operations */
unsigned int test_strict_low_part_mixed(int *int_array, short *short_array, int size) {
    unsigned int sum = 0;
    
    for (int i = 0; i < size; i++) {
        /* Read int, operate on short part */
        int full = int_array[i];
        short half = (short)(full & 0xFFFF);
        
        /* Modify short, write back to int array (partial update) */
        half = (short)(half + i);
        int_array[i] = (int_array[i] & 0xFFFF0000) | (half & 0xFFFF);
        
        /* Access through volatile short pointer */
        volatile short *vp = (volatile short *)&int_array[i];
        *vp = (short)(*vp - 1);
        
        sum += half + (full >> 16);
    }
    
    return sum;
}

/* ==================== SUBREG PATTERNS ==================== */

/* Union for type-punning (generates SUBREG) */
union type_pun {
    uint32_t full;
    uint16_t halves[2];
    uint8_t bytes[4];
    struct {
        uint16_t low;
        uint16_t high;
    } parts;
};

/* Function 7: Union-based SUBREG patterns */
unsigned int test_subreg_union(union type_pun *data, int count) {
    unsigned int sum = 0;
    
    for (int i = 0; i < count; i++) {
        /* Access different views of the same data (SUBREG) */
        data[i].full = i * 0x01010101;
        
        /* Access sub-parts through union */
        sum += data[i].halves[0];      /* Low 16 bits */
        sum += data[i].halves[1];      /* High 16 bits */
        sum += data[i].bytes[1];       /* Middle byte */
        sum += data[i].parts.low;      /* Through struct */
        sum += data[i].parts.high;     /* Through struct */
        
        /* Modify through one view, read through another */
        data[i].halves[0] = (sum & 0xFFFF);
        sum += data[i].full;           /* Read back full */
    }
    
    return sum;
}

/* Function 8: Casting between types (generates SUBREG) */
unsigned int test_subreg_casts(unsigned int *array, int size) {
    unsigned int sum = 0;
    
    for (int i = 0; i < size; i++) {
        /* Various casts that may generate SUBREG */
        uint32_t val32 = array[i];
        uint16_t val16 = (uint16_t)(val32 & 0xFFFF);
        uint8_t val8 = (uint8_t)(val32 & 0xFF);
        
        /* Operations on different-sized types */
        sum += (unsigned int)val16;
        sum += (unsigned int)val8 << 8;
        
        /* Pointer casting for sub-part access */
        unsigned short *short_ptr = (unsigned short *)&array[i];
        sum += short_ptr[0] + short_ptr[1];
        
        /* Reconstruct with sub-parts */
        array[i] = (val16 << 16) | (val8 << 8) | (sum & 0xFF);
    }
    
    return sum;
}

/* Function 9: SIMD-like operations using unions */
unsigned int test_subreg_simd_like(union type_pun *vectors, int count) {
    unsigned int sum = 0;
    
    for (int i = 0; i < count; i++) {
        /* Simulate packed operations */
        vectors[i].parts.low = (vectors[i].parts.low + 1) & 0x7F7F;
        vectors[i].parts.high = (vectors[i].parts.high - 1) & 0x7F7F;
        
        /* Extract and combine */
        sum += (vectors[i].bytes[0] << 24) |
               (vectors[i].bytes[1] << 16) |
               (vectors[i].bytes[2] << 8) |
               (vectors[i].bytes[3]);
        
        /* Cross-type operations */
        vectors[i].halves[0] ^= vectors[i].halves[1];
        vectors[i].halves[1] ^= sum & 0xFFFF;
    }
    
    return sum;
}

/* ==================== COMPLEX MEMORY REFERENCES ==================== */

/* Function 10: Complex addressing modes */
unsigned int test_complex_memory(int *base_array, int *index_array, int size) {
    unsigned int sum = 0;
    
    for (int i = 0; i < size; i++) {
        /* Complex address calculation */
        int *ptr = &base_array[index_array[i] & (size - 1)];
        
        /* Access with offset */
        sum += ptr[0];
        sum += ptr[1];
        sum += ptr[-1];  /* Negative offset */
        
        /* Modify through pointer */
        *ptr = (sum & 0xFFFF) | ((~sum) << 16);
        
        /* Pointer arithmetic */
        int *next_ptr = ptr + (i & 3);
        sum += *next_ptr;
        
        /* Cast to different type pointer */
        short *short_ptr = (short *)ptr;
        sum += short_ptr[0] + short_ptr[1];
    }
    
    return sum;
}

/* ==================== MAIN TEST DRIVER ==================== */

int main(int argc, char *argv[]) {
    /* Use argc to prevent constant folding */
    int iterations = (argc > 1) ? atoi(argv[1]) : 100;
    if (iterations < 10) iterations = 10;
    if (iterations > 1000) iterations = 1000;
    
    unsigned int final_sum = 0;
    
    /* Initialize test data */
    struct bitfield_struct bf = {1, 2, 3, 4};
    unsigned int int_array[100];
    char char_array[100];
    short short_array[100];
    union type_pun unions[50];
    int base_array[200];
    int index_array[100];
    
    for (int i = 0; i < 100; i++) {
        int_array[i] = i * 0x1234567;
        char_array[i] = (char)(i * 3);
        short_array[i] = (short)(i * 5);
        if (i < 50) unions[i].full = i * 0x11111111;
        if (i < 200) base_array[i] = i * 7;
        if (i < 100) index_array[i] = i * 11;
    }
    
    /* Run all tests to trigger various RTL patterns */
    final_sum += test_zero_extract_int(int_array[0], int_array[1]);
    final_sum += test_zero_extract_struct(&bf, iterations % 20);
    final_sum += test_zero_extract_mixed(int_array, 50);
    
    final_sum += test_strict_low_part_chars(char_array, short_array, 50);
    final_sum += test_strict_low_part_asm(short_array, 50);
    final_sum += test_strict_low_part_mixed(int_array, short_array, 50);
    
    final_sum += test_subreg_union(unions, 25);
    final_sum += test_subreg_casts(int_array, 50);
    final_sum += test_subreg_simd_like(unions, 25);
    
    final_sum += test_complex_memory(base_array, index_array, 50);
    
    /* Mix in volatile to prevent optimization */
    final_sum ^= g_volatile_seed;
    
    /* Use result to affect control flow */
    if (final_sum & 1) {
        printf("Result: %u (odd)\n", final_sum);
    } else {
        printf("Result: %u (even)\n", final_sum);
    }
    
    return (int)(final_sum & 0x7FFFFFFF);
}
