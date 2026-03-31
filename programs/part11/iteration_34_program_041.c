/* test_resource.c - Test program to cover ZERO_EXTRACT and STRICT_LOW_PART handling in GCC's resource.cc */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Compiler barrier to prevent optimization */
#define COMPILER_BARRIER() asm volatile("" ::: "memory")

/* ========== Test 1: ZERO_EXTRACT from volatile integer ========== */
volatile unsigned int test1_result = 0;

void test1_zero_extract_volatile(void) {
    volatile unsigned int source = 0xDEADBEEF;
    volatile unsigned int output = 0;
    
    /* This should generate ZERO_EXTRACT RTL */
    for (int i = 0; i < 100; ++i) {
        /* Extract bits 8-15 (byte 1) */
        unsigned int extracted = (source >> 8) & 0xFF;
        output ^= extracted;  /* XOR to prevent optimization */
        
        /* Extract bits 16-23 (byte 2) with variable width */
        unsigned int width = 6;
        unsigned int mask = (1U << width) - 1;
        extracted = (source >> 16) & mask;
        output += extracted;
        
        /* Extract bits 4-11 */
        extracted = (source >> 4) & 0xFF;
        output |= extracted;
        
        COMPILER_BARRIER();
    }
    
    test1_result = output;
    COMPILER_BARRIER();
}

/* ========== Test 2: Packed struct with bitfields ========== */
struct packed_bitfields {
    unsigned int field_a : 5;
    unsigned int field_b : 11;
    unsigned int field_c : 7;
    unsigned int field_d : 9;
} __attribute__((packed));

volatile int test2_result = 0;

void test2_packed_struct_bitfields(void) {
    volatile struct packed_bitfields s = {0};
    volatile int sum = 0;
    
    /* Initialize with pattern */
    s.field_a = 0x1F;  /* Max for 5 bits */
    s.field_b = 0x7FF; /* Max for 11 bits */
    s.field_c = 0x7F;  /* Max for 7 bits */
    s.field_d = 0x1FF; /* Max for 9 bits */
    
    for (int i = 0; i < 50; ++i) {
        /* Read bitfields - should generate ZERO_EXTRACT */
        int val_a = s.field_a;
        int val_b = s.field_b;
        int val_c = s.field_c;
        int val_d = s.field_d;
        
        /* Complex bitfield operations */
        s.field_a = (val_b + i) & 0x1F;  /* 5-bit result */
        s.field_c = (val_d ^ val_a) & 0x7F;  /* 7-bit result */
        
        sum += val_a + val_b + val_c + val_d;
        
        COMPILER_BARRIER();
    }
    
    test2_result = sum;
    COMPILER_BARRIER();
}

/* ========== Test 3: STRICT_LOW_PART via conditional merge ========== */
volatile unsigned int test3_result = 0;

void test3_strict_low_part_conditional(void) {
    volatile unsigned int reg = 0x12345678;
    volatile unsigned int temp;
    
    for (int i = 0; i < 100; ++i) {
        volatile int condition = (i % 3) == 0;
        
        if (condition) {
            /* This pattern often generates STRICT_LOW_PART:
               Update only low 8 bits, preserving high bits */
            unsigned char new_byte = (i & 0xFF);
            reg = (reg & ~0xFF) | (new_byte & 0xFF);
        } else if ((i % 5) == 0) {
            /* Update only low 16 bits */
            unsigned short new_word = (i & 0xFFFF);
            reg = (reg & ~0xFFFF) | (new_word & 0xFFFF);
        }
        
        /* Another STRICT_LOW_PART pattern */
        temp = (reg & ~0xF) | ((reg + i) & 0xF);
        
        COMPILER_BARRIER();
    }
    
    test3_result = reg ^ temp;
    COMPILER_BARRIER();
}

/* ========== Test 4: Inline assembly for partial register updates ========== */
volatile unsigned int test4_result = 0;

void test4_inline_asm_partial_store(void) {
    volatile unsigned int value = 0x87654321;
    volatile unsigned int output = 0;
    
    for (int i = 0; i < 75; ++i) {
        unsigned int temp;
        
        /* Inline asm that operates on partial register */
        asm volatile (
            "and %0, %1, %2\n\t"
            : "=r" (temp)
            : "r" (value), "i" (0xFF)  /* Only keep low byte */
        );
        
        output += temp;
        
        /* Another asm pattern that might generate STRICT_LOW_PART */
        asm volatile (
            "orr %0, %1, %2\n\t"
            : "=r" (temp)
            : "r" (value & ~0xFF00), "r" ((i << 8) & 0xFF00)
        );
        
        value = temp ^ (i * 0x10001);
        
        COMPILER_BARRIER();
    }
    
    test4_result = output;
    COMPILER_BARRIER();
}

/* ========== Test 5: Mixed operations with memory and control flow ========== */
volatile unsigned int test5_result = 0;

void test5_mixed_operations(void) {
    volatile unsigned int array[16];
    volatile struct packed_bitfields struct_array[4];
    
    /* Initialize arrays */
    for (int i = 0; i < 16; ++i) {
        array[i] = i * 0x11111111;
    }
    
    for (int i = 0; i < 4; ++i) {
        struct_array[i].field_a = i;
        struct_array[i].field_b = i * 3;
        struct_array[i].field_c = i * 5;
        struct_array[i].field_d = i * 7;
    }
    
    register unsigned int reg_sum = 0;  /* Encourage register allocation */
    
    for (int i = 0; i < 100; ++i) {
        /* Memory operations that may generate MEM_P references */
        unsigned int idx = i & 0xF;
        volatile unsigned int *ptr = &array[idx];
        
        /* Bitfield extraction from struct array */
        int bf_val = struct_array[idx & 0x3].field_b;
        
        /* Switch based on extracted bitfield */
        switch (bf_val & 0x7) {  /* Use low 3 bits */
            case 0:
                /* Update low byte of memory */
                *ptr = (*ptr & ~0xFF) | ((i + bf_val) & 0xFF);
                break;
            case 1:
                /* Extract and store bits 8-15 */
                reg_sum += (*ptr >> 8) & 0xFF;
                break;
            case 2:
                /* Complex bitfield update */
                struct_array[0].field_a = 
                    (struct_array[1].field_c + struct_array[2].field_d) & 0x1F;
                break;
            default:
                /* Partial update with conditional */
                if (i % 2) {
                    *ptr = (*ptr & ~0xFFFF0000) | ((i << 16) & 0xFFFF0000);
                }
                break;
        }
        
        /* Pointer arithmetic and partial store */
        volatile unsigned char *byte_ptr = (volatile unsigned char *)ptr;
        byte_ptr[1] = (i >> 1) & 0xFF;  /* May generate partial store RTL */
        
        COMPILER_BARRIER();
    }
    
    test5_result = reg_sum + array[0] + array[15];
    COMPILER_BARRIER();
}

/* ========== Main driver ========== */
int main(int argc, char *argv[]) {
    unsigned int total_result = 0;
    
    /* Run tests based on command line arguments */
    int run_all = (argc <= 1);  /* Run all if no arguments */
    
    if (run_all || strstr(argv[0], "test1") || (argc > 1 && atoi(argv[1]) == 1)) {
        test1_zero_extract_volatile();
        total_result += test1_result;
        printf("Test1 completed: result = %u\n", test1_result);
    }
    
    if (run_all || strstr(argv[0], "test2") || (argc > 1 && atoi(argv[1]) == 2)) {
        test2_packed_struct_bitfields();
        total_result += test2_result;
        printf("Test2 completed: result = %d\n", test2_result);
    }
    
    if (run_all || strstr(argv[0], "test3") || (argc > 1 && atoi(argv[1]) == 3)) {
        test3_strict_low_part_conditional();
        total_result += test3_result;
        printf("Test3 completed: result = %u\n", test3_result);
    }
    
    if (run_all || strstr(argv[0], "test4") || (argc > 1 && atoi(argv[1]) == 4)) {
        test4_inline_asm_partial_store();
        total_result += test4_result;
        printf("Test4 completed: result = %u\n", test4_result);
    }
    
    if (run_all || strstr(argv[0], "test5") || (argc > 1 && atoi(argv[1]) == 5)) {
        test5_mixed_operations();
        total_result += test5_result;
        printf("Test5 completed: result = %u\n", test5_result);
    }
    
    /* Final aggregate to prevent dead code elimination */
    printf("Total result: %u\n", total_result);
    
    return (total_result > 0) ? 0 : 1;
}
