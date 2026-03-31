/* test_resource.c - Generate ZERO_EXTRACT and STRICT_LOW_PART RTL patterns */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Compiler barrier to prevent optimization */
#define COMPILER_BARRIER() asm volatile("" ::: "memory")

/* Test 1: Bitfield extraction from volatile integer (ZERO_EXTRACT) */
volatile unsigned int test1_bitfield_extract(void) {
    volatile unsigned int source = 0xDEADBEEF;
    volatile unsigned int result = 0;
    
    /* Multiple extraction patterns that may generate ZERO_EXTRACT */
    for (int i = 0; i < 100; ++i) {
        /* Extract bits 4-11 */
        unsigned int extracted = (source >> 4) & ((1U << 8) - 1);
        result += extracted;
        
        /* Extract bits 16-23 with different width */
        extracted = (source >> 16) & ((1U << 8) - 1);
        result ^= extracted;
        
        /* Extract bits 8-15 */
        extracted = (source >> 8) & 0xFF;
        result |= extracted;
        
        COMPILER_BARRIER();
    }
    
    return result;
}

/* Test 2: Packed struct with bitfields (ZERO_EXTRACT) */
volatile unsigned int test2_packed_struct(void) {
    /* Packed struct with various bitfield widths */
    struct __attribute__((packed)) BitfieldStruct {
        unsigned int header : 4;
        unsigned int data   : 12;
        unsigned int flags  : 8;
        unsigned int tail   : 8;
    };
    
    volatile struct BitfieldStruct bs = {0};
    volatile unsigned int accumulator = 0;
    
    /* Initialize with pattern */
    bs.header = 0xA;
    bs.data   = 0xABC;
    bs.flags  = 0x3F;
    bs.tail   = 0xCC;
    
    /* Operations that may generate ZERO_EXTRACT */
    for (int i = 0; i < 50; ++i) {
        /* Read bitfields - may generate ZERO_EXTRACT */
        unsigned int val1 = bs.data;
        unsigned int val2 = bs.flags;
        
        /* Write with extraction from other field */
        bs.tail = (bs.data + i) & 0xFF;
        
        /* Complex bitfield expression */
        bs.flags = (bs.header << 4) | (bs.data & 0x0F);
        
        accumulator += val1 + val2 + bs.tail;
        
        COMPILER_BARRIER();
    }
    
    return accumulator;
}

/* Test 3: Mixed bitfield operations with arrays (ZERO_EXTRACT + MEM_P) */
volatile unsigned int test3_mixed_bitfield_array(void) {
    /* Array of packed structs */
    struct __attribute__((packed)) Mixed {
        unsigned int low : 6;
        unsigned int mid : 10;
        unsigned int high : 16;
    };
    
    volatile struct Mixed array[16];
    volatile unsigned int sum = 0;
    
    /* Initialize array */
    for (int i = 0; i < 16; i++) {
        array[i].low = i & 0x3F;
        array[i].mid = (i * 13) & 0x3FF;
        array[i].high = (i * 257) & 0xFFFF;
    }
    
    /* Operations that mix bitfields and memory access */
    for (int i = 0; i < 100; ++i) {
        int idx = i & 0xF;
        
        /* Bitfield extraction from array element */
        unsigned int extracted = array[idx].mid;
        
        /* Conditional update of bitfield */
        if (extracted & 0x100) {
            array[idx].low = (array[idx].high >> 8) & 0x3F;
        }
        
        /* Complex expression with bitfields */
        array[(idx + 1) & 0xF].high = 
            (array[idx].low << 10) | (extracted & 0x3FF);
        
        sum += extracted + array[idx].low;
        
        COMPILER_BARRIER();
    }
    
    return sum;
}

/* Test 4: STRICT_LOW_PART via inline assembly */
volatile unsigned int test4_strict_low_part_asm(void) {
    volatile unsigned int value = 0x12345678;
    volatile unsigned int result = 0;
    
    for (int i = 0; i < 100; ++i) {
        unsigned int temp = value;
        
        /* Inline assembly that modifies only low bits */
        asm volatile (
            "and %0, %1, %2\n\t"
            "orr %0, %0, %3"
            : "=r" (temp)
            : "r" (temp), "i" (0xFFFFFF00), "r" (i & 0xFF)
            : /* No clobbers */
        );
        
        /* Another assembly pattern that might generate STRICT_LOW_PART */
        unsigned int low_byte;
        asm volatile (
            "ubfx %0, %1, #0, #8"
            : "=r" (low_byte)
            : "r" (temp)
        );
        
        result += temp + low_byte;
        value = (value * 1103515245 + 12345) & 0x7FFFFFFF;
        
        COMPILER_BARRIER();
    }
    
    return result;
}

/* Test 5: STRICT_LOW_PART via conditional merge operations */
volatile unsigned int test5_conditional_merge(void) {
    volatile unsigned int reg = 0x87654321;
    volatile unsigned int counter = 0;
    
    for (int i = 0; i < 200; ++i) {
        /* Conditional update of low byte */
        if (i & 1) {
            /* This pattern may generate STRICT_LOW_PART: 
               (reg & ~0xFF) | (new_val & 0xFF) */
            reg = (reg & ~0xFF) | ((i * 7) & 0xFF);
        }
        
        /* Conditional update of low 16 bits */
        if (i % 3 == 0) {
            reg = (reg & ~0xFFFF) | ((i * 13) & 0xFFFF);
        }
        
        /* Update only specific bits */
        if (i % 5 == 0) {
            unsigned int mask = 0xF0F;
            unsigned int new_bits = (i * 17) & mask;
            reg = (reg & ~mask) | new_bits;
        }
        
        counter += reg & 0xFF;
        
        COMPILER_BARRIER();
    }
    
    return counter;
}

/* Test 6: Mixed patterns with switch statement */
volatile unsigned int test6_mixed_with_switch(void) {
    volatile unsigned int value = 0x9ABCDEF0;
    volatile unsigned int total = 0;
    
    /* Packed struct for bitfield test */
    struct __attribute__((packed)) SwitchStruct {
        unsigned int opcode : 4;
        unsigned int operand : 12;
        unsigned int mode : 2;
        unsigned int reserved : 14;
    };
    
    volatile struct SwitchStruct ss = {0};
    ss.opcode = 0x5;
    ss.operand = 0xABC;
    ss.mode = 0x2;
    
    for (int i = 0; i < 150; ++i) {
        /* Use bitfield to control switch */
        unsigned int selector = ss.operand & 0x7;
        
        switch (selector) {
            case 0:
                /* Bitfield extraction */
                total += ss.opcode;
                /* Partial store */
                value = (value & ~0xFF) | (i & 0xFF);
                break;
            case 1:
                /* Different extraction */
                total += (ss.operand >> 4) & 0xF;
                /* Another partial store pattern */
                if (i & 1) {
                    value = (value & ~0xF0F0) | ((i * 3) & 0xF0F0);
                }
                break;
            case 2:
                /* Update bitfield based on value */
                ss.mode = (value >> 16) & 0x3;
                total += ss.mode;
                break;
            default:
                /* Mixed operation */
                unsigned int extracted = (value >> 8) & 0xFF;
                ss.operand = (ss.operand + extracted) & 0xFFF;
                total += extracted;
                break;
        }
        
        /* Rotate value */
        value = (value << 1) | (value >> 31);
        
        COMPILER_BARRIER();
    }
    
    return total + ss.operand;
}

/* Test 7: Pointer-based partial updates (may generate STRICT_LOW_PART) */
volatile unsigned int test7_pointer_partial_updates(void) {
    volatile unsigned int data = 0x13579BDF;
    volatile unsigned char *byte_ptr = (volatile unsigned char *)&data;
    volatile unsigned int sum = 0;
    
    for (int i = 0; i < 100; ++i) {
        /* Partial update via byte pointer - may be represented as partial store */
        byte_ptr[i % 4] = (i * 11) & 0xFF;
        
        /* Extract byte via pointer */
        unsigned int byte_val = byte_ptr[(i + 1) % 4];
        
        /* Combine with bitfield-like extraction */
        unsigned int extracted = (data >> ((i % 4) * 8)) & 0xFF;
        
        /* Conditional partial word update */
        if (byte_val > 0x80) {
            /* Update low 16 bits only */
            data = (data & ~0xFFFF) | ((data + 0x1111) & 0xFFFF);
        }
        
        sum += byte_val + extracted + (data & 0xFF);
        
        COMPILER_BARRIER();
    }
    
    return sum;
}

/* Main driver function */
int main(int argc, char *argv[]) {
    volatile unsigned int final_result = 0;
    
    /* Run tests based on command line argument or all if no argument */
    int run_all = (argc <= 1);
    
    if (run_all || strstr(argv[1], "1")) {
        final_result += test1_bitfield_extract();
    }
    
    if (run_all || (argc > 1 && strstr(argv[1], "2"))) {
        final_result += test2_packed_struct();
    }
    
    if (run_all || (argc > 1 && strstr(argv[1], "3"))) {
        final_result += test3_mixed_bitfield_array();
    }
    
    if (run_all || (argc > 1 && strstr(argv[1], "4"))) {
        final_result += test4_strict_low_part_asm();
    }
    
    if (run_all || (argc > 1 && strstr(argv[1], "5"))) {
        final_result += test5_conditional_merge();
    }
    
    if (run_all || (argc > 1 && strstr(argv[1], "6"))) {
        final_result += test6_mixed_with_switch();
    }
    
    if (run_all || (argc > 1 && strstr(argv[1], "7"))) {
        final_result += test7_pointer_partial_updates();
    }
    
    /* Print result to prevent dead code elimination */
    printf("Result: 0x%08X\n", final_result);
    
    return 0;
}
