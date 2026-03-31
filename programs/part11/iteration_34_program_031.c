/* test_resource.c - Test program to cover ZERO_EXTRACT and STRICT_LOW_PART RTL patterns */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Compiler barrier to prevent optimization */
#define COMPILER_BARRIER() asm volatile("" ::: "memory")

/* Packed structs with bitfields */
struct packed_bitfield_16 {
    unsigned int field1 : 3;
    unsigned int field2 : 5;
    unsigned int field3 : 8;
} __attribute__((packed));

struct packed_bitfield_32 {
    unsigned int low : 10;
    unsigned int mid : 12;
    unsigned int high : 10;
} __attribute__((packed));

/* Test 1: Bitfield extraction from volatile integer */
void test_zero_extract_volatile(void) {
    volatile unsigned int source = 0xDEADBEEF;
    volatile unsigned int result = 0;
    
    /* Multiple extraction patterns to increase chances */
    for (int i = 0; i < 100; ++i) {
        /* Extract bits 4-11 (8 bits) */
        unsigned int extracted = (source >> 4) & 0xFF;
        result += extracted;
        
        /* Extract bits 8-15 with different width */
        extracted = (source >> 8) & 0x3F;  /* 6 bits */
        result -= extracted;
        
        /* Extract bits 16-23 */
        extracted = (source >> 16) & 0x7F; /* 7 bits */
        result ^= extracted;
        
        COMPILER_BARRIER();
    }
    
    /* Use result to prevent dead code elimination */
    volatile unsigned int sink __attribute__((unused)) = result;
}

/* Test 2: Packed struct bitfield operations */
void test_zero_extract_struct(void) {
    volatile struct packed_bitfield_16 s1 = {0};
    volatile struct packed_bitfield_32 s2 = {0};
    
    /* Initialize with pattern */
    s1.field1 = 5;
    s1.field2 = 20;
    s1.field3 = 150;
    
    s2.low = 512;
    s2.mid = 2047;
    s2.high = 768;
    
    volatile unsigned int sum = 0;
    
    for (int i = 0; i < 50; ++i) {
        /* Complex bitfield extraction and assignment */
        unsigned int temp = s1.field3;
        s1.field2 = (temp + i) & 0x1F;  /* 5 bits */
        
        /* Cross-struct operations */
        s2.low = (s1.field1 + s2.mid) & 0x3FF;
        
        /* Nested extraction */
        sum += s1.field2;
        sum += s2.low;
        sum += s2.high;
        
        /* Conditional extraction */
        if (i & 1) {
            sum += s1.field3;
        }
        
        COMPILER_BARRIER();
    }
    
    volatile unsigned int sink __attribute__((unused)) = sum;
}

/* Test 3: Mixed bitfield and memory operations */
void test_zero_extract_mixed(void) {
    volatile unsigned int array[4] = {0x12345678, 0x9ABCDEF0, 0x13579BDF, 0x2468ACE0};
    volatile struct packed_bitfield_16 bf_array[2];
    
    unsigned int accumulator = 0;
    
    for (int i = 0; i < 25; ++i) {
        /* Extract from array elements */
        unsigned int val = array[i & 3];
        
        /* Multiple extractions with different widths */
        unsigned int ext1 = (val >> 0) & 0xF;   /* 4 bits */
        unsigned int ext2 = (val >> 4) & 0x1F;  /* 5 bits */
        unsigned int ext3 = (val >> 9) & 0x3F;  /* 6 bits */
        unsigned int ext4 = (val >> 15) & 0x7F; /* 7 bits */
        
        /* Store to bitfield struct */
        bf_array[0].field1 = ext1;
        bf_array[0].field2 = ext2;
        bf_array[0].field3 = ext3;
        
        /* Read back and combine */
        accumulator += bf_array[0].field1;
        accumulator += bf_array[0].field2;
        accumulator ^= bf_array[0].field3;
        
        /* Update array with extracted value */
        array[i & 3] = (array[i & 3] & ~0xFF) | (ext4 & 0xFF);
        
        COMPILER_BARRIER();
    }
    
    volatile unsigned int sink __attribute__((unused)) = accumulator;
}

/* Test 4: STRICT_LOW_PART via inline assembly */
void test_strict_low_part_asm(void) {
    volatile unsigned int reg_var = 0x12345678;
    volatile unsigned int result = 0;
    
    for (int i = 0; i < 100; ++i) {
        unsigned int temp = reg_var;
        
        /* Inline assembly that modifies only low parts */
        asm volatile (
            "and %0, %1, %2\n\t"
            : "=r" (temp)
            : "r" (temp), "i" (0xFF)  /* Only keep low byte */
        );
        
        /* Another assembly pattern */
        asm volatile (
            "and %0, %1, %2\n\t"
            "orr %0, %0, %3\n\t"
            : "=r" (temp)
            : "r" (temp), "i" (0xFFFF), "r" (i & 0xFF)
        );
        
        result += temp;
        reg_var = result ^ 0x55AA55AA;
        
        COMPILER_BARRIER();
    }
    
    volatile unsigned int sink __attribute__((unused)) = result;
}

/* Test 5: STRICT_LOW_PART via conditional merge operations */
void test_strict_low_part_merge(void) {
    volatile unsigned int data = 0x87654321;
    volatile unsigned int mask = 0;
    volatile int condition = 1;
    
    for (int i = 0; i < 75; ++i) {
        /* Conditional update of low byte */
        if (condition) {
            data = (data & ~0xFF) | ((i * 3) & 0xFF);
        }
        
        /* Conditional update of low word */
        if (i % 3 == 0) {
            data = (data & ~0xFFFF) | ((i * 7) & 0xFFFF);
        }
        
        /* Complex merge with bitfield extraction */
        unsigned int extracted = (data >> 8) & 0x3F;
        data = (data & ~0x3F00) | ((extracted * 2) << 8);
        
        /* Toggle condition */
        condition = !condition;
        mask = (mask << 1) | (condition & 1);
        
        COMPILER_BARRIER();
    }
    
    volatile unsigned int sink __attribute__((unused)) = data + mask;
}

/* Test 6: STRICT_LOW_PART via volatile char pointer */
void test_strict_low_part_char_ptr(void) {
    volatile unsigned int word = 0x11223344;
    volatile unsigned char *byte_ptr = (volatile unsigned char *)&word;
    
    for (int i = 0; i < 60; ++i) {
        /* Partial store through char pointer */
        byte_ptr[0] = i & 0xFF;
        byte_ptr[1] = (i * 2) & 0xFF;
        
        /* Read-modify-write of specific byte */
        byte_ptr[2] = byte_ptr[0] + byte_ptr[1];
        
        /* Update word based on byte operations */
        word = (word << 8) | (byte_ptr[3] & 0xFF);
        
        COMPILER_BARRIER();
    }
    
    volatile unsigned int sink __attribute__((unused)) = word;
}

/* Test 7: Combined patterns with switch statement */
void test_combined_patterns(void) {
    volatile struct packed_bitfield_32 bf = {0};
    volatile unsigned int reg = 0;
    volatile unsigned int counter = 0;
    
    for (int i = 0; i < 40; ++i) {
        /* Update bitfields */
        bf.low = (bf.low + i) & 0x3FF;
        bf.mid = (bf.mid ^ i) & 0xFFF;
        
        /* Create value for switch */
        unsigned int switch_val = (bf.low >> 2) & 0x7;
        
        /* Switch on extracted bitfield */
        switch (switch_val) {
            case 0:
                reg = (reg & ~0xFF) | (i & 0xFF);
                break;
            case 1:
                reg = (reg & ~0x3FF) | (bf.low & 0x3FF);
                break;
            case 2:
                reg = (reg >> 4) & 0xFFFFFF;
                break;
            case 3:
                reg = (reg << 2) | (bf.mid & 0x3);
                break;
            default:
                reg = (reg & ~0xFFFF) | ((i * 5) & 0xFFFF);
                break;
        }
        
        counter += reg;
        COMPILER_BARRIER();
    }
    
    volatile unsigned int sink __attribute__((unused)) = counter + bf.low + bf.mid;
}

/* Test 8: Complex nested loops with register variables */
void test_complex_nested(void) {
    register unsigned int r1 asm("r12") = 0;
    register unsigned int r2 asm("r13") = 0;
    volatile unsigned int mem[8];
    
    /* Initialize memory */
    for (int i = 0; i < 8; ++i) {
        mem[i] = i * 0x11111111;
    }
    
    for (int outer = 0; outer < 10; ++outer) {
        for (int inner = 0; inner < 5; ++inner) {
            /* Bitfield extraction from memory */
            unsigned int val = mem[inner & 7];
            unsigned int ext = (val >> (inner * 3)) & 0x7;
            
            /* Update register with partial store pattern */
            r1 = (r1 & ~0x7) | (ext & 0x7);
            
            /* Another extraction */
            ext = (val >> 8) & 0xF;
            r2 = (r2 & ~0xF) | (ext & 0xF);
            
            /* Combine and store back */
            mem[inner & 7] = (r1 << 16) | (r2 << 8) | (inner & 0xFF);
            
            COMPILER_BARRIER();
        }
        
        /* Rotate registers */
        unsigned int temp = r1;
        r1 = r2;
        r2 = temp;
    }
    
    volatile unsigned int sink __attribute__((unused)) = r1 + r2 + mem[0];
}

/* Main driver function */
int main(int argc, char *argv[]) {
    unsigned int total_result = 0;
    
    /* Run tests based on command line arguments */
    if (argc == 1) {
        /* Run all tests if no arguments */
        test_zero_extract_volatile();
        test_zero_extract_struct();
        test_zero_extract_mixed();
        test_strict_low_part_asm();
        test_strict_low_part_merge();
        test_strict_low_part_char_ptr();
        test_combined_patterns();
        test_complex_nested();
    } else {
        /* Run specific tests based on arguments */
        for (int i = 1; i < argc; ++i) {
            int test_num = atoi(argv[i]);
            switch (test_num) {
                case 1: test_zero_extract_volatile(); break;
                case 2: test_zero_extract_struct(); break;
                case 3: test_zero_extract_mixed(); break;
                case 4: test_strict_low_part_asm(); break;
                case 5: test_strict_low_part_merge(); break;
                case 6: test_strict_low_part_char_ptr(); break;
                case 7: test_combined_patterns(); break;
                case 8: test_complex_nested(); break;
                default: break;
            }
        }
    }
    
    /* Create a simple aggregate result to prevent optimization */
    volatile unsigned int final_sink = total_result;
    
    /* Print something to ensure execution */
    printf("Tests completed. Result marker: %u\n", (unsigned int)final_sink);
    
    return 0;
}
