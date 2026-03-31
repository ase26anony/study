/* test_resource.c - Generate ZERO_EXTRACT and STRICT_LOW_PART RTL patterns */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Compiler barrier to prevent optimization */
#define COMPILER_BARRIER() asm volatile("" ::: "memory")

/* Test 1: Bitfield extraction from volatile integer */
void test_zero_extract_volatile(void) {
    volatile unsigned int source = 0x89ABCDEF;
    volatile unsigned int result = 0;
    
    /* This should generate ZERO_EXTRACT RTL */
    for (int i = 0; i < 100; ++i) {
        /* Extract bits 8-15 (byte 1) */
        unsigned int extracted = (source >> 8) & 0xFF;
        /* Extract bits 4-11 with variable width */
        unsigned int extracted2 = (source >> 4) & ((1U << 8) - 1);
        /* Combine extractions */
        result = extracted + extracted2;
        COMPILER_BARRIER();
        source = source ^ 0x12345678; /* Modify source to prevent optimization */
    }
    
    printf("Test 1 result: %u\n", result);
}

/* Test 2: Packed struct with bitfields */
struct packed_bitfields {
    unsigned int field1 : 5;
    unsigned int field2 : 11;
    unsigned int field3 : 7;
    unsigned int field4 : 9;
} __attribute__((packed));

void test_zero_extract_struct(void) {
    volatile struct packed_bitfields s = {0};
    volatile unsigned int results[4] = {0};
    
    /* Initialize with pattern */
    s.field1 = 0x1F;
    s.field2 = 0x7FF;
    s.field3 = 0x7F;
    s.field4 = 0x1FF;
    
    /* Multiple reads/writes to generate ZERO_EXTRACT */
    for (int i = 0; i < 50; ++i) {
        /* Read bitfields - should generate ZERO_EXTRACT */
        unsigned int val1 = s.field1;
        unsigned int val2 = s.field2;
        unsigned int val3 = s.field3;
        unsigned int val4 = s.field4;
        
        /* Write back with transformations */
        s.field1 = (val2 + i) & 0x1F;
        s.field2 = (val3 ^ val4) & 0x7FF;
        s.field3 = (val1 * 2) & 0x7F;
        s.field4 = (val2 >> 3) & 0x1FF;
        
        results[0] += val1;
        results[1] += val2;
        results[2] += val3;
        results[3] += val4;
        
        COMPILER_BARRIER();
    }
    
    printf("Test 2 results: %u %u %u %u\n", 
           results[0], results[1], results[2], results[3]);
}

/* Test 3: Complex bitfield operations with arrays */
void test_zero_extract_array(void) {
    volatile unsigned int data[16];
    volatile unsigned int output[16];
    
    /* Initialize array */
    for (int i = 0; i < 16; ++i) {
        data[i] = 0x12345678 ^ (i * 0x11111111);
    }
    
    /* Nested loops with bitfield extractions */
    for (int outer = 0; outer < 10; ++outer) {
        for (int i = 0; i < 16; ++i) {
            /* Extract different bit ranges */
            unsigned int bits_0_7 = data[i] & 0xFF;
            unsigned int bits_8_15 = (data[i] >> 8) & 0xFF;
            unsigned int bits_16_23 = (data[i] >> 16) & 0xFF;
            unsigned int bits_24_31 = (data[i] >> 24) & 0xFF;
            
            /* Combine with arithmetic */
            output[i] = (bits_0_7 * bits_8_15) + 
                       (bits_16_23 ^ bits_24_31);
            
            /* Update data with rotated pattern */
            data[i] = (data[i] << 1) | (data[i] >> 31);
        }
        COMPILER_BARRIER();
    }
    
    unsigned int sum = 0;
    for (int i = 0; i < 16; ++i) {
        sum += output[i];
    }
    printf("Test 3 sum: %u\n", sum);
}

/* Test 4: STRICT_LOW_PART via conditional merge operations */
void test_strict_low_part_conditional(void) {
    volatile unsigned int value = 0x87654321;
    volatile unsigned int mask = 0x0000FFFF;
    volatile unsigned int new_low = 0;
    volatile int condition = 1;
    
    /* This pattern may generate STRICT_LOW_PART */
    for (int i = 0; i < 100; ++i) {
        if (condition) {
            /* Update only low 16 bits */
            value = (value & ~mask) | (new_low & mask);
        } else {
            /* Update only high 16 bits */
            value = (value & mask) | ((new_low << 16) & ~mask);
        }
        
        /* Alternate condition */
        condition = !condition;
        new_low = (new_low + 0x1111) & 0xFFFF;
        
        COMPILER_BARRIER();
    }
    
    printf("Test 4 value: 0x%08X\n", value);
}

/* Test 5: STRICT_LOW_PART via inline assembly and pointer casting */
void test_strict_low_part_asm(void) {
    volatile unsigned int data = 0xDEADBEEF;
    volatile unsigned char *byte_ptr = (volatile unsigned char *)&data;
    
    /* Partial updates through byte pointer */
    for (int i = 0; i < 50; ++i) {
        /* Update only specific bytes - may generate partial store patterns */
        byte_ptr[0] = i & 0xFF;
        byte_ptr[1] = (i >> 2) & 0xFF;
        
        /* Inline assembly that hints at partial register updates */
        unsigned int temp = data;
        asm volatile (
            "and %0, %1, %2\n\t"
            : "=r" (temp)
            : "r" (temp), "i" (0x0000FFFF)
        );
        data = temp | 0xCAFE0000;
        
        COMPILER_BARRIER();
    }
    
    printf("Test 5 data: 0x%08X\n", data);
}

/* Test 6: Mixed patterns with switch statement */
void test_mixed_patterns(void) {
    volatile unsigned int reg = 0;
    volatile struct {
        unsigned int low : 12;
        unsigned int high : 20;
    } bitfield = {0};
    
    for (int i = 0; i < 100; ++i) {
        /* Switch on derived value */
        unsigned int selector = (reg >> 4) & 0x7;
        
        switch (selector) {
            case 0:
                /* ZERO_EXTRACT pattern */
                bitfield.low = (reg >> 8) & 0xFFF;
                break;
            case 1:
                /* STRICT_LOW_PART-like pattern */
                reg = (reg & 0xFFFF0000) | (i & 0xFFFF);
                break;
            case 2:
                /* Both patterns */
                bitfield.high = (reg >> 16) & 0xFFFFF;
                reg = (reg & 0xFFF00000) | (bitfield.low << 8);
                break;
            default:
                /* Memory operation to trigger MEM_P path elsewhere */
                volatile unsigned int *ptr = &reg;
                *ptr = *ptr ^ 0x12345678;
                break;
        }
        
        /* Register variable to encourage register allocation */
        register unsigned int counter asm ("r12") = i;
        reg += counter;
        
        COMPILER_BARRIER();
    }
    
    printf("Test 6 reg: 0x%08X, bitfield: low=0x%03X high=0x%05X\n",
           reg, bitfield.low, bitfield.high);
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
        run_all = 1; /* Default: run all tests */
    }
    
    volatile unsigned int total_result = 0;
    
    if (run_all || test_num == 1) {
        test_zero_extract_volatile();
        total_result += 1;
    }
    
    if (run_all || test_num == 2) {
        test_zero_extract_struct();
        total_result += 2;
    }
    
    if (run_all || test_num == 3) {
        test_zero_extract_array();
        total_result += 3;
    }
    
    if (run_all || test_num == 4) {
        test_strict_low_part_conditional();
        total_result += 4;
    }
    
    if (run_all || test_num == 5) {
        test_strict_low_part_asm();
        total_result += 5;
    }
    
    if (run_all || test_num == 6) {
        test_mixed_patterns();
        total_result += 6;
    }
    
    /* Print final aggregate to prevent dead code elimination */
    printf("Total indicator: %u\n", total_result);
    
    return 0;
}
