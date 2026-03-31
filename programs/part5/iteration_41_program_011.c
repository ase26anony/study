/* test_resource_patterns.c
 * Compile with: gcc -O2 -fschedule-insns -fno-omit-frame-pointer test_resource_patterns.c -o test
 * For RTL dumps: gcc -O1 -da -fdump-rtl-all test_resource_patterns.c
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent constant folding */
volatile int g_volatile_seed = 42;

/* ========== ZERO_EXTRACT patterns ========== */
struct BitFieldStruct {
    unsigned int field1 : 5;
    unsigned int field2 : 7;
    unsigned int field3 : 4;
    unsigned int field4 : 16;
};

int test_zero_extract(struct BitFieldStruct *bfs, int iterations) {
    int sum = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Pattern 1: Bit-field extraction from struct */
        unsigned int val1 = bfs[i].field2;  /* Should generate ZERO_EXTRACT */
        
        /* Pattern 2: Explicit mask and shift */
        unsigned int raw = (unsigned int)(bfs[i].field1 + g_volatile_seed);
        unsigned int val2 = (raw & 0x1F) >> 1;  /* Extract bits 4:1 */
        
        /* Pattern 3: Multiple bit-field operations */
        unsigned int combined = (bfs[i].field3 << 3) | bfs[i].field1;
        
        /* Pattern 4: Bit-field comparison */
        if (bfs[i].field4 == (unsigned int)g_volatile_seed) {
            val1 += 1;
        }
        
        /* Pattern 5: Complex bit-field arithmetic */
        unsigned int temp = bfs[i].field2 * 3;
        unsigned int val3 = (temp & 0x7F) + bfs[i].field1;  /* Keep within field2's width */
        
        sum += val1 + val2 + combined + val3;
    }
    
    return sum;
}

/* ========== STRICT_LOW_PART patterns ========== */
int test_strict_low_part(short *short_array, char *char_array, int size) {
    int sum = 0;
    
    for (int i = 0; i < size; i++) {
        /* Pattern 1: Partial register update through pointer */
        volatile short *vs = &short_array[i];
        *vs = (short)(*vs + g_volatile_seed);  /* Should generate STRICT_LOW_PART */
        
        /* Pattern 2: Char assignment that promotes */
        char c = char_array[i];
        int promoted = c * 2;  /* Promote to int */
        char_array[i] = (char)(promised & 0xFF);  /* Write back only low 8 bits */
        
        /* Pattern 3: In-register partial update */
        short s = short_array[i];
        int temp = s * 3 + i;
        short_array[i] = (short)temp;  /* Only low 16 bits written back */
        
        /* Pattern 4: Mixed-size operations */
        int accumulator = sum;
        accumulator += short_array[i];  /* short promoted to int */
        short_array[i] = (short)(accumulator & 0xFFFF);  /* STRICT_LOW_PART store */
        
        sum = accumulator;
    }
    
    /* Use inline assembly to force partial register updates */
    asm volatile (
        "movw %1, %%ax\n\t"
        "addw $1, %%ax\n\t"
        "movw %%ax, %0"
        : "=m" (short_array[0])
        : "m" (short_array[0])
        : "ax"
    );
    
    return sum + short_array[0];
}

/* ========== SUBREG patterns ========== */
union TypePun {
    uint32_t full;
    struct {
        uint16_t low;
        uint16_t high;
    } halves;
    uint8_t bytes[4];
};

int test_subreg(union TypePun *data, int count) {
    int sum = 0;
    
    for (int i = 0; i < count; i++) {
        /* Pattern 1: Union type-punning */
        uint16_t low_half = data[i].halves.low;  /* Should generate SUBREG */
        uint16_t high_half = data[i].halves.high;
        
        /* Pattern 2: Byte extraction through union */
        uint8_t byte0 = data[i].bytes[0];
        uint8_t byte2 = data[i].bytes[2];
        
        /* Pattern 3: Reconstruct with different sub-parts */
        uint32_t reconstructed = 
            (uint32_t)data[i].bytes[3] << 24 |
            (uint32_t)data[i].bytes[1] << 16 |
            (uint32_t)low_half;
        
        /* Pattern 4: Cast between different integer sizes */
        uint32_t temp = data[i].full + g_volatile_seed;
        uint16_t truncated = (uint16_t)temp;  /* SUBREG for truncation */
        
        /* Pattern 5: Mixed-size arithmetic */
        sum += (int)low_half + (int)high_half + byte0 + byte2 + truncated;
        
        /* Update through union */
        data[i].halves.high = (uint16_t)(sum & 0xFFFF);
    }
    
    return sum;
}

/* ========== Complex memory patterns ========== */
struct ComplexData {
    int base;
    struct BitFieldStruct bfs;
    union TypePun pun;
    short shorts[4];
    char chars[8];
};

int test_complex_memory(struct ComplexData *data, int index) {
    int result = 0;
    
    /* Complex addressing mode with bit-field */
    result += data[index].bfs.field1;
    result += data[index].bfs.field2 << 3;
    
    /* SUBREG access through union in memory */
    result += data[index].pun.halves.low;
    
    /* STRICT_LOW_PART through pointer to member */
    volatile short *vs = &data[index].shorts[2];
    *vs = (short)(result & 0xFFFF);
    
    /* Array access with index calculation */
    for (int i = 0; i < 4; i++) {
        data[index].shorts[i] = (short)(data[index].shorts[i] + g_volatile_seed);
        result += data[index].shorts[i];
    }
    
    /* Byte access through char array */
    for (int i = 0; i < 8; i++) {
        data[index].chars[i] = (char)(data[index].chars[i] + i);
        result += data[index].chars[i];
    }
    
    return result;
}

/* ========== Main test driver ========== */
int main(int argc, char **argv) {
    int iterations = 100;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    /* Allocate and initialize test data */
    struct BitFieldStruct *bfs_array = 
        (struct BitFieldStruct*)calloc(iterations, sizeof(struct BitFieldStruct));
    short *short_array = (short*)calloc(iterations * 2, sizeof(short));
    char *char_array = (char*)calloc(iterations * 4, sizeof(char));
    union TypePun *pun_array = (union TypePun*)calloc(iterations, sizeof(union TypePun));
    struct ComplexData *complex_array = 
        (struct ComplexData*)calloc(iterations / 10 + 1, sizeof(struct ComplexData));
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < iterations; i++) {
        /* Bit-field struct */
        bfs_array[i].field1 = (i * 3) & 0x1F;
        bfs_array[i].field2 = (i * 5) & 0x7F;
        bfs_array[i].field3 = (i * 7) & 0x0F;
        bfs_array[i].field4 = (i * 11) & 0xFFFF;
        
        /* Short and char arrays */
        short_array[i] = (short)(i * 13);
        char_array[i] = (char)(i * 17);
        
        /* Union type-punning data */
        pun_array[i].full = i * 19;
        pun_array[i].halves.low = (uint16_t)(i * 23);
        pun_array[i].halves.high = (uint16_t)(i * 29);
        
        /* Complex data (every 10th iteration) */
        if (i % 10 == 0) {
            int idx = i / 10;
            complex_array[idx].base = i;
            complex_array[idx].bfs = bfs_array[i];
            complex_array[idx].pun = pun_array[i];
            for (int j = 0; j < 4; j++) {
                complex_array[idx].shorts[j] = (short)(i * (j + 2));
            }
            for (int j = 0; j < 8; j++) {
                complex_array[idx].chars[j] = (char)(i * (j + 3));
            }
        }
    }
    
    /* Run all tests */
    int total = 0;
    
    total += test_zero_extract(bfs_array, iterations);
    total += test_strict_low_part(short_array, char_array, iterations);
    total += test_subreg(pun_array, iterations);
    
    /* Test complex memory patterns */
    for (int i = 0; i < iterations / 10 + 1; i++) {
        total += test_complex_memory(complex_array, i);
    }
    
    /* Add some volatile reads to prevent optimization */
    total += g_volatile_seed;
    
    /* Cleanup */
    free(bfs_array);
    free(short_array);
    free(char_array);
    free(pun_array);
    free(complex_array);
    
    printf("Result: %d\n", total);
    return total & 0xFF;  /* Return only low byte to avoid large exit codes */
}
