/* test_resource.c - Test program for GCC RTL resource.cc coverage */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Compiler barrier to prevent optimization */
#define COMPILER_BARRIER() asm volatile("" ::: "memory")

/* Test 1: Bitfield extraction from volatile integer */
void test_zero_extract_volatile(void) {
    volatile unsigned int source = 0x12345678;
    volatile unsigned int result = 0;
    
    /* Multiple extraction patterns that should generate ZERO_EXTRACT */
    for (int i = 0; i < 100; ++i) {
        /* Extract bits 4-11 */
        unsigned int extracted = (source >> 4) & 0xFF;
        result += extracted;
        
        /* Extract bits 8-15 with variable shift */
        extracted = (source >> (i % 8)) & ((1U << 8) - 1);
        result ^= extracted;
        
        /* Extract bits 16-23 and combine */
        extracted = (source >> 16) & 0xFF;
        result |= extracted;
        
        COMPILER_BARRIER();
    }
    
    volatile unsigned int sink = result;
    (void)sink;
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
    volatile unsigned int accumulator = 0;
    
    /* Initialize with pattern */
    s.field1 = 0x1F;
    s.field2 = 0x7FF;
    s.field3 = 0x7F;
    s.field4 = 0x1FF;
    
    for (int i = 0; i < 50; ++i) {
        /* Read bitfields - should generate ZERO_EXTRACT */
        unsigned int val1 = s.field1;
        unsigned int val2 = s.field2;
        unsigned int val3 = s.field3;
        unsigned int val4 = s.field4;
        
        /* Write back with transformation - may generate both ZERO_EXTRACT and STRICT_LOW_PART */
        s.field1 = (val2 + i) & 0x1F;
        s.field2 = (val3 ^ val4) & 0x7FF;
        s.field3 = (val1 * 2) & 0x7F;
        s.field4 = (val2 >> 3) & 0x1FF;
        
        accumulator += s.field1 + s.field2 + s.field3 + s.field4;
        COMPILER_BARRIER();
    }
    
    volatile unsigned int sink = accumulator;
    (void)sink;
}

/* Test 3: Inline assembly for partial register updates */
void test_strict_low_part_asm(void) {
    volatile unsigned int data = 0xDEADBEEF;
    volatile unsigned int result = 0;
    
    for (int i = 0; i < 75; ++i) {
        unsigned int temp = data;
        
        /* Inline assembly that modifies only low bits */
        asm volatile (
            "and %0, %1, %2\n\t"
            : "=r" (temp)
            : "r" (temp), "i" (0xFF)
        );
        
        /* Another pattern with different mask */
        asm volatile (
            "and %0, %1, %2\n\t"
            : "+r" (temp)
            : "i" (0xFFFF)
        );
        
        result += temp;
        data = (data << 1) | (data >> 31); /* Rotate */
        COMPILER_BARRIER();
    }
    
    volatile unsigned int sink = result;
    (void)sink;
}

/* Test 4: Conditional merge operations */
void test_strict_low_part_conditional(void) {
    volatile unsigned int reg = 0x12345678;
    volatile unsigned int updates[4] = {0xAA, 0xBB, 0xCC, 0xDD};
    volatile int conditions[4] = {1, 0, 1, 0};
    
    for (int i = 0; i < 100; ++i) {
        int idx = i & 3;
        
        /* Conditional update of low byte - may generate STRICT_LOW_PART */
        if (conditions[idx]) {
            reg = (reg & ~0xFF) | (updates[idx] & 0xFF);
        }
        
        /* Conditional update of low word */
        if (i % 3 == 0) {
            reg = (reg & ~0xFFFF) | ((updates[idx] * i) & 0xFFFF);
        }
        
        /* Update conditions array */
        conditions[idx] = !conditions[idx];
        COMPILER_BARRIER();
    }
    
    volatile unsigned int sink = reg;
    (void)sink;
}

/* Test 5: Mixed operations with memory references */
void test_mixed_operations(void) {
    volatile unsigned int array[16];
    volatile struct packed_bitfields struct_array[8];
    volatile unsigned int result = 0;
    
    /* Initialize arrays */
    for (int i = 0; i < 16; ++i) {
        array[i] = i * 0x11111111;
    }
    
    for (int i = 0; i < 8; ++i) {
        struct_array[i].field1 = i;
        struct_array[i].field2 = i * 2;
        struct_array[i].field3 = i * 3;
        struct_array[i].field4 = i * 4;
    }
    
    /* Complex loop with mixed operations */
    for (int i = 0; i < 50; ++i) {
        /* Array access with pointer arithmetic */
        volatile unsigned int* ptr = &array[i % 16];
        
        /* Bitfield extraction from struct array */
        unsigned int bf_val = struct_array[i % 8].field2;
        
        /* Conditional partial update of array element */
        if (bf_val & 1) {
            *ptr = (*ptr & ~0xFF00) | ((bf_val << 8) & 0xFF00);
        }
        
        /* Extract from array element */
        unsigned int extracted = (*ptr >> 16) & 0xFF;
        
        /* Update struct bitfield */
        struct_array[i % 8].field1 = extracted & 0x1F;
        
        result += *ptr + bf_val + extracted;
        COMPILER_BARRIER();
    }
    
    volatile unsigned int sink = result;
    (void)sink;
}

/* Test 6: Switch statement based on bitfield extraction */
void test_switch_with_bitfields(void) {
    volatile struct packed_bitfields s = {0};
    volatile unsigned int counter = 0;
    
    s.field1 = 5;
    s.field2 = 1023;
    s.field3 = 63;
    s.field4 = 255;
    
    for (int i = 0; i < 100; ++i) {
        /* Extract bitfield for switch */
        unsigned int selector = s.field1;
        
        switch (selector & 0x7) { /* Use low 3 bits */
            case 0:
                s.field2 = (s.field2 + 1) & 0x7FF;
                break;
            case 1:
                s.field3 = (s.field3 ^ s.field4) & 0x7F;
                break;
            case 2:
                s.field4 = (s.field4 >> 1) & 0x1FF;
                break;
            case 3:
                s.field1 = (s.field2 & 0x1F);
                break;
            case 4:
                s.field2 = (s.field3 << 2) & 0x7FF;
                break;
            case 5:
                s.field3 = (s.field4 + s.field1) & 0x7F;
                break;
            case 6:
                s.field4 = (s.field1 * 3) & 0x1FF;
                break;
            case 7:
                s.field1 = (s.field1 + 1) & 0x1F;
                break;
        }
        
        counter += s.field1 + s.field2 + s.field3 + s.field4;
        COMPILER_BARRIER();
    }
    
    volatile unsigned int sink = counter;
    (void)sink;
}

/* Main driver function */
int main(int argc, char *argv[]) {
    volatile unsigned int total = 0;
    
    /* Default: run all tests if no arguments */
    int run_all = (argc == 1);
    
    /* Run tests based on arguments or all by default */
    if (run_all || strstr(argv[0], "test1") || (argc > 1 && atoi(argv[1]) == 1)) {
        test_zero_extract_volatile();
        total += 1;
    }
    
    if (run_all || strstr(argv[0], "test2") || (argc > 1 && atoi(argv[1]) == 2)) {
        test_zero_extract_struct();
        total += 2;
    }
    
    if (run_all || strstr(argv[0], "test3") || (argc > 1 && atoi(argv[1]) == 3)) {
        test_strict_low_part_asm();
        total += 3;
    }
    
    if (run_all || strstr(argv[0], "test4") || (argc > 1 && atoi(argv[1]) == 4)) {
        test_strict_low_part_conditional();
        total += 4;
    }
    
    if (run_all || strstr(argv[0], "test5") || (argc > 1 && atoi(argv[1]) == 5)) {
        test_mixed_operations();
        total += 5;
    }
    
    if (run_all || strstr(argv[0], "test6") || (argc > 1 && atoi(argv[1]) == 6)) {
        test_switch_with_bitfields();
        total += 6;
    }
    
    /* Print result to prevent dead code elimination */
    printf("Test result indicator: %u\n", total);
    
    return 0;
}
