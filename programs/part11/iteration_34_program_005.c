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
    
    /* Multiple extraction patterns that should generate ZERO_EXTRACT */
    for (int i = 0; i < 100; ++i) {
        /* Extract bits 4-11 */
        unsigned int extracted = (source >> 4) & 0xFF;
        result += extracted;
        
        /* Extract bits 16-23 with variable shift */
        int shift = i % 16;
        extracted = (source >> shift) & 0xFF;
        result += extracted;
        
        /* Extract bits 8-15 and use in calculation */
        extracted = ((source >> 8) & 0xFF) + 1;
        result ^= extracted;
        
        COMPILER_BARRIER();
    }
    
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
        
        /* Write bitfields with computation */
        s.field1 = (val2 + i) & 0x1F;
        s.field2 = (val3 ^ val4) & 0x7FF;
        s.field3 = (val1 * 2) & 0x7F;
        s.field4 = (val2 >> 3) & 0x1FF;
        
        accumulator += val1 + val2 + val3 + val4;
        COMPILER_BARRIER();
    }
    
    volatile unsigned int sink __attribute__((unused)) = accumulator;
}

/* Test 3: STRICT_LOW_PART via conditional narrow stores */
void test_strict_low_part_conditional(void) {
    volatile unsigned int data = 0x12345678;
    volatile unsigned char flag = 1;
    volatile unsigned int result = 0;
    
    for (int i = 0; i < 100; ++i) {
        /* Conditional update of low byte - may generate STRICT_LOW_PART */
        if (flag) {
            /* Pattern: (data & ~mask) | (new_val & mask) */
            unsigned int new_low_byte = (i & 0xFF);
            data = (data & ~0xFF) | (new_low_byte & 0xFF);
        }
        
        /* Conditional update of low 16 bits */
        if (i % 3 == 0) {
            unsigned int new_low_word = (i * 7) & 0xFFFF;
            data = (data & ~0xFFFF) | (new_low_word & 0xFFFF);
        }
        
        /* Update flag based on data */
        flag = (data & 0x80) ? 1 : 0;
        result += data;
        COMPILER_BARRIER();
    }
    
    volatile unsigned int sink __attribute__((unused)) = result;
}

/* Test 4: STRICT_LOW_PART via inline assembly with constraints */
void test_strict_low_part_asm(void) {
    volatile unsigned int in_val = 0x87654321;
    volatile unsigned int out_val = 0;
    
    for (int i = 0; i < 50; ++i) {
        /* Inline assembly that operates on low parts */
        unsigned int temp = in_val + i;
        
        /* Assembly that might generate partial register updates */
        asm volatile (
            "and %0, %1, %2\n\t"
            "orr %0, %0, %3"
            : "=r" (temp)
            : "r" (temp), "i" (0xFFFFFF00), "r" (i & 0xFF)
            : /* no clobber */
        );
        
        /* Another pattern using byte operations */
        unsigned char low_byte = (temp & 0xFF);
        asm volatile (
            "add %0, %0, %1"
            : "+r" (low_byte)
            : "r" (i & 0x7F)
            : /* no clobber */
        );
        
        out_val = (out_val << 1) | (low_byte & 1);
        COMPILER_BARRIER();
    }
    
    volatile unsigned int sink __attribute__((unused)) = out_val;
}

/* Test 5: Mixed operations with memory references */
void test_mixed_operations(void) {
    volatile unsigned int array[64];
    volatile struct packed_bitfields struct_array[16];
    volatile unsigned int sum = 0;
    
    /* Initialize arrays */
    for (int i = 0; i < 64; ++i) {
        array[i] = i * 0x01010101;
    }
    for (int i = 0; i < 16; ++i) {
        struct_array[i].field1 = i & 0x1F;
        struct_array[i].field2 = (i * 17) & 0x7FF;
        struct_array[i].field3 = (i * 3) & 0x7F;
        struct_array[i].field4 = (i * 5) & 0x1FF;
    }
    
    /* Complex loop with mixed operations */
    for (int i = 0; i < 32; ++i) {
        /* Memory access that creates MEM_P references */
        unsigned int mem_val = array[i];
        
        /* Bitfield extraction from struct array */
        unsigned int bf_val = struct_array[i % 16].field2;
        
        /* Conditional partial update */
        if (mem_val & 1) {
            array[i] = (array[i] & ~0xFFFF) | (bf_val & 0xFFFF);
        }
        
        /* Extract and manipulate bits */
        unsigned int extracted = (mem_val >> 8) & 0xFF;
        extracted = (extracted * bf_val) & 0xFF;
        
        /* Update struct bitfield */
        struct_array[i % 16].field1 = extracted & 0x1F;
        
        sum += mem_val + bf_val + extracted;
        COMPILER_BARRIER();
    }
    
    /* Use register variables to increase allocation pressure */
    register unsigned int reg1 asm ("r8") = sum;
    register unsigned int reg2 asm ("r9") = 0;
    
    for (int i = 0; i < 10; ++i) {
        reg2 += (reg1 >> (i * 2)) & 0xF;
    }
    
    volatile unsigned int sink __attribute__((unused)) = reg2;
}

/* Test 6: Switch statement based on bitfield values */
void test_switch_bitfield(void) {
    volatile struct packed_bitfields s = {0};
    volatile unsigned int counter = 0;
    
    s.field1 = 5;
    s.field2 = 1023;
    s.field3 = 63;
    s.field4 = 255;
    
    for (int i = 0; i < 100; ++i) {
        /* Switch on bitfield-derived value */
        unsigned int selector = (s.field1 + s.field2 + s.field3 + s.field4) & 0xF;
        
        switch (selector) {
            case 0:
                s.field1 = (s.field2 >> 1) & 0x1F;
                break;
            case 1:
            case 2:
                s.field2 = (s.field3 * 3) & 0x7FF;
                break;
            case 3:
            case 4:
            case 5:
                s.field3 = (s.field4 - s.field1) & 0x7F;
                break;
            default:
                s.field4 = (s.field1 ^ s.field2) & 0x1FF;
                break;
        }
        
        /* Partial update based on condition */
        if (selector & 1) {
            unsigned int temp = s.field2;
            s.field2 = (s.field2 & ~0x3F) | (temp & 0x3F);
        }
        
        counter += selector;
        COMPILER_BARRIER();
    }
    
    volatile unsigned int sink __attribute__((unused)) = counter;
}

/* Main driver function */
int main(int argc, char *argv[]) {
    volatile unsigned int total_result = 0;
    
    /* Run selected tests based on command line arguments */
    int run_all = (argc <= 1);
    
    if (run_all || strstr(argv[1], "1")) {
        test_zero_extract_volatile();
        total_result += 1;
    }
    
    if (run_all || strstr(argv[1], "2")) {
        test_zero_extract_struct();
        total_result += 2;
    }
    
    if (run_all || strstr(argv[1], "3")) {
        test_strict_low_part_conditional();
        total_result += 3;
    }
    
    if (run_all || strstr(argv[1], "4")) {
        test_strict_low_part_asm();
        total_result += 4;
    }
    
    if (run_all || strstr(argv[1], "5")) {
        test_mixed_operations();
        total_result += 5;
    }
    
    if (run_all || strstr(argv[1], "6")) {
        test_switch_bitfield();
        total_result += 6;
    }
    
    /* Print result to prevent dead code elimination */
    printf("Total result marker: %u\n", total_result);
    
    return 0;
}
