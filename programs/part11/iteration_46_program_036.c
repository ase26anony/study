/* test_resource_marking.c */
#include <stdio.h>
#include <stdint.h>

/* Force specific RTL patterns for resource.cc coverage testing */

/* Test 1: Generate ZERO_EXTRACT in SET_DEST through bitfield operations */
void test_bitfield_extract(void) {
    /* Volatile bitfield struct - encourages ZERO_EXTRACT for stores */
    volatile struct {
        unsigned int field1 : 4;
        unsigned int field2 : 8;
        unsigned int field3 : 12;
    } bf = {0};
    
    /* Complex expressions that might generate ZERO_EXTRACT */
    unsigned int a = 0xABCD;
    unsigned int b = 0x1234;
    
    /* These assignments should generate ZERO_EXTRACT for bitfield stores */
    bf.field1 = (a & 0xF) + (b & 0xF);          /* 4-bit field */
    bf.field2 = ((a >> 4) & 0xFF) ^ ((b >> 4) & 0xFF); /* 8-bit field */
    bf.field3 = ((a + b) & 0xFFF);              /* 12-bit field */
    
    /* Use __builtin_parity on sub-word data - may involve bit extraction */
    unsigned char c = (unsigned char)(a ^ b);
    int parity = __builtin_parity(c);
    
    /* Prevent optimization */
    asm volatile("" : "+m" (bf));
}

/* Test 2: Generate SUBREG in SET_DEST through type narrowing */
void test_subreg_narrowing(void) {
    /* Volatile short destination - forces SUBREG for narrowing */
    volatile short dest_short;
    volatile char dest_char;
    
    /* Register variables to encourage register operations */
    register int reg_int1 = 0x12345678;
    register int reg_int2 = 0x9ABCDEF0;
    
    /* These should generate SUBREG in SET_DEST when storing narrowed values */
    dest_short = (short)(reg_int1 + reg_int2);  /* int -> short narrowing */
    dest_char = (char)(reg_int1 ^ reg_int2);    /* int -> char narrowing */
    
    /* Arithmetic with implicit narrowing */
    unsigned char uc1 = 200;
    unsigned char uc2 = 100;
    volatile unsigned char result;
    result = uc1 + uc2;  /* May generate SUBREG due to overflow truncation */
    
    /* Prevent optimization */
    asm volatile("" : "+m" (dest_short), "+m" (dest_char), "+m" (result));
}

/* Test 3: Generate MEM_P with complex addressing */
void test_complex_memory_addressing(void) {
    /* Multi-dimensional array with non-trivial indexing */
    int array[64][8];
    int * restrict ptr = &array[0][0];
    
    /* Complex address calculations */
    for (int i = 0; i < 16; i++) {
        /* Non-linear index calculation */
        int idx = (i * 3 + 7) & 63;
        int jdx = (i * 5 + 11) & 7;
        
        /* Store with complex addressing - should generate MEM with non-simple address */
        array[idx][jdx] = i * 100 + jdx;
        
        /* Pointer arithmetic with multiple offsets */
        *(ptr + idx * 8 + jdx) += i;  /* Additional complex addressing */
    }
    
    /* Struct with array member accessed via pointer */
    struct {
        int data[32];
        int count;
    } s = {0};
    
    struct s_with_array *sptr = &s;
    for (int i = 0; i < 16; i++) {
        /* Complex struct member access */
        sptr->data[(i * 7 + 3) & 31] = i * 50;
    }
    
    /* Prevent optimization */
    asm volatile("" : "+m" (array), "+m" (s));
}

/* Test 4: Combined patterns - bitfield and complex memory */
void test_combined_patterns(void) {
    /* Struct combining bitfield and array */
    volatile struct combined {
        unsigned int flags : 16;
        unsigned int status : 4;
        short values[32];
    } comb = {0};
    
    register int reg_val = 0x87654321;
    
    /* Combined assignment: bitfield store (ZERO_EXTRACT) */
    comb.flags = (reg_val & 0xFFFF);
    
    /* Combined assignment: array with complex index and narrowing */
    for (int i = 0; i < 8; i++) {
        int idx = (i * 13 + 17) & 31;  /* Complex index */
        comb.values[idx] = (short)(reg_val >> (i * 2));  /* SUBREG narrowing */
    }
    
    /* Additional bitfield with computation */
    comb.status = ((reg_val >> 16) & 0xF) ^ ((reg_val >> 20) & 0xF);
    
    /* Prevent optimization */
    asm volatile("" : "+m" (comb));
}

/* Test 5: Inline assembly to directly influence RTL generation */
void test_inline_asm_patterns(void) {
    int array[64];
    volatile short vs;
    int idx;
    
    /* Complex memory addressing via inline asm output constraint */
    for (int i = 0; i < 8; i++) {
        idx = (i * 11 + 23) & 63;
        /* Memory output with complex addressing */
        asm volatile("# complex mem" : "=m" (array[idx]) : : "memory");
    }
    
    /* Narrowing store via inline asm */
    register int r = 0x12345678;
    asm volatile("# narrowing" : "=m" (vs) : "r" (r) : "memory");
    
    /* Prevent optimization */
    asm volatile("" : "+m" (array), "+m" (vs));
}

/* Main function executing all tests */
int main(void) {
    int checksum = 0;
    
    printf("Testing RTL patterns for resource.cc coverage...\n");
    
    /* Execute all tests */
    test_bitfield_extract();
    checksum += 1;
    
    test_subreg_narrowing();
    checksum += 2;
    
    test_complex_memory_addressing();
    checksum += 3;
    
    test_combined_patterns();
    checksum += 4;
    
    test_inline_asm_patterns();
    checksum += 5;
    
    /* Use checksum to prevent dead code elimination */
    printf("Checksum: %d\n", checksum);
    
    /* Additional volatile store to ensure all code executes */
    volatile int final_check = checksum;
    asm volatile("" : "+m" (final_check));
    
    return final_check > 0 ? 0 : 1;
}
