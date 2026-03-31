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
    
    /* Multiple bitfield extractions that should generate ZERO_EXTRACT */
    for (int i = 0; i < 100; ++i) {
        /* Extract bits 4-11 */
        unsigned int extract1 = (source >> 4) & ((1U << 8) - 1);
        
        /* Extract bits 12-19 with arithmetic */
        unsigned int extract2 = ((source >> 12) + i) & ((1U << 8) - 1);
        
        /* Extract bits 20-27, combine with previous */
        unsigned int extract3 = (source >> 20) & ((1U << 8) - 1);
        
        result = extract1 + extract2 + extract3;
        COMPILER_BARRIER();
    }
    
    printf("Test1 result: %u\n", result);
}

/* Test 2: Packed struct with bitfields */
struct __attribute__((packed)) BitfieldStruct {
    unsigned int header : 4;
    unsigned int data1  : 12;
    unsigned int data2  : 8;
    unsigned int data3  : 7;
    unsigned int footer : 1;
};

void test_zero_extract_struct(void) {
    volatile struct BitfieldStruct bs = {0};
    volatile unsigned int accumulator = 0;
    
    /* Initialize with pattern */
    *(volatile unsigned int*)&bs = 0x12345678;
    
    for (int i = 0; i < 50; ++i) {
        /* Read bitfields - should generate ZERO_EXTRACT */
        unsigned int val1 = bs.data1;
        unsigned int val2 = bs.data2;
        unsigned int val3 = bs.data3;
        
        /* Write bitfields - may generate ZERO_EXTRACT in SET_DEST */
        bs.data1 = (val1 + i) & 0xFFF;
        bs.data2 = (val2 ^ val3) & 0xFF;
        bs.data3 = (val1 >> 4) & 0x7F;
        
        accumulator += bs.header + bs.data1 + bs.data2 + bs.data3 + bs.footer;
        COMPILER_BARRIER();
    }
    
    printf("Test2 result: %u\n", accumulator);
}

/* Test 3: Complex bitfield operations with arrays */
void test_zero_extract_array(void) {
    volatile unsigned int array[16];
    volatile unsigned int results[16];
    
    /* Initialize array */
    for (int i = 0; i < 16; ++i) {
        array[i] = 0x87654321 ^ (i * 0x11111111);
    }
    
    /* Nested loops with bitfield extractions */
    for (int outer = 0; outer < 10; ++outer) {
        for (int inner = 0; inner < 16; ++inner) {
            /* Extract different bit ranges */
            unsigned int bits_0_7   = array[inner] & 0xFF;
            unsigned int bits_8_15  = (array[inner] >> 8) & 0xFF;
            unsigned int bits_16_23 = (array[inner] >> 16) & 0xFF;
            unsigned int bits_24_31 = (array[inner] >> 24) & 0xFF;
            
            /* Combine with arithmetic */
            results[inner] = (bits_0_7 * bits_8_15) + 
                            (bits_16_23 ^ bits_24_31) + 
                            (inner & 0xF);
            
            /* Update array with rotated bitfields */
            array[(inner + 1) % 16] = ((bits_24_31 << 24) | 
                                      (bits_0_7 << 16) | 
                                      (bits_8_15 << 8) | 
                                      bits_16_23);
        }
        COMPILER_BARRIER();
    }
    
    unsigned int sum = 0;
    for (int i = 0; i < 16; ++i) sum += results[i];
    printf("Test3 result: %u\n", sum);
}

/* Test 4: STRICT_LOW_PART via conditional narrow stores */
void test_strict_low_part_conditional(void) {
    volatile unsigned int data = 0x12345678;
    volatile unsigned int mask = 0;
    volatile unsigned int result = 0;
    
    for (int i = 0; i < 100; ++i) {
        /* Conditional update of low byte - may generate STRICT_LOW_PART */
        if (i & 1) {
            /* Update only low 8 bits */
            data = (data & ~0xFF) | ((i + 0x55) & 0xFF);
        }
        
        /* Conditional update of low 16 bits */
        if (i % 3 == 0) {
            /* Update only low 16 bits */
            data = (data & ~0xFFFF) | ((data + 0x1234) & 0xFFFF);
        }
        
        /* Merge operation preserving high bits */
        mask = (mask << 1) | 1;
        data = (data & ~0xF) | ((data >> 4) & 0xF);
        
        result += data;
        COMPILER_BARRIER();
    }
    
    printf("Test4 result: %u\n", result);
}

/* Test 5: STRICT_LOW_PART via inline assembly and pointer casts */
void test_strict_low_part_asm(void) {
    volatile unsigned int value = 0x98765432;
    volatile unsigned char *byte_ptr;
    volatile unsigned short *short_ptr;
    
    for (int i = 0; i < 50; ++i) {
        /* Update through char pointer - partial store */
        byte_ptr = (volatile unsigned char*)&value;
        byte_ptr[1] = i & 0x7F;  /* Modify only second byte */
        
        /* Update through short pointer */
        short_ptr = (volatile unsigned short*)&value;
        if (i % 2 == 0) {
            *short_ptr = (*short_ptr + 0x100) & 0xFFFF;  /* Modify only low 16 bits */
        }
        
        /* Inline assembly that hints at partial register updates */
        unsigned int temp = value;
        asm volatile (
            "and %0, %1, %2\n\t"
            : "=r" (temp)
            : "r" (temp), "i" (0x00FF00FF)
        );
        value = temp;
        
        COMPILER_BARRIER();
    }
    
    printf("Test5 result: %u\n", value);
}

/* Test 6: Mixed operations with switch statement */
void test_mixed_with_switch(void) {
    volatile unsigned int reg_var = 0;
    register unsigned int r1 asm ("r12") = 0x11111111;
    register unsigned int r2 asm ("r13") = 0x22222222;
    
    volatile unsigned int array[4] = {0xAA, 0xBB, 0xCC, 0xDD};
    volatile unsigned int *ptr = array;
    
    for (int i = 0; i < 75; ++i) {
        /* Bitfield extraction */
        unsigned int field = (r1 >> (i % 24)) & 0xF;
        
        /* Switch on extracted bitfield */
        switch (field & 0x7) {
            case 0:
                /* STRICT_LOW_PART style update */
                reg_var = (reg_var & ~0xF) | (field & 0xF);
                ptr[0] = (ptr[0] & ~0xFF00) | ((field << 8) & 0xFF00);
                break;
            case 1:
                /* ZERO_EXTRACT from memory */
                field = (*ptr >> 16) & 0xFF;
                reg_var = (reg_var & ~0xFF0000) | (field << 16);
                break;
            case 2:
                /* Complex bitfield merge */
                r2 = (r2 & ~0xFFFFF) | ((r1 + r2) & 0xFFFFF);
                break;
            default:
                /* Memory update with bitfield */
                ptr[field % 4] = (ptr[field % 4] & ~0xF0) | 
                                ((reg_var << 4) & 0xF0);
                break;
        }
        
        /* Rotate registers */
        unsigned int temp = r1;
        r1 = r2;
        r2 = temp ^ field;
        
        COMPILER_BARRIER();
    }
    
    printf("Test6 result: %u\n", reg_var + r1 + r2);
}

/* Main driver */
int main(int argc, char *argv[]) {
    unsigned int total_result = 0;
    
    /* Run specific tests based on arguments, or all if no arguments */
    int run_all = (argc == 1);
    
    if (run_all || strstr(argv[0], "1") || (argc > 1 && atoi(argv[1]) == 1)) {
        test_zero_extract_volatile();
        total_result += 1;
    }
    
    if (run_all || strstr(argv[0], "2") || (argc > 1 && atoi(argv[1]) == 2)) {
        test_zero_extract_struct();
        total_result += 2;
    }
    
    if (run_all || strstr(argv[0], "3") || (argc > 1 && atoi(argv[1]) == 3)) {
        test_zero_extract_array();
        total_result += 3;
    }
    
    if (run_all || strstr(argv[0], "4") || (argc > 1 && atoi(argv[1]) == 4)) {
        test_strict_low_part_conditional();
        total_result += 4;
    }
    
    if (run_all || strstr(argv[0], "5") || (argc > 1 && atoi(argv[1]) == 5)) {
        test_strict_low_part_asm();
        total_result += 5;
    }
    
    if (run_all || strstr(argv[0], "6") || (argc > 1 && atoi(argv[1]) == 6)) {
        test_mixed_with_switch();
        total_result += 6;
    }
    
    /* Final aggregate to prevent dead code elimination */
    volatile unsigned int final = total_result;
    printf("Final aggregate: %u\n", final);
    
    return 0;
}
