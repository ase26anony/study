/* test_resource_patterns.c */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Prevent constant folding and dead code elimination */
volatile int g_volatile_seed = 42;

/* ========== ZERO_EXTRACT patterns ========== */

/* Bit-field structure for ZERO_EXTRACT */
struct bitfield_struct {
    unsigned int field1 : 5;
    unsigned int field2 : 7;
    unsigned int field3 : 3;
    unsigned int field4 : 17;
};

/* Function to test ZERO_EXTRACT patterns */
unsigned int test_zero_extract(unsigned int *array, int size) {
    unsigned int sum = 0;
    struct bitfield_struct bf;
    
    /* 1. Bit-field extraction from structure */
    for (int i = 0; i < size; i++) {
        /* Force compiler to use bit-field operations */
        bf.field1 = (array[i] >> 0) & 0x1F;
        bf.field2 = (array[i] >> 5) & 0x7F;
        bf.field3 = (array[i] >> 12) & 0x07;
        bf.field4 = (array[i] >> 15) & 0x1FFFF;
        
        /* Use bit-field comparisons (triggers ZERO_EXTRACT) */
        if (bf.field1 == (g_volatile_seed & 0x1F)) {
            sum += bf.field1;
        }
        if (bf.field2 > 10) {
            sum += bf.field2;
        }
        if (bf.field3 != 0) {
            sum += bf.field3;
        }
        if (bf.field4 < 0x8000) {
            sum += bf.field4 & 0xFF;
        }
    }
    
    /* 2. Explicit bit-field extraction with mask and shift */
    for (int i = 0; i < size; i++) {
        /* These should generate ZERO_EXTRACT RTL */
        unsigned int low_byte = (array[i] & 0xFF);
        unsigned int high_byte = (array[i] >> 8) & 0xFF;
        unsigned int middle_bits = (array[i] >> 4) & 0x0FFF;
        
        sum += low_byte + high_byte + (middle_bits & 0xFF);
    }
    
    /* 3. Complex bit-field arithmetic */
    for (int i = 0; i < size; i++) {
        /* Multiple extractions in one expression */
        unsigned int val = ((array[i] & 0xF0F0F0F0) >> 4) | 
                          ((array[i] & 0x0F0F0F0F) << 4);
        sum += (val & 0xFFFF) + ((val >> 16) & 0xFFFF);
    }
    
    return sum;
}

/* ========== STRICT_LOW_PART patterns ========== */

/* Function to test STRICT_LOW_PART patterns */
unsigned int test_strict_low_part(short *short_array, char *char_array, int size) {
    unsigned int sum = 0;
    
    /* 1. Partial register updates with char/short */
    for (int i = 0; i < size; i++) {
        /* These assignments should generate STRICT_LOW_PART */
        char c = char_array[i];
        short s = short_array[i];
        
        /* Promote to int, modify, then assign back */
        int temp_int = c;
        temp_int = (temp_int * 3 + 7) & 0xFF;
        c = temp_int;  /* STRICT_LOW_PART for char */
        
        temp_int = s;
        temp_int = (temp_int * 5 - 11) & 0xFFFF;
        s = temp_int;  /* STRICT_LOW_PART for short */
        
        sum += c + s;
    }
    
    /* 2. Volatile pointer writes */
    volatile short *volatile_short_ptr = (volatile short *)short_array;
    volatile char *volatile_char_ptr = (volatile char *)char_array;
    
    for (int i = 0; i < size && i < 10; i++) {
        /* Volatile writes to partial registers */
        volatile_short_ptr[i] = (short)(sum + i);
        volatile_char_ptr[i] = (char)(sum + i * 3);
    }
    
    /* 3. Inline assembly for byte register operations */
    for (int i = 0; i < size && i < 5; i++) {
        unsigned char byte_val;
        unsigned int int_val = short_array[i] + char_array[i];
        
        /* Assembly that operates on byte register */
        __asm__ volatile (
            "movb %1, %0\n\t"
            "addb $1, %0"
            : "=q" (byte_val)   /* "q" constraint selects byte register */
            : "r" (int_val)
            : "cc"
        );
        
        sum += byte_val;
    }
    
    /* 4. Function arguments with small types */
    for (int i = 0; i < size; i++) {
        /* Force partial register updates through function calls */
        short local_short = process_short(short_array[i], char_array[i]);
        char local_char = process_char(short_array[i] >> 8, char_array[i]);
        sum += local_short + local_char;
    }
    
    return sum;
}

/* Helper functions for STRICT_LOW_PART testing */
short process_short(short a, char b) {
    /* Mix operations to force register promotion and partial write-back */
    int temp = a + b;
    temp = (temp * 3) & 0xFFFF;
    return (short)temp;  /* Should generate STRICT_LOW_PART */
}

char process_char(short a, char b) {
    int temp = (a & 0xFF) + b;
    temp = (temp * 7) & 0xFF;
    return (char)temp;   /* Should generate STRICT_LOW_PART */
}

/* ========== SUBREG patterns ========== */

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

/* Function to test SUBREG patterns */
unsigned int test_subreg(uint32_t *data, int size) {
    unsigned int sum = 0;
    union type_pun pun;
    
    /* 1. Union-based type-punning */
    for (int i = 0; i < size; i++) {
        pun.full = data[i];
        
        /* Access different views of the same register */
        sum += pun.halves[0];      /* SUBREG for low 16 bits */
        sum += pun.halves[1];      /* SUBREG for high 16 bits */
        sum += pun.bytes[0] + pun.bytes[3];  /* SUBREG for byte access */
        
        /* Modify through one view, read through another */
        pun.parts.low = (pun.parts.low * 3) & 0xFFFF;
        sum += pun.full & 0xFF;
    }
    
    /* 2. Casting between different integer sizes */
    for (int i = 0; i < size; i++) {
        uint32_t val32 = data[i];
        uint16_t val16 = (uint16_t)(val32 & 0xFFFF);      /* SUBREG */
        uint8_t val8 = (uint8_t)((val32 >> 16) & 0xFF);   /* SUBREG */
        
        /* Chain of casts */
        int32_t signed32 = (int32_t)val32;
        int16_t signed16 = (int16_t)val16;                /* SUBREG */
        int8_t signed8 = (int8_t)val8;                    /* SUBREG */
        
        sum += signed32 + signed16 + signed8;
    }
    
    /* 3. Packed structure access */
    struct packed_data {
        uint16_t a;
        uint16_t b;
        uint32_t c;
    } __attribute__((packed));
    
    /* Create packed data in memory */
    char buffer[sizeof(struct packed_data) * 4];
    struct packed_data *packed = (struct packed_data *)buffer;
    
    for (int i = 0; i < 4 && i < size; i++) {
        packed[i].a = data[i] & 0xFFFF;
        packed[i].b = (data[i] >> 16) & 0xFFFF;
        packed[i].c = data[i] * 3;
        
        /* Access packed fields - may generate SUBREG for unaligned access */
        sum += packed[i].a + packed[i].b + (packed[i].c & 0xFFFF);
    }
    
    /* 4. SIMD-like operations using unions */
    for (int i = 0; i + 1 < size; i += 2) {
        union {
            uint32_t words[2];
            uint64_t dword;
        } simd;
        
        simd.words[0] = data[i];
        simd.words[1] = data[i + 1];
        
        /* Extract and manipulate parts */
        uint32_t low_part = (uint32_t)(simd.dword & 0xFFFFFFFF);    /* SUBREG */
        uint32_t high_part = (uint32_t)(simd.dword >> 32);          /* SUBREG */
        
        sum += low_part + high_part;
    }
    
    return sum;
}

/* ========== Complex memory references ========== */

/* Function combining all patterns with complex addressing */
unsigned int test_complex_memory(int *base_array, int index, int offset) {
    unsigned int sum = 0;
    
    /* Complex addressing modes with bit-field operations */
    for (int i = 0; i < 8; i++) {
        /* Array access with index and offset */
        int *ptr = &base_array[index + i * offset];
        
        /* Bit-field extract from memory location */
        unsigned int val = *ptr;
        unsigned int field = (val >> (i * 3)) & 0x7;  /* ZERO_EXTRACT */
        
        /* Partial write to memory */
        short *short_ptr = (short *)ptr;
        short old_short = short_ptr[1];
        short new_short = (old_short & 0xFF00) | (field << 4);  /* STRICT_LOW_PART */
        short_ptr[1] = new_short;
        
        /* Type-punning through union */
        union type_pun pun;
        pun.full = *ptr;
        pun.halves[0] = pun.halves[0] + field;  /* SUBREG */
        
        *ptr = pun.full;
        sum += *ptr;
    }
    
    return sum;
}

/* ========== Main test driver ========== */

int main(int argc, char **argv) {
    /* Initialize test data with volatile to prevent constant folding */
    const int DATA_SIZE = 100;
    unsigned int *int_data = (unsigned int *)malloc(DATA_SIZE * sizeof(unsigned int));
    short *short_data = (short *)malloc(DATA_SIZE * sizeof(short));
    char *char_data = (char *)malloc(DATA_SIZE);
    int *base_array = (int *)malloc(256 * sizeof(int));
    
    /* Seed with pseudo-random but volatile data */
    for (int i = 0; i < DATA_SIZE; i++) {
        int_data[i] = (i * 1103515245U + 12345U) ^ g_volatile_seed;
        short_data[i] = (short)(int_data[i] & 0xFFFF);
        char_data[i] = (char)(int_data[i] & 0xFF);
    }
    
    for (int i = 0; i < 256; i++) {
        base_array[i] = (i * 97 + 53) ^ g_volatile_seed;
    }
    
    unsigned int total_sum = 0;
    
    /* Test ZERO_EXTRACT patterns */
    total_sum += test_zero_extract(int_data, DATA_SIZE);
    
    /* Test STRICT_LOW_PART patterns */
    total_sum += test_strict_low_part(short_data, char_data, DATA_SIZE);
    
    /* Test SUBREG patterns */
    total_sum += test_subreg(int_data, DATA_SIZE);
    
    /* Test complex memory patterns */
    total_sum += test_complex_memory(base_array, 10, 7);
    
    /* Use result to prevent dead code elimination */
    printf("Result checksum: %u\n", total_sum);
    
    free(int_data);
    free(short_data);
    free(char_data);
    free(base_array);
    
    return (total_sum & 0xFF);  /* Return non-constant result */
}
