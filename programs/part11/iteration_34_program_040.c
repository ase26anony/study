/* test_resource.c - Generate RTL patterns for ZERO_EXTRACT and STRICT_LOW_PART coverage */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Compiler barrier to prevent optimization */
#define COMPILER_BARRIER() asm volatile("" ::: "memory")

/* Test 1: Bitfield extraction from volatile integer */
void test_zero_extract_volatile(void) {
    volatile unsigned int source = 0xDEADBEEF;
    volatile unsigned int result = 0;
    
    /* Loop to increase scheduling opportunities */
    for (int i = 0; i < 100; ++i) {
        /* Extract bits 8-15 (8 bits starting at bit 8) */
        unsigned int mask = (1U << 8) - 1;
        unsigned int extracted = (source >> 8) & mask;
        
        /* Mix with arithmetic to create more complex RTL */
        result = extracted + (i & 0xF);
        
        /* Update source to vary pattern */
        source = source * 1103515245 + 12345;
        
        COMPILER_BARRIER();
    }
    
    /* Use result to prevent dead code elimination */
    volatile unsigned int sink __attribute__((unused)) = result;
}

/* Test 2: Packed struct with bitfields */
void test_zero_extract_packed_struct(void) {
    /* Packed struct with varying bitfield widths */
    struct __attribute__((packed)) bitfield_struct {
        unsigned int header : 4;
        unsigned int data_a : 11;
        unsigned int data_b : 9;
        unsigned int footer : 8;
    };
    
    volatile struct bitfield_struct bs;
    volatile unsigned int accumulator = 0;
    
    /* Initialize with pattern */
    bs.header = 0xA;
    bs.data_a = 0x7FF;  /* Max 11-bit value */
    bs.data_b = 0x1FF;  /* Max 9-bit value */
    bs.footer = 0x55;
    
    for (int i = 0; i < 50; ++i) {
        /* Complex bitfield operations that may generate ZERO_EXTRACT */
        unsigned int temp = bs.data_a;
        
        /* Conditional update mixing bitfields */
        if (i & 1) {
            bs.data_b = (bs.data_a + bs.footer) & 0x1FF;  /* Keep in 9 bits */
        } else {
            bs.data_a = (bs.data_b * 3) & 0x7FF;  /* Keep in 11 bits */
        }
        
        /* Extract and combine */
        accumulator = (bs.header << 24) | (bs.data_a << 13) | (bs.data_b << 4) | bs.footer;
        
        COMPILER_BARRIER();
    }
    
    volatile unsigned int sink __attribute__((unused)) = accumulator;
}

/* Test 3: Inline assembly for potential STRICT_LOW_PART */
void test_strict_low_part_asm(void) {
    volatile unsigned int value = 0x12345678;
    volatile unsigned int result = 0;
    
    for (int i = 0; i < 100; ++i) {
        /* Inline assembly that operates on low part of register */
        unsigned int temp = value;
        
        /* Assembly that might generate STRICT_LOW_PART */
        asm volatile (
            "and %0, %1, %2\n\t"
            "orr %0, %0, %3"
            : "=r" (temp)
            : "r" (temp), "i" (0x0000FFFF), "r" (i & 0xFFFF)
            : /* No clobbers */
        );
        
        result = temp;
        
        /* Update value */
        value = (value << 1) | ((value >> 31) & 1);  /* Rotate right */
        
        COMPILER_BARRIER();
    }
    
    volatile unsigned int sink __attribute__((unused)) = result;
}

/* Test 4: Conditional merge operations for STRICT_LOW_PART */
void test_strict_low_part_conditional(void) {
    volatile unsigned int reg = 0x87654321;
    volatile unsigned char flag = 0;
    volatile unsigned int results[10];
    
    for (int i = 0; i < 100; ++i) {
        flag = i & 0xFF;
        
        /* Conditional update of low byte - may generate STRICT_LOW_PART */
        if (flag > 128) {
            /* Update only low 8 bits */
            reg = (reg & ~0xFF) | (flag & 0xFF);
        } else if (flag > 64) {
            /* Update only low 16 bits */
            reg = (reg & ~0xFFFF) | ((flag * 257) & 0xFFFF);
        }
        
        /* Store intermediate results in array */
        results[i % 10] = reg;
        
        COMPILER_BARRIER();
    }
    
    /* Use results to prevent elimination */
    volatile unsigned int sum = 0;
    for (int i = 0; i < 10; ++i) {
        sum += results[i];
    }
    volatile unsigned int sink __attribute__((unused)) = sum;
}

/* Test 5: Mixed operations with memory references */
void test_mixed_operations(void) {
    /* Array for memory operations */
    volatile unsigned int mem_array[16];
    for (int i = 0; i < 16; ++i) {
        mem_array[i] = i * 0x11111111;
    }
    
    /* Packed struct for bitfields */
    struct __attribute__((packed)) mixed_struct {
        unsigned int low : 10;
        unsigned int mid : 12;
        unsigned int high : 10;
    };
    
    volatile struct mixed_struct ms;
    volatile unsigned int total = 0;
    
    /* Register variable to encourage register allocation */
    register unsigned int reg_var asm("r12") = 0;
    
    for (int i = 0; i < 100; ++i) {
        /* Bitfield extraction from struct */
        ms.low = i & 0x3FF;
        ms.mid = (i * 7) & 0xFFF;
        ms.high = (i * 13) & 0x3FF;
        
        /* Extract and combine */
        unsigned int extracted = (ms.mid << 10) | ms.low;
        
        /* Conditional partial update */
        if (extracted & 1) {
            reg_var = (reg_var & ~0x3FF) | (extracted & 0x3FF);
        }
        
        /* Memory operation */
        mem_array[i & 0xF] = reg_var + ms.high;
        
        /* Switch based on bitfield to create control flow */
        switch (ms.low & 0x7) {
            case 0: total += extracted; break;
            case 1: total += reg_var; break;
            case 2: total += ms.high; break;
            case 3: total += mem_array[0]; break;
            default: total += i; break;
        }
        
        COMPILER_BARRIER();
    }
    
    volatile unsigned int sink __attribute__((unused)) = total + reg_var;
}

/* Test 6: Pointer-based partial store */
void test_partial_store_pointer(void) {
    volatile unsigned int value = 0xABCD1234;
    volatile unsigned int *ptr = &value;
    volatile unsigned char *byte_ptr = (volatile unsigned char *)ptr;
    
    for (int i = 0; i < 100; ++i) {
        /* Store through char pointer - partial update */
        byte_ptr[i % 4] = (i * 17) & 0xFF;
        
        /* Also read through different-sized pointers */
        unsigned short *short_ptr = (unsigned short *)ptr;
        unsigned short low_half = short_ptr[0];
        unsigned short high_half = short_ptr[1];
        
        /* Combine with bitfield-like extraction */
        value = (value & 0xFFFF0000) | ((low_half + high_half) & 0xFFFF);
        
        COMPILER_BARRIER();
    }
    
    volatile unsigned int sink __attribute__((unused)) = value;
}

/* Main driver function */
int main(int argc, char *argv[]) {
    unsigned int test_mask = 0x3F;  /* Run all tests by default */
    
    /* Parse command line arguments */
    if (argc > 1) {
        test_mask = strtoul(argv[1], NULL, 0);
        if (test_mask == 0) test_mask = 0x3F;
    }
    
    volatile unsigned int final_result = 0;
    
    /* Execute selected tests */
    if (test_mask & 0x01) {
        test_zero_extract_volatile();
        final_result += 1;
    }
    if (test_mask & 0x02) {
        test_zero_extract_packed_struct();
        final_result += 2;
    }
    if (test_mask & 0x04) {
        test_strict_low_part_asm();
        final_result += 4;
    }
    if (test_mask & 0x08) {
        test_strict_low_part_conditional();
        final_result += 8;
    }
    if (test_mask & 0x10) {
        test_mixed_operations();
        final_result += 16;
    }
    if (test_mask & 0x20) {
        test_partial_store_pointer();
        final_result += 32;
    }
    
    /* Print result to prevent optimization */
    printf("Test result: %u\n", final_result);
    
    return 0;
}
