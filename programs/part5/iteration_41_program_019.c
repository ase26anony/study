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

/* Function 1: Bit-field extraction from integer */
unsigned int test_zero_extract_int(unsigned int x, unsigned int y) {
    unsigned int sum = 0;
    
    /* Multiple bit-field extractions that should generate ZERO_EXTRACT */
    sum += (x >> 3) & 0x1F;        /* Extract 5 bits */
    sum += (y >> 8) & 0xFF;        /* Extract 8 bits */
    sum += (x >> 16) & 0x7;        /* Extract 3 bits */
    sum += (y >> 0) & 0xFFFF;      /* Extract 16 bits */
    
    /* Combined operations */
    sum += ((x & 0xFF00) >> 8) + ((y & 0xF0) >> 4);
    
    return sum;
}

/* Function 2: Bit-field structure operations */
unsigned int test_zero_extract_struct(struct BitFieldStruct *s, int iterations) {
    unsigned int sum = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Bit-field comparisons and assignments */
        if (s->field1 == 10) {
            sum += s->field2;
        }
        
        if (s->field3 > 2) {
            sum += s->field4;
        }
        
        /* Modify bit-fields */
        s->field1 = (s->field1 + 1) & 0x1F;
        s->field2 = (s->field2 * 2) & 0xFF;
        s->field3 = (s->field3 ^ 1) & 0x7;
        s->field4 = (s->field4 + g_volatile_seed) & 0xFFFF;
    }
    
    return sum;
}

/* ========== STRICT_LOW_PART patterns ========== */
/* Function 3: Partial register updates with small types */
unsigned int test_strict_low_part(short *arr, int size, char *chars) {
    unsigned int sum = 0;
    
    for (int i = 0; i < size; i++) {
        /* These should generate STRICT_LOW_PART for partial register writes */
        short temp_short = arr[i];
        char temp_char = chars[i];
        
        /* Promote to int, operate, then write back low part */
        int promoted = temp_short;
        promoted = (promoted * 3 + 7) & 0xFFFF;  /* Keep within 16 bits */
        arr[i] = promoted;                       /* Partial write back */
        
        /* Char operations */
        int char_promoted = temp_char;
        char_promoted = (char_promoted + g_volatile_seed) & 0xFF;
        chars[i] = char_promoted;                /* Partial write back */
        
        sum += arr[i] + chars[i];
    }
    
    /* Volatile pointer to force memory operations */
    volatile short *volatile_ptr = arr;
    for (int i = 0; i < 4; i++) {
        *volatile_ptr = *volatile_ptr + 1;       /* Partial volatile write */
        volatile_ptr++;
    }
    
    return sum;
}

/* Function 4: Inline assembly for partial register access */
unsigned int test_strict_low_part_asm(int x) {
    unsigned int result = 0;
    short s;
    char c;
    
    /* Use inline assembly to force partial register updates */
    asm volatile (
        "movw %1, %0\n\t"        /* 16-bit move */
        : "=r"(s)
        : "r"(x)
    );
    
    asm volatile (
        "movb %b1, %b0\n\t"      /* 8-bit move using byte register constraint */
        : "=q"(c)                /* "q" constraint for byte-addressable register */
        : "r"(x)
    );
    
    result = s + c;
    
    /* Additional partial write through pointer */
    volatile short *ptr = &s;
    *ptr = result & 0xFFFF;
    
    return result + s;
}

/* ========== SUBREG patterns ========== */
/* Function 5: Union type-punning for SUBREG generation */
unsigned int test_subreg_union(int x, int y) {
    union {
        uint32_t full;
        uint16_t halves[2];
        uint8_t bytes[4];
    } u;
    
    unsigned int sum = 0;
    
    /* Type-punning through union */
    u.full = x;
    sum += u.halves[0];          /* Should generate SUBREG */
    sum += u.halves[1];          /* Should generate SUBREG */
    sum += u.bytes[0] + u.bytes[2];  /* Multiple SUBREGs */
    
    /* Cast between different sizes */
    u.full = y;
    uint16_t low_half = (uint16_t)u.full;        /* SUBREG for truncation */
    uint8_t high_byte = (uint8_t)(u.full >> 16); /* Shift then SUBREG */
    
    sum += low_half + high_byte;
    
    /* Packed structure simulation */
    struct Packed {
        uint16_t a;
        uint16_t b;
    } __attribute__((packed));
    
    /* Force register packing and extraction */
    uint32_t packed = (x & 0xFFFF) | ((y & 0xFFFF) << 16);
    uint16_t extracted_a = packed & 0xFFFF;      /* Implicit SUBREG */
    uint16_t extracted_b = packed >> 16;         /* Shift then possible SUBREG */
    
    sum += extracted_a + extracted_b;
    
    return sum;
}

/* Function 6: Memory references with complex addressing */
unsigned int test_memory_refs(int *base_arr, short *short_arr, int index) {
    unsigned int sum = 0;
    
    /* Complex addressing modes */
    sum += base_arr[index * 2 + 1];              /* Scaled index */
    sum += *(base_arr + index + g_volatile_seed); /* Pointer arithmetic */
    
    /* SUBREG with memory */
    short_arr[index] = (short)(base_arr[index] & 0xFFFF); /* Truncation store */
    
    /* ZERO_EXTRACT from memory */
    int val = base_arr[index];
    sum += (val >> 8) & 0xFF;                    /* Extract from memory value */
    
    /* Multiple memory operations in loop */
    for (int i = 0; i < 8; i++) {
        /* STRICT_LOW_PART pattern with memory */
        short_arr[i] = (short)(short_arr[i] + base_arr[i % 4]);
        
        /* SUBREG pattern through union in memory */
        union {
            int i;
            short s[2];
        } *u_ptr = (union { int i; short s[2]; } *)&base_arr[i];
        sum += u_ptr->s[0];                      /* SUBREG from memory */
    }
    
    return sum;
}

/* ========== Main test driver ========== */
int main(int argc, char **argv) {
    unsigned int total_sum = 0;
    
    /* Initialize test data with some variability */
    int iterations = (argc > 1) ? atoi(argv[1]) : 100;
    if (iterations < 10) iterations = 10;
    if (iterations > 1000) iterations = 1000;
    
    /* Arrays for testing */
    int int_array[100];
    short short_array[100];
    char char_array[100];
    
    for (int i = 0; i < 100; i++) {
        int_array[i] = (i * 37 + g_volatile_seed) & 0xFFFFFFFF;
        short_array[i] = (i * 13 + g_volatile_seed) & 0xFFFF;
        char_array[i] = (i * 7 + g_volatile_seed) & 0xFF;
    }
    
    /* Bit-field structure */
    struct BitFieldStruct bf = {5, 100, 3, 50000};
    
    /* Run all tests multiple times to ensure execution */
    for (int run = 0; run < 3; run++) {
        /* Test ZERO_EXTRACT patterns */
        total_sum += test_zero_extract_int(int_array[run], int_array[run + 1]);
        total_sum += test_zero_extract_struct(&bf, iterations % 50);
        
        /* Test STRICT_LOW_PART patterns */
        total_sum += test_strict_low_part(short_array, 50, char_array);
        total_sum += test_strict_low_part_asm(int_array[run]);
        
        /* Test SUBREG patterns */
        total_sum += test_subreg_union(int_array[run * 2], int_array[run * 2 + 1]);
        
        /* Test memory references with complex patterns */
        total_sum += test_memory_refs(int_array, short_array, run * 10);
        
        /* Modify global volatile to prevent optimization */
        g_volatile_seed = (g_volatile_seed * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    
    /* Use the result to prevent dead code elimination */
    printf("Result checksum: %u\n", total_sum);
    
    /* Return non-zero if checksum looks reasonable (not 0 or trivial) */
    return (total_sum > 1000 && total_sum < 0xFFFFFFF) ? 0 : 1;
}
