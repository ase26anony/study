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
    
    /* Multiple extraction patterns that may generate ZERO_EXTRACT */
    for (int i = 0; i < 100; ++i) {
        /* Extract 5 bits starting at position 3 */
        unsigned int extracted = (source >> 3) & ((1U << 5) - 1);
        result += extracted;
        
        /* Extract 11 bits starting at position 8 */
        extracted = (source >> 8) & ((1U << 11) - 1);
        result ^= extracted;
        
        /* Extract 7 bits starting at position 16 */
        extracted = (source >> 16) & ((1U << 7) - 1);
        result |= extracted;
        
        COMPILER_BARRIER();
        source += 1; /* Change source to prevent constant folding */
    }
    
    volatile unsigned int sink = result;
    (void)sink; /* Prevent unused variable warning */
}

/* Test 2: Packed struct with bitfields */
void test_zero_extract_packed_struct(void) {
    struct __attribute__((packed)) BitFieldStruct {
        unsigned int field1 : 5;
        unsigned int field2 : 11;
        unsigned int field3 : 7;
        unsigned int field4 : 9;
    };
    
    volatile struct BitFieldStruct s = {0};
    volatile unsigned int accumulator = 0;
    
    /* Initialize with pattern */
    s.field1 = 0x1F;
    s.field2 = 0x7FF;
    s.field3 = 0x7F;
    s.field4 = 0x1FF;
    
    for (int i = 0; i < 50; ++i) {
        /* Read bitfields - may generate ZERO_EXTRACT */
        accumulator += s.field1;
        accumulator ^= s.field2;
        accumulator |= s.field3;
        accumulator &= s.field4;
        
        /* Write bitfields - may generate both ZERO_EXTRACT and STRICT_LOW_PART */
        s.field1 = (s.field2 + i) & 0x1F;
        s.field3 = (s.field4 - i) & 0x7F;
        
        COMPILER_BARRIER();
    }
    
    volatile unsigned int sink = accumulator + s.field1;
    (void)sink;
}

/* Test 3: Conditional partial store operations */
void test_strict_low_part_conditional(void) {
    volatile unsigned int data = 0xABCD1234;
    volatile unsigned char flag = 1;
    
    for (int i = 0; i < 100; ++i) {
        /* Conditional update of low byte - may generate STRICT_LOW_PART */
        if (flag) {
            data = (data & ~0xFF) | ((i & 0xFF) & 0xFF);
        }
        
        /* Conditional update of low 16 bits */
        if (i % 3 == 0) {
            data = (data & ~0xFFFF) | ((i * 2) & 0xFFFF);
        }
        
        /* Update flag based on low bits */
        flag = (data & 0x1) ^ ((i >> 1) & 0x1);
        
        COMPILER_BARRIER();
    }
    
    volatile unsigned int sink = data;
    (void)sink;
}

/* Test 4: Inline assembly for partial register operations */
void test_strict_low_part_asm(void) {
    volatile unsigned int value = 0x87654321;
    volatile unsigned int mask = 0x0000FFFF;
    
    for (int i = 0; i < 50; ++i) {
        /* Inline assembly that operates on partial registers */
        unsigned int temp;
        asm volatile (
            "and %0, %1, %2\n\t"
            : "=r" (temp)
            : "r" (value), "r" (mask)
        );
        
        /* Another assembly pattern */
        unsigned int low_part;
        asm volatile (
            "mov %0, %1\n\t"
            "and %0, %0, #255\n\t"
            : "=r" (low_part)
            : "r" (value)
        );
        
        value = (value >> 8) | (temp << 24);
        mask = (mask ^ low_part) & 0xFFFF;
        
        COMPILER_BARRIER();
    }
    
    volatile unsigned int sink = value + mask;
    (void)sink;
}

/* Test 5: Mixed operations with arrays and pointer arithmetic */
void test_mixed_operations(void) {
    volatile unsigned int array[64];
    volatile unsigned int *ptr = array;
    
    /* Initialize array */
    for (int i = 0; i < 64; ++i) {
        array[i] = i * 0x01010101;
    }
    
    volatile unsigned int result = 0;
    
    /* Complex loop with mixed operations */
    for (int i = 0; i < 32; ++i) {
        /* Memory operation */
        volatile unsigned int mem_val = ptr[i];
        
        /* Bitfield extraction */
        unsigned int extracted = (mem_val >> (i % 16)) & ((1U << 8) - 1);
        
        /* Partial store back to memory */
        ptr[i + 32] = (ptr[i + 32] & ~0xFF) | (extracted & 0xFF);
        
        /* Switch based on extracted value */
        switch (extracted & 0x7) {
            case 0: result += 1; break;
            case 1: result ^= mem_val; break;
            case 2: result |= extracted; break;
            case 3: result &= ~extracted; break;
            default: result = result << 1; break;
        }
        
        COMPILER_BARRIER();
    }
    
    volatile unsigned int sink = result;
    (void)sink;
}

/* Test 6: Register variables with bitfield operations */
void test_register_variables(void) {
    register unsigned int reg1 asm("r12") = 0x13579BDF;
    register unsigned int reg2 asm("r11") = 0x2468ACE0;
    volatile unsigned int output = 0;
    
    for (int i = 0; i < 25; ++i) {
        /* Extract varying bit widths */
        unsigned int ext1 = (reg1 >> (i % 16)) & ((1U << 4) - 1);
        unsigned int ext2 = (reg2 >> ((i + 3) % 16)) & ((1U << 6) - 1);
        
        /* Partial update of register */
        reg1 = (reg1 & ~0xF) | (ext2 & 0xF);
        reg2 = (reg2 & ~0x3F) | (ext1 & 0x3F);
        
        output += ext1 * ext2;
        
        COMPILER_BARRIER();
    }
    
    volatile unsigned int sink = output + reg1 + reg2;
    (void)sink;
}

/* Main driver function */
int main(int argc, char *argv[]) {
    volatile unsigned int final_result = 0;
    
    /* Run tests based on command line arguments or all by default */
    int run_all = (argc <= 1);
    
    if (run_all || strstr(argv[1], "1") != NULL) {
        test_zero_extract_volatile();
        final_result += 1;
    }
    
    if (run_all || strstr(argv[1], "2") != NULL) {
        test_zero_extract_packed_struct();
        final_result += 2;
    }
    
    if (run_all || strstr(argv[1], "3") != NULL) {
        test_strict_low_part_conditional();
        final_result += 3;
    }
    
    if (run_all || strstr(argv[1], "4") != NULL) {
        test_strict_low_part_asm();
        final_result += 4;
    }
    
    if (run_all || strstr(argv[1], "5") != NULL) {
        test_mixed_operations();
        final_result += 5;
    }
    
    if (run_all || strstr(argv[1], "6") != NULL) {
        test_register_variables();
        final_result += 6;
    }
    
    /* Print result to prevent dead code elimination */
    printf("Test result: %u\n", final_result);
    
    return 0;
}
