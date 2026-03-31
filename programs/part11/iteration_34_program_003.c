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
    
    /* Multiple extraction patterns to increase chances */
    for (int i = 0; i < 100; ++i) {
        /* Extract bits 4-11 (8 bits) */
        unsigned int extracted = (source >> 4) & 0xFF;
        result = extracted;
        
        /* Extract bits 16-23 */
        extracted = (source >> 16) & 0xFF;
        result ^= extracted;
        
        /* Extract bits 8-15 with variable width */
        int width = 8;
        extracted = (source >> 8) & ((1U << width) - 1);
        result += extracted;
        
        COMPILER_BARRIER();
    }
    
    /* Prevent dead code elimination */
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
    volatile unsigned int results[4] = {0};
    
    /* Initialize with pattern */
    s.field1 = 0x1F;
    s.field2 = 0x7FF;
    s.field3 = 0x7F;
    s.field4 = 0x1FF;
    
    for (int i = 0; i < 100; ++i) {
        /* Read bitfields - should generate ZERO_EXTRACT */
        results[0] = s.field1;
        results[1] = s.field2;
        results[2] = s.field3;
        results[3] = s.field4;
        
        /* Write with arithmetic - complex pattern */
        s.field1 = (s.field2 + i) & 0x1F;
        s.field3 = (s.field4 >> 2) & 0x7F;
        
        COMPILER_BARRIER();
    }
    
    volatile unsigned int sink __attribute__((unused)) = 
        results[0] + results[1] + results[2] + results[3];
}

/* Test 3: Inline assembly for partial register updates */
void test_strict_low_part_asm(void) {
    volatile unsigned int value = 0x12345678;
    volatile unsigned int temp;
    
    for (int i = 0; i < 100; ++i) {
        /* Update only low byte using inline asm */
        asm volatile (
            "and %0, %1, %2\n\t"
            : "=r"(temp)
            : "r"(value), "i"(0xFF)
        );
        
        /* Another pattern: clear high bits */
        asm volatile (
            "andi %0, %1, 0xFFFF\n\t"
            : "=r"(temp)
            : "r"(value + i)
        );
        
        COMPILER_BARRIER();
    }
    
    volatile unsigned int sink __attribute__((unused)) = temp;
}

/* Test 4: Conditional merge operations */
void test_strict_low_part_conditional(void) {
    volatile unsigned int reg = 0x87654321;
    volatile unsigned int new_low;
    volatile int condition = 1;
    
    for (int i = 0; i < 100; ++i) {
        new_low = (i * 7) & 0xFF;
        
        /* Conditional update of low byte */
        if (condition) {
            /* Pattern: (reg & ~mask) | (new_val & mask) */
            reg = (reg & ~0xFF) | (new_low & 0xFF);
        }
        
        /* Update low 16 bits based on condition */
        if (i % 2) {
            unsigned short new_word = (i * 13) & 0xFFFF;
            reg = (reg & ~0xFFFF) | (new_word & 0xFFFF);
        }
        
        condition = !condition;
        COMPILER_BARRIER();
    }
    
    volatile unsigned int sink __attribute__((unused)) = reg;
}

/* Test 5: Mixed operations with memory references */
void test_mixed_patterns(void) {
    volatile unsigned int array[16];
    volatile struct packed_bitfields structs[4];
    volatile unsigned int results = 0;
    
    /* Initialize arrays */
    for (int i = 0; i < 16; ++i) {
        array[i] = i * 0x11111111;
    }
    
    for (int i = 0; i < 4; ++i) {
        structs[i].field1 = i;
        structs[i].field2 = i * 2;
        structs[i].field3 = i * 3;
        structs[i].field4 = i * 4;
    }
    
    /* Complex loop with mixed operations */
    for (int i = 0; i < 100; ++i) {
        /* Memory access (triggers MEM_P path) */
        unsigned int idx = i & 0xF;
        volatile unsigned int *ptr = &array[idx];
        unsigned int val = *ptr;
        
        /* Bitfield extraction from struct */
        unsigned int bf_val = structs[idx & 0x3].field2;
        
        /* Partial update */
        val = (val & ~0xFFFF) | (bf_val & 0xFFFF);
        
        /* Store back */
        *ptr = val;
        
        /* Switch based on bitfield value */
        switch (structs[idx & 0x3].field1 & 0x7) {
            case 0: results += 1; break;
            case 1: results += 2; break;
            case 2: results += 3; break;
            default: results += 4; break;
        }
        
        COMPILER_BARRIER();
    }
    
    volatile unsigned int sink __attribute__((unused)) = results;
}

/* Test 6: Pointer casting for partial stores */
void test_partial_store_pointers(void) {
    volatile unsigned int value = 0xABCD1234;
    volatile unsigned char *byte_ptr;
    
    for (int i = 0; i < 100; ++i) {
        /* Cast to char pointer for byte store */
        byte_ptr = (volatile unsigned char*)&value;
        byte_ptr[1] = (i * 3) & 0xFF;  /* Store to second byte */
        
        /* Store to low word */
        volatile unsigned short *short_ptr = 
            (volatile unsigned short*)&value;
        short_ptr[0] = (i * 5) & 0xFFFF;
        
        COMPILER_BARRIER();
    }
    
    volatile unsigned int sink __attribute__((unused)) = value;
}

/* Main driver */
int main(int argc, char *argv[]) {
    int test_to_run = -1;  /* -1 means run all */
    
    /* Parse command line argument if provided */
    if (argc > 1) {
        test_to_run = atoi(argv[1]);
    }
    
    volatile unsigned int total_result = 0;
    
    /* Run selected tests */
    if (test_to_run == -1 || test_to_run == 1) {
        test_zero_extract_volatile();
        total_result += 1;
    }
    
    if (test_to_run == -1 || test_to_run == 2) {
        test_zero_extract_struct();
        total_result += 2;
    }
    
    if (test_to_run == -1 || test_to_run == 3) {
        test_strict_low_part_asm();
        total_result += 3;
    }
    
    if (test_to_run == -1 || test_to_run == 4) {
        test_strict_low_part_conditional();
        total_result += 4;
    }
    
    if (test_to_run == -1 || test_to_run == 5) {
        test_mixed_patterns();
        total_result += 5;
    }
    
    if (test_to_run == -1 || test_to_run == 6) {
        test_partial_store_pointers();
        total_result += 6;
    }
    
    /* Print result to prevent optimization */
    printf("Test result: %u\n", total_result);
    
    return 0;
}
