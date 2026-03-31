/* test_resource.c - Test program to cover ZERO_EXTRACT and STRICT_LOW_PART RTL patterns */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Compiler barrier to prevent optimization */
#define COMPILER_BARRIER() asm volatile("" ::: "memory")

/* ========== Test 1: Bitfield extraction from volatile integer ========== */
volatile unsigned int test1_result = 0;

void test_bitfield_extract_volatile(void) {
    volatile unsigned int source = 0xDEADBEEF;
    volatile unsigned int result = 0;
    
    /* Loop to increase scheduling opportunities */
    for (int i = 0; i < 100; ++i) {
        /* Multiple bitfield extractions - should generate ZERO_EXTRACT */
        unsigned int bits_8_15 = (source >> 8) & 0xFF;   /* Extract bits 8-15 */
        unsigned int bits_16_23 = (source >> 16) & 0xFF; /* Extract bits 16-23 */
        unsigned int bits_0_7 = source & 0xFF;           /* Extract bits 0-7 */
        
        /* Combine with arithmetic to create complex pattern */
        result = (bits_8_15 * bits_16_23) & 0x3F;        /* Mask to 6 bits */
        result |= (bits_0_7 << 6) & 0x1C0;               /* More bit manipulation */
        
        /* Use result to affect source for next iteration */
        source = (source << 1) | (result & 1);
        
        COMPILER_BARRIER();
    }
    
    test1_result = result;
    COMPILER_BARRIER();
}

/* ========== Test 2: Packed struct with bitfields ========== */
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
    s.field_a = 0x1F;      /* 5 bits max */
    s.field_b = 0x7FF;     /* 11 bits max */
    s.field_c = 0x7F;      /* 7 bits max */
    s.field_d = 0x1FF;     /* 9 bits max */
    
    for (int i = 0; i < 50; ++i) {
        /* Complex bitfield operations - should generate ZERO_EXTRACT */
        unsigned int temp = s.field_b;
        s.field_a = (temp + i) & 0x1F;           /* Write to bitfield */
        
        temp = s.field_c;
        s.field_b = (temp * s.field_a) & 0x7FF;  /* Another bitfield write */
        
        /* Nested loop for more scheduling complexity */
        for (int j = 0; j < 10; ++j) {
            accumulator += s.field_d;
            s.field_c = (accumulator >> j) & 0x7F;
        }
        
        /* Conditional bitfield update */
        if (i & 1) {
            s.field_d = (s.field_d * 3) & 0x1FF;
        }
        
        COMPILER_BARRIER();
    }
    
    test2_result = accumulator + s.field_a + s.field_b + s.field_c + s.field_d;
    COMPILER_BARRIER();
}

/* ========== Test 3: Inline assembly for partial register updates ========== */
volatile unsigned int test3_result = 0;

void test_inline_asm_partial_store(void) {
    volatile unsigned int reg = 0x12345678;
    volatile unsigned int output = 0;
    
    for (int i = 0; i < 100; ++i) {
        /* Inline assembly that might generate STRICT_LOW_PART */
        unsigned int temp = reg;
        
        /* Assembly that modifies only low bits */
        asm volatile (
            "and %0, %1, %2\n\t"
            "orr %0, %0, %3"
            : "=r" (temp)
            : "r" (temp), "i" (0xFFFFFF00), "r" (i & 0xFF)
            : /* No clobbers */
        );
        
        /* Another pattern: conditional merge of low byte */
        if (i & 1) {
            asm volatile (
                "bfi %0, %1, #0, #8"
                : "+r" (reg)
                : "r" (i)
                : /* No clobbers */
            );
        }
        
        /* C code pattern that may also generate STRICT_LOW_PART */
        unsigned int mask = 0xFF;
        reg = (reg & ~mask) | ((i * 7) & mask);
        
        output += temp;
        COMPILER_BARRIER();
    }
    
    test3_result = output + reg;
    COMPILER_BARRIER();
}

/* ========== Test 4: Conditional narrow stores and pointer casting ========== */
volatile unsigned int test4_result = 0;

void test_conditional_partial_stores(void) {
    volatile unsigned int data = 0x87654321;
    volatile unsigned char *byte_ptr = (volatile unsigned char *)&data;
    volatile unsigned int sum = 0;
    
    /* Array to create memory pressure */
    volatile unsigned int array[16];
    for (int i = 0; i < 16; ++i) {
        array[i] = i * 0x11111111;
    }
    
    for (int i = 0; i < 100; ++i) {
        /* Conditional update of low byte - may generate STRICT_LOW_PART */
        if (i % 3 == 0) {
            /* Update only low 8 bits */
            data = (data & 0xFFFFFF00) | ((i + array[i & 0xF]) & 0xFF);
        }
        
        /* Update through char pointer - partial store */
        byte_ptr[(i % 4)] = (i * 13) & 0xFF;
        
        /* Complex conditional with bitfield extraction */
        unsigned int extracted = (data >> 8) & 0xFFFF;  /* ZERO_EXTRACT */
        
        /* Switch based on extracted bits to create control flow */
        switch (extracted & 0x7) {
            case 0:
                data = (data & 0xFFFF0000) | (array[0] & 0xFFFF);
                break;
            case 1:
                data = (data & 0xFF0000FF) | ((i << 8) & 0x00FFFF00);
                break;
            case 2:
                /* Another partial update */
                data = (data & 0xFFFFFF00) | (byte_ptr[2] & 0xFF);
                break;
            default:
                data ^= 0x00FF00FF;  /* Flip specific bytes */
                break;
        }
        
        sum += data + extracted;
        COMPILER_BARRIER();
    }
    
    test4_result = sum;
    COMPILER_BARRIER();
}

/* ========== Test 5: Mixed patterns with register variables ========== */
volatile unsigned int test5_result = 0;

void test_mixed_patterns_register_vars(void) {
    /* Use register keyword to encourage register allocation */
    register unsigned int r1 asm ("r8") = 0x11111111;
    register unsigned int r2 asm ("r9") = 0x22222222;
    volatile unsigned int memory_var = 0x33333333;
    
    /* Packed struct with bitfields */
    struct __attribute__((packed)) {
        unsigned int low : 12;
        unsigned int mid : 12;
        unsigned int high : 8;
    } bits = {0};
    
    bits.low = 0xFFF;
    bits.mid = 0xFFF;
    bits.high = 0xFF;
    
    for (int i = 0; i < 75; ++i) {
        /* Mix of operations that should generate both target patterns */
        
        /* 1. ZERO_EXTRACT from bitfield read */
        unsigned int extracted = bits.mid;
        
        /* 2. Conditional STRICT_LOW_PART store */
        if (extracted & 1) {
            r1 = (r1 & 0xFFFFFF00) | (i & 0xFF);
        }
        
        /* 3. Bitfield write (ZERO_EXTRACT in SET_DEST) */
        bits.low = (r2 + extracted) & 0xFFF;
        
        /* 4. Complex expression with partial update */
        r2 = ((r2 << 4) & 0xFFFFFFF0) | (bits.high & 0xF);
        
        /* 5. Memory operation to trigger MEM_P path elsewhere */
        memory_var = array_operation(memory_var, i);
        
        /* 6. Another bitfield extraction */
        bits.high = (extracted >> 4) & 0xFF;
        
        /* Use register variables in computation */
        r1 = r1 ^ r2;
        r2 = r2 + bits.low;
        
        COMPILER_BARRIER();
    }
    
    test5_result = r1 + r2 + memory_var + bits.low + bits.mid + bits.high;
    COMPILER_BARRIER();
}

/* Helper function for memory operations */
volatile unsigned int array_operation(volatile unsigned int val, int idx) {
    static volatile unsigned int buffer[8] = {0};
    buffer[idx & 7] = val;
    return buffer[(idx + 1) & 7];
}

/* ========== Main driver ========== */
int main(int argc, char *argv[]) {
    unsigned int total_result = 0;
    
    /* Default: run all tests if no arguments */
    int run_all = (argc == 1);
    
    printf("Running resource pattern tests...\n");
    
    if (run_all || strstr(argv[0], "test1") || (argc > 1 && atoi(argv[1]) == 1)) {
        printf("Test 1: Bitfield extraction from volatile integer\n");
        test_bitfield_extract_volatile();
        total_result += test1_result;
    }
    
    if (run_all || strstr(argv[0], "test2") || (argc > 1 && atoi(argv[1]) == 2)) {
        printf("Test 2: Packed struct with bitfields\n");
        test_packed_struct_bitfields();
        total_result += test2_result;
    }
    
    if (run_all || strstr(argv[0], "test3") || (argc > 1 && atoi(argv[1]) == 3)) {
        printf("Test 3: Inline assembly for partial register updates\n");
        test_inline_asm_partial_store();
        total_result += test3_result;
    }
    
    if (run_all || strstr(argv[0], "test4") || (argc > 1 && atoi(argv[1]) == 4)) {
        printf("Test 4: Conditional narrow stores and pointer casting\n");
        test_conditional_partial_stores();
        total_result += test4_result;
    }
    
    if (run_all || strstr(argv[0], "test5") || (argc > 1 && atoi(argv[1]) == 5)) {
        printf("Test 5: Mixed patterns with register variables\n");
        test_mixed_patterns_register_vars();
        total_result += test5_result;
    }
    
    /* Print final result to prevent dead code elimination */
    printf("Total result: %u\n", total_result);
    
    return 0;
}
