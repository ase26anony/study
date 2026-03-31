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
    
    /* Multiple bitfield extractions that should generate ZERO_EXTRACT */
    for (int i = 0; i < 100; ++i) {
        /* Extract bits 4-11 */
        unsigned int field1 = (source >> 4) & ((1U << 8) - 1);
        
        /* Extract bits 12-19 with arithmetic */
        unsigned int field2 = ((source >> 12) + i) & ((1U << 8) - 1);
        
        /* Extract bits 20-27, combine with previous */
        unsigned int field3 = (source >> 20) & ((1U << 8) - 1);
        
        result = field1 + field2 * field3;
        COMPILER_BARRIER();
    }
    
    /* Force use of result */
    volatile unsigned int sink = result;
    (void)sink;
}

/* Test 2: Packed struct with bitfields */
struct __attribute__((packed)) packed_bitfields {
    unsigned int header : 4;
    unsigned int data   : 20;
    unsigned int footer : 8;
};

void test_zero_extract_struct(void) {
    volatile struct packed_bitfields s = {0};
    volatile unsigned int results[4] = {0};
    
    /* Initialize with pattern */
    s.header = 0xA;
    s.data = 0x12345;
    s.footer = 0xBC;
    
    for (int i = 0; i < 50; ++i) {
        /* Read bitfields - should generate ZERO_EXTRACT */
        unsigned int h = s.header;
        unsigned int d = s.data;
        unsigned int f = s.footer;
        
        /* Write back with transformation - may generate both read and write extracts */
        s.data = (d + h) & 0xFFFFF;
        s.footer = (f ^ i) & 0xFF;
        
        results[i % 4] = h + d + f;
        COMPILER_BARRIER();
    }
    
    /* Complex bitfield manipulation */
    struct __attribute__((packed)) nested_fields {
        unsigned int a : 3;
        unsigned int b : 7;
        unsigned int c : 10;
        unsigned int d : 12;
    } n = {0};
    
    n.a = 5;
    n.b = 0x7F;
    n.c = (n.a << 7) | n.b;
    n.d = n.c ^ 0x3FF;
    
    volatile unsigned int n_sum = n.a + n.b + n.c + n.d;
    (void)n_sum;
}

/* Test 3: STRICT_LOW_PART via conditional narrow stores */
void test_strict_low_part_conditional(void) {
    volatile unsigned int value = 0x12345678;
    volatile unsigned int temp;
    
    for (int i = 0; i < 100; ++i) {
        /* Conditional update of low byte - may generate STRICT_LOW_PART */
        if (i & 1) {
            /* Update only low 8 bits */
            value = (value & ~0xFF) | ((i + 0xAB) & 0xFF);
        }
        
        /* Update low 16 bits based on condition */
        if (i % 3 == 0) {
            value = (value & ~0xFFFF) | ((value + 0x1111) & 0xFFFF);
        }
        
        /* Complex conditional merge */
        temp = value;
        if (i % 5 == 0) {
            /* Only modify bits 8-15 */
            unsigned int middle = (temp >> 8) & 0xFF;
            middle = (middle * 3) & 0xFF;
            temp = (temp & ~0xFF00) | (middle << 8);
            value = temp;
        }
        
        COMPILER_BARRIER();
    }
    
    volatile unsigned int sink = value;
    (void)sink;
}

/* Test 4: Inline assembly for partial register updates */
void test_strict_low_part_asm(void) {
    register unsigned int r1 asm("r8") = 0x87654321;
    register unsigned int r2 asm("r9") = 0;
    volatile unsigned int output;
    
    for (int i = 0; i < 50; ++i) {
        /* Inline asm that operates on partial register - may hint at STRICT_LOW_PART */
        asm volatile (
            "and %0, %1, %2\n\t"
            "orr %0, %0, %3"
            : "=r" (r2)
            : "r" (r1), "i" (0x0000FFFF), "i" (i << 16)
            : /* No clobbers */
        );
        
        /* Another partial update pattern */
        asm volatile (
            "bfi %0, %1, #0, #8"
            : "+r" (r1)
            : "r" (i)
            : /* No clobbers */
        );
        
        r1 = (r1 & ~0xFF00) | ((r2 & 0xFF) << 8);
        COMPILER_BARRIER();
    }
    
    output = r1 + r2;
    (void)output;
}

/* Test 5: Mixed operations with memory references */
void test_mixed_operations(void) {
    volatile unsigned int array[16];
    volatile unsigned int accum = 0;
    
    /* Initialize array */
    for (int i = 0; i < 16; ++i) {
        array[i] = 0x100 * i + 0xAB;
    }
    
    /* Mixed bitfield and memory operations */
    for (int i = 0; i < 100; ++i) {
        /* Bitfield extraction from memory value */
        unsigned int val = array[i % 16];
        unsigned int low_bits = val & 0xF;
        unsigned int high_bits = (val >> 28) & 0xF;
        
        /* Conditional partial store back to memory */
        if (low_bits > high_bits) {
            /* Update only middle 16 bits */
            array[i % 16] = (val & ~0x00FFFF00) | 
                           (((val & 0x00FFFF00) + 0x1111) & 0x00FFFF00);
        }
        
        /* Switch based on bitfield to create control flow */
        switch (low_bits & 0x7) {
            case 0: accum += val; break;
            case 1: accum += val >> 4; break;
            case 2: accum += (val >> 8) & 0xFF; break;
            case 3: accum += (val >> 16) & 0xFFFF; break;
            default: accum ^= val; break;
        }
        
        COMPILER_BARRIER();
    }
    
    volatile unsigned int sink = accum;
    (void)sink;
}

/* Main driver */
int main(int argc, char *argv[]) {
    int test_to_run = 0;
    
    /* Parse command line argument if provided */
    if (argc > 1) {
        test_to_run = atoi(argv[1]);
    }
    
    volatile unsigned int final_result = 0;
    
    /* Run selected tests or all tests */
    if (test_to_run == 0 || test_to_run == 1) {
        test_zero_extract_volatile();
        final_result += 1;
    }
    
    if (test_to_run == 0 || test_to_run == 2) {
        test_zero_extract_struct();
        final_result += 2;
    }
    
    if (test_to_run == 0 || test_to_run == 3) {
        test_strict_low_part_conditional();
        final_result += 4;
    }
    
    if (test_to_run == 0 || test_to_run == 4) {
        test_strict_low_part_asm();
        final_result += 8;
    }
    
    if (test_to_run == 0 || test_to_run == 5) {
        test_mixed_operations();
        final_result += 16;
    }
    
    /* Print result to prevent dead code elimination */
    printf("Test result: %u\n", final_result);
    
    return 0;
}
