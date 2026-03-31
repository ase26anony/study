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
    
    /* Loop to increase scheduling opportunities */
    for (int i = 0; i < 100; ++i) {
        /* Extract bits 8-15: should generate ZERO_EXTRACT */
        unsigned int extracted = (source >> 8) & 0xFF;
        
        /* Mix with arithmetic to create complex pattern */
        extracted = (extracted + i) & 0x7F;
        
        /* Store to volatile to prevent elimination */
        result = extracted;
        
        /* Modify source to create data dependencies */
        source = source ^ (1 << (i % 32));
    }
    
    COMPILER_BARRIER();
    printf("Test1 result: %u\n", result);
}

/* Test 2: Packed struct with bitfields */
struct __attribute__((packed)) BitfieldStruct {
    unsigned int header : 4;
    unsigned int data   : 12;
    unsigned int footer : 8;
    unsigned int extra  : 7;
    unsigned int pad    : 1;
};

void test_zero_extract_struct(void) {
    volatile struct BitfieldStruct bs = {0};
    volatile unsigned int results[4] = {0};
    
    /* Initialize with pattern */
    bs.header = 0xA;
    bs.data   = 0xABC;
    bs.footer = 0x42;
    bs.extra  = 0x3F;
    
    for (int i = 0; i < 50; ++i) {
        /* Multiple bitfield reads - should generate ZERO_EXTRACT */
        unsigned int val1 = bs.data;
        unsigned int val2 = bs.footer;
        
        /* Bitfield write - may generate ZERO_EXTRACT in SET_DEST */
        bs.data = (val1 + val2) & 0xFFF;
        
        /* Complex expression with bitfields */
        bs.extra = (bs.header << 2) | (i & 0x3);
        
        /* Store results */
        results[i % 4] = val1 + val2;
    }
    
    COMPILER_BARRIER();
    printf("Test2 result: %u\n", results[0] + results[1]);
}

/* Test 3: Inline assembly for partial register updates */
void test_strict_low_part_asm(void) {
    volatile unsigned int reg = 0x12345678;
    volatile unsigned int output = 0;
    
    for (int i = 0; i < 100; ++i) {
        unsigned int temp = reg;
        
        /* Inline assembly that modifies only low bits */
        /* May generate STRICT_LOW_PART during RTL expansion */
        asm volatile (
            "and %0, %1, %2\n\t"
            "orr %0, %0, %3"
            : "=r" (temp)
            : "r" (temp), "i" (0xFFFFFF00), "r" (i & 0xFF)
            : /* no clobber */
        );
        
        /* Alternative: explicit mask operation */
        temp = (temp & ~0xFF) | ((i * 3) & 0xFF);
        
        reg = temp;
        output += temp;
    }
    
    COMPILER_BARRIER();
    printf("Test3 result: %u\n", output);
}

/* Test 4: Conditional partial store */
void test_strict_low_part_conditional(void) {
    volatile unsigned int value = 0x87654321;
    volatile unsigned int accumulator = 0;
    volatile int condition = 1;
    
    for (int i = 0; i < 200; ++i) {
        /* Vary condition to prevent optimization */
        condition = (i % 17) > 8;
        
        if (condition) {
            /* Conditional update of low 16 bits */
            /* May generate STRICT_LOW_PART pattern */
            value = (value & ~0xFFFF) | ((value + i) & 0xFFFF);
        } else {
            /* Update high part instead */
            value = (value & 0xFFFF) | (((value >> 16) + i) << 16);
        }
        
        /* Extract low byte - ZERO_EXTRACT */
        unsigned char low_byte = value & 0xFF;
        accumulator += low_byte;
    }
    
    COMPILER_BARRIER();
    printf("Test4 result: %u\n", accumulator);
}

/* Test 5: Mixed operations with memory references */
void test_mixed_operations(void) {
    volatile unsigned int array[16];
    volatile unsigned int registers[4] = {0x11111111, 0x22222222, 0x33333333, 0x44444444};
    
    /* Initialize array */
    for (int i = 0; i < 16; ++i) {
        array[i] = i * 0x01010101;
    }
    
    register unsigned int r1 asm ("r8") = 0;
    register unsigned int r2 asm ("r9") = 0;
    
    for (int i = 0; i < 100; ++i) {
        /* Memory load with bitfield extract */
        unsigned int mem_val = array[i % 16];
        unsigned int extracted = (mem_val >> (i % 24)) & ((1 << 8) - 1);
        
        /* Partial store to register variable */
        r1 = (r1 & ~0xFF) | (extracted & 0xFF);
        
        /* Complex bitfield operation */
        struct {
            unsigned int low : 10;
            unsigned int mid : 10;
            unsigned int high : 12;
        } __attribute__((packed)) bf;
        
        bf.low = r1 & 0x3FF;
        bf.mid = (r1 >> 10) & 0x3FF;
        bf.high = extracted;
        
        /* Switch based on bitfield to create control flow */
        switch (bf.low & 0x7) {
            case 0: r2 = bf.mid; break;
            case 1: r2 = bf.high; break;
            case 2: r2 = bf.low + bf.mid; break;
            case 3: r2 = bf.low | bf.high; break;
            default: r2 = bf.mid ^ bf.high; break;
        }
        
        /* Store to memory */
        array[(i + 1) % 16] = r2;
        
        /* Update register array */
        registers[i % 4] = (registers[i % 4] & 0xFFFF0000) | (r2 & 0xFFFF);
    }
    
    COMPILER_BARRIER();
    printf("Test5 result: %u\n", array[0] + registers[0]);
}

/* Test 6: Pointer-based partial store */
void test_pointer_partial_store(void) {
    volatile unsigned int data = 0x89ABCDEF;
    volatile unsigned char *byte_ptr = (volatile unsigned char *)&data;
    volatile unsigned int sum = 0;
    
    for (int i = 0; i < 100; ++i) {
        /* Store through char pointer - partial update */
        byte_ptr[(i % 4)] = (i * 7) & 0xFF;
        
        /* Extract various bit ranges */
        unsigned int bits_4_11 = (data >> 4) & 0xFF;
        unsigned int bits_12_19 = (data >> 12) & 0xFF;
        unsigned int bits_20_27 = (data >> 20) & 0xFF;
        
        /* Combine with mask operation */
        data = (data & 0xFF000000) | 
               ((bits_4_11 + bits_12_19) << 12) | 
               (bits_20_27 << 4) | 
               (i & 0xF);
        
        sum += data;
    }
    
    COMPILER_BARRIER();
    printf("Test6 result: %u\n", sum);
}

int main(int argc, char *argv[]) {
    int test_to_run = 0;
    
    /* Parse command line argument */
    if (argc > 1) {
        test_to_run = atoi(argv[1]);
    }
    
    /* Run specific test or all tests */
    if (test_to_run == 0 || test_to_run == 1) {
        test_zero_extract_volatile();
    }
    if (test_to_run == 0 || test_to_run == 2) {
        test_zero_extract_struct();
    }
    if (test_to_run == 0 || test_to_run == 3) {
        test_strict_low_part_asm();
    }
    if (test_to_run == 0 || test_to_run == 4) {
        test_strict_low_part_conditional();
    }
    if (test_to_run == 0 || test_to_run == 5) {
        test_mixed_operations();
    }
    if (test_to_run == 0 || test_to_run == 6) {
        test_pointer_partial_store();
    }
    
    /* Force all volatiles to be live at exit */
    volatile int dummy = 0;
    COMPILER_BARRIER();
    
    return 0;
}
