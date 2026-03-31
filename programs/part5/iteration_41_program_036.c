/* test_resource_patterns.c */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Prevent constant folding */
volatile int g_volatile_seed = 42;

/* ========== ZERO_EXTRACT Patterns ========== */

/* Bit-field structure for ZERO_EXTRACT */
struct BitFieldStruct {
    unsigned int field1 : 5;
    unsigned int field2 : 7;
    unsigned int field3 : 10;
    unsigned int field4 : 10;
};

/* Function to trigger ZERO_EXTRACT RTL */
unsigned int test_zero_extract(struct BitFieldStruct *bfs, unsigned int *arr, int size) {
    unsigned int sum = 0;
    
    /* Bit-field assignments and comparisons */
    for (int i = 0; i < size; i++) {
        /* Bit-field assignment - likely generates ZERO_EXTRACT */
        bfs->field1 = (arr[i] & 0x1F);  /* 5 bits */
        bfs->field2 = (arr[i] >> 5) & 0x7F;  /* 7 bits */
        
        /* Bit-field comparison - may generate ZERO_EXTRACT */
        if (bfs->field1 == (unsigned int)(g_volatile_seed & 0x1F)) {
            bfs->field3 = (arr[i] >> 12) & 0x3FF;  /* 10 bits */
        }
        
        /* Explicit bit-field extraction */
        unsigned int extracted = (arr[i] & 0xFF00) >> 8;
        sum += extracted + bfs->field1 + bfs->field2 + bfs->field3;
        
        /* Complex bit-field operation */
        bfs->field4 = ((arr[i] >> 3) & 0x3FF) + (extracted & 0x3F);
    }
    
    return sum;
}

/* Manual bit-field extraction that should generate ZERO_EXTRACT */
unsigned int extract_bit_field(unsigned int value, int pos, int width) {
    /* This should generate ZERO_EXTRACT RTL */
    return (value >> pos) & ((1U << width) - 1);
}

/* ========== STRICT_LOW_PART Patterns ========== */

/* Function to trigger STRICT_LOW_PART RTL */
unsigned int test_strict_low_part(short *short_arr, char *char_arr, int size) {
    unsigned int sum = 0;
    
    for (int i = 0; i < size; i++) {
        /* Partial register updates - should generate STRICT_LOW_PART */
        short s = short_arr[i];
        char c = char_arr[i];
        
        /* Operations that promote then write back partial values */
        int temp_int = s + g_volatile_seed;
        short_arr[i] = (short)(temp_int & 0xFFFF);  /* Partial write */
        
        /* Char operations with promotion */
        char temp_char = c + (char)(g_volatile_seed & 0xFF);
        char_arr[i] = temp_char;  /* Partial write to byte */
        
        /* Volatile pointer to force partial store */
        volatile short *volatile_ptr = &short_arr[i];
        *volatile_ptr = (short)(temp_int ^ 0x1234);
        
        sum += short_arr[i] + (unsigned char)char_arr[i];
    }
    
    /* Inline assembly for partial register access */
    unsigned int asm_result = 0;
    __asm__ volatile (
        "movw %1, %%ax\n\t"
        "addw $100, %%ax\n\t"
        "movw %%ax, %0"
        : "=m" (short_arr[size/2])
        : "r" ((unsigned short)g_volatile_seed)
        : "ax"
    );
    
    return sum + short_arr[size/2];
}

/* ========== SUBREG Patterns ========== */

/* Union for type-punning to generate SUBREG */
union TypePun {
    uint32_t full;
    uint16_t halves[2];
    uint8_t bytes[4];
    struct {
        uint16_t low;
        uint16_t high;
    } parts;
};

/* Function to trigger SUBREG RTL */
unsigned int test_subreg(union TypePun *data, int count) {
    unsigned int sum = 0;
    
    for (int i = 0; i < count; i++) {
        /* Type punning through union - should generate SUBREG */
        data[i].full = i * 0x01020304U + g_volatile_seed;
        
        /* Access sub-parts - generates SUBREG */
        sum += data[i].halves[0] + data[i].halves[1];
        sum += data[i].bytes[1] * 2;
        
        /* Cast between different sizes */
        uint32_t temp = data[i].full;
        uint16_t low_half = (uint16_t)(temp & 0xFFFF);  /* SUBREG */
        uint16_t high_half = (uint16_t)(temp >> 16);    /* SUBREG */
        
        /* Recombine with SUBREG operations */
        data[i].parts.low = high_half;  /* Swap halves */
        data[i].parts.high = low_half;
        
        sum += data[i].full;
    }
    
    /* SIMD-like operations using type punning */
    for (int i = 0; i < count - 1; i++) {
        /* Pack two 16-bit values into 32-bit */
        uint32_t packed = (data[i].halves[0] << 16) | data[i+1].halves[1];
        /* Unpack with SUBREG accesses */
        data[i].full = packed;
        sum += data[i].bytes[0] + data[i].bytes[2];
    }
    
    return sum;
}

/* ========== Complex Memory References ========== */

/* Function combining multiple patterns with memory references */
unsigned int test_complex_memory(int *base_array, int index, int offset) {
    unsigned int result = 0;
    
    /* Complex addressing mode */
    int *ptr = base_array + index;
    
    /* Bit-field extraction from memory */
    unsigned int mem_val = *ptr;
    result = extract_bit_field(mem_val, offset & 7, 8);
    
    /* Type punning with memory access */
    union TypePun *pun_ptr = (union TypePun *)ptr;
    result += pun_ptr->halves[0];
    
    /* Partial store to memory */
    short *short_ptr = (short *)ptr;
    *short_ptr = (short)(result & 0xFFFF);  /* STRICT_LOW_PART */
    
    /* Another memory access with different type */
    char *char_ptr = (char *)ptr + 2;
    *char_ptr = (char)(result >> 8);
    
    return result + *ptr;
}

/* ========== Main Test Driver ========== */

int main(int argc, char **argv) {
    /* Use argc to prevent constant folding */
    int size = (argc > 1) ? atoi(argv[1]) : 100;
    if (size <= 0) size = 100;
    if (size > 1000) size = 1000;
    
    /* Initialize test data */
    struct BitFieldStruct bfs = {0};
    unsigned int *int_array = (unsigned int *)malloc(size * sizeof(unsigned int));
    short *short_array = (short *)malloc(size * sizeof(short));
    char *char_array = (char *)malloc(size * sizeof(char));
    union TypePun *union_array = (union TypePun *)malloc(size * sizeof(union TypePun));
    
    /* Fill with pseudo-random data using volatile seed */
    for (int i = 0; i < size; i++) {
        int_array[i] = (i * 1103515245U + 12345) ^ g_volatile_seed;
        short_array[i] = (short)(int_array[i] & 0xFFFF);
        char_array[i] = (char)(int_array[i] & 0xFF);
    }
    
    unsigned int total_sum = 0;
    
    /* Test ZERO_EXTRACT patterns */
    total_sum += test_zero_extract(&bfs, int_array, size);
    
    /* Test STRICT_LOW_PART patterns */
    total_sum += test_strict_low_part(short_array, char_array, size);
    
    /* Test SUBREG patterns */
    total_sum += test_subreg(union_array, size);
    
    /* Test complex memory patterns */
    for (int i = 0; i < size; i += 4) {
        total_sum += test_complex_memory((int *)int_array, i, i & 7);
    }
    
    /* Additional mixed pattern tests */
    for (int i = 0; i < size; i++) {
        /* Mix bit-field and type-punning */
        unsigned int val = int_array[i];
        
        /* ZERO_EXTRACT pattern */
        unsigned int field = (val >> (i % 16)) & ((1 << 8) - 1);
        
        /* SUBREG pattern through union */
        union TypePun temp;
        temp.full = val;
        temp.halves[0] = (uint16_t)field;  /* Partial write */
        
        /* STRICT_LOW_PART pattern */
        char_array[i] = (char)(temp.bytes[0] + field);
        
        total_sum += temp.full + char_array[i];
    }
    
    /* Prevent dead code elimination */
    if (total_sum == 0) {
        printf("Unexpected zero result\n");
    }
    
    /* Cleanup */
    free(int_array);
    free(short_array);
    free(char_array);
    free(union_array);
    
    /* Return checksum to prevent optimization */
    return (int)(total_sum & 0x7FFFFFFF);
}
