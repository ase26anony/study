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
        /* Extract bits 4-11 (8 bits) */
        unsigned int extracted = (source >> 4) & 0xFF;
        result += extracted;
        
        /* Extract bits 16-23 */
        extracted = (source >> 16) & 0xFF;
        result -= extracted;
        
        /* Extract bits 8-15 with variable width */
        int width = 8;
        extracted = (source >> 8) & ((1U << width) - 1);
        result ^= extracted;
        
        COMPILER_BARRIER();
    }
    
    volatile unsigned int sink = result;
    (void)sink;
}

/* Test 2: Packed struct with bitfields */
struct __attribute__((packed)) packed_bitfields {
    unsigned int field1 : 5;
    unsigned int field2 : 11;
    unsigned int field3 : 7;
    unsigned int field4 : 9;
};

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
        s.field4 = (val2 - val3) & 0x1FF;
        
        accumulator += s.field1 + s.field2 + s.field3 + s.field4;
        COMPILER_BARRIER();
    }
    
    volatile unsigned int sink = accumulator;
    (void)sink;
}

/* Test 3: Conditional partial store for STRICT_LOW_PART */
void test_strict_low_part_conditional(void) {
    volatile unsigned int data = 0x12345678;
    volatile unsigned int mask = 0x000000FF;
    volatile int condition = 1;
    
    for (int i = 0; i < 100; ++i) {
        /* Pattern that may generate STRICT_LOW_PART: conditional update of low byte */
        if (condition) {
            /* Update only low 8 bits, preserve high bits */
            data = (data & ~mask) | ((i * 7) & mask);
        } else {
            /* Update only high 8 bits */
            data = (data & ~(mask << 24)) | (((i * 13) & mask) << 24);
        }
        
        /* Toggle condition */
        condition = !condition;
        
        /* Another pattern: merge operations */
        unsigned int new_low = (data >> 8) & 0xFF;
        data = (data & 0xFFFFFF00) | (new_low & 0xFF);
        
        COMPILER_BARRIER();
    }
    
    volatile unsigned int sink = data;
    (void)sink;
}

/* Test 4: Inline assembly for partial register updates */
void test_strict_low_part_asm(void) {
    volatile unsigned int value = 0x87654321;
    volatile unsigned int result;
    
    for (int i = 0; i < 75; ++i) {
        /* Inline assembly that operates on partial register */
        asm volatile (
            "and %0, %1, %2\n\t"
            "orr %0, %0, %3"
            : "=r" (result)
            : "r" (value), "i" (0x0000FFFF), "r" (i * 0x10000)
            : /* No clobbers */
        );
        
        /* Another pattern using byte operations */
        unsigned char *byte_ptr = (unsigned char *)&value;
        byte_ptr[1] = (i * 3) & 0xFF;  /* Modify only second byte */
        
        value = result ^ value;
        COMPILER_BARRIER();
    }
    
    volatile unsigned int sink = value;
    (void)sink;
}

/* Test 5: Complex mixed pattern with memory references */
void test_mixed_patterns(void) {
    volatile unsigned int array[4] = {0xA5A5A5A5, 0x5A5A5A5A, 0x33333333, 0xCCCCCCCC};
    volatile struct packed_bitfields struct_array[2];
    volatile unsigned int total = 0;
    
    /* Initialize struct array */
    for (int i = 0; i < 2; ++i) {
        struct_array[i].field1 = i * 3;
        struct_array[i].field2 = i * 7;
        struct_array[i].field3 = i * 11;
        struct_array[i].field4 = i * 13;
    }
    
    for (int i = 0; i < 25; ++i) {
        /* Memory operations that create MEM_P references */
        unsigned int idx = i & 3;
        volatile unsigned int *ptr = &array[idx];
        
        /* Bitfield extraction from struct */
        unsigned int bf_val = struct_array[idx & 1].field2;
        
        /* Partial update of array element - may generate STRICT_LOW_PART */
        *ptr = (*ptr & 0xFFFF0000) | ((bf_val + i) & 0xFFFF);
        
        /* Extract from updated value - may generate ZERO_EXTRACT */
        unsigned int extracted = (*ptr >> 8) & 0xFF;
        
        /* Update struct with extracted value */
        struct_array[idx & 1].field1 = extracted & 0x1F;
        
        total += *ptr + bf_val + extracted;
        COMPILER_BARRIER();
    }
    
    /* Switch statement to create control flow */
    switch (total & 0xF) {
        case 0: total += array[0]; break;
        case 1: total += array[1]; break;
        case 2: total += array[2]; break;
        default: total += array[3]; break;
    }
    
    volatile unsigned int sink = total;
    (void)sink;
}

/* Test 6: Nested loops with register variables */
void test_nested_loops(void) {
    register unsigned int reg_var1 asm("r8") = 0x11111111;
    register unsigned int reg_var2 asm("r9") = 0x22222222;
    volatile unsigned int output = 0;
    
    for (int i = 0; i < 10; ++i) {
        for (int j = 0; j < 10; ++j) {
            /* Bitfield extraction from register variable */
            unsigned int low_bits = reg_var1 & 0xF;
            unsigned int high_bits = (reg_var2 >> 28) & 0xF;
            
            /* Partial update - may generate STRICT_LOW_PART */
            reg_var1 = (reg_var1 & 0xFFFFFFF0) | ((low_bits + high_bits) & 0xF);
            
            /* Complex extraction pattern */
            int shift = (i + j) & 0x7;
            unsigned int extracted = (reg_var2 >> shift) & ((1U << 4) - 1);
            
            output += reg_var1 + extracted;
            COMPILER_BARRIER();
        }
    }
    
    volatile unsigned int sink = output;
    (void)sink;
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
        test_strict_low_part_conditional();
        final_result += 4;
    }
    
    if (run_all || strstr(argv[0], "test4") || (argc > 1 && atoi(argv[1]) == 4)) {
        test_strict_low_part_asm();
        final_result += 8;
    }
    
    if (run_all || strstr(argv[0], "test5") || (argc > 1 && atoi(argv[1]) == 5)) {
        test_mixed_patterns();
        final_result += 16;
    }
    
    if (run_all || strstr(argv[0], "test6") || (argc > 1 && atoi(argv[1]) == 6)) {
        test_nested_loops();
        final_result += 32;
    }
    
    /* Print result to prevent dead code elimination */
    printf("Final result: %u\n", final_result);
    
    return 0;
}
