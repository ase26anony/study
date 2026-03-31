/* This program is designed to trigger specific RTL patterns in GCC's
   resource tracking pass during optimization. It creates operations
   that generate ZERO_EXTRACT, STRICT_LOW_PART, SUBREG, and complex
   MEM expressions in the RTL representation. */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Volatile variables to prevent optimization and create unpredictable
   control flow */
volatile int v_flag1 = 1;
volatile int v_flag2 = 0;
volatile int v_counter = 0;

/* Global variables for memory operations */
volatile unsigned int global_bitfield = 0xDEADBEEF;
int global_array[256];
struct ComplexStruct {
    int32_t full;
    int16_t parts[4];
    unsigned int flags : 4;
    unsigned int mode : 3;
} global_struct;

/* 1. ZERO_EXTRACT patterns - bit-field operations */
int zero_extract_pattern_1(volatile unsigned int *p) {
    /* This should generate ZERO_EXTRACT for bitfield access */
    return (*p >> 8) & 0xFF;  /* Extract bits 8-15 */
}

int zero_extract_pattern_2(void) {
    /* Using bit-field structure member */
    struct BitField {
        unsigned int low : 4;
        unsigned int mid : 8;
        unsigned int high : 20;
    };
    
    volatile struct BitField bf;
    bf.low = 5;
    bf.mid = 0xAB;
    bf.high = 0x12345;
    
    /* Taking address and accessing should create ZERO_EXTRACT */
    unsigned int *ptr = (unsigned int*)&bf;
    return (*ptr >> 4) & 0xFF;  /* Access mid field */
}

/* 2. STRICT_LOW_PART patterns - partial register writes */
void strict_low_part_pattern_1(volatile uint32_t *p, uint8_t value) {
    /* Writing only low byte of a 32-bit value */
    *p = (*p & ~0xFF) | value;  /* Only modify low 8 bits */
}

void strict_low_part_pattern_2(void) {
    /* Cast and assignment to create partial write */
    volatile int32_t x = 0x12345678;
    *(int16_t*)&x = 0xABCD;  /* Write only low 16 bits */
    
    /* Another variation with char */
    volatile int32_t y = 0x87654321;
    *(char*)&y = 0x42;  /* Write only low 8 bits */
}

/* 3. SUBREG patterns - mixed-size type access */
int subreg_pattern_1(void) {
    /* Union for type punning - creates SUBREG accesses */
    union MixedTypes {
        int64_t big;
        int32_t medium[2];
        int16_t small[4];
        int8_t tiny[8];
    };
    
    volatile union MixedTypes u;
    u.big = 0x1122334455667788LL;
    
    /* Access different sized parts */
    u.small[1] = 0xABCD;      /* SUBREG for 16-bit access to 64-bit */
    u.medium[0] = 0x88776655; /* SUBREG for 32-bit access */
    
    return u.tiny[3] + u.small[2];
}

int subreg_pattern_2(volatile int64_t *p) {
    /* Pointer casting between different sizes */
    int32_t low_part = *(int32_t*)p;          /* Get low 32 bits */
    int16_t high_part = *((int16_t*)p + 2);   /* Get bits 32-47 */
    
    return low_part + high_part;
}

/* 4. Complex MEM patterns - non-trivial addressing */
int complex_mem_pattern_1(int *base, int idx1, int idx2, int stride) {
    /* Complex addressing with multiple computations */
    return base[(idx1 * 3 + idx2 * 7) % stride];
}

int complex_mem_pattern_2(struct ComplexStruct *s, int index) {
    /* Mixed struct and array access with computation */
    int result = s->parts[index % 4];
    result += s->full >> (index * 2);
    
    /* Complex addressing with struct offset */
    int *ptr = &s->parts[0];
    ptr += index & 3;
    result += *ptr;
    
    return result;
}

/* 5. Combined function with control flow */
int combined_patterns(int iterations) {
    int result = 0;
    volatile int temp;
    
    for (int i = 0; i < iterations; i++) {
        /* Control flow based on volatile to prevent optimization */
        if (v_flag1) {
            /* ZERO_EXTRACT pattern */
            result ^= zero_extract_pattern_1(&global_bitfield);
            
            /* Update volatile to affect control flow */
            v_counter++;
        }
        
        if (v_flag2 || (i % 3 == 0)) {
            /* STRICT_LOW_PART pattern */
            strict_low_part_pattern_1(&global_struct.full, i & 0xFF);
            
            /* SUBREG pattern */
            result += subreg_pattern_1();
        }
        
        /* Complex MEM pattern with array */
        result += complex_mem_pattern_1(global_array, i, v_counter, 256);
        
        /* More control flow variations */
        switch (i % 4) {
            case 0:
                temp = zero_extract_pattern_2();
                break;
            case 1:
                strict_low_part_pattern_2();
                break;
            case 2:
                temp = subreg_pattern_2((int64_t*)&global_struct);
                break;
            case 3:
                temp = complex_mem_pattern_2(&global_struct, i);
                break;
        }
        
        result += temp;
        
        /* Modify global for next iteration */
        global_array[i % 256] = result;
        global_bitfield = (global_bitfield << 1) | (result & 1);
    }
    
    return result;
}

/* Helper functions to increase pass activity */
void helper_function_1(void) {
    /* Focus on ZERO_EXTRACT and SUBREG */
    volatile uint32_t x = 0x89ABCDEF;
    uint16_t y = (x >> 12) & 0x0FFF;  /* ZERO_EXTRACT */
    
    union { uint32_t a; uint16_t b[2]; } u;
    u.a = x;
    u.b[0] = y;  /* SUBREG */
    
    global_struct.parts[0] = u.b[1];
}

void helper_function_2(void) {
    /* Focus on STRICT_LOW_PART and complex MEM */
    volatile int64_t big = 0x123456789ABCDEF0LL;
    
    /* STRICT_LOW_PART through pointer cast */
    *(int32_t*)&big = 0x87654321;
    
    /* Complex memory addressing */
    int *ptr = global_array;
    for (int i = 0; i < 16; i++) {
        ptr[i * (v_counter % 8 + 1)] = i;  /* Non-linear addressing */
    }
}

/* Main function with observable side effects */
int main(void) {
    /* Initialize globals */
    for (int i = 0; i < 256; i++) {
        global_array[i] = i * 3;
    }
    
    global_struct.full = 0xCAFEBABE;
    for (int i = 0; i < 4; i++) {
        global_struct.parts[i] = i * 0x1111;
    }
    global_struct.flags = 7;
    global_struct.mode = 3;
    
    /* Call pattern functions in non-trivial order */
    int result = 0;
    
    /* Loop with volatile-dependent iterations */
    for (int outer = 0; outer < 3; outer++) {
        v_flag1 = outer & 1;
        v_flag2 = (outer >> 1) & 1;
        
        /* Combined patterns */
        result += combined_patterns(10 + outer * 5);
        
        /* Individual helpers */
        helper_function_1();
        helper_function_2();
        
        /* More direct pattern usage */
        result ^= zero_extract_pattern_1(&global_bitfield);
        strict_low_part_pattern_1(&global_struct.full, result & 0xFF);
        result += subreg_pattern_1();
        result += complex_mem_pattern_2(&global_struct, outer);
    }
    
    /* Final computation to use all results */
    int final_result = 0;
    for (int i = 0; i < 256; i++) {
        final_result ^= global_array[i];
    }
    final_result += result;
    final_result += global_struct.full;
    final_result += global_struct.parts[0];
    
    /* Print to prevent optimization */
    printf("Result: %d\n", final_result);
    
    return final_result & 0xFF;
}
