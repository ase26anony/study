/* test_resource_patterns.c */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Prevent constant folding and dead code elimination */
volatile int g_volatile_seed = 42;

/* ========== ZERO_EXTRACT patterns ========== */
struct BitFieldStruct {
    unsigned int field1 : 5;
    unsigned int field2 : 7;
    unsigned int field3 : 3;
    unsigned int field4 : 17;
};

/* Function 1: Bit-field extraction from structure */
unsigned int test_zero_extract_struct(struct BitFieldStruct *s, int iterations) {
    unsigned int sum = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* These operations should generate ZERO_EXTRACT for bit-field access */
        sum += s->field1;                    /* Extract 5-bit field */
        sum += s->field2 << 2;               /* Extract 7-bit field and shift */
        sum += (s->field3 == 2) ? 1 : 0;     /* Bit-field comparison */
        sum += s->field4 & 0x1FFF;           /* Extract lower 13 bits of 17-bit field */
        
        /* Modify structure to prevent optimization */
        s->field1 = (s->field1 + 1) & 0x1F;   /* Keep within 5 bits */
        s->field2 ^= (i & 0x7F);              /* Keep within 7 bits */
    }
    
    return sum;
}

/* Function 2: Explicit bit-field extraction from integers */
unsigned int test_zero_extract_integer(unsigned int *arr, int size) {
    unsigned int sum = 0;
    
    for (int i = 0; i < size; i++) {
        /* These should generate ZERO_EXTRACT RTL */
        unsigned int val = arr[i];
        
        /* Extract various bit fields with different widths */
        sum += (val >> 3) & 0x1F;      /* Extract bits 3-7 (5 bits) */
        sum += (val >> 8) & 0xFF;      /* Extract bits 8-15 (8 bits) */
        sum += (val >> 16) & 0x7FF;    /* Extract bits 16-26 (11 bits) */
        sum += (val & 0x7) << 2;       /* Extract bits 0-2 and shift */
        
        /* Complex extraction pattern */
        unsigned int extracted = ((val & 0xFF00) >> 8) | ((val & 0xF) << 4);
        sum += extracted;
    }
    
    return sum;
}

/* ========== STRICT_LOW_PART patterns ========== */
/* Function 3: Partial register updates with small types */
unsigned int test_strict_low_part(volatile short *sarray, volatile char *carray, 
                                  int size, int modifier) {
    unsigned int sum = 0;
    
    for (int i = 0; i < size; i++) {
        /* These assignments should generate STRICT_LOW_PART for partial updates */
        short s_val = sarray[i];
        char c_val = carray[i];
        
        /* Partial register updates - compiler must preserve upper bits */
        s_val = (s_val + modifier) & 0xFFFF;    /* Update only low 16 bits */
        c_val = (c_val ^ i) & 0xFF;             /* Update only low 8 bits */
        
        /* Store back through volatile pointers */
        sarray[i] = s_val;      /* Should generate STRICT_LOW_PART store */
        carray[i] = c_val;      /* Should generate STRICT_LOW_PART store */
        
        sum += s_val + c_val;
    }
    
    return sum;
}

/* Function 4: Inline assembly for partial register access */
unsigned int test_strict_low_part_asm(int iterations) {
    unsigned int sum = 0;
    unsigned int accumulator = g_volatile_seed;
    
    for (int i = 0; i < iterations; i++) {
        unsigned short low_part;
        
        /* Inline assembly that operates on partial register */
        #if defined(__i386__) || defined(__x86_64__)
        /* Byte register operation - should generate STRICT_LOW_PART */
        asm volatile (
            "movb %1, %%al\n\t"
            "addb $1, %%al\n\t"
            "movb %%al, %0"
            : "=r" (low_part)
            : "r" ((unsigned char)(i & 0xFF))
            : "%al"
        );
        #else
        /* Portable fallback */
        low_part = (i + 1) & 0xFF;
        #endif
        
        /* Mix with upper bits */
        accumulator = (accumulator & 0xFFFF0000) | low_part;
        sum += accumulator;
    }
    
    return sum;
}

/* ========== SUBREG patterns ========== */
/* Function 5: Union-based type punning */
unsigned int test_subreg_union(int *data, int size) {
    union PunningUnion {
        uint32_t full;
        uint16_t halves[2];
        uint8_t bytes[4];
    } u;
    
    unsigned int sum = 0;
    
    for (int i = 0; i < size; i++) {
        u.full = data[i];
        
        /* Access sub-parts - should generate SUBREG */
        sum += u.halves[0];          /* Low 16 bits */
        sum += u.halves[1] << 8;     /* High 16 bits, shifted */
        sum += u.bytes[1] * 3;       /* Second byte */
        
        /* Modify and reassemble */
        u.halves[0] = (u.halves[0] + i) & 0xFFFF;
        u.bytes[2] ^= 0xAA;
        
        data[i] = u.full;  /* Write back full word */
    }
    
    return sum;
}

/* Function 6: Casting between different integer sizes */
unsigned int test_subreg_casting(long *array, int size) {
    unsigned int sum = 0;
    
    for (int i = 0; i < size; i++) {
        /* Casts that should generate SUBREG */
        int truncated = (int)(array[i] & 0xFFFFFFFF);
        short lower_half = (short)(truncated & 0xFFFF);
        char quarter = (char)(truncated & 0xFF);
        
        /* Operations on sub-register parts */
        sum += truncated;
        sum += lower_half * 2;
        sum += quarter << 4;
        
        /* Modify and write back with new sub-region */
        array[i] = (long)((array[i] & ~0xFFFFL) | ((lower_half + i) & 0xFFFF));
    }
    
    return sum;
}

/* ========== Combined patterns with memory references ========== */
/* Function 7: Complex pattern combining multiple RTL types */
unsigned int test_combined_patterns(struct BitFieldStruct *bfs, 
                                    unsigned int *int_array,
                                    volatile short *short_array,
                                    int iterations) {
    unsigned int sum = 0;
    union {
        unsigned int full;
        struct {
            unsigned short low;
            unsigned short high;
        } parts;
    } converter;
    
    for (int i = 0; i < iterations; i++) {
        /* ZERO_EXTRACT from bit-field */
        unsigned int bf_val = (bfs->field2 << 3) | bfs->field1;
        
        /* STRICT_LOW_PART update */
        short_array[i % 16] = (short)(bf_val & 0xFFFF);
        
        /* SUBREG access via union */
        converter.full = int_array[i];
        sum += converter.parts.low;      /* SUBREG for low half */
        sum += converter.parts.high << 1; /* SUBREG for high half */
        
        /* Memory reference with complex addressing */
        int_array[(i * 7) % iterations] += 
            (converter.parts.low & 0xF) + (bfs->field3 << 1);
        
        /* Update bit-field structure */
        bfs->field1 = (bfs->field1 + converter.parts.low) & 0x1F;
        bfs->field4 = (bfs->field4 ^ int_array[i]) & 0x1FFFF;
    }
    
    return sum;
}

/* ========== Main test driver ========== */
int main(int argc, char **argv) {
    /* Use command line or volatile to prevent constant folding */
    int iterations = (argc > 1) ? atoi(argv[1]) : 100;
    if (iterations < 10) iterations = 100;
    
    /* Initialize test data */
    struct BitFieldStruct bfs = {1, 2, 3, 1000};
    
    unsigned int *int_array = (unsigned int*)malloc(iterations * sizeof(unsigned int));
    volatile short *short_array = (volatile short*)malloc(iterations * sizeof(short));
    volatile char *char_array = (volatile char*)malloc(iterations);
    long *long_array = (long*)malloc(iterations * sizeof(long));
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < iterations; i++) {
        int_array[i] = (i * 1103515245 + 12345) & 0xFFFFFFFF;
        short_array[i] = (short)(i * 32767 / iterations);
        char_array[i] = (char)(i ^ 0x55);
        long_array[i] = (long)int_array[i] * 3;
    }
    
    unsigned int total_sum = 0;
    
    /* Test ZERO_EXTRACT patterns */
    total_sum += test_zero_extract_struct(&bfs, iterations / 2);
    total_sum += test_zero_extract_integer(int_array, iterations);
    
    /* Test STRICT_LOW_PART patterns */
    total_sum += test_strict_low_part(short_array, char_array, iterations, g_volatile_seed);
    total_sum += test_strict_low_part_asm(iterations / 10);
    
    /* Test SUBREG patterns */
    total_sum += test_subreg_union(int_array, iterations);
    total_sum += test_subreg_casting(long_array, iterations / 2);
    
    /* Test combined patterns */
    total_sum += test_combined_patterns(&bfs, int_array, short_array, iterations / 4);
    
    /* Cleanup */
    free(int_array);
    free((void*)short_array);
    free((void*)char_array);
    free(long_array);
    
    /* Return result to prevent dead code elimination */
    printf("Total checksum: %u\n", total_sum);
    return (int)(total_sum & 0x7FFFFFFF);
}
