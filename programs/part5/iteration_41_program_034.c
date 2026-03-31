/* test_resource_patterns.c */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Prevent constant propagation */
volatile int g_volatile_seed = 42;

/* ========== ZERO_EXTRACT Patterns ========== */

/* Bit-field structure for ZERO_EXTRACT */
struct bitfield_struct {
    unsigned int field1 : 5;
    unsigned int field2 : 7;
    unsigned int field3 : 3;
    unsigned int field4 : 17;
};

/* Function to trigger ZERO_EXTRACT RTL */
unsigned int test_zero_extract(struct bitfield_struct *bf, unsigned int *arr, int n) {
    unsigned int sum = 0;
    
    /* 1. Bit-field extraction from structure */
    for (int i = 0; i < n; i++) {
        /* These generate ZERO_EXTRACT for bit-field reads */
        sum += bf[i].field1;
        sum += bf[i].field2;
        sum += bf[i].field3;
        sum += bf[i].field4;
        
        /* 2. Bit-field assignment (also generates ZERO_EXTRACT on write) */
        bf[i].field1 = (arr[i] >> 0) & 0x1F;
        bf[i].field2 = (arr[i] >> 5) & 0x7F;
        bf[i].field3 = (arr[i] >> 12) & 0x7;
        bf[i].field4 = (arr[i] >> 15) & 0x1FFFF;
    }
    
    /* 3. Explicit bit-field extraction from integers */
    for (int i = 0; i < n; i++) {
        /* This should generate ZERO_EXTRACT RTL */
        unsigned int val = arr[i];
        unsigned int extracted = (val >> 8) & 0xFF;  /* Extract byte 1 */
        sum += extracted;
        
        /* Another pattern: extract multiple non-contiguous bits */
        unsigned int bits = (val & 0x00FF00FF);
        bits = ((bits >> 8) & 0xFF) | ((bits << 8) & 0xFF00);
        sum += bits;
    }
    
    /* 4. Bit-field comparison */
    for (int i = 0; i < n; i++) {
        if (bf[i].field1 == 10) sum += 1;
        if (bf[i].field2 > 50) sum += 2;
        if (bf[i].field3 != 0) sum += bf[i].field3;
    }
    
    return sum;
}

/* ========== STRICT_LOW_PART Patterns ========== */

/* Function to trigger STRICT_LOW_PART RTL */
unsigned int test_strict_low_part(short *short_arr, char *char_arr, int n) {
    unsigned int sum = 0;
    
    /* 1. Partial register updates through small types */
    for (int i = 0; i < n; i++) {
        /* These assignments may generate STRICT_LOW_PART */
        short s = short_arr[i];
        char c = char_arr[i];
        
        /* Arithmetic that keeps values in registers */
        s = (s * 3 + 7) & 0xFFFF;
        c = (c + i) & 0xFF;
        
        /* Write back - partial register update */
        short_arr[i] = s;
        char_arr[i] = c;
        
        sum += s + c;
    }
    
    /* 2. Volatile pointer to force partial stores */
    volatile short *vol_short = short_arr;
    volatile char *vol_char = char_arr;
    
    for (int i = 0; i < n; i += 2) {
        /* These volatile stores may use STRICT_LOW_PART */
        vol_short[i] = (short)(sum + i);
        vol_char[i] = (char)(sum + i * 3);
    }
    
    /* 3. In-register partial updates */
    {
        int accumulator = g_volatile_seed;
        for (int i = 0; i < n; i++) {
            /* Mix of full and partial register operations */
            accumulator += short_arr[i];
            
            /* Partial update of accumulator */
            short partial = (accumulator >> 8) & 0xFF;
            accumulator = (accumulator & ~0xFF00) | (partial << 8);
            
            sum += accumulator;
        }
    }
    
    /* 4. Function with small integer parameters */
    for (int i = 0; i < n; i++) {
        /* Force register usage with small types */
        short temp = process_short(short_arr[i], (char)i);
        sum += temp;
    }
    
    return sum;
}

/* Helper function for STRICT_LOW_PART */
short process_short(short a, char b) {
    /* Operations that keep values in registers */
    short result = a + b;
    /* Partial modification */
    result = (result & 0xFF) | ((b << 8) & 0xFF00);
    return result;
}

/* ========== SUBREG Patterns ========== */

/* Union for type-punning (SUBREG generation) */
union type_pun {
    uint32_t full;
    uint16_t halves[2];
    uint8_t bytes[4];
    struct {
        uint16_t low;
        uint16_t high;
    } parts;
};

/* Function to trigger SUBREG RTL */
unsigned int test_subreg(union type_pun *data, int n) {
    unsigned int sum = 0;
    
    /* 1. Union-based type punning */
    for (int i = 0; i < n; i++) {
        /* These accesses generate SUBREG */
        sum += data[i].halves[0];
        sum += data[i].halves[1];
        sum += data[i].bytes[0];
        sum += data[i].bytes[3];
        
        /* Modify through different views */
        data[i].halves[1] = (data[i].halves[0] + i) & 0xFFFF;
        data[i].bytes[2] = (data[i].bytes[1] * 3) & 0xFF;
    }
    
    /* 2. Casting between different integer sizes */
    for (int i = 0; i < n; i++) {
        uint32_t val = data[i].full;
        
        /* These casts may generate SUBREG */
        uint16_t low16 = (uint16_t)(val & 0xFFFF);
        uint16_t high16 = (uint16_t)(val >> 16);
        uint8_t low8 = (uint8_t)(val & 0xFF);
        
        /* Operations on sub-parts */
        low16 = (low16 * 3 + 7) & 0xFFFF;
        high16 = (high16 + low8) & 0xFFFF;
        
        /* Recombine */
        data[i].full = (high16 << 16) | low16;
        sum += data[i].full;
    }
    
    /* 3. SIMD-like operations on packed data */
    for (int i = 0; i < n; i++) {
        /* Treat 32-bit as two 16-bit values */
        uint32_t packed = data[i].full;
        
        /* Extract and process halves */
        uint16_t a = packed & 0xFFFF;
        uint16_t b = (packed >> 16) & 0xFFFF;
        
        /* Swap and recombine */
        packed = (a << 16) | b;
        
        /* Extract bytes */
        uint8_t b0 = packed & 0xFF;
        uint8_t b1 = (packed >> 8) & 0xFF;
        uint8_t b2 = (packed >> 16) & 0xFF;
        uint8_t b3 = (packed >> 24) & 0xFF;
        
        /* Rotate bytes */
        packed = (b1 << 24) | (b2 << 16) | (b3 << 8) | b0;
        
        data[i].full = packed;
        sum += packed;
    }
    
    return sum;
}

/* ========== Complex Memory References ========== */

/* Function combining patterns with complex addressing */
unsigned int test_complex_memory(int *base_arr, int index, int stride) {
    unsigned int sum = 0;
    
    /* Complex addressing modes */
    for (int i = 0; i < 16; i++) {
        /* Array access with index computation */
        int *ptr = &base_arr[index + i * stride];
        
        /* Bit-field extraction from memory */
        int val = *ptr;
        int extracted = (val >> (i % 16)) & ((1 << 8) - 1);
        
        /* Partial write back */
        short *short_ptr = (short *)ptr;
        *short_ptr = (short)(extracted + i);  /* STRICT_LOW_PART */
        
        /* Union access through pointer */
        union type_pun *up = (union type_pun *)ptr;
        sum += up->halves[0];  /* SUBREG */
        
        /* Update with bit-field operation */
        val = *ptr;
        val = (val & ~(0x1F << 10)) | ((extracted & 0x1F) << 10);  /* ZERO_EXTRACT-like */
        *ptr = val;
    }
    
    return sum;
}

/* ========== Main Test Driver ========== */

int main(int argc, char **argv) {
    /* Use argc to prevent constant folding */
    int size = (argc > 1) ? atoi(argv[1]) : 100;
    if (size < 10) size = 100;
    
    /* Allocate test data */
    struct bitfield_struct *bf_arr = 
        (struct bitfield_struct *)calloc(size, sizeof(struct bitfield_struct));
    unsigned int *int_arr = (unsigned int *)malloc(size * sizeof(unsigned int));
    short *short_arr = (short *)malloc(size * sizeof(short));
    char *char_arr = (char *)malloc(size * sizeof(char));
    union type_pun *union_arr = (union type_pun *)malloc(size * sizeof(union type_pun));
    int *base_arr = (int *)malloc(1024 * sizeof(int));
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < size; i++) {
        int_arr[i] = (i * 1103515245 + 12345) & 0x7FFFFFFF;
        short_arr[i] = (short)(int_arr[i] & 0xFFFF);
        char_arr[i] = (char)(int_arr[i] & 0xFF);
        union_arr[i].full = int_arr[i];
        
        /* Initialize bit-fields */
        bf_arr[i].field1 = int_arr[i] & 0x1F;
        bf_arr[i].field2 = (int_arr[i] >> 5) & 0x7F;
        bf_arr[i].field3 = (int_arr[i] >> 12) & 0x7;
        bf_arr[i].field4 = (int_arr[i] >> 15) & 0x1FFFF;
    }
    
    for (int i = 0; i < 1024; i++) {
        base_arr[i] = (i * 997 + 7919) & 0x7FFFFFFF;
    }
    
    /* Run all tests */
    unsigned int total = 0;
    
    total += test_zero_extract(bf_arr, int_arr, size);
    total += test_strict_low_part(short_arr, char_arr, size);
    total += test_subreg(union_arr, size);
    total += test_complex_memory(base_arr, size % 512, 3);
    
    /* Mix in volatile to prevent dead code elimination */
    total += g_volatile_seed;
    
    /* Use result */
    printf("Result: %u\n", total);
    
    /* Cleanup */
    free(bf_arr);
    free(int_arr);
    free(short_arr);
    free(char_arr);
    free(union_arr);
    free(base_arr);
    
    return (int)(total % 256);
}
