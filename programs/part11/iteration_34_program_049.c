/* test_resource.c - Test program for GCC RTL resource.cc coverage */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Compiler barriers to prevent optimization */
#define COMPILER_BARRIER() asm volatile("" ::: "memory")

/* Test 1: Bitfield extraction from volatile integers */
void test_zero_extract_volatile(void) {
    volatile unsigned int source = 0x89ABCDEF;
    volatile unsigned int result = 0;
    
    /* Multiple extraction patterns that should generate ZERO_EXTRACT */
    for (int i = 0; i < 100; ++i) {
        /* Extract 5 bits starting at position 3 */
        unsigned int extract1 = (source >> 3) & ((1U << 5) - 1);
        
        /* Extract 11 bits starting at position 8 */
        unsigned int extract2 = (source >> 8) & ((1U << 11) - 1);
        
        /* Extract 7 bits starting at position 16 */
        unsigned int extract3 = (source >> 16) & ((1U << 7) - 1);
        
        /* Combine extractions with arithmetic */
        result = extract1 + (extract2 << 5) | (extract3 << 16);
        
        /* Modify source to create variation */
        source = (source * 1103515245U + 12345U) & 0xFFFFFFFFU;
        
        COMPILER_BARRIER();
    }
    
    /* Prevent dead code elimination */
    volatile unsigned int *dummy = &result;
    (void)dummy;
}

/* Test 2: Packed structs with bitfields */
struct packed_bitfields {
    unsigned int field_a : 5;
    unsigned int field_b : 11;
    unsigned int field_c : 7;
    unsigned int field_d : 9;
} __attribute__((packed));

void test_zero_extract_struct(void) {
    volatile struct packed_bitfields s = {0};
    volatile unsigned int accumulator = 0;
    
    /* Initialize with pattern */
    s.field_a = 0x1F;
    s.field_b = 0x7FF;
    s.field_c = 0x7F;
    s.field_d = 0x1FF;
    
    for (int i = 0; i < 100; ++i) {
        /* Read bitfields - should generate ZERO_EXTRACT */
        unsigned int val_a = s.field_a;
        unsigned int val_b = s.field_b;
        unsigned int val_c = s.field_c;
        unsigned int val_d = s.field_d;
        
        /* Write bitfields with arithmetic */
        s.field_a = (val_b + i) & 0x1F;
        s.field_c = (val_d ^ val_a) & 0x7F;
        
        /* Complex bitfield expression */
        s.field_b = ((val_a << 3) | (val_c >> 2)) & 0x7FF;
        
        accumulator += val_a + val_b + val_c + val_d;
        
        COMPILER_BARRIER();
    }
    
    volatile unsigned int *dummy = &accumulator;
    (void)dummy;
}

/* Test 3: Inline assembly for STRICT_LOW_PART */
void test_strict_low_part_asm(void) {
    volatile unsigned int reg = 0x12345678;
    volatile unsigned int result = 0;
    
    for (int i = 0; i < 100; ++i) {
        /* Inline assembly that modifies only low bits */
        unsigned int temp = reg;
        
        /* Clear low byte and set new value */
        asm volatile (
            "and %0, %1, %2\n\t"
            "orr %0, %0, %3"
            : "=r" (temp)
            : "r" (temp), "i" (~0xFFU), "r" ((i & 0xFFU))
            : "cc"
        );
        
        /* Another pattern: extract low 16 bits */
        unsigned short low_half;
        asm volatile (
            "uxth %0, %1"
            : "=r" (low_half)
            : "r" (temp)
        );
        
        result = temp + low_half;
        reg = result ^ 0x87654321;
        
        COMPILER_BARRIER();
    }
    
    volatile unsigned int *dummy = &result;
    (void)dummy;
}

/* Test 4: Conditional merge operations for STRICT_LOW_PART */
void test_strict_low_part_conditional(void) {
    volatile unsigned int value = 0xDEADBEEF;
    volatile unsigned int mask = 0;
    volatile unsigned int result = 0;
    
    for (int i = 0; i < 100; ++i) {
        /* Conditional update of low byte */
        if (i & 1) {
            /* This pattern often generates STRICT_LOW_PART */
            value = (value & ~0xFFU) | ((i * 7) & 0xFFU);
        }
        
        /* Conditional update of low 16 bits */
        if (i & 2) {
            value = (value & ~0xFFFFU) | ((i * 13) & 0xFFFFU);
        }
        
        /* Merge operation with mask */
        mask = (mask << 1) | 1;
        unsigned int new_val = i * 17;
        value = (value & ~mask) | (new_val & mask);
        
        result += value;
        
        COMPILER_BARRIER();
    }
    
    volatile unsigned int *dummy = &result;
    (void)dummy;
}

/* Test 5: Mixed operations with memory references */
void test_mixed_operations(void) {
    volatile unsigned int array[256];
    volatile unsigned int index = 0;
    
    /* Initialize array */
    for (int i = 0; i < 256; ++i) {
        array[i] = i * 0x01010101U;
    }
    
    /* Packed struct on stack */
    struct {
        unsigned int low : 12;
        unsigned int high : 20;
    } __attribute__((packed)) bitfield;
    
    volatile unsigned int sum = 0;
    
    for (int i = 0; i < 100; ++i) {
        /* Memory load with bitfield extraction */
        unsigned int mem_val = array[index & 0xFF];
        
        /* Extract parts using bitfield operations */
        bitfield.low = mem_val & 0xFFF;
        bitfield.high = (mem_val >> 12) & 0xFFFFF;
        
        /* Conditional partial store */
        if (mem_val & 1) {
            /* Update only low 12 bits */
            unsigned int temp = array[(index + 1) & 0xFF];
            temp = (temp & ~0xFFFU) | (bitfield.low & 0xFFFU);
            array[(index + 1) & 0xFF] = temp;
        }
        
        /* Switch based on extracted bitfield */
        switch (bitfield.low & 0x7) {
            case 0: sum += mem_val; break;
            case 1: sum += bitfield.high; break;
            case 2: sum += bitfield.low << 1; break;
            case 3: sum += bitfield.high >> 1; break;
            case 4: sum += mem_val ^ bitfield.low; break;
            case 5: sum += mem_val | bitfield.high; break;
            case 6: sum += mem_val & bitfield.low; break;
            default: sum += i; break;
        }
        
        index = (index * 1103515245U + 12345U) & 0xFFFFFFFFU;
        
        COMPILER_BARRIER();
    }
    
    volatile unsigned int *dummy = &sum;
    (void)dummy;
}

/* Test 6: Pointer-based partial updates */
void test_pointer_partial_updates(void) {
    volatile unsigned int data = 0x12345678;
    volatile unsigned char *byte_ptr = (volatile unsigned char *)&data;
    volatile unsigned int result = 0;
    
    for (int i = 0; i < 100; ++i) {
        /* Partial store through byte pointer */
        byte_ptr[i % 4] = (i * 3) & 0xFF;
        
        /* Extract and manipulate */
        unsigned int low_word = data & 0xFFFF;
        unsigned int high_word = data >> 16;
        
        /* Conditional merge */
        if (low_word > high_word) {
            data = (data & ~0xFFFFU) | ((high_word + 1) & 0xFFFFU);
        }
        
        /* Complex bitfield-like operation */
        unsigned int rotated = (data << 8) | (data >> 24);
        unsigned int extracted = (rotated >> 4) & 0x0FFFFFF0U;
        
        result += extracted;
        
        COMPILER_BARRIER();
    }
    
    volatile unsigned int *dummy = &result;
    (void)dummy;
}

/* Main driver function */
int main(int argc, char *argv[]) {
    volatile unsigned int total = 0;
    
    /* Run selected tests based on command line */
    int run_all = (argc <= 1);
    
    if (run_all || strstr(argv[1], "1")) {
        test_zero_extract_volatile();
        total += 1;
    }
    
    if (run_all || strstr(argv[1], "2")) {
        test_zero_extract_struct();
        total += 2;
    }
    
    if (run_all || strstr(argv[1], "3")) {
        test_strict_low_part_asm();
        total += 3;
    }
    
    if (run_all || strstr(argv[1], "4")) {
        test_strict_low_part_conditional();
        total += 4;
    }
    
    if (run_all || strstr(argv[1], "5")) {
        test_mixed_operations();
        total += 5;
    }
    
    if (run_all || strstr(argv[1], "6")) {
        test_pointer_partial_updates();
        total += 6;
    }
    
    /* Print result to prevent optimization */
    printf("Test result: %u\n", total);
    
    return 0;
}
