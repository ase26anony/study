/* test_resource_marking.c
 * Generates RTL patterns to trigger uncovered lines in resource.cc:
 * - ZERO_EXTRACT and STRICT_LOW_PART in SET_DEST
 * - SUBREG in SET_DEST  
 * - MEM_P(x) with complex addressing
 */

#include <stdio.h>
#include <stdint.h>

/* Prevent optimizations from removing our test patterns */
static volatile int sink;

/* Test 1: Bitfield operations for ZERO_EXTRACT/STRICT_LOW_PART */
void test_bitfield_operations(void) {
    /* Volatile bitfield struct - encourages ZERO_EXTRACT */
    volatile struct {
        unsigned int field1 : 4;
        unsigned int field2 : 8;
        unsigned int field3 : 12;
        unsigned int field4 : 8;
    } bf = {0};
    
    /* Variables to use in expressions */
    unsigned int a = 0xABCD;
    unsigned int b = 0x1234;
    unsigned int c = 0x5678;
    
    /* Multiple bitfield assignments with complex expressions */
    bf.field1 = (a & 0xF) + (b & 0x1);          /* Should generate ZERO_EXTRACT */
    bf.field2 = (b >> 4) & 0xFF;                /* Another bitfield extract */
    bf.field3 = __builtin_popcount(a) + (c & 0xFFF); /* Builtin + masking */
    bf.field4 = (a ^ b ^ c) & 0xFF;             /* XOR chain with mask */
    
    /* Read back to prevent elimination */
    sink = bf.field1 + bf.field2 + bf.field3 + bf.field4;
}

/* Test 2: SUBREG patterns through type narrowing */
void test_subreg_patterns(void) {
    /* Volatile sub-word destinations */
    volatile int16_t v_short;
    volatile int8_t v_char;
    
    /* Register variables to encourage register operations */
    register int32_t reg_int1 = 0x12345678;
    register int32_t reg_int2 = 0x9ABCDEF0;
    
    /* Explicit narrowing casts - should create SUBREG in SET_DEST */
    v_short = (int16_t)(reg_int1 + reg_int2);      /* Addition then truncation */
    v_char = (int8_t)(reg_int1 * reg_int2);        /* Multiplication then truncation */
    
    /* Implicit narrowing through arithmetic */
    int16_t temp1 = 1000;
    int16_t temp2 = 2000;
    v_short = temp1 + temp2;                       /* Potential overflow truncation */
    
    /* Read back */
    sink = v_short + v_char;
}

/* Test 3: Complex memory addressing for MEM_P(x) */
void test_complex_addressing(void) {
    /* Multi-dimensional array with non-trivial indexing */
    int array[64][8] = {0};
    int *restrict ptr = &array[0][0];
    
    /* Complex address calculations */
    for (int i = 0; i < 16; i++) {
        /* Multiple index calculations */
        int idx1 = (i * 3 + 7) & 63;
        int idx2 = (i * 5 + 11) & 7;
        
        /* Complex addressing: base + row*stride + column */
        array[idx1][idx2] = i * 100 + idx1 + idx2;
        
        /* Pointer arithmetic with multiple offsets */
        *(ptr + idx1 * 8 + idx2 + 4) = i * 200;
    }
    
    /* Struct with array member */
    struct {
        int data[32];
        int count;
    } s = {0};
    
    int *data_ptr = s.data;
    for (int i = 0; i < 16; i++) {
        /* Access through pointer with offset calculation */
        data_ptr[i * 2 + 1] = i * 300;
    }
    
    /* Compute checksum */
    int sum = 0;
    for (int i = 0; i < 64; i++) {
        for (int j = 0; j < 8; j++) {
            sum += array[i][j];
        }
    }
    sink = sum + s.data[31];
}

/* Test 4: Combined patterns */
void test_combined_patterns(void) {
    /* Struct combining bitfield and array */
    volatile struct {
        unsigned int flags : 16;
        int16_t values[32];
        unsigned int status : 8;
    } combined = {0};
    
    /* Register source for SUBREG pattern */
    register int32_t reg_val = 0x87654321;
    
    /* Combined assignment: bitfield + sub-word array with complex index */
    int idx = (reg_val & 31) * 3 + 7;
    
    /* Bitfield assignment (ZERO_EXTRACT) */
    combined.flags = (reg_val >> 16) & 0xFFFF;
    
    /* Sub-word array assignment with narrowing (SUBREG) */
    combined.values[idx & 31] = (int16_t)(reg_val + idx);
    
    /* Another bitfield with expression */
    combined.status = __builtin_parity(reg_val) | (idx & 0x7F);
    
    /* Complex memory store through pointer */
    int16_t *val_ptr = combined.values;
    val_ptr[(idx + 5) & 31] = (int16_t)(reg_val >> 8);
    
    sink = combined.flags + combined.values[0] + combined.status;
}

/* Test 5: Inline assembly for direct RTL influence */
void test_asm_patterns(void) {
    int array[64] = {0};
    int index = 0;
    
    /* Complex addressing in asm output */
    for (int i = 0; i < 16; i++) {
        index = (i * 7 + 13) & 63;
        
        /* Memory output with complex addressing - may generate MEM with complex address */
        asm volatile (
            "# Force memory store with complex address\n"
            : "=m" (array[index])   /* Memory output constraint */
            : 
            : "memory"
        );
        
        /* Another asm with register input, memory output */
        register int val = i * 100;
        asm volatile (
            "# Store register to memory with offset\n"
            : "=m" (*(array + index + 8))  /* Pointer arithmetic in constraint */
            : "r" (val)                    /* Register input */
            : "memory"
        );
    }
    
    /* Compute sum */
    int sum = 0;
    for (int i = 0; i < 64; i++) {
        sum += array[i];
    }
    sink = sum;
}

int main(void) {
    int total = 0;
    
    printf("Testing RTL pattern generation for resource marking...\n");
    
    /* Run all tests */
    test_bitfield_operations();
    total += sink;
    
    test_subreg_patterns();
    total += sink;
    
    test_complex_addressing();
    total += sink;
    
    test_combined_patterns();
    total += sink;
    
    test_asm_patterns();
    total += sink;
    
    printf("Total checksum: %d\n", total);
    printf("All tests completed.\n");
    
    return total != 0 ? 0 : 1;
}
