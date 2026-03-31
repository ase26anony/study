/* test_resource.c - Generate ZERO_EXTRACT and STRICT_LOW_PART RTL patterns */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Compiler barrier to prevent optimization */
#define COMPILER_BARRIER() asm volatile("" ::: "memory")

/* Test 1: Bitfield extraction from volatile integer (ZERO_EXTRACT) */
volatile unsigned int test1_volatile_in = 0x89ABCDEF;
volatile unsigned int test1_volatile_out = 0;

void test_bitfield_extract_volatile(void) {
    for (int i = 0; i < 100; ++i) {
        /* Multiple bitfield extractions with different widths */
        unsigned int val = test1_volatile_in;
        
        /* Extract bits 4-8 (5 bits) - likely ZERO_EXTRACT */
        unsigned int field1 = (val >> 4) & 0x1F;
        
        /* Extract bits 12-19 (8 bits) */
        unsigned int field2 = (val >> 12) & 0xFF;
        
        /* Extract bits 20-31 (12 bits) */
        unsigned int field3 = (val >> 20) & 0xFFF;
        
        /* Combine with arithmetic to create complex pattern */
        test1_volatile_out = field1 + (field2 << 5) + (field3 << 13);
        
        COMPILER_BARRIER();
    }
}

/* Test 2: Packed struct with bitfields (ZERO_EXTRACT) */
struct __attribute__((packed)) packed_bitfields {
    unsigned int a : 5;
    unsigned int b : 11;
    unsigned int c : 7;
    unsigned int d : 9;
};

volatile struct packed_bitfields test2_struct = {0};
volatile unsigned int test2_result = 0;

void test_packed_struct_bitfields(void) {
    for (int i = 0; i < 50; ++i) {
        /* Read bitfields - may generate ZERO_EXTRACT */
        unsigned int a_val = test2_struct.a;
        unsigned int b_val = test2_struct.b;
        unsigned int c_val = test2_struct.c;
        unsigned int d_val = test2_struct.d;
        
        /* Write bitfields with conditional logic */
        if (i & 1) {
            test2_struct.a = (b_val + 1) & 0x1F;  /* 5-bit mask */
        }
        
        if (i & 2) {
            test2_struct.c = (d_val ^ a_val) & 0x7F;  /* 7-bit mask */
        }
        
        /* Complex bitfield operation */
        test2_struct.b = ((a_val << 3) | (c_val & 0x07)) & 0x7FF;
        
        test2_result = a_val + b_val + c_val + d_val;
        
        COMPILER_BARRIER();
    }
}

/* Test 3: Inline assembly for partial register store (STRICT_LOW_PART) */
volatile unsigned int test3_reg = 0x12345678;
volatile unsigned int test3_temp = 0;

void test_inline_asm_partial_store(void) {
    for (int i = 0; i < 75; ++i) {
        unsigned int in_val = test3_reg + i;
        unsigned int out_val;
        
        /* Inline assembly that modifies only low bits */
        /* The 'i' constraint for immediate may lead to STRICT_LOW_PART */
        asm volatile (
            "and %0, %1, %2\n\t"
            : "=r" (out_val)
            : "r" (in_val), "i" (0xFF)  /* Only keep low 8 bits */
        );
        
        /* Another partial operation */
        asm volatile (
            "and %0, %1, %2\n\t"
            "orr %0, %0, %3\n\t"
            : "=r" (out_val)
            : "r" (in_val), "i" (0xFFFF), "r" (0xAA00)
            : "cc"
        );
        
        test3_temp = out_val;
        COMPILER_BARRIER();
    }
}

/* Test 4: Conditional merge operations (STRICT_LOW_PART) */
volatile unsigned int test4_var = 0xDEADBEEF;
volatile unsigned char test4_byte = 0x42;
volatile int test4_cond = 1;

void test_conditional_merge(void) {
    for (int i = 0; i < 60; ++i) {
        unsigned int var = test4_var;
        unsigned char new_byte = test4_byte + i;
        
        /* Conditional update of only low byte */
        /* This pattern often generates STRICT_LOW_PART */
        if (test4_cond || (i % 3 == 0)) {
            var = (var & ~0xFF) | (new_byte & 0xFF);
        }
        
        /* Update only low 16 bits based on condition */
        if (i % 5 == 0) {
            unsigned short new_word = (new_byte << 8) | new_byte;
            var = (var & ~0xFFFF) | (new_word & 0xFFFF);
        }
        
        test4_var = var;
        COMPILER_BARRIER();
    }
}

/* Test 5: Mixed operations with memory references */
volatile unsigned int test5_array[64] = {0};
volatile unsigned int test5_index = 0;

void test_mixed_operations(void) {
    /* Initialize array with pattern */
    for (int i = 0; i < 64; ++i) {
        test5_array[i] = i * 0x01010101;
    }
    
    for (int i = 0; i < 40; ++i) {
        /* Memory access that may generate MEM_P(x) */
        unsigned int val = test5_array[test5_index % 64];
        
        /* Bitfield extraction (ZERO_EXTRACT) */
        unsigned int low_bits = (val >> 8) & 0x3F;  /* 6 bits */
        unsigned int high_bits = (val >> 16) & 0xFF; /* 8 bits */
        
        /* Conditional partial store (STRICT_LOW_PART) */
        unsigned int new_val = val;
        if (low_bits > 32) {
            new_val = (new_val & ~0xFF) | (high_bits & 0xFF);
        }
        
        /* Write back to memory */
        test5_array[test5_index % 64] = new_val;
        
        /* Update index with bitfield arithmetic */
        test5_index = (test5_index + low_bits) & 0x3F;
        
        COMPILER_BARRIER();
    }
}

/* Test 6: Switch statement with bitfield-derived values */
volatile unsigned int test6_control = 0;
volatile int test6_result = 0;

void test_switch_with_bitfields(void) {
    struct {
        unsigned int opcode : 4;
        unsigned int operand : 12;
        unsigned int mode : 2;
    } __attribute__((packed)) instruction;
    
    instruction.opcode = test6_control & 0xF;
    instruction.operand = (test6_control >> 4) & 0xFFF;
    instruction.mode = (test6_control >> 16) & 0x3;
    
    /* Switch on bitfield-extracted value */
    switch (instruction.opcode) {
        case 0: test6_result = instruction.operand & 0xFF; break;
        case 1: test6_result = (instruction.operand >> 4) & 0xF; break;
        case 2: test6_result = instruction.operand + instruction.mode; break;
        case 3: test6_result = instruction.operand - instruction.mode; break;
        default: test6_result = instruction.operand ^ 0xFF; break;
    }
    
    /* Partial update based on mode bitfield */
    if (instruction.mode == 1) {
        test6_control = (test6_control & ~0xFF) | (test6_result & 0xFF);
    }
    
    COMPILER_BARRIER();
}

/* Test 7: Pointer-based partial store through char pointer */
volatile unsigned int test7_data = 0x87654321;

void test_pointer_partial_store(void) {
    for (int i = 0; i < 30; ++i) {
        /* Cast to char pointer for partial store */
        volatile unsigned char *byte_ptr = (volatile unsigned char *)&test7_data;
        
        /* Store through char pointer - may generate partial store RTL */
        byte_ptr[1] = i & 0xFF;      /* Modify 2nd byte */
        byte_ptr[3] = (i >> 4) & 0xFF; /* Modify 4th byte */
        
        /* Read back and combine */
        unsigned int reconstructed = 
            (byte_ptr[0]) |
            (byte_ptr[1] << 8) |
            (byte_ptr[2] << 16) |
            (byte_ptr[3] << 24);
        
        /* Bitfield extraction from result */
        unsigned int extracted = (reconstructed >> 12) & 0xFFF;
        
        test7_data = reconstructed ^ extracted;
        
        COMPILER_BARRIER();
    }
}

/* Main driver function */
int main(int argc, char *argv[]) {
    unsigned int run_all = 0;
    unsigned int test_mask = 0xFF;  /* Run all tests by default */
    
    /* Parse command line arguments */
    if (argc > 1) {
        if (strcmp(argv[1], "all") == 0) {
            run_all = 1;
            test_mask = 0xFF;
        } else {
            test_mask = atoi(argv[1]) & 0xFF;
        }
    }
    
    /* Initialize volatile data */
    test1_volatile_in = 0x89ABCDEF;
    test2_struct.a = 1; test2_struct.b = 2047; test2_struct.c = 63; test2_struct.d = 511;
    test3_reg = 0x12345678;
    test4_var = 0xDEADBEEF;
    test4_byte = 0x42;
    test4_cond = 1;
    test5_index = 0;
    test6_control = 0x1A2B3C4D;
    test7_data = 0x87654321;
    
    /* Run selected tests */
    if (run_all || (test_mask & 0x01)) test_bitfield_extract_volatile();
    if (run_all || (test_mask & 0x02)) test_packed_struct_bitfields();
    if (run_all || (test_mask & 0x04)) test_inline_asm_partial_store();
    if (run_all || (test_mask & 0x08)) test_conditional_merge();
    if (run_all || (test_mask & 0x10)) test_mixed_operations();
    if (run_all || (test_mask & 0x20)) test_switch_with_bitfields();
    if (run_all || (test_mask & 0x40)) test_pointer_partial_store();
    
    /* Aggregate results to prevent dead code elimination */
    unsigned int final_result = 
        test1_volatile_out +
        test2_result +
        test3_temp +
        test4_var +
        test5_array[0] +
        test6_result +
        test7_data;
    
    /* Print something to ensure execution */
    printf("Result: 0x%08X\n", final_result & 0xFFFFFFFF);
    
    return 0;
}
