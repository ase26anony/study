/* Test program to trigger specific RTL patterns in GCC's resource tracking pass */
#include <stdio.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
volatile int g_volatile_flag = 1;
volatile unsigned int g_volatile_counter = 0;

/* Global variables for memory operations */
unsigned int g_bitfield_var = 0x12345678;
long long g_large_var = 0x1122334455667788LL;
int g_array[256];
int g_result = 0;

/* Struct with bit-fields for ZERO_EXTRACT */
struct BitFieldStruct {
    unsigned int low8 : 8;
    unsigned int mid8 : 8;
    unsigned int high16 : 16;
};

/* Union for SUBREG patterns */
union MixedSizeUnion {
    int32_t full;
    int16_t halves[2];
    int8_t bytes[4];
};

/* Struct for complex memory addressing */
struct ComplexMemStruct {
    int data[64];
    int padding[16];
    int more_data[32];
};

/* 1. Generate ZERO_EXTRACT patterns */
int extract_bitfield_patterns(void) {
    struct BitFieldStruct bf;
    unsigned int temp;
    
    /* Direct bit-field access - may generate ZERO_EXTRACT */
    bf.low8 = 0xAB;
    bf.mid8 = 0xCD;
    bf.high16 = 0xEF01;
    
    /* Manual bit extraction that might generate ZERO_EXTRACT */
    temp = g_bitfield_var;
    
    /* Multiple extraction patterns */
    int result = 0;
    result += (temp >> 0) & 0xFF;   /* Low 8 bits */
    result += (temp >> 8) & 0xFF;   /* Next 8 bits */
    result += (temp >> 16) & 0x0F;  /* Just 4 bits */
    result += (temp >> 20) & 0x0F;  /* Another 4 bits */
    
    /* Conditional extraction based on volatile */
    if (g_volatile_flag) {
        result += (temp >> 24) & 0xFF;
    }
    
    return result;
}

/* 2. Generate STRICT_LOW_PART patterns */
void strict_low_part_patterns(int value) {
    volatile unsigned int *p = &g_bitfield_var;
    unsigned char byte_val = (unsigned char)value;
    
    /* Writing only low part of a larger variable */
    *p = (*p & ~0xFF) | byte_val;
    
    /* Using different sizes */
    uint32_t word = 0x87654321;
    uint16_t half = 0xABCD;
    
    /* This assignment might generate STRICT_LOW_PART */
    *(uint16_t*)&word = half;
    
    /* Another pattern with masking */
    word = (word & 0xFFFF0000) | (value & 0xFFFF);
    
    /* Complex conditional low-part write */
    if (g_volatile_counter % 2) {
        *(uint8_t*)&word = (uint8_t)(value >> 8);
    }
}

/* 3. Generate SUBREG patterns */
int subreg_patterns(void) {
    union MixedSizeUnion u;
    int result = 0;
    
    u.full = 0x89ABCDEF;
    
    /* Access different parts through smaller types */
    result += u.halves[0];  /* Low 16 bits */
    result += u.halves[1];  /* High 16 bits */
    
    /* Byte access */
    for (int i = 0; i < 4; i++) {
        result += u.bytes[i];
    }
    
    /* Pointer casting between different sizes */
    long long ll = g_large_var;
    int *ip = (int*)&ll;
    result += ip[0];
    result += ip[1];
    
    /* Mixed operations */
    int16_t s1 = 1000;
    int16_t s2 = 2000;
    int32_t sum = s1 + s2;  /* May involve SUBREG for promotion */
    
    return result + sum;
}

/* 4. Generate complex MEM_P patterns */
int complex_mem_patterns(struct ComplexMemStruct *s, int idx1, int idx2) {
    int result = 0;
    
    /* Complex addressing modes */
    result += s->data[idx1 * 2 + idx2];
    result += s->data[idx1 + idx2 * 3];
    
    /* Pointer arithmetic with multiple indices */
    int *ptr = &s->data[0];
    result += ptr[idx1 * 4 - idx2];
    
    /* Nested array access */
    result += s->more_data[idx1 % 32];
    
    /* Address computation with constants */
    result += (&s->padding[0])[idx2 % 8];
    
    return result;
}

/* 5. Combined function with control flow */
int combined_patterns(int iterations) {
    struct ComplexMemStruct cmem;
    union MixedSizeUnion u;
    int total = 0;
    
    /* Initialize memory */
    for (int i = 0; i < 64; i++) {
        cmem.data[i] = i * 3;
    }
    for (int i = 0; i < 32; i++) {
        cmem.more_data[i] = i * 5;
    }
    
    /* Loop with mixed operations */
    for (int i = 0; i < iterations; i++) {
        g_volatile_counter++;
        
        /* Conditional execution based on volatile */
        if (g_volatile_flag) {
            /* ZERO_EXTRACT pattern */
            total += extract_bitfield_patterns();
            
            /* Update global with low-part write */
            strict_low_part_patterns(i);
        }
        
        /* SUBREG pattern on every other iteration */
        if (i % 2 == 0) {
            total += subreg_patterns();
        }
        
        /* Complex memory access */
        total += complex_mem_patterns(&cmem, i % 16, (i * 7) % 16);
        
        /* Array access with complex index */
        g_array[i % 256] = total & 0xFF;
    }
    
    return total;
}

/* Main function to drive everything */
int main(void) {
    int final_result = 0;
    
    /* Initialize array */
    for (int i = 0; i < 256; i++) {
        g_array[i] = i;
    }
    
    /* Run combined patterns multiple times */
    for (int run = 0; run < 3; run++) {
        g_volatile_flag = (run % 2) ? 1 : 0;
        
        /* Call the combined function */
        int run_result = combined_patterns(10 + run * 5);
        
        /* Mix in some direct operations */
        strict_low_part_patterns(run_result);
        final_result += run_result;
        
        /* Access global through pointer with offset */
        final_result += g_array[run_result % 256];
    }
    
    /* Final mixed operations */
    union MixedSizeUnion final_u;
    final_u.full = final_result;
    final_result += final_u.halves[0] - final_u.halves[1];
    
    /* Complex memory access as final step */
    struct ComplexMemStruct final_cmem;
    final_result += complex_mem_patterns(&final_cmem, final_result % 8, (final_result >> 4) % 8);
    
    /* Ensure result is used */
    printf("Final result: %d\n", final_result);
    
    return final_result & 0xFF;
}
