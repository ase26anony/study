/* test_resource.c - Cover ZERO_EXTRACT and STRICT_LOW_PART RTL patterns */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Compiler barrier to prevent optimization */
#define COMPILER_BARRIER() asm volatile("" ::: "memory")

/* Test 1: Bitfield extraction from volatile integer (ZERO_EXTRACT) */
volatile unsigned int test1_bitfield_extract(volatile unsigned int base) {
    volatile unsigned int result = 0;
    
    /* Loop to increase scheduling opportunities */
    for (int i = 0; i < 100; ++i) {
        /* Extract bits 5-9 (5 bits wide) - should generate ZERO_EXTRACT */
        unsigned int extracted = (base >> 5) & ((1U << 5) - 1);
        
        /* Mix with arithmetic to create more complex RTL */
        extracted = (extracted * 3 + 7) & 0x1F;
        
        /* Store to volatile to prevent elimination */
        result = extracted;
        
        /* Modify base to create variation */
        base = (base * 1103515245U + 12345U) & 0x7FFFFFFF;
    }
    
    COMPILER_BARRIER();
    return result;
}

/* Test 2: Packed struct with bitfields (ZERO_EXTRACT) */
struct __attribute__((packed)) packed_bitfields {
    unsigned int a : 5;
    unsigned int b : 11;
    unsigned int c : 8;
    unsigned int d : 8;
};

volatile unsigned int test2_packed_struct(void) {
    volatile struct packed_bitfields s = {0};
    volatile unsigned int results[4] = {0};
    
    /* Initialize with pattern */
    s.a = 0x1F;
    s.b = 0x7FF;
    s.c = 0xFF;
    s.d = 0xAA;
    
    for (int i = 0; i < 50; ++i) {
        /* Multiple bitfield reads - should generate ZERO_EXTRACT */
        unsigned int val1 = s.a;
        unsigned int val2 = s.b;
        unsigned int val3 = s.c;
        
        /* Complex bitfield assignment */
        s.a = (s.b + s.c) & 0x1F;
        s.b = (s.c * 2) & 0x7FF;
        
        /* Store results to volatile array */
        results[0] = val1;
        results[1] = val2;
        results[2] = val3;
        results[3] = s.d;
        
        /* Rotate values */
        s.d = (s.d >> 1) | ((s.d & 1) << 7);
    }
    
    COMPILER_BARRIER();
    return results[0] + results[1] + results[2] + results[3];
}

/* Test 3: Inline assembly for partial register store (STRICT_LOW_PART) */
volatile unsigned int test3_asm_partial_store(volatile unsigned int input) {
    volatile unsigned int output = 0;
    
    for (int i = 0; i < 75; ++i) {
        unsigned int temp = input;
        
        /* Inline assembly that modifies only low 8 bits */
        asm volatile (
            "and %0, %1, %2\n\t"
            "orr %0, %0, %3"
            : "=r" (temp)
            : "r" (temp), "i" (0xFFFFFF00), "i" (0x00000055)
            : /* No clobbers */
        );
        
        /* Another assembly pattern with different constraints */
        unsigned int temp2;
        asm volatile (
            "mov %0, %1\n\t"
            "bic %0, %0, #0xFF\n\t"
            "and %2, %2, #0xFF\n\t"
            "orr %0, %0, %2"
            : "=r" (temp2), "+r" (temp)
            : "r" (input)
            : /* No clobbers */
        );
        
        output = temp + temp2;
        input = (input * 1664525U + 1013904223U) & 0xFFFFFFFF;
    }
    
    COMPILER_BARRIER();
    return output;
}

/* Test 4: Conditional merge operations (STRICT_LOW_PART) */
volatile unsigned int test4_conditional_merge(volatile unsigned int base_val) {
    volatile unsigned int result = 0;
    volatile int condition = 1;
    
    /* Array to create memory pressure */
    volatile unsigned int buffer[16];
    for (int i = 0; i < 16; i++) buffer[i] = i * 0x11111111;
    
    for (int i = 0; i < 100; ++i) {
        unsigned int var = base_val;
        
        /* Conditional update of low byte - may generate STRICT_LOW_PART */
        if (condition) {
            /* Update only low 8 bits */
            var = (var & ~0xFF) | ((var + i) & 0xFF);
        }
        
        /* Nested conditional for more complex pattern */
        if (i % 3 == 0) {
            /* Update low 16 bits */
            var = (var & ~0xFFFF) | ((var * 3) & 0xFFFF);
        }
        
        /* Use register variable to encourage register allocation */
        register unsigned int reg_var asm("r12") = var;
        reg_var = (reg_var & ~0xF) | (i & 0xF);
        
        result = reg_var;
        buffer[i % 16] = result;
        
        /* Toggle condition */
        condition = !condition;
        base_val = (base_val << 1) | (base_val >> 31); /* Rotate right */
    }
    
    COMPILER_BARRIER();
    return result + buffer[0];
}

/* Test 5: Mixed patterns with switch statement */
volatile unsigned int test5_mixed_patterns(void) {
    volatile struct packed_bitfields s = {0};
    volatile unsigned int value = 0x89ABCDEF;
    volatile unsigned int result = 0;
    
    s.a = 0x10;
    s.b = 0x400;
    
    for (int i = 0; i < 60; ++i) {
        /* Bitfield extraction */
        unsigned int field = s.b;
        
        /* Switch on extracted bits to create control flow */
        switch (field & 0x7) {
            case 0:
                /* Partial store using pointer to char */
                {
                    volatile unsigned char *byte_ptr = (volatile unsigned char *)&value;
                    byte_ptr[0] = (field + i) & 0xFF;
                }
                break;
            case 1:
                /* Another bitfield extraction pattern */
                result = (value >> 8) & ((1U << 12) - 1);
                break;
            case 2:
                /* Conditional merge */
                if (i % 2) {
                    value = (value & ~0xFF00) | ((value * 2) & 0xFF00);
                }
                break;
            default:
                /* Complex bitfield update */
                s.a = (s.b >> 3) & 0x1F;
                break;
        }
        
        /* Update bitfields */
        s.b = (s.b + 0x111) & 0x7FF;
        value = value ^ (value << 13);
        value = value ^ (value >> 17);
        value = value ^ (value << 5);
    }
    
    COMPILER_BARRIER();
    return result + s.a + s.b + (value & 0xFF);
}

/* Main driver function */
int main(int argc, char *argv[]) {
    volatile unsigned int final_result = 0;
    volatile unsigned int base_value = 0x12345678;
    
    /* Default: run all tests if no arguments */
    int run_all = (argc <= 1);
    
    /* Run selected tests based on arguments */
    for (int i = 1; i < argc || run_all; i++) {
        int test_num = run_all ? (i - 1) : atoi(argv[i]);
        
        switch (test_num) {
            case 1:
                final_result += test1_bitfield_extract(base_value + 0x11111111);
                break;
            case 2:
                final_result += test2_packed_struct();
                break;
            case 3:
                final_result += test3_asm_partial_store(base_value + 0x22222222);
                break;
            case 4:
                final_result += test4_conditional_merge(base_value + 0x33333333);
                break;
            case 5:
                final_result += test5_mixed_patterns();
                break;
            default:
                if (!run_all) {
                    fprintf(stderr, "Unknown test: %d\n", test_num);
                }
                break;
        }
        
        if (!run_all && i >= argc - 1) break;
        if (run_all && i >= 5) break;
    }
    
    /* Print result to prevent dead code elimination */
    printf("Result: 0x%08X\n", final_result);
    
    return 0;
}
