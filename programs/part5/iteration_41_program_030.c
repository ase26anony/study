/* test_resource_patterns.c */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Prevent constant propagation */
volatile int g_volatile_seed = 42;

/* ========== ZERO_EXTRACT patterns ========== */
struct BitFieldStruct {
    unsigned int field1 : 5;
    unsigned int field2 : 8;
    unsigned int field3 : 3;
    unsigned int field4 : 16;
};

/* Function 1: Bit-field extraction from structure */
unsigned int test_zero_extract_struct(struct BitFieldStruct *s, int iterations) {
    unsigned int sum = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* These operations should generate ZERO_EXTRACT for bit-field access */
        sum += s->field1;           /* Extract 5 bits */
        sum += s->field2 << 3;      /* Extract 8 bits, then shift */
        sum += (s->field3 == 2);    /* Bit-field comparison */
        sum += s->field4 & 0x7FFF;  /* Extract 16 bits with mask */
        
        /* Modify structure to prevent optimization */
        s->field1 = (s->field1 + 1) & 0x1F;  /* Wrap within 5 bits */
        s->field2 = (s->field2 * 3) & 0xFF;  /* Wrap within 8 bits */
    }
    
    return sum;
}

/* Function 2: Explicit bit-field extraction from integers */
unsigned int test_zero_extract_integer(unsigned int *arr, int size) {
    unsigned int sum = 0;
    
    for (int i = 0; i < size; i++) {
        /* These should generate ZERO_EXTRACT RTL */
        unsigned int val = arr[i];
        
        /* Extract various bit ranges */
        sum += (val >> 3) & 0x1F;      /* Extract bits 3-7 (5 bits) */
        sum += (val >> 8) & 0xFF;      /* Extract bits 8-15 (8 bits) */
        sum += (val >> 16) & 0x7;      /* Extract bits 16-18 (3 bits) */
        sum += (val & 0x80000000) ? 1 : 0;  /* Test sign bit */
        
        /* Complex extraction with variable shift */
        int shift = (i & 0x3);  /* 0-3 */
        sum += (val >> shift) & ((1 << (shift + 1)) - 1);
    }
    
    return sum;
}

/* ========== STRICT_LOW_PART patterns ========== */
/* Function 3: Partial register updates with small types */
unsigned int test_strict_low_part(short *short_arr, char *char_arr, int size) {
    unsigned int sum = 0;
    
    for (int i = 0; i < size; i++) {
        /* These assignments should generate STRICT_LOW_PART */
        short s = short_arr[i];
        char c = char_arr[i];
        
        /* Promote to int, modify, assign back - partial update */
        int temp_int = s;
        temp_int = temp_int * 2 + i;
        s = (short)temp_int;  /* STRICT_LOW_PART for 16-bit store */
        sum += s;
        
        /* Char operations */
        int temp_char = c;
        temp_char = (temp_char ^ 0x55) + 1;
        c = (char)temp_char;  /* STRICT_LOW_PART for 8-bit store */
        sum += c;
        
        /* Write back to arrays */
        short_arr[i] = s;
        char_arr[i] = c;
    }
    
    return sum;
}

/* Function 4: Volatile partial writes */
unsigned int test_strict_low_part_volatile(volatile short *vs, volatile char *vc, int iterations) {
    unsigned int sum = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Volatile partial writes often use STRICT_LOW_PART */
        short temp_s = *vs;
        *vs = (short)(temp_s + i);  /* Partial write through volatile pointer */
        sum += *vs;
        
        char temp_c = *vc;
        *vc = (char)(temp_c ^ i);   /* Another partial write */
        sum += *vc;
    }
    
    return sum;
}

/* ========== SUBREG patterns ========== */
/* Function 5: Union-based type punning */
union TypePun {
    uint32_t full;
    uint16_t halves[2];
    uint8_t bytes[4];
    struct {
        uint16_t low;
        uint16_t high;
    } parts;
};

unsigned int test_subreg_union(union TypePun *data, int count) {
    unsigned int sum = 0;
    
    for (int i = 0; i < count; i++) {
        /* These accesses should generate SUBREG */
        sum += data[i].halves[0];      /* Access low 16 bits */
        sum += data[i].halves[1];      /* Access high 16 bits */
        sum += data[i].bytes[1] << 8;  /* Access middle byte */
        
        /* Modify through different views */
        data[i].parts.low = (data[i].parts.low + sum) & 0xFFFF;
        data[i].parts.high ^= data[i].parts.low;
        
        /* Cast between different sizes */
        uint16_t low_part = (uint16_t)(data[i].full & 0xFFFF);
        uint16_t high_part = (uint16_t)(data[i].full >> 16);
        sum += low_part + high_part;
    }
    
    return sum;
}

/* Function 6: SIMD-like operations using sub-registers */
unsigned int test_subreg_simd_like(uint32_t *arr, int size) {
    unsigned int sum = 0;
    
    for (int i = 0; i < size; i++) {
        /* Treat 32-bit value as packed 16-bit values */
        uint32_t val = arr[i];
        
        /* Extract and process 16-bit halves */
        uint16_t low = (uint16_t)val;           /* SUBREG for low 16 bits */
        uint16_t high = (uint16_t)(val >> 16);  /* Shift then SUBREG */
        
        /* Process halves independently */
        low = (low * 3) & 0xFFFF;
        high = (high + 0x1234) & 0xFFFF;
        
        /* Recombine */
        arr[i] = ((uint32_t)high << 16) | low;
        sum += low + high;
    }
    
    return sum;
}

/* ========== Combined patterns with memory references ========== */
/* Function 7: Complex pattern combining multiple RTL types */
unsigned int test_combined_patterns(struct BitFieldStruct *bfs, 
                                   short *short_arr, 
                                   union TypePun *unions,
                                   int iterations) {
    unsigned int sum = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* ZERO_EXTRACT from bit-field */
        unsigned int field_val = bfs[i].field2;
        
        /* STRICT_LOW_PART assignment to short */
        short s = short_arr[i];
        int temp = s + field_val;
        s = (short)temp;  /* Partial write */
        short_arr[i] = s;
        
        /* SUBREG access through union */
        unions[i].halves[0] = (uint16_t)s;
        unions[i].halves[1] = (uint16_t)field_val;
        
        /* Memory reference with complex addressing */
        sum += bfs[i].field1 + 
               short_arr[(i + 1) % iterations] + 
               unions[i].bytes[0];
        
        /* Modify bit-field through memory */
        bfs[i].field3 = (bfs[i].field3 + 1) & 0x7;
    }
    
    return sum;
}

/* ========== Main test driver ========== */
int main(int argc, char **argv) {
    /* Use argc to prevent constant folding */
    int iterations = (argc > 1) ? atoi(argv[1]) : 100;
    if (iterations < 10) iterations = 10;
    if (iterations > 1000) iterations = 1000;
    
    /* Initialize test data */
    struct BitFieldStruct *bfs = malloc(iterations * sizeof(struct BitFieldStruct));
    short *short_arr = malloc(iterations * sizeof(short));
    char *char_arr = malloc(iterations * sizeof(char));
    unsigned int *int_arr = malloc(iterations * sizeof(unsigned int));
    union TypePun *unions = malloc(iterations * sizeof(union TypePun));
    volatile short volatile_short = 1000;
    volatile char volatile_char = 50;
    
    /* Seed with pseudo-random but deterministic values */
    for (int i = 0; i < iterations; i++) {
        bfs[i].field1 = (i * 3) & 0x1F;
        bfs[i].field2 = (i * 5) & 0xFF;
        bfs[i].field3 = (i * 7) & 0x7;
        bfs[i].field4 = (i * 11) & 0xFFFF;
        
        short_arr[i] = (short)(i * 13);
        char_arr[i] = (char)(i * 17);
        int_arr[i] = (unsigned int)(i * 19 + 0x12345678);
        
        unions[i].full = (uint32_t)(i * 23 + 0x9ABCDEF0);
    }
    
    /* Run all tests */
    unsigned int total_sum = 0;
    
    total_sum += test_zero_extract_struct(bfs, iterations);
    total_sum += test_zero_extract_integer(int_arr, iterations);
    total_sum += test_strict_low_part(short_arr, char_arr, iterations);
    total_sum += test_strict_low_part_volatile(&volatile_short, &volatile_char, iterations);
    total_sum += test_subreg_union(unions, iterations);
    total_sum += test_subreg_simd_like(int_arr, iterations);
    total_sum += test_combined_patterns(bfs, short_arr, unions, iterations);
    
    /* Add volatile seed to prevent optimization */
    total_sum += g_volatile_seed;
    
    /* Clean up */
    free(bfs);
    free(short_arr);
    free(char_arr);
    free(int_arr);
    free(unions);
    
    /* Print result to ensure side effects */
    printf("Result checksum: %u\n", total_sum);
    
    return (int)(total_sum & 0x7FFFFFFF);
}
