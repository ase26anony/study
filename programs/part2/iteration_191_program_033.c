/* This program is designed to trigger specific RTL patterns in GCC's
   resource tracking pass (resource.cc lines 282-290). It creates
   operations that generate ZERO_EXTRACT, STRICT_LOW_PART, SUBREG,
   and complex MEM expressions in the RTL representation. */

#include <stdio.h>
#include <stdint.h>

/* Volatile variables to prevent optimization and create control flow */
volatile int cond1 = 1;
volatile int cond2 = 0;
volatile int cond3 = 1;

/* Global variables for memory operations */
volatile unsigned int global_bitfield = 0xDEADBEEF;
volatile unsigned int global_value = 0x12345678;
int global_array[256];
struct ComplexStruct {
    int32_t full;
    int16_t parts[4];
    unsigned int flags: 8;
    unsigned int status: 4;
} global_struct;

/* 1. ZERO_EXTRACT patterns - bit-field operations */
unsigned int zero_extract_pattern_1(void) {
    /* Direct bit-field extract using shift and mask */
    return (global_bitfield >> 8) & 0xFF;
}

unsigned int zero_extract_pattern_2(void) {
    /* Struct bit-field access */
    struct S {
        unsigned int field1: 8;
        unsigned int field2: 16;
        unsigned int field3: 8;
    };
    static volatile struct S s = {0xAA, 0xBBBB, 0xCC};
    
    /* Taking address and accessing bit-field */
    unsigned int val = s.field2;
    return val;
}

/* 2. STRICT_LOW_PART patterns - partial register writes */
void strict_low_part_pattern_1(volatile unsigned int *p, unsigned char v) {
    /* Writing only low byte while preserving high bytes */
    *p = (*p & ~0xFF) | v;
}

void strict_low_part_pattern_2(void) {
    /* Cast to smaller type assignment */
    int32_t x = 0x11223344;
    *(int16_t*)&x = 0x5566;  /* Only affects low 16 bits */
    
    /* Use volatile to prevent optimization */
    volatile int32_t *vp = &x;
    *vp = *vp;  /* Force use */
}

/* 3. SUBREG patterns - mixed-size type access */
int32_t subreg_pattern_1(void) {
    /* Union for type aliasing */
    union U {
        int32_t i;
        int16_t s[2];
        int8_t b[4];
    };
    static union U u = {0x12345678};
    
    /* Access through different sized views */
    u.s[0] = 0x9ABC;  /* SUBREG of the union */
    return u.i;
}

int32_t subreg_pattern_2(void) {
    /* Pointer casting between different sizes */
    long long ll = 0x1122334455667788LL;
    int32_t i = *(int32_t*)&ll;  /* Extracting low part */
    
    /* Another SUBREG through pointer */
    int16_t s = *(int16_t*)((char*)&ll + 2);
    return i + s;
}

/* 4. Complex MEM patterns - non-trivial addressing */
int complex_mem_pattern_1(int *base, int idx1, int idx2) {
    /* Complex addressing with arithmetic */
    return base[idx1 + idx2 * 4];
}

int complex_mem_pattern_2(struct ComplexStruct *cs, int index) {
    /* Multiple struct field accesses with addressing */
    int val = cs->parts[index] + cs->full;
    
    /* Bit-field access within struct (may combine patterns) */
    val += cs->flags;
    return val;
}

/* 5. Combined function with control flow */
int combined_operations(void) {
    int result = 0;
    volatile int i;
    
    for (i = 0; i < 10; i++) {
        if (cond1) {
            /* ZERO_EXTRACT */
            result ^= zero_extract_pattern_1();
            
            /* STRICT_LOW_PART */
            strict_low_part_pattern_1(&global_value, i & 0xFF);
        }
        
        if (cond2) {
            /* SUBREG */
            result += subreg_pattern_1();
            
            /* Complex MEM */
            result += complex_mem_pattern_1(global_array, i, i % 4);
        }
        
        if (cond3) {
            /* Another ZERO_EXTRACT variant */
            result |= zero_extract_pattern_2();
            
            /* Another SUBREG variant */
            result ^= subreg_pattern_2();
        }
        
        /* Additional STRICT_LOW_PART */
        strict_low_part_pattern_2();
    }
    
    return result;
}

/* Helper functions to increase pass activity */
void helper_function_1(void) {
    /* Focus on ZERO_EXTRACT and MEM */
    struct S2 {
        unsigned int a: 3;
        unsigned int b: 5;
        unsigned int c: 24;
    };
    static volatile struct S2 s2 = {7, 31, 0xFFFFFF};
    
    int val = s2.b;  /* Bit-field extract */
    global_array[val] = complex_mem_pattern_2(&global_struct, val & 3);
}

void helper_function_2(void) {
    /* Focus on STRICT_LOW_PART and SUBREG */
    union U2 {
        uint64_t full;
        uint32_t halves[2];
        uint16_t words[4];
    };
    static union U2 u2 = {0x8877665544332211ULL};
    
    /* Multiple SUBREG accesses */
    u2.words[1] = 0xABCD;  /* STRICT_LOW_PART of the 32-bit half */
    u2.halves[0] = u2.halves[1] & 0xFFFF;  /* Another low part */
    
    /* Complex addressing within union */
    uint16_t *wp = &u2.words[2];
    *wp = (*wp & 0xFF00) | 0xEF;  /* STRICT_LOW_PART of word */
}

/* Main function to drive everything */
int main(void) {
    int final_result = 0;
    
    /* Initialize data */
    for (int i = 0; i < 256; i++) {
        global_array[i] = i * 3;
    }
    global_struct.full = 0x89ABCDEF;
    for (int i = 0; i < 4; i++) {
        global_struct.parts[i] = i * 0x1111;
    }
    global_struct.flags = 0x7;
    global_struct.status = 0xA;
    
    /* Execute pattern functions multiple times */
    for (volatile int iter = 0; iter < 5; iter++) {
        final_result += combined_operations();
        helper_function_1();
        helper_function_2();
        
        /* Modify conditions to change control flow */
        cond1 = iter & 1;
        cond2 = iter & 2;
        cond3 = iter & 4;
        
        /* More complex MEM patterns */
        final_result += complex_mem_pattern_1(
            global_array, 
            final_result & 0xFF,
            (final_result >> 8) & 0x3
        );
    }
    
    /* Use printf to ensure all operations have observable effects */
    printf("Final result: %d (0x%08X)\n", final_result, final_result);
    
    return final_result & 0xFF;  /* Return non-zero to indicate execution */
}
