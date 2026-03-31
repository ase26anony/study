/* test_resource_marking.c
 * Designed to trigger specific RTL patterns in GCC's resource.cc:
 * - ZERO_EXTRACT and STRICT_LOW_PART in SET_DEST
 * - SUBREG in SET_DEST  
 * - MEM_P(x) with complex addressing
 */

#include <stdio.h>
#include <stdint.h>

/* Force compiler to keep computations */
static volatile int sink;

/* Test 1: Bitfield operations for ZERO_EXTRACT/STRICT_LOW_PART */
void test_bitfield_operations(void) {
    /* Volatile bitfield struct - encourages ZERO_EXTRACT */
    volatile struct {
        unsigned int field1 : 4;
        unsigned int field2 : 8;
        unsigned int field3 : 12;
        unsigned int field4 : 3;
    } bf;
    
    /* Initialize with non-constant values */
    unsigned int a = 0xABCD;
    unsigned int b = 0x1234;
    unsigned int c = 0x5678;
    
    /* Multiple bitfield assignments with complex expressions */
    bf.field1 = (a & 0xF) + (b & 0x1);      /* Should generate ZERO_EXTRACT */
    bf.field2 = (b >> 4) & 0xFF;            /* Another bitfield store */
    bf.field3 = (a ^ b) & 0xFFF;            /* Complex expression to bitfield */
    bf.field4 = __builtin_popcount(c) & 0x7; /* Builtin with bitfield */
    
    /* Read back to prevent elimination */
    sink = bf.field1 + bf.field2 + bf.field3 + bf.field4;
}

/* Test 2: Sub-word type operations for SUBREG */
void test_subreg_operations(void) {
    /* Volatile sub-word destinations */
    volatile int8_t v8;
    volatile int16_t v16;
    volatile int32_t v32 = 0x12345678;
    
    /* Register variables to encourage register operations */
    register int32_t r32_1 = 0xABCDEF01;
    register int32_t r32_2 = 0x98765432;
    
    /* Narrowing assignments that may create SUBREG in SET_DEST */
    v8 = (int8_t)(r32_1 + r32_2);           /* int32 -> int8 with SUBREG */
    v16 = (int16_t)(r32_1 * 2);             /* int32 -> int16 */
    
    /* Arithmetic with implicit narrowing */
    int8_t c1 = 100, c2 = 50;
    volatile int8_t result;
    result = c1 + c2;                       /* char + char -> char with potential SUBREG */
    
    /* Mixed-size operations */
    int16_t s1 = 30000;
    int8_t s2 = 100;
    volatile int16_t mixed;
    mixed = s1 + s2;                        /* May involve SUBREG promotion */
    
    sink = v8 + v16 + result + mixed;
}

/* Test 3: Complex memory addressing for MEM_P(x) */
void test_complex_addressing(void) {
    /* Multi-dimensional array with non-trivial indexing */
    int arr[64][8];
    int * restrict ptr = &arr[0][0];  /* restrict helps keep address computation */
    
    /* Complex address calculations */
    for (int i = 0; i < 16; i++) {
        /* Non-linear index calculation */
        int idx = (i * 13 + 7) & 63;
        int idy = (i * 5 + 3) & 7;
        
        /* Store with complex addressing */
        arr[idx][idy] = i * 1000 + 123;
        
        /* Pointer arithmetic with multiple offsets */
        *(ptr + idx * 8 + idy + 1) = i * 500 + 456;
    }
    
    /* Struct with array member */
    struct {
        int header;
        int data[32];
        int footer;
    } s;
    
    /* Access through pointer with offset */
    int *data_ptr = s.data;
    for (int i = 0; i < 16; i++) {
        int offset = (i * 3 + 1) & 31;
        data_ptr[offset] = i * 200 + 789;
    }
    
    /* Compute checksum */
    int sum = 0;
    for (int i = 0; i < 64; i++) {
        for (int j = 0; j < 8; j++) {
            sum += arr[i][j];
        }
    }
    sink = sum + s.data[0];
}

/* Test 4: Combined patterns */
void test_combined_patterns(void) {
    /* Struct combining bitfield and array */
    volatile struct {
        unsigned int flags : 8;
        int16_t values[16];
        unsigned int status : 4;
    } combined;
    
    /* Register variable for source */
    register int32_t src = 0x89ABCDEF;
    
    /* Combined assignment: bitfield + sub-word array store */
    combined.flags = (src >> 16) & 0xFF;    /* Bitfield assignment */
    
    /* Complex index calculation */
    for (int i = 0; i < 8; i++) {
        int idx = (i * 7 + 3) & 15;
        
        /* Narrowing store to array with complex addressing */
        combined.values[idx] = (int16_t)(src + i * 1000);
        
        /* Additional bit manipulation */
        combined.status = __builtin_parity(combined.values[idx]) & 0xF;
    }
    
    /* Inline assembly to directly influence RTL generation */
    int temp_array[16];
    int complex_idx = 5 * 3 + 2;
    
    /* Memory output constraint with complex addressing */
    asm volatile (
        "# Force memory operand with addressing"
        : "=m" (temp_array[complex_idx])
        : 
        : "memory"
    );
    
    sink = combined.flags + combined.values[0] + combined.status + temp_array[complex_idx];
}

/* Test 5: Additional patterns for coverage */
void test_additional_patterns(void) {
    /* STRICT_LOW_PART might be generated for certain byte operations */
    volatile uint32_t word;
    volatile uint8_t *byte_ptr = (volatile uint8_t*)&word;
    
    /* Store to individual bytes - may generate low-part operations */
    byte_ptr[0] = 0xAA;
    byte_ptr[1] = 0xBB;
    byte_ptr[2] = 0xCC;
    byte_ptr[3] = 0xDD;
    
    /* Bitfield extract and store */
    struct {
        volatile uint32_t full;
        volatile uint32_t partial : 24;
    } s2;
    
    s2.full = 0x12345678;
    s2.partial = s2.full & 0xFFFFFF;  /* 24-bit store */
    
    /* Sub-word memory operations with shifting */
    volatile uint16_t shorts[8];
    register uint32_t rval = 0x87654321;
    
    for (int i = 0; i < 4; i++) {
        /* Store shifted sub-word values */
        shorts[i] = (rval >> (i * 4)) & 0xFFFF;
    }
    
    sink = word + s2.partial + shorts[0];
}

int main(void) {
    int total_checksum = 0;
    
    printf("Testing RTL pattern generation for resource.cc coverage...\n");
    
    /* Execute all tests */
    test_bitfield_operations();
    total_checksum += sink;
    
    test_subreg_operations();
    total_checksum += sink;
    
    test_complex_addressing();
    total_checksum += sink;
    
    test_combined_patterns();
    total_checksum += sink;
    
    test_additional_patterns();
    total_checksum += sink;
    
    /* Print result to ensure execution */
    printf("Total checksum: %d\n", total_checksum);
    printf("All tests completed.\n");
    
    return total_checksum != 0 ? 0 : 1;
}
