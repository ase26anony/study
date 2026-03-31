/* test_resource.c - Generate RTL patterns for ZERO_EXTRACT and STRICT_LOW_PART coverage */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Compiler barrier to prevent optimization */
#define COMPILER_BARRIER() asm volatile("" ::: "memory")

/* Test 1: Bitfield extraction from volatile integer */
void test_zero_extract_volatile(void) {
    volatile unsigned int source = 0x12345678;
    volatile unsigned int result = 0;
    
    /* This should generate ZERO_EXTRACT RTL */
    for (int i = 0; i < 100; ++i) {
        /* Extract bits 8-15 (8 bits starting at bit 8) */
        unsigned int extracted = (source >> 8) & 0xFF;
        result += extracted;
        
        /* Extract bits 4-10 (7 bits starting at bit 4) */
        extracted = (source >> 4) & ((1U << 7) - 1);
        result ^= extracted;
        
        /* Extract bits 16-23 with variable width */
        int width = 8;
        extracted = (source >> 16) & ((1U << width) - 1);
        result |= extracted;
        
        COMPILER_BARRIER();
    }
    
    /* Use result to prevent dead code elimination */
    volatile unsigned int sink __attribute__((unused)) = result;
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
        
        /* Complex bitfield operation */
        s.field1 = (s.field2 + s.field3) & 0x1F;
        s.field4 = (s.field1 ^ s.field3) & 0x1FF;
        
        accumulator += val1 + val2 + val3 + val4;
        COMPILER_BARRIER();
    }
    
    volatile unsigned int sink __attribute__((unused)) = accumulator;
}

/* Test 3: Inline assembly for partial register updates (STRICT_LOW_PART) */
void test_strict_low_part_asm(void) {
    volatile unsigned int reg = 0x12345678;
    volatile unsigned int result = 0;
    
    for (int i = 0; i < 75; ++i) {
        unsigned int temp = reg;
        
        /* Inline assembly that modifies only low bits */
        asm volatile (
            "and %0, %1, %2\n\t"
            "orr %0, %0, %3"
            : "=r"(temp)
            : "r"(temp), "i"(0xFFFFFF00), "i"(0x000000AA)
            : /* No clobbers */
        );
        
        /* Another pattern using byte operations */
        unsigned char low_byte;
        asm volatile (
            "and %0, %1, #0xFF"
            : "=r"(low_byte)
            : "r"(reg)
        );
        
        result += temp + low_byte;
        reg = (reg * 1103515245 + 12345) & 0x7FFFFFFF; /* PRNG to vary value */
        COMPILER_BARRIER();
    }
    
    volatile unsigned int sink __attribute__((unused)) = result;
}

/* Test 4: Conditional merge operations for STRICT_LOW_PART */
void test_strict_low_part_conditional(void) {
    volatile unsigned int value = 0xDEADBEEF;
    volatile unsigned int mask = 0x0000FFFF;
    volatile int condition = 1;
    volatile unsigned int result = 0;
    
    for (int i = 0; i < 60; ++i) {
        unsigned int new_low = i * 0x1111;
        
        /* Conditional update of low 16 bits */
        if (condition || (i % 3 == 0)) {
            /* This pattern often generates STRICT_LOW_PART */
            value = (value & ~mask) | (new_low & mask);
        }
        
        /* Update only low byte based on condition */
        if (i % 2 == 0) {
            unsigned char *byte_ptr = (unsigned char *)&value;
            byte_ptr[0] = (i & 0xFF);
        }
        
        /* Switch on low bits to create control flow */
        switch (value & 0xF) {
            case 0: result += 1; break;
            case 1: result += 2; break;
            case 2: result += 3; break;
            default: result += 4; break;
        }
        
        condition = !condition;
        COMPILER_BARRIER();
    }
    
    volatile unsigned int sink __attribute__((unused)) = result;
}

/* Test 5: Mixed operations with memory references */
void test_mixed_operations(void) {
    volatile unsigned int array[16];
    volatile struct packed_bitfields struct_array[8];
    volatile unsigned int total = 0;
    
    /* Initialize arrays */
    for (int i = 0; i < 16; i++) {
        array[i] = i * 0x11111111;
    }
    for (int i = 0; i < 8; i++) {
        struct_array[i].field1 = i & 0x1F;
        struct_array[i].field2 = (i * 17) & 0x7FF;
        struct_array[i].field3 = (i * 23) & 0x7F;
        struct_array[i].field4 = (i * 29) & 0x1FF;
    }
    
    register unsigned int reg_acc __asm__("r12") = 0; /* Suggest register */
    
    for (int i = 0; i < 40; ++i) {
        /* Memory access that may generate MEM_P references */
        unsigned int idx = i & 0xF;
        unsigned int mem_val = array[idx];
        
        /* Bitfield extraction from struct array */
        unsigned int bf_val = struct_array[i & 0x7].field2;
        
        /* Partial update of memory location */
        array[idx] = (array[idx] & 0xFFFF0000) | (bf_val & 0xFFFF);
        
        /* Complex expression mixing operations */
        reg_acc = ((mem_val >> 8) & 0xFF) + (bf_val & 0x3F);
        
        /* Conditional store with partial update */
        if (reg_acc > 100) {
            struct_array[i & 0x7].field1 = reg_acc & 0x1F;
        }
        
        total += reg_acc + mem_val;
        COMPILER_BARRIER();
    }
    
    volatile unsigned int sink __attribute__((unused)) = total + reg_acc;
}

/* Main driver */
int main(int argc, char *argv[]) {
    volatile unsigned int final_result = 0;
    
    /* Run tests based on command line or all by default */
    int run_all = (argc == 1);
    
    if (run_all || strstr(argv[0], "test1") || (argc > 1 && atoi(argv[1]) == 1)) {
        test_zero_extract_volatile();
        final_result += 1;
    }
    
    if (run_all || strstr(argv[0], "test2") || (argc > 1 && atoi(argv[1]) == 2)) {
        test_zero_extract_struct();
        final_result += 2;
    }
    
    if (run_all || strstr(argv[0], "test3") || (argc > 1 && atoi(argv[1]) == 3)) {
        test_strict_low_part_asm();
        final_result += 3;
    }
    
    if (run_all || strstr(argv[0], "test4") || (argc > 1 && atoi(argv[1]) == 4)) {
        test_strict_low_part_conditional();
        final_result += 4;
    }
    
    if (run_all || strstr(argv[0], "test5") || (argc > 1 && atoi(argv[1]) == 5)) {
        test_mixed_operations();
        final_result += 5;
    }
    
    /* Print result to prevent optimization */
    printf("Test result indicator: %u\n", final_result);
    
    return 0;
}
