/* test_resource_patterns.c */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Prevent constant propagation */
volatile int g_volatile_seed = 42;

/* ========== ZERO_EXTRACT patterns ========== */

/* Bit-field structure for ZERO_EXTRACT */
struct bitfield_struct {
    unsigned int field1 : 5;
    unsigned int field2 : 7;
    unsigned int field3 : 3;
    unsigned int field4 : 17;
};

/* Function to trigger ZERO_EXTRACT from bit-field operations */
unsigned int test_zero_extract(struct bitfield_struct *bf, unsigned int *arr, int n) {
    unsigned int sum = 0;
    
    /* Process bit-field reads (will generate ZERO_EXTRACT) */
    for (int i = 0; i < n; i++) {
        /* Bit-field comparisons and extracts */
        if (bf->field1 == (i & 0x1F)) {
            sum += bf->field2;
        }
        
        /* Combine multiple bit-fields */
        unsigned int combined = (bf->field3 << 10) | (bf->field1 << 5) | bf->field2;
        sum += combined;
        
        /* Explicit bit-field extract from integer */
        unsigned int val = arr[i];
        unsigned int extracted = (val >> 8) & 0xFF;  /* Should generate ZERO_EXTRACT */
        sum += extracted;
        
        /* Rotate to next structure (prevents optimization) */
        bf->field1 = (bf->field1 + 1) & 0x1F;
        bf->field2 = (bf->field2 * 3) & 0x7F;
    }
    
    return sum;
}

/* Additional ZERO_EXTRACT patterns with direct bit manipulation */
unsigned int test_bit_extract(unsigned int x, int shift, unsigned int mask) {
    /* Multiple extract patterns that should generate ZERO_EXTRACT */
    unsigned int result = 0;
    
    /* Pattern 1: Classic extract with variable shift */
    result += (x >> shift) & mask;
    
    /* Pattern 2: Extract with constant mask (width < word size) */
    result += (x >> 4) & 0x0FFF;  /* 12-bit extract from 32-bit word */
    
    /* Pattern 3: Multiple extracts in sequence */
    result += (x & 0xFF000000) >> 24;
    result += (x & 0x00FF0000) >> 16;
    result += (x & 0x0000FF00) >> 8;
    result += (x & 0x000000FF);
    
    /* Pattern 4: Conditional extract */
    if (x & 0x80000000) {
        result += (x >> 1) & 0x7FFFFFFF;
    }
    
    return result;
}

/* ========== STRICT_LOW_PART patterns ========== */

/* Function to trigger STRICT_LOW_PART for partial register updates */
unsigned int test_strict_low_part(short *short_arr, char *char_arr, int n) {
    unsigned int sum = 0;
    
    /* Partial writes to promoted variables */
    for (int i = 0; i < n; i++) {
        /* char variable that gets promoted to int, then partial write back */
        char c = char_arr[i];
        c = (c * 3 + i) & 0xFF;  /* Partial update of promoted register */
        sum += c;
        char_arr[i] = c;  /* Store back only low 8 bits */
        
        /* short variable with arithmetic */
        short s = short_arr[i];
        s = (s * 5 - i) & 0xFFFF;  /* Partial update of promoted register */
        sum += s;
        short_arr[i] = s;  /* Store back only low 16 bits */
        
        /* Volatile pointer to force partial store */
        volatile short *vs = &short_arr[i];
        *vs = (*vs + 1) & 0x7FFF;
        
        /* Inline assembly for byte register update (x86 specific) */
        #ifdef __x86_64__
        unsigned char byte_val = (i & 0xFF);
        asm volatile (
            "movb %1, %0\n\t"
            : "=q"(c)  /* q constraint = a, b, c, or d register (byte accessible) */
            : "r"(byte_val)
            : "cc"
        );
        sum += c;
        #endif
    }
    
    return sum;
}

/* Additional STRICT_LOW_PART patterns */
unsigned int test_partial_updates(int *int_arr, int n) {
    unsigned int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Small integer types in registers */
        unsigned short us = int_arr[i] & 0xFFFF;
        unsigned char uc = int_arr[i] & 0xFF;
        
        /* Operations that preserve only part of the register */
        us = (us * 2 + 1) & 0xFFFF;  /* Only low 16 bits matter */
        uc = (uc + g_volatile_seed) & 0xFF;  /* Only low 8 bits matter */
        
        sum += us + uc;
        
        /* Write to memory through typed pointer */
        *((volatile short*)(&int_arr[i])) = us;
    }
    
    return sum;
}

/* ========== SUBREG patterns ========== */

/* Union for type-punning to generate SUBREG */
union type_pun {
    uint32_t word;
    uint16_t half[2];
    uint8_t byte[4];
    struct {
        uint16_t low;
        uint16_t high;
    } parts;
};

/* Function to trigger SUBREG patterns */
unsigned int test_subreg(union type_pun *data, int n) {
    unsigned int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Access different views of the same data (should generate SUBREG) */
        sum += data[i].half[0];  /* SUBREG for low 16 bits */
        sum += data[i].half[1];  /* SUBREG for high 16 bits */
        
        /* Cast between types of different sizes */
        uint32_t val = data[i].word;
        uint16_t low_part = (uint16_t)(val & 0xFFFF);  /* SUBREG pattern */
        uint16_t high_part = (uint16_t)(val >> 16);    /* Another SUBREG */
        
        /* Packed structure access */
        data[i].parts.low = low_part + i;
        data[i].parts.high = high_part - i;
        
        /* Byte access through union */
        for (int j = 0; j < 4; j++) {
            sum += data[i].byte[j];
        }
    }
    
    return sum;
}

/* Additional SUBREG patterns with SIMD-like operations */
unsigned int test_vector_extract(unsigned int *arr, int n) {
    unsigned int sum = 0;
    
    /* Simulate vector element extraction */
    for (int i = 0; i < n; i += 2) {
        /* Treat two consecutive ints as a "vector" */
        uint64_t vector = ((uint64_t)arr[i + 1] << 32) | arr[i];
        
        /* Extract elements (should use SUBREG on 64-bit architectures) */
        uint32_t elem0 = (uint32_t)(vector & 0xFFFFFFFF);
        uint32_t elem1 = (uint32_t)(vector >> 32);
        
        sum += elem0 + elem1;
        
        /* Modify and store back */
        arr[i] = elem0 ^ 0xAAAAAAAA;
        arr[i + 1] = elem1 ^ 0x55555555;
    }
    
    return sum;
}

/* ========== Complex memory references ========== */

/* Function combining multiple patterns with complex addressing */
unsigned int test_complex_memory(int *base_arr, struct bitfield_struct *bf_arr, 
                                 union type_pun *pun_arr, int n) {
    unsigned int sum = 0;
    
    /* Complex addressing modes with bit-field operations */
    for (int i = 0; i < n; i++) {
        /* Array access with index computation */
        int *ptr = &base_arr[i * 2 + (i & 1)];
        
        /* Bit-field extract from memory */
        unsigned int val = *ptr;
        unsigned int extracted = (val >> (i % 16)) & ((1 << 8) - 1);
        
        /* Partial write to memory */
        *((volatile short*)ptr) = extracted & 0xFFFF;
        
        /* Union type-punning with memory access */
        union type_pun *up = &pun_arr[i];
        up->half[0] = (up->half[0] + extracted) & 0x7FFF;
        
        /* Bit-field structure update */
        bf_arr[i].field1 = (bf_arr[i].field1 + i) & 0x1F;
        bf_arr[i].field2 = (bf_arr[i].field2 * 3) & 0x7F;
        
        sum += *ptr + up->word + bf_arr[i].field1;
    }
    
    return sum;
}

/* ========== Main test driver ========== */

int main(int argc, char **argv) {
    /* Use command line or volatile to prevent compile-time optimization */
    int n = (argc > 1) ? atoi(argv[1]) : 100;
    if (n < 10) n = 10;
    if (n > 1000) n = 1000;
    
    /* Allocate and initialize test data */
    struct bitfield_struct *bf_arr = calloc(n, sizeof(struct bitfield_struct));
    unsigned int *int_arr = malloc(n * sizeof(unsigned int));
    short *short_arr = malloc(n * sizeof(short));
    char *char_arr = malloc(n * sizeof(char));
    union type_pun *pun_arr = calloc(n, sizeof(union type_pun));
    int *base_arr = malloc(2 * n * sizeof(int));
    
    /* Initialize with pseudo-random but deterministic data */
    for (int i = 0; i < n; i++) {
        bf_arr[i].field1 = (i * 3) & 0x1F;
        bf_arr[i].field2 = (i * 5) & 0x7F;
        bf_arr[i].field3 = (i * 7) & 0x07;
        bf_arr[i].field4 = (i * 11) & 0x1FFFF;
        
        int_arr[i] = (i * 13) ^ 0x12345678;
        short_arr[i] = (i * 17) & 0x7FFF;
        char_arr[i] = (i * 19) & 0x7F;
        
        pun_arr[i].word = (i * 23) ^ 0x89ABCDEF;
        
        base_arr[2*i] = (i * 29) ^ 0xF0F0F0F0;
        base_arr[2*i + 1] = (i * 31) ^ 0x0F0F0F0F;
    }
    
    unsigned int total_sum = 0;
    
    /* Execute all test patterns */
    total_sum += test_zero_extract(bf_arr, int_arr, n);
    total_sum += test_bit_extract(g_volatile_seed, 3, 0x1F);
    
    total_sum += test_strict_low_part(short_arr, char_arr, n);
    total_sum += test_partial_updates((int*)int_arr, n);
    
    total_sum += test_subreg(pun_arr, n);
    total_sum += test_vector_extract(int_arr, n/2);
    
    total_sum += test_complex_memory(base_arr, bf_arr, pun_arr, n);
    
    /* Additional mixed pattern to ensure coverage */
    for (int i = 0; i < n; i++) {
        /* Mix all patterns in one loop */
        unsigned int val = int_arr[i];
        
        /* ZERO_EXTRACT pattern */
        unsigned int extracted = (val >> (i % 24)) & ((1 << 6) - 1);
        
        /* STRICT_LOW_PART pattern */
        short partial = (short)(extracted * i);
        *((volatile short*)&int_arr[i]) = partial;
        
        /* SUBREG pattern via union */
        union type_pun tmp;
        tmp.word = val;
        tmp.half[0] ^= tmp.half[1];
        
        total_sum += tmp.word + partial;
    }
    
    /* Clean up */
    free(bf_arr);
    free(int_arr);
    free(short_arr);
    free(char_arr);
    free(pun_arr);
    free(base_arr);
    
    /* Return the checksum to prevent dead code elimination */
    printf("Result checksum: %u\n", total_sum);
    return (int)(total_sum & 0x7FFFFFFF);
}
