/* test_resource.c - Test program for GCC RTL resource.cc coverage */
/* Compile with: gcc -O2 -fschedule-insns -fno-omit-frame-pointer -c test_resource.c */

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
    
    /* This should generate ZERO_EXTRACT RTL */
    for (int i = 0; i < 100; ++i) {
        /* Extract bits 8-15 (8 bits wide) */
        unsigned int extracted = (source >> 8) & 0xFF;
        result += extracted;
        
        /* Extract bits 16-23 with variable width */
        int width = 8;
        unsigned int mask = (1U << width) - 1;
        extracted = (source >> 16) & mask;
        result ^= extracted;
        
        /* Extract bits 4-11 */
        extracted = (source >> 4) & 0xFF;
        result |= extracted;
        
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
    s.field_a = 0x1F;
    s.field_b = 0x7FF;
    s.field_c = 0x7F;
    s.field_d = 0x1FF;
    
    for (int i = 0; i < 50; ++i) {
        /* Multiple bitfield reads - should generate ZERO_EXTRACT */
        unsigned int val1 = s.field_b;  /* 11-bit extraction */
        unsigned int val2 = s.field_c;  /* 7-bit extraction */
        unsigned int val3 = s.field_d;  /* 9-bit extraction */
        
        /* Bitfield write - may generate ZERO_EXTRACT in SET_DEST */
        s.field_a = (val1 + val2) & 0x1F;
        
        /* Complex bitfield operation */
        s.field_b = (s.field_c * 3 + s.field_d) & 0x7FF;
        
        accumulator += val1 + val2 + val3 + s.field_a;
        
        COMPILER_BARRIER();
    }
    
    /* Nested loop for scheduling pressure */
    for (int i = 0; i < 10; ++i) {
        for (int j = 0; j < 10; ++j) {
            unsigned int temp = s.field_c;
            s.field_c = (s.field_d >> (j % 4)) & 0x7F;
            s.field_d = (temp << (i % 4)) & 0x1FF;
            accumulator ^= s.field_c | s.field_d;
        }
    }
    
    test2_result = accumulator;
    COMPILER_BARRIER();
}

/* ========== Test 3: Inline assembly for partial register store ========== */
volatile unsigned int test3_result = 0;

void test_inline_asm_partial_store(void) {
    volatile unsigned int reg = 0x12345678;
    volatile unsigned int output = 0;
    
    for (int i = 0; i < 100; ++i) {
        unsigned int temp;
        
        /* Inline asm that modifies only low 8 bits - may generate STRICT_LOW_PART */
        asm volatile (
            "and %0, %1, %2\n\t"
            : "=r"(temp)
            : "r"(reg), "i"(0xFF)
        );
        
        /* Another asm with different mask */
        asm volatile (
            "and %0, %1, %2\n\t"
            : "=r"(temp)
            : "r"(reg), "i"(0xFFFF)
        );
        
        /* Store only low 16 bits */
        asm volatile (
            "mov %0, %1\n\t"
            "and %0, %0, %2\n\t"
            : "=r"(temp)
            : "r"(i), "i"(0xFFFF)
        );
        
        output += temp;
        reg = (reg * 1103515245U + 12345U) & 0x7FFFFFFF; /* PRNG */
        
        COMPILER_BARRIER();
    }
    
    test3_result = output;
    COMPILER_BARRIER();
}

/* ========== Test 4: Conditional merge operations ========== */
volatile unsigned int test4_result = 0;

void test_conditional_merge(void) {
    volatile unsigned int var = 0x87654321;
    volatile unsigned int accumulator = 0;
    
    for (int i = 0; i < 100; ++i) {
        volatile int condition = (i % 3) == 0;
        volatile unsigned char new_byte = i & 0xFF;
        
        /* Conditional update of low byte - may generate STRICT_LOW_PART */
        if (condition) {
            var = (var & ~0xFF) | (new_byte & 0xFF);
        }
        
        /* Conditional update of low 16 bits */
        if ((i % 5) == 0) {
            unsigned short new_word = (i * 7) & 0xFFFF;
            var = (var & ~0xFFFF) | (new_word & 0xFFFF);
        }
        
        /* Switch statement for control flow complexity */
        switch (i % 4) {
            case 0:
                var = (var & ~0xF) | (0xA & 0xF);  /* Update low nibble */
                break;
            case 1:
                var = (var & ~0xF0) | ((0xB << 4) & 0xF0); /* Update next nibble */
                break;
            case 2:
                var = (var & ~0xF00) | ((0xC << 8) & 0xF00);
                break;
            case 3:
                var = (var & ~0xF000) | ((0xD << 12) & 0xF000);
                break;
        }
        
        accumulator += var;
        
        COMPILER_BARRIER();
    }
    
    test4_result = accumulator;
    COMPILER_BARRIER();
}

/* ========== Test 5: Mixed operations with memory references ========== */
volatile unsigned int test5_result = 0;

void test_mixed_operations(void) {
    /* Array for memory operations */
    volatile unsigned int array[64];
    volatile struct packed_bitfields struct_array[8];
    
    /* Initialize arrays */
    for (int i = 0; i < 64; ++i) {
        array[i] = i * 0x01010101;
    }
    
    for (int i = 0; i < 8; ++i) {
        struct_array[i].field_a = i & 0x1F;
        struct_array[i].field_b = (i * 17) & 0x7FF;
        struct_array[i].field_c = (i * 23) & 0x7F;
        struct_array[i].field_d = (i * 29) & 0x1FF;
    }
    
    register unsigned int reg_accum = 0; /* register variable */
    
    for (int i = 0; i < 32; ++i) {
        /* Bitfield extraction from struct array */
        unsigned int bf_val = struct_array[i % 8].field_b;
        
        /* Memory operation with pointer arithmetic */
        volatile unsigned int* ptr = &array[i % 56];
        unsigned int mem_val = *ptr;
        
        /* Combine with bitfield extraction */
        unsigned int extracted = (mem_val >> (i % 24)) & 0xFF;
        
        /* Conditional partial store */
        if (bf_val > 0x400) {
            *ptr = (*ptr & ~0xFFFF) | (extracted & 0xFFFF);
        }
        
        /* Complex expression that may generate ZERO_EXTRACT */
        unsigned int combined = ((mem_val & 0xFF00) >> 8) | 
                               ((bf_val & 0x7F) << 8);
        
        /* Update through volatile char pointer (partial store) */
        volatile unsigned char* char_ptr = (volatile unsigned char*)ptr;
        char_ptr[1] = (combined >> 4) & 0xFF;  /* Store to byte 1 */
        
        reg_accum += extracted + combined + bf_val;
        
        COMPILER_BARRIER();
    }
    
    /* Nested loops for scheduling */
    for (int i = 0; i < 16; ++i) {
        for (int j = 0; j < 16; j += 2) {
            /* Bitfield operations in inner loop */
            unsigned int temp = struct_array[j / 2].field_c;
            struct_array[j / 2].field_c = struct_array[j / 2].field_d & 0x7F;
            
            /* Memory update with bitfield-derived index */
            int idx = temp % 56;
            array[idx] = (array[idx] & ~0xFF0000) | 
                        ((struct_array[j / 2].field_a << 16) & 0xFF0000);
            
            reg_accum ^= array[idx];
        }
    }
    
    test5_result = reg_accum;
    COMPILER_BARRIER();
}

/* ========== Main driver ========== */
int main(int argc, char *argv[]) {
    unsigned int final_result = 0;
    
    /* Run tests based on command line arguments */
    if (argc <= 1) {
        /* Run all tests if no arguments */
        test_bitfield_extract_volatile();
        test_packed_struct_bitfields();
        test_inline_asm_partial_store();
        test_conditional_merge();
        test_mixed_operations();
    } else {
        /* Run specific tests based on arguments */
        for (int i = 1; i < argc; i++) {
            if (strcmp(argv[i], "test1") == 0) {
                test_bitfield_extract_volatile();
            } else if (strcmp(argv[i], "test2") == 0) {
                test_packed_struct_bitfields();
            } else if (strcmp(argv[i], "test3") == 0) {
                test_inline_asm_partial_store();
            } else if (strcmp(argv[i], "test4") == 0) {
                test_conditional_merge();
            } else if (strcmp(argv[i], "test5") == 0) {
                test_mixed_operations();
            } else if (strcmp(argv[i], "all") == 0) {
                test_bitfield_extract_volatile();
                test_packed_struct_bitfields();
                test_inline_asm_partial_store();
                test_conditional_merge();
                test_mixed_operations();
            }
        }
    }
    
    /* Aggregate results to prevent dead code elimination */
    final_result = test1_result + test2_result + test3_result + 
                   test4_result + test5_result;
    
    /* Print result (prevents optimization) */
    printf("Result: %u\n", final_result);
    
    return 0;
}
