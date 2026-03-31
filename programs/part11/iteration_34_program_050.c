/* test_resource.c - Generate ZERO_EXTRACT and STRICT_LOW_PART RTL patterns */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Compiler barrier to prevent optimization */
#define COMPILER_BARRIER() asm volatile("" ::: "memory")

/* Test 1: Bitfield extraction from volatile integer (ZERO_EXTRACT) */
volatile unsigned int test1_result = 0;
void test_bitfield_extract_volatile(void) {
    volatile unsigned int source = 0xDEADBEEF;
    volatile unsigned int result = 0;
    
    /* Loop to increase scheduling opportunities */
    for (int i = 0; i < 100; ++i) {
        /* Extract bits 8-15 (byte extraction) */
        unsigned int extracted = (source >> 8) & 0xFF;
        
        /* Extract bits 4-7 (nibble extraction) */
        extracted |= ((source >> 4) & 0xF) << 8;
        
        /* Extract bits 16-23 with variable width */
        int width = 8;
        unsigned int mask = (1U << width) - 1;
        extracted |= ((source >> 16) & mask) << 12;
        
        result ^= extracted; /* Combine results */
        COMPILER_BARRIER();
    }
    
    test1_result = result;
}

/* Test 2: Packed struct with bitfields (ZERO_EXTRACT) */
struct __attribute__((packed)) packed_bitfields {
    unsigned int field_a : 5;
    unsigned int field_b : 11;
    unsigned int field_c : 7;
    unsigned int field_d : 9;
};

volatile unsigned int test2_result = 0;
void test_packed_struct_bitfields(void) {
    volatile struct packed_bitfields s = {0};
    volatile unsigned int accumulator = 0;
    
    /* Initialize with pattern */
    s.field_a = 0x1F;
    s.field_b = 0x7FF;
    s.field_c = 0x7F;
    s.field_d = 0x1FF;
    
    for (int i = 0; i < 50; ++i) {
        /* Read bitfields - should generate ZERO_EXTRACT */
        unsigned int val = s.field_b;
        accumulator += val;
        
        /* Complex bitfield operation */
        s.field_a = (s.field_c + i) & 0x1F;
        val = s.field_a;
        accumulator ^= val;
        
        /* Cross-field operation */
        s.field_d = (s.field_b >> 3) | (s.field_c << 6);
        val = s.field_d;
        accumulator |= val;
        
        COMPILER_BARRIER();
    }
    
    test2_result = accumulator;
}

/* Test 3: Inline assembly for partial register store (STRICT_LOW_PART) */
volatile unsigned int test3_result = 0;
void test_inline_asm_partial_store(void) {
    volatile unsigned int reg = 0x12345678;
    volatile unsigned int temp;
    
    for (int i = 0; i < 75; ++i) {
        /* Inline asm that operates on low byte */
        asm volatile (
            "and %0, %1, %2\n\t"
            : "=r" (temp)
            : "r" (reg), "i" (0xFF)
        );
        
        /* Another asm with low 16-bit mask */
        asm volatile (
            "and %0, %1, %2\n\t"
            : "=r" (temp)
            : "r" (reg), "i" (0xFFFF)
        );
        
        /* Mix with arithmetic */
        reg = (reg & ~0xFF) | ((reg + i) & 0xFF);
        
        COMPILER_BARRIER();
    }
    
    test3_result = reg;
}

/* Test 4: Conditional merge operations (STRICT_LOW_PART) */
volatile unsigned int test4_result = 0;
void test_conditional_merge(void) {
    volatile unsigned int value = 0x87654321;
    volatile unsigned int new_low;
    volatile int condition = 1;
    
    for (int i = 0; i < 60; ++i) {
        /* Generate new low byte based on condition */
        new_low = (i * 3) & 0xFF;
        
        /* Conditional update of low byte only */
        if (condition) {
            value = (value & ~0xFF) | (new_low & 0xFF);
        }
        
        /* Sometimes update low 16 bits */
        if (i % 3 == 0) {
            value = (value & ~0xFFFF) | ((value + 0x1111) & 0xFFFF);
        }
        
        /* Toggle condition */
        condition = !condition;
        COMPILER_BARRIER();
    }
    
    test4_result = value;
}

/* Test 5: Mixed operations with memory references */
volatile unsigned int test5_result = 0;
void test_mixed_with_memory(void) {
    volatile unsigned int array[16];
    volatile struct packed_bitfields struct_array[4];
    register unsigned int reg_var; /* Encourage register allocation */
    
    /* Initialize arrays */
    for (int i = 0; i < 16; ++i) {
        array[i] = i * 0x11111111;
    }
    
    for (int i = 0; i < 4; ++i) {
        struct_array[i].field_a = i;
        struct_array[i].field_b = i * 64;
        struct_array[i].field_c = i * 8;
        struct_array[i].field_d = i * 128;
    }
    
    reg_var = 0;
    
    /* Complex loop with mixed operations */
    for (int i = 0; i < 40; ++i) {
        /* Memory access */
        unsigned int mem_val = array[i % 16];
        
        /* Bitfield extraction from struct array */
        unsigned int bf_val = struct_array[i % 4].field_b;
        
        /* Partial update using pointer to char */
        volatile unsigned char *byte_ptr = (volatile unsigned char *)&array[i % 16];
        *byte_ptr = (i * 7) & 0xFF; /* Update low byte only */
        
        /* Combine operations */
        reg_var = (mem_val >> bf_val) & 0xFF;
        
        /* Switch based on bitfield value */
        switch (struct_array[i % 4].field_a & 0x7) {
            case 0: reg_var += 1; break;
            case 1: reg_var += 2; break;
            case 2: reg_var += 3; break;
            case 3: reg_var += 4; break;
            default: reg_var += 5; break;
        }
        
        /* Conditional merge */
        if (reg_var & 1) {
            array[i % 16] = (array[i % 16] & ~0xFF) | (reg_var & 0xFF);
        }
        
        COMPILER_BARRIER();
    }
    
    test5_result = reg_var + array[0] + array[15];
}

/* Test 6: Nested loops with bitfield operations */
volatile unsigned int test6_result = 0;
void test_nested_loops_bitfields(void) {
    volatile struct {
        unsigned int a : 3;
        unsigned int b : 5;
        unsigned int c : 10;
        unsigned int d : 14;
    } __attribute__((packed)) data;
    
    data.a = 0;
    data.b = 0;
    data.c = 0;
    data.d = 0;
    
    volatile unsigned int sum = 0;
    
    /* Nested loops to create scheduling complexity */
    for (int i = 0; i < 20; ++i) {
        for (int j = 0; j < 10; ++j) {
            /* Update bitfields */
            data.a = (data.a + 1) & 0x7;
            data.b = (data.b + data.a) & 0x1F;
            data.c = (data.c + data.b) & 0x3FF;
            data.d = (data.d + data.c) & 0x3FFF;
            
            /* Extract and combine */
            unsigned int val = data.a | (data.b << 3) | (data.c << 8) | (data.d << 18);
            sum += val;
            
            /* Partial store simulation */
            if (j % 2 == 0) {
                volatile unsigned int *ptr = &sum;
                *ptr = (*ptr & ~0xFFFF) | (val & 0xFFFF);
            }
        }
        COMPILER_BARRIER();
    }
    
    test6_result = sum;
}

/* Main driver function */
int main(int argc, char *argv[]) {
    unsigned int run_all = 0;
    unsigned int test_mask = 0;
    
    /* Parse command line arguments */
    if (argc > 1) {
        if (strcmp(argv[1], "all") == 0) {
            run_all = 1;
            test_mask = 0x3F; /* Run all 6 tests */
        } else {
            test_mask = atoi(argv[1]) & 0x3F;
        }
    } else {
        /* Default: run all tests */
        run_all = 1;
        test_mask = 0x3F;
    }
    
    printf("Running tests with mask: 0x%02X\n", test_mask);
    
    /* Execute selected tests */
    if (run_all || (test_mask & 0x01)) {
        test_bitfield_extract_volatile();
        printf("Test 1 complete: result = 0x%08X\n", test1_result);
    }
    
    if (run_all || (test_mask & 0x02)) {
        test_packed_struct_bitfields();
        printf("Test 2 complete: result = 0x%08X\n", test2_result);
    }
    
    if (run_all || (test_mask & 0x04)) {
        test_inline_asm_partial_store();
        printf("Test 3 complete: result = 0x%08X\n", test3_result);
    }
    
    if (run_all || (test_mask & 0x08)) {
        test_conditional_merge();
        printf("Test 4 complete: result = 0x%08X\n", test4_result);
    }
    
    if (run_all || (test_mask & 0x10)) {
        test_mixed_with_memory();
        printf("Test 5 complete: result = 0x%08X\n", test5_result);
    }
    
    if (run_all || (test_mask & 0x20)) {
        test_nested_loops_bitfields();
        printf("Test 6 complete: result = 0x%08X\n", test6_result);
    }
    
    /* Compute and print final aggregate to prevent dead code elimination */
    unsigned int final_result = 
        test1_result ^ test2_result ^ test3_result ^ 
        test4_result ^ test5_result ^ test6_result;
    
    printf("Final aggregate result: 0x%08X\n", final_result);
    
    return 0;
}
