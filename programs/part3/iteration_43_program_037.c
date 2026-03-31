/* resource_patterns.c - Generate RTL patterns for GCC resource tracking coverage */

#include <stdint.h>
#include <stdlib.h>

/* Force no optimization on specific functions */
#define NOINLINE __attribute__((noinline))
#define VOLATILE_VAR volatile

/* Pattern 1: ZERO_EXTRACT and MEM */
NOINLINE static void pattern_zero_extract_mem(int i, int j) {
    /* Struct with volatile bit-fields for ZERO_EXTRACT */
    struct BitFieldStruct {
        VOLATILE_VAR unsigned int field1 : 5;
        VOLATILE_VAR unsigned int field2 : 7;
        VOLATILE_VAR unsigned int field3 : 3;
    };
    
    /* Array with complex addressing for MEM */
    static VOLATILE_VAR int mem_array[32][32];
    
    struct BitFieldStruct bfs;
    /* ZERO_EXTRACT pattern: assignment to volatile bit-field */
    bfs.field1 = (i & 0x1F);        /* Likely generates ZERO_EXTRACT */
    bfs.field2 = (j & 0x7F);        /* Another ZERO_EXTRACT */
    
    /* MEM pattern with complex addressing */
    int* volatile ptr = &mem_array[i & 31][j & 31];
    /* Force MEM reference with address computation */
    VOLATILE_VAR int val = *ptr + bfs.field1;
    
    /* Additional MEM with pointer arithmetic */
    ptr += (i * j) & 31;
    val += *ptr;
    
    /* Prevent dead code elimination */
    asm volatile("" : "+r"(val) : : "memory");
}

/* Pattern 2: STRICT_LOW_PART and SUBREG */
NOINLINE static void pattern_strict_low_part_subreg(int i) {
    /* Use different sized types for SUBREG patterns */
    VOLATILE_VAR uint32_t var32 = i;
    VOLATILE_VAR uint16_t var16;
    VOLATILE_VAR uint8_t var8;
    
    /* SUBREG pattern: type punning through pointers */
    uint16_t* p16 = (uint16_t*)&var32;
    *p16 = (uint16_t)(var32 + 1);  /* May generate SUBREG */
    
    /* Another SUBREG pattern with char access */
    uint8_t* p8 = (uint8_t*)&var32;
    p8[1] = (uint8_t)i;            /* Another potential SUBREG */
    
    /* STRICT_LOW_PART pattern: inline assembly modifying low part */
    /* Using byte-sized operation on 32-bit register */
    asm volatile (
        "addb %b1, %b0\n\t"        /* Add byte part */
        : "=q"(var8)               /* =q constraint for byte-addressable reg */
        : "q"((uint8_t)i), "0"(var8)
        : "cc"
    );
    
    /* Mixed-size operations to encourage SUBREG */
    var16 = (uint16_t)var32 + (uint16_t)var8;
    var32 = var32 ^ (var16 << 16);
    
    /* Prevent optimization */
    asm volatile("" : "+r"(var32), "+r"(var16), "+r"(var8) : : "memory");
}

/* Pattern 3: Complex expression mixing patterns */
NOINLINE static void pattern_complex_mix(int i, int j, int k) {
    /* Volatile bit-field in struct */
    struct MixedStruct {
        VOLATILE_VAR unsigned int flags : 4;
        VOLATILE_VAR unsigned int value : 12;
        VOLATILE_VAR unsigned int data[8];
    };
    
    static VOLATILE_VAR struct MixedStruct ms[16];
    
    /* Complex addressing with ternary operator */
    struct MixedStruct* ptr = (i & 1) ? &ms[j & 15] : &ms[k & 15];
    
    /* ZERO_EXTRACT through bit-field assignment */
    ptr->flags = (i & 0xF);
    ptr->value = (j & 0xFFF);
    
    /* MEM with complex index calculation */
    int idx = (i * j + k) & 7;
    VOLATILE_VAR int val = ptr->data[idx];
    
    /* Additional SUBREG through type punning */
    uint16_t* data_as_short = (uint16_t*)ptr->data;
    data_as_short[idx] = (uint16_t)val;
    
    /* Inline asm that might generate STRICT_LOW_PART */
    uint8_t low_byte = (uint8_t)val;
    asm volatile (
        "orb %1, %0\n\t"
        : "+q"(low_byte)
        : "q"((uint8_t)i)
        : "cc"
    );
    
    /* Prevent dead code */
    asm volatile("" : "+r"(val) : : "memory");
}

/* Pattern 4: Loop-based pattern generator */
NOINLINE static void pattern_loop_based(VOLATILE_VAR int iterations) {
    VOLATILE_VAR int array[64];
    VOLATILE_VAR struct {
        VOLATILE_VAR unsigned int bf1 : 3;
        VOLATILE_VAR unsigned int bf2 : 5;
    } bit_struct;
    
    /* Initialize array */
    for (VOLATILE_VAR int i = 0; i < 64; i++) {
        array[i] = i * i;
    }
    
    /* Loop that generates various patterns */
    for (VOLATILE_VAR int i = 0; i < iterations; i++) {
        /* MEM with complex addressing in loop */
        int idx1 = (i * 3) & 63;
        int idx2 = (i * 5) & 63;
        VOLATILE_VAR int temp = array[idx1] + array[idx2];
        
        /* ZERO_EXTRACT in loop */
        bit_struct.bf1 = (temp & 0x7);
        bit_struct.bf2 = ((temp >> 3) & 0x1F);
        
        /* SUBREG through pointer casting */
        uint8_t* byte_ptr = (uint8_t*)&temp;
        byte_ptr[1] = (uint8_t)i;  /* May generate SUBREG */
        
        /* STRICT_LOW_PART via inline asm */
        uint8_t low_part = (uint8_t)temp;
        asm volatile (
            "xchgb %b0, %b1\n\t"
            : "+q"(low_part), "+q"(byte_ptr[0])
            :
            : "cc"
        );
        
        /* Write back to prevent elimination */
        array[idx1] = temp;
    }
}

/* Main function that drives all patterns */
int main(int argc, char* argv[]) {
    VOLATILE_VAR int iterations = 10;
    
    /* Use command line argument for iteration count if available */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 10;
        if (iterations > 100) iterations = 100; /* Bound for safety */
    }
    
    VOLATILE_VAR int result = 0;
    
    /* Call pattern functions in a loop */
    for (VOLATILE_VAR int i = 0; i < iterations; i++) {
        for (VOLATILE_VAR int j = 0; j < 4; j++) {
            pattern_zero_extract_mem(i, j);
            pattern_strict_low_part_subreg(i + j);
            pattern_complex_mix(i, j, i * j);
            
            /* Accumulate dummy result to prevent elimination */
            result += i + j;
        }
        
        /* Call loop-based pattern generator */
        pattern_loop_based(5);
    }
    
    /* Final dummy use of result */
    asm volatile("" : "+r"(result) : : "memory");
    
    return result & 0xFF; /* Return non-zero to prevent optimization */
}
