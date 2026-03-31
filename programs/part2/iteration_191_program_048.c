/* This program is designed to trigger specific RTL patterns in GCC's
   resource tracking pass (resource.cc lines 282-290). It creates code
   patterns that generate ZERO_EXTRACT, STRICT_LOW_PART, SUBREG, and
   complex MEM expressions in RTL representation. */

#include <stdio.h>
#include <stdint.h>

/* Volatile variables to prevent optimization and create control flow */
volatile int cond1 = 1;
volatile int cond2 = 0;
volatile int cond3 = 1;

/* Global variables for memory access patterns */
volatile unsigned int global_bitfield = 0xDEADBEEF;
int global_array[256];
struct ComplexStruct {
    int32_t full;
    int16_t parts[4];
    unsigned int flags : 4;
    unsigned int status : 12;
} global_struct;

/* 1. ZERO_EXTRACT patterns - bit-field operations */
int zero_extract_pattern_1(volatile unsigned int *p) {
    /* This should generate ZERO_EXTRACT for bitfield access */
    return (*p >> 8) & 0xFF;  /* Extract bits 8-15 */
}

int zero_extract_pattern_2(void) {
    /* Bitfield struct access - may generate ZERO_EXTRACT */
    struct BitField {
        unsigned int low : 8;
        unsigned int mid : 8;
        unsigned int high : 16;
    };
    volatile struct BitField bf = {0};
    bf.low = 0xAB;
    bf.mid = 0xCD;
    return bf.mid;  /* Access specific bit range */
}

/* 2. STRICT_LOW_PART patterns - partial register writes */
void strict_low_part_pattern_1(volatile uint32_t *p, uint8_t v) {
    /* Writing only low byte of a larger register */
    *p = (*p & ~0xFF) | v;  /* Only affects low 8 bits */
}

void strict_low_part_pattern_2(void) {
    /* Cast to smaller type assignment */
    volatile int32_t x = 0x12345678;
    *(int16_t*)&x = 0xABCD;  /* Write only low 16 bits */
    
    /* Another variation with char */
    volatile long long ll = 0x1122334455667788ULL;
    *(char*)&ll = 0x99;  /* Write only low 8 bits */
}

/* 3. SUBREG patterns - mixed type access */
int subreg_pattern_1(void) {
    /* Union for type punning - generates SUBREG accesses */
    union MixedTypes {
        int32_t i32;
        int16_t i16[2];
        int8_t i8[4];
    };
    
    volatile union MixedTypes u;
    u.i32 = 0x12345678;
    u.i16[0] = 0xABCD;  /* SUBREG access to part of register */
    return u.i8[2];     /* Another SUBREG access */
}

int subreg_pattern_2(volatile double *d) {
    /* Access parts of double as integers */
    union DoubleParts {
        double d;
        uint32_t parts[2];
    };
    
    volatile union DoubleParts dp;
    dp.d = *d;
    dp.parts[0] = 0xDEADBEEF;  /* SUBREG access to half of double */
    return dp.parts[1];
}

/* 4. Complex MEM patterns - non-trivial addressing */
int complex_mem_pattern_1(int *base, int idx1, int idx2) {
    /* Complex addressing with multiple computations */
    return base[(idx1 * 3 + idx2 * 7) & 0xFF];  /* Non-simple address */
}

int complex_mem_pattern_2(struct ComplexStruct *cs, int i) {
    /* Multiple struct field accesses with addressing */
    int result = cs->parts[i % 4];
    result += cs->full;
    result += cs->flags;  /* Bitfield access */
    return result;
}

/* 5. Combined function with control flow */
int combined_patterns(int iterations) {
    volatile int checksum = 0;
    volatile int temp;
    
    for (int i = 0; i < iterations; i++) {
        /* Control flow based on volatile conditions */
        if (cond1) {
            /* ZERO_EXTRACT pattern */
            temp = zero_extract_pattern_1(&global_bitfield);
            checksum ^= temp;
            
            /* STRICT_LOW_PART pattern */
            strict_low_part_pattern_1((volatile uint32_t*)&temp, i & 0xFF);
        }
        
        if (cond2 || (i % 3 == 0)) {
            /* SUBREG pattern */
            checksum += subreg_pattern_1();
            
            /* Complex MEM pattern */
            checksum += complex_mem_pattern_1(global_array, i, i*2);
        }
        
        if (cond3) {
            /* Mixed patterns */
            double dval = (double)i;
            checksum += subreg_pattern_2(&dval);
            
            /* Another memory pattern */
            checksum += complex_mem_pattern_2(&global_struct, i);
        }
        
        /* Additional STRICT_LOW_PART */
        strict_low_part_pattern_2();
    }
    
    return checksum;
}

/* Helper to initialize data */
void initialize_data(void) {
    for (int i = 0; i < 256; i++) {
        global_array[i] = i * 3;
    }
    
    global_struct.full = 0x12345678;
    for (int i = 0; i < 4; i++) {
        global_struct.parts[i] = i * 0x1111;
    }
    global_struct.flags = 0xA;
    global_struct.status = 0xBC;
}

/* Main function with observable side effects */
int main(void) {
    initialize_data();
    
    /* Create complex control flow */
    volatile int result = 0;
    
    /* Call pattern functions multiple times */
    for (int outer = 0; outer < 10; outer++) {
        /* Vary conditions */
        cond1 = outer % 2;
        cond2 = outer % 3;
        cond3 = outer % 5;
        
        /* Combined patterns */
        result += combined_patterns(5 + (outer % 3));
        
        /* Individual patterns */
        result ^= zero_extract_pattern_2();
        
        volatile uint32_t var = 0x87654321;
        strict_low_part_pattern_1(&var, outer & 0xFF);
        result += (int)var;
    }
    
    /* Ensure all code has observable effect */
    printf("Result: %d\n", result);
    
    /* Additional memory access to prevent dead code elimination */
    global_array[result % 256] = result;
    
    return result != 0;
}
