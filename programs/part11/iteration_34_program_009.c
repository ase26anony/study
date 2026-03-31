/* test_resource.c - Generate ZERO_EXTRACT and STRICT_LOW_PART RTL patterns */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Compiler barrier to prevent optimization */
#define COMPILER_BARRIER() asm volatile("" ::: "memory")

/* Test 1: Bitfield extraction from volatile integer */
void test_zero_extract_volatile(void) {
    volatile unsigned int source = 0x12345678;
    volatile unsigned int result = 0;
    
    /* Multiple extraction patterns that may generate ZERO_EXTRACT */
    for (int i = 0; i < 100; ++i) {
        /* Extract bits 4-11 */
        unsigned int extracted = (source >> 4) & ((1U << 8) - 1);
        result += extracted;
        
        /* Extract bits 16-23 with variable shift */
        extracted = (source >> 16) & 0xFF;
        result ^= extracted;
        
        /* Extract bits 8-15 using mask */
        extracted = (source & 0xFF00) >> 8;
        result |= extracted;
        
        /* Modify source to create variation */
        source = (source * 1103515245U + 12345U) & 0xFFFFFFFFU;
    }
    
    COMPILER_BARRIER();
    printf("Test1 result: %u\n", result);
}

/* Test 2: Packed struct with bitfields */
struct __attribute__((packed)) packed_bitfields {
    unsigned int field_a : 5;
    unsigned int field_b : 11;
    unsigned int field_c : 7;
    unsigned int field_d : 9;
};

void test_zero_extract_struct(void) {
    volatile struct packed_bitfields s = {0};
    volatile unsigned int accumulator = 0;
    
    /* Initialize with pattern */
    s.field_a = 0x1F;
    s.field_b = 0x7FF;
    s.field_c = 0x7F;
    s.field_d = 0x1FF;
    
    for (int i = 0; i < 100; ++i) {
        /* Read bitfields - may generate ZERO_EXTRACT */
        unsigned int val = s.field_b;
        accumulator += val;
        
        /* Complex bitfield operation */
        s.field_a = (s.field_b + s.field_c) & 0x1F;
        
        /* Cross-field assignment */
        val = s.field_d;
        s.field_c = (val >> 2) & 0x7F;
        
        /* Rotate pattern */
        unsigned int temp = s.field_b;
        s.field_b = s.field_d;
        s.field_d = temp;
    }
    
    COMPILER_BARRIER();
    printf("Test2 result: %u\n", accumulator);
}

/* Test 3: Inline assembly for partial register updates */
void test_strict_low_part_asm(void) {
    volatile unsigned int reg = 0xDEADBEEF;
    volatile unsigned int output = 0;
    
    for (int i = 0; i < 50; ++i) {
        /* Assembly that might generate STRICT_LOW_PART */
        unsigned int low_byte;
        asm volatile (
            "and %0, %1, %2\n\t"
            : "=r" (low_byte)
            : "r" (reg), "i" (0xFF)
        );
        output += low_byte;
        
        /* Another pattern with different mask */
        unsigned int low_word;
        asm volatile (
            "and %0, %1, %2\n\t"
            : "=r" (low_word)
            : "r" (reg), "i" (0xFFFF)
        );
        output ^= low_word;
        
        /* Update register */
        reg = (reg * 1664525U + 1013904223U) & 0xFFFFFFFFU;
    }
    
    COMPILER_BARRIER();
    printf("Test3 result: %u\n", output);
}

/* Test 4: Conditional partial updates */
void test_strict_low_part_conditional(void) {
    volatile unsigned int value = 0x12345678;
    volatile unsigned int counter = 0;
    
    for (int i = 0; i < 100; ++i) {
        /* Conditional update of low byte */
        if (i & 1) {
            /* This pattern may generate STRICT_LOW_PART */
            value = (value & ~0xFF) | ((i + 0x55) & 0xFF);
        }
        
        /* Conditional update of low word */
        if (i % 3 == 0) {
            value = (value & ~0xFFFF) | ((value * 3) & 0xFFFF);
        }
        
        /* Extract and accumulate */
        counter += (value & 0xFF);
        counter ^= (value & 0xFF00) >> 8;
        
        /* Memory barrier to prevent reordering */
        COMPILER_BARRIER();
    }
    
    printf("Test4 result: %u\n", counter);
}

/* Test 5: Mixed operations with memory references */
void test_mixed_operations(void) {
    volatile unsigned int array[16];
    volatile unsigned int results[4] = {0};
    
    /* Initialize array */
    for (int i = 0; i < 16; ++i) {
        array[i] = i * 0x11111111U;
    }
    
    /* Use register variables to increase pressure */
    register unsigned int r1 asm ("r8");
    register unsigned int r2 asm ("r9");
    
    for (int i = 0; i < 100; ++i) {
        /* Memory access that may generate MEM_P references */
        unsigned int idx = i & 0xF;
        r1 = array[idx];
        
        /* Bitfield extraction from memory value */
        unsigned int extracted = (r1 >> 8) & 0xFFF;  /* 12 bits */
        results[0] += extracted;
        
        /* Partial update */
        r2 = array[(idx + 1) & 0xF];
        if (extracted & 1) {
            r2 = (r2 & ~0xFF) | (extracted & 0xFF);
        }
        array[(idx + 1) & 0xF] = r2;
        
        /* Switch based on bitfield */
        switch (r1 & 0x7) {
            case 0: results[1] += 1; break;
            case 1: results[1] += 2; break;
            case 2: results[1] += 3; break;
            default: results[1] += 4; break;
        }
        
        /* Another extraction pattern */
        extracted = (r2 >> 16) & ((1U << 10) - 1);
        results[2] ^= extracted;
    }
    
    COMPILER_BARRIER();
    printf("Test5 results: %u %u %u %u\n", 
           results[0], results[1], results[2], results[3]);
}

/* Test 6: Pointer-based partial stores */
void test_pointer_partial_stores(void) {
    volatile unsigned int data = 0x87654321;
    volatile unsigned char *byte_ptr = (volatile unsigned char *)&data;
    volatile unsigned int sum = 0;
    
    for (int i = 0; i < 100; ++i) {
        /* Store through char pointer - partial update */
        byte_ptr[i % 4] = (i * 17) & 0xFF;
        
        /* Extract and combine */
        unsigned int low_part = data & 0xFFFFFF;
        unsigned int high_part = (data >> 24) & 0xFF;
        
        sum += low_part;
        sum ^= high_part << 8;
        
        /* Complex bitfield operation */
        unsigned int rotated = (data << 8) | (data >> 24);
        unsigned int extracted = (rotated >> 4) & ((1U << 20) - 1);
        sum += extracted;
    }
    
    COMPILER_BARRIER();
    printf("Test6 result: %u\n", sum);
}

/* Main driver */
int main(int argc, char *argv[]) {
    int run_all = 0;
    int test_num = 0;
    
    /* Parse command line */
    if (argc > 1) {
        if (strcmp(argv[1], "all") == 0) {
            run_all = 1;
        } else {
            test_num = atoi(argv[1]);
        }
    } else {
        run_all = 1;  /* Default: run all tests */
    }
    
    volatile unsigned int final_result = 0;
    
    if (run_all || test_num == 1) {
        test_zero_extract_volatile();
        final_result += 1;
    }
    
    if (run_all || test_num == 2) {
        test_zero_extract_struct();
        final_result += 2;
    }
    
    if (run_all || test_num == 3) {
        test_strict_low_part_asm();
        final_result += 3;
    }
    
    if (run_all || test_num == 4) {
        test_strict_low_part_conditional();
        final_result += 4;
    }
    
    if (run_all || test_num == 5) {
        test_mixed_operations();
        final_result += 5;
    }
    
    if (run_all || test_num == 6) {
        test_pointer_partial_stores();
        final_result += 6;
    }
    
    /* Prevent dead code elimination */
    printf("Final aggregate: %u\n", final_result);
    
    return 0;
}
