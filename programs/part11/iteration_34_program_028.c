/* test_resource.c - Generate ZERO_EXTRACT and STRICT_LOW_PART RTL patterns */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Compiler barrier to prevent optimization */
#define COMPILER_BARRIER() asm volatile("" ::: "memory")

/* Test 1: Bitfield extraction from volatile integer */
void test_zero_extract_volatile(void) {
    volatile unsigned int source = 0xDEADBEEF;
    volatile unsigned int result = 0;
    
    /* Multiple extraction patterns to increase chances */
    for (int i = 0; i < 100; ++i) {
        /* Extract bits 4-11 (8 bits) */
        unsigned int extracted = (source >> 4) & 0xFF;
        result = extracted;
        
        /* Extract bits 16-23 */
        extracted = (source >> 16) & 0xFF;
        result ^= extracted;
        
        /* Extract variable-width field */
        int width = (i % 8) + 1;
        int start = (i % 16);
        unsigned int mask = (1U << width) - 1;
        extracted = (source >> start) & mask;
        result += extracted;
        
        COMPILER_BARRIER();
    }
    
    /* Use result to prevent dead code elimination */
    volatile unsigned int *dummy = &result;
    (void)dummy;
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
    
    for (int i = 0; i < 50; ++i) {
        /* Read bitfields - should generate ZERO_EXTRACT */
        results[0] = s.field1;
        results[1] = s.field2;
        results[2] = s.field3;
        results[3] = s.field4;
        
        /* Write with arithmetic - complex pattern */
        s.field1 = (s.field2 + i) & 0x1F;
        s.field3 = (s.field4 - s.field1) & 0x7F;
        
        /* Cross-field operations */
        unsigned int temp = s.field1 | (s.field2 << 5);
        s.field4 = temp & 0x1FF;
        
        COMPILER_BARRIER();
    }
}

/* Test 3: Inline assembly for STRICT_LOW_PART */
void test_strict_low_part_asm(void) {
    volatile unsigned int value = 0x12345678;
    volatile unsigned int result = 0;
    
    for (int i = 0; i < 100; ++i) {
        /* Inline assembly that modifies only low bits */
        unsigned int temp = value;
        
        /* Mask low byte */
        asm volatile (
            "and %0, %1, %2"
            : "=r"(temp)
            : "r"(temp), "i"(0xFF)
        );
        
        result = temp;
        
        /* Another pattern with low 16 bits */
        temp = value;
        asm volatile (
            "and %0, %1, %2"
            : "=r"(temp)
            : "r"(temp), "i"(0xFFFF)
        );
        
        result ^= temp;
        
        /* Conditional assembly */
        if (i & 1) {
            asm volatile (
                "and %0, %1, %2"
                : "=r"(temp)
                : "r"(value), "i"(0x0F)
            );
            result += temp;
        }
        
        COMPILER_BARRIER();
    }
}

/* Test 4: Conditional merge operations for STRICT_LOW_PART */
void test_strict_low_part_merge(void) {
    volatile unsigned int var = 0x87654321;
    volatile unsigned char *byte_ptr = (volatile unsigned char *)&var;
    volatile int condition = 1;
    
    for (int i = 0; i < 100; ++i) {
        /* Conditional update of low byte */
        if (condition) {
            /* This pattern often generates STRICT_LOW_PART */
            var = (var & ~0xFF) | ((i & 0xFF) << 0);
        }
        
        /* Update low 16 bits based on condition */
        condition = !condition;
        if (condition) {
            var = (var & ~0xFFFF) | ((i & 0xFFFF) << 0);
        }
        
        /* Partial store through char pointer */
        *byte_ptr = i & 0xFF;
        
        /* Complex conditional merge */
        unsigned int mask = (i % 2) ? 0xFF00FF00 : 0x00FF00FF;
        var = (var & mask) | ((~var) & ~mask);
        
        COMPILER_BARRIER();
    }
}

/* Test 5: Mixed operations with memory references */
void test_mixed_operations(void) {
    volatile unsigned int array[16];
    volatile unsigned int results = 0;
    
    /* Initialize array */
    for (int i = 0; i < 16; ++i) {
        array[i] = i * 0x11111111;
    }
    
    /* Use register variables for pressure */
    register unsigned int r1 asm ("r8") = 0;
    register unsigned int r2 asm ("r9") = 0;
    
    for (int i = 0; i < 100; ++i) {
        /* Memory access combined with bitfield extract */
        volatile unsigned int val = array[i % 16];
        
        /* Extract bits 8-15 */
        unsigned int extracted = (val >> 8) & 0xFF;
        
        /* Conditional store to low part */
        if (extracted > 0x80) {
            r1 = (r1 & ~0xFF) | (extracted & 0xFF);
        }
        
        /* Switch on bitfield value */
        switch (extracted & 0x3) {
            case 0:
                r2 = array[0] & 0xFFFF;
                break;
            case 1:
                r2 = (array[1] >> 16) & 0xFFFF;
                break;
            case 2:
                r2 = (array[2] >> 8) & 0xFF;
                break;
            default:
                r2 = array[3] & 0xFF;
                break;
        }
        
        /* Update array with merged value */
        array[i % 16] = (array[i % 16] & 0xFFFF0000) | (r2 & 0xFFFF);
        
        results += r1 + r2;
        COMPILER_BARRIER();
    }
    
    /* Force use of register variables */
    asm volatile ("" : : "r"(r1), "r"(r2));
}

/* Test 6: Nested loops with bitfield operations */
void test_nested_loops(void) {
    volatile struct {
        unsigned int a : 3;
        unsigned int b : 5;
        unsigned int c : 10;
        unsigned int d : 14;
    } bits = {0};
    
    volatile unsigned int counters[4] = {0};
    
    for (int i = 0; i < 10; ++i) {
        for (int j = 0; j < 10; ++j) {
            /* Complex bitfield calculations */
            bits.a = (i + j) & 0x7;
            bits.b = (i * j) & 0x1F;
            bits.c = (bits.a << 7) | bits.b;
            bits.d = bits.c * 3;
            
            /* Extract and accumulate */
            counters[0] += bits.a;
            counters[1] += bits.b;
            counters[2] += bits.c;
            counters[3] += bits.d;
            
            /* Conditional partial update */
            if ((i + j) & 1) {
                unsigned int temp = counters[0];
                temp = (temp & ~0x3FF) | (bits.d & 0x3FF);
                counters[0] = temp;
            }
        }
        COMPILER_BARRIER();
    }
}

/* Main driver */
int main(int argc, char *argv[]) {
    int test_to_run = 0;
    
    /* Parse command line argument */
    if (argc > 1) {
        test_to_run = atoi(argv[1]);
    }
    
    volatile unsigned int total_result = 0;
    
    /* Run selected test or all tests */
    if (test_to_run == 0 || test_to_run == 1) {
        test_zero_extract_volatile();
        total_result += 1;
    }
    
    if (test_to_run == 0 || test_to_run == 2) {
        test_zero_extract_struct();
        total_result += 2;
    }
    
    if (test_to_run == 0 || test_to_run == 3) {
        test_strict_low_part_asm();
        total_result += 3;
    }
    
    if (test_to_run == 0 || test_to_run == 4) {
        test_strict_low_part_merge();
        total_result += 4;
    }
    
    if (test_to_run == 0 || test_to_run == 5) {
        test_mixed_operations();
        total_result += 5;
    }
    
    if (test_to_run == 0 || test_to_run == 6) {
        test_nested_loops();
        total_result += 6;
    }
    
    /* Print result to prevent optimization */
    printf("Test result: %u\n", total_result);
    
    return 0;
}
