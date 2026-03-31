/* test_resource_patterns.c
 * Designed to trigger specific RTL patterns in GCC's resource.cc:
 * - ZERO_EXTRACT for bit-field operations
 * - STRICT_LOW_PART for partial register updates
 * - SUBREG for register sub-parts
 * - Complex memory addressing modes
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent constant folding and dead code elimination */
volatile int g_volatile_seed = 42;

/* ========== ZERO_EXTRACT PATTERNS ========== */

/* Bit-field structure for ZERO_EXTRACT */
struct bitfield_struct {
    unsigned int field1 : 5;
    unsigned int field2 : 8;
    unsigned int field3 : 3;
    unsigned int field4 : 16;
};

/* Function 1: Explicit bit-field extraction */
unsigned int test_zero_extract_explicit(unsigned int x, int shift) {
    /* This should generate ZERO_EXTRACT for bit-field extraction */
    unsigned int mask = (1 << 9) - 1;  /* 9-bit mask, not power of 2 */
    unsigned int result = (x >> shift) & mask;
    
    /* Prevent optimization */
    if (g_volatile_seed & 1) {
        result ^= 0x5555;
    }
    
    return result;
}

/* Function 2: Bit-field structure operations */
unsigned int test_zero_extract_struct(struct bitfield_struct *s, unsigned int val) {
    unsigned int sum = 0;
    
    /* Multiple bit-field operations that should generate ZERO_EXTRACT */
    s->field1 = val & 0x1F;           /* 5-bit field */
    s->field2 = (val >> 5) & 0xFF;    /* 8-bit field */
    s->field3 = (val >> 13) & 0x7;    /* 3-bit field */
    
    /* Bit-field comparison (should generate ZERO_EXTRACT for comparison) */
    if (s->field1 == 0x10) {
        sum += 1;
    }
    
    /* Bit-field arithmetic */
    sum += s->field2 * 2;
    sum += s->field3 << 1;
    
    /* Complex bit-field expression */
    s->field4 = ((val & 0xFF00) >> 8) + ((val & 0x00FF) << 2);
    
    return sum + s->field4;
}

/* Function 3: Mixed bit-field operations with memory */
unsigned int test_zero_extract_memory(unsigned int *array, int size) {
    struct bitfield_struct s;
    unsigned int checksum = 0;
    
    for (int i = 0; i < size && i < 10; i++) {
        /* Array access creates memory reference */
        unsigned int val = array[i] ^ g_volatile_seed;
        
        /* Multiple bit-field extractions */
        unsigned int low5 = val & 0x1F;
        unsigned int mid8 = (val >> 5) & 0xFF;
        unsigned int high3 = (val >> 24) & 0x7;
        
        /* Use in computation to prevent elimination */
        checksum += (low5 * mid8) | (high3 << 16);
        
        /* Store back through pointer with bit-field */
        array[i] = (array[i] & ~0xFF) | (checksum & 0xFF);
    }
    
    return checksum;
}

/* ========== STRICT_LOW_PART PATTERNS ========== */

/* Function 4: Partial register updates with small types */
unsigned int test_strict_low_part_chars(char *data, int length) {
    unsigned int sum = 0;
    
    /* Loop with char operations - should generate STRICT_LOW_PART */
    for (int i = 0; i < length; i++) {
        /* char assignment in wider register context */
        char c = data[i] + i;
        
        /* Force register promotion and partial write-back */
        int temp = c * 2;
        data[i] = temp & 0x7F;  /* Partial write of low 8 bits */
        
        sum += data[i];
    }
    
    return sum;
}

/* Function 5: Short operations with volatile */
unsigned int test_strict_low_part_shorts(short *array, int count) {
    unsigned int result = 0;
    
    for (int i = 0; i < count; i++) {
        /* volatile short pointer - may generate STRICT_LOW_PART */
        volatile short *ptr = &array[i];
        
        /* Operation that requires partial register update */
        short old = *ptr;
        *ptr = (old + g_volatile_seed) & 0x7FFF;
        
        result += *ptr;
        
        /* Mixed-size operations */
        int widened = *ptr;
        widened = widened * 3;
        *ptr = widened & 0xFFFF;  /* Partial write-back */
    }
    
    return result;
}

/* Function 6: Inline assembly for byte register operations */
unsigned int test_strict_low_part_asm(unsigned int x) {
    unsigned char result;
    
    /* Inline assembly that operates on byte register */
    __asm__ volatile (
        "movb %%al, %0\n\t"
        : "=q" (result)   /* 'q' constraint for byte-addressable register */
        : "a" (x)         /* Input in eax/rax */
        : 
    );
    
    /* Force multiple partial register updates */
    unsigned short s;
    __asm__ volatile (
        "movw %%ax, %0\n\t"
        : "=r" (s)
        : "a" (x * 2)
        :
    );
    
    return result + s;
}

/* ========== SUBREG PATTERNS ========== */

/* Function 7: Union for type-punning (SUBREG generation) */
unsigned int test_subreg_union(unsigned int value) {
    union {
        uint32_t i;
        uint16_t s[2];
        uint8_t b[4];
    } u;
    
    u.i = value ^ g_volatile_seed;
    
    /* Access sub-parts - should generate SUBREG */
    unsigned int sum = u.s[0] + u.s[1];
    sum += u.b[0] * 3;
    sum += u.b[2] << 4;
    
    /* Modify through sub-parts */
    u.s[1] = (sum & 0xFFFF);
    
    return u.i + sum;
}

/* Function 8: Casting between different integer sizes */
unsigned int test_subreg_casts(int *array, int size) {
    unsigned int checksum = 0;
    
    for (int i = 0; i < size; i++) {
        /* Casts that should generate SUBREG */
        int val = array[i];
        short s = (short)(val & 0xFFFF);
        char c = (char)((val >> 16) & 0xFF);
        
        /* Operations on sub-parts */
        checksum += s * 2;
        checksum += c * 3;
        
        /* Store back partial results */
        array[i] = (array[i] & 0xFFFF0000) | (checksum & 0xFFFF);
    }
    
    return checksum;
}

/* Function 9: Packed structure with mixed types */
struct packed_data {
    int a;
    short b;
    char c;
    int d;
} __attribute__((packed));

unsigned int test_subreg_packed(struct packed_data *data, int count) {
    unsigned int sum = 0;
    
    for (int i = 0; i < count; i++) {
        /* Access packed fields - may require SUBREG */
        sum += data[i].a;
        sum += data[i].b << 8;
        sum += data[i].c * 5;
        
        /* Modify through different-sized views */
        data[i].b = (sum & 0xFFFF);
        data[i].c = ((sum >> 16) & 0xFF);
    }
    
    return sum;
}

/* ========== COMPLEX MEMORY ADDRESSING ========== */

/* Function 10: Complex addressing modes with bit operations */
unsigned int test_complex_addressing(int *base, int *offsets, int n) {
    unsigned int result = 0;
    
    for (int i = 0; i < n; i++) {
        /* Complex address calculation */
        int *addr = base + (offsets[i] & 0xF);
        
        /* Bit-field extraction from memory */
        int val = *addr;
        int field = (val >> (i * 2)) & 0x3;  /* 2-bit field extraction */
        
        /* Store with partial update */
        *addr = (*addr & ~(0x3 << (i * 2))) | (field << (i * 2));
        
        result += field * i;
    }
    
    return result;
}

/* ========== MAIN TEST DRIVER ========== */

int main(int argc, char **argv) {
    unsigned int final_result = 0;
    
    /* Initialize test data with some variability */
    unsigned int array[20];
    for (int i = 0; i < 20; i++) {
        array[i] = (i * 137) ^ g_volatile_seed;
    }
    
    short short_array[15];
    for (int i = 0; i < 15; i++) {
        short_array[i] = (i * 97) & 0x7FFF;
    }
    
    char char_data[25];
    for (int i = 0; i < 25; i++) {
        char_data[i] = (i * 53) & 0x7F;
    }
    
    struct bitfield_struct bf_struct;
    struct packed_data packed_array[5];
    for (int i = 0; i < 5; i++) {
        packed_array[i].a = i * 1000;
        packed_array[i].b = i * 100;
        packed_array[i].c = i * 10;
        packed_array[i].d = i;
    }
    
    int offset_array[10] = {0, 2, 4, 1, 3, 5, 7, 6, 8, 9};
    
    /* Run all tests to trigger various RTL patterns */
    
    /* ZERO_EXTRACT patterns */
    final_result += test_zero_extract_explicit(array[0], 3);
    final_result += test_zero_extract_struct(&bf_struct, array[1]);
    final_result += test_zero_extract_memory(array, 10);
    
    /* STRICT_LOW_PART patterns */
    final_result += test_strict_low_part_chars(char_data, 20);
    final_result += test_strict_low_part_shorts(short_array, 10);
    final_result += test_strict_low_part_asm(array[2]);
    
    /* SUBREG patterns */
    final_result += test_subreg_union(array[3]);
    final_result += test_subreg_casts(array, 5);
    final_result += test_subreg_packed(packed_array, 3);
    
    /* Complex addressing */
    final_result += test_complex_addressing(array, offset_array, 8);
    
    /* Mix everything together */
    for (int i = 0; i < 5; i++) {
        /* Force all patterns in a loop */
        unsigned int temp = array[i];
        
        /* ZERO_EXTRACT */
        temp = (temp >> (i * 2)) & ((1 << (i + 3)) - 1);
        
        /* STRICT_LOW_PART simulation */
        short_array[i % 10] = (temp & 0xFFFF);
        
        /* SUBREG through union */
        union { unsigned int i; unsigned short s[2]; } u;
        u.i = temp;
        final_result += u.s[0] + u.s[1];
    }
    
    /* Use volatile to prevent dead code elimination */
    if (g_volatile_seed) {
        printf("Result: %u\n", final_result);
    }
    
    return (final_result & 0xFF);
}
