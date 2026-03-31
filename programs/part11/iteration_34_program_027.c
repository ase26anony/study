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
    
    /* Multiple extraction patterns to increase chances */
    for (int i = 0; i < 100; ++i) {
        /* Extract bits 4-11 (8 bits) - should generate ZERO_EXTRACT */
        unsigned int extracted = (source >> 4) & 0xFF;
        result += extracted;
        
        /* Extract bits 16-23 */
        extracted = (source >> 16) & 0xFF;
        result ^= extracted;
        
        /* Variable width extraction */
        int width = (i % 8) + 1;
        unsigned int mask = (1U << width) - 1;
        extracted = (source >> (i % 24)) & mask;
        result |= extracted;
        
        COMPILER_BARRIER();
    }
    
    return result;
}

/* Test 2: Packed struct with bitfields (ZERO_EXTRACT) */
volatile unsigned int test2_packed_struct(void) {
    /* Packed struct to ensure bitfield layout */
    struct __attribute__((packed)) BitFieldStruct {
        unsigned int header : 4;
        unsigned int data   : 20;
        unsigned int footer : 8;
    };
    
    volatile struct BitFieldStruct s = {0};
    volatile unsigned int accumulator = 0;
    
    /* Initialize with pattern */
    s.header = 0xA;
    s.data = 0x12345;
    s.footer = 0xBC;
    
    for (int i = 0; i < 50; ++i) {
        /* Read bitfields - should generate ZERO_EXTRACT */
        unsigned int val1 = s.header;
        unsigned int val2 = s.data;
        unsigned int val3 = s.footer;
        
        /* Write back with modification - may generate both read and write extracts */
        s.data = (s.data + i) & 0xFFFFF;
        s.header = (s.header ^ val2) & 0xF;
        
        accumulator += val1 + val2 + val3;
        COMPILER_BARRIER();
    }
    
    return accumulator + s.header + s.data + s.footer;
}

/* Test 3: Complex bitfield operations (ZERO_EXTRACT) */
volatile unsigned int test3_complex_bitfields(void) {
    /* Struct with overlapping/nested bitfields */
    struct __attribute__((packed)) ComplexBitfield {
        unsigned int a : 3;
        unsigned int b : 7;
        unsigned int c : 10;
        unsigned int d : 12;
    };
    
    volatile struct ComplexBitfield cb = {0};
    volatile unsigned int results[4] = {0};
    
    cb.a = 0x5;
    cb.b = 0x7F;
    cb.c = 0x3FF;
    cb.d = 0xFFF;
    
    for (int i = 0; i < 75; ++i) {
        /* Complex extraction and assignment */
        unsigned int temp = cb.b;
        cb.a = (temp >> 2) & 0x7;  /* Extract 3 bits from b */
        
        temp = cb.c;
        cb.b = (temp + i) & 0x7F;  /* Modify with mask */
        
        /* Cross-field operations */
        cb.d = (cb.a << 9) | (cb.b << 2) | (cb.c & 0x3);
        
        /* Store results to volatile array */
        results[i % 4] = cb.a + cb.b + cb.c + cb.d;
        COMPILER_BARRIER();
    }
    
    return results[0] + results[1] + results[2] + results[3];
}

/* Test 4: STRICT_LOW_PART via conditional partial stores */
volatile unsigned int test4_strict_low_part(void) {
    volatile unsigned int reg = 0x12345678;
    volatile unsigned char *byte_ptr = (volatile unsigned char *)&reg;
    volatile unsigned int result = 0;
    
    for (int i = 0; i < 100; ++i) {
        /* Conditional update of low byte - may generate STRICT_LOW_PART */
        if (i & 1) {
            /* Update only low 8 bits */
            reg = (reg & ~0xFF) | ((i + 0xAB) & 0xFF);
        }
        
        /* Update only low 16 bits based on condition */
        if (i % 3 == 0) {
            reg = (reg & ~0xFFFF) | ((reg + 0x1111) & 0xFFFF);
        }
        
        /* Pointer-based partial store */
        byte_ptr[1] = i ^ 0x55;  /* Modify second byte */
        
        result += reg;
        COMPILER_BARRIER();
    }
    
    return result;
}

/* Test 5: STRICT_LOW_PART via inline assembly and merge operations */
volatile unsigned int test5_asm_partial_stores(void) {
    volatile unsigned int value = 0x87654321;
    volatile unsigned int mask = 0;
    volatile unsigned int result = 0;
    
    for (int i = 0; i < 80; ++i) {
        /* Inline assembly that hints at partial register updates */
        unsigned int temp;
        asm volatile (
            "and %0, %1, %2\n\t"
            : "=r" (temp)
            : "r" (value), "i" (0x00FF00FF)  /* Mask constant */
        );
        
        /* Merge operation - update only parts of the value */
        mask = (i << 8) | 0xFF;
        value = (value & ~mask) | ((value + i) & mask);
        
        /* Switch to create control flow */
        switch (i & 3) {
            case 0:
                value = (value & ~0xF) | 0xA;
                break;
            case 1:
                value = (value & ~0xF0) | 0xB0;
                break;
            case 2:
                value = (value & ~0xF00) | 0xC00;
                break;
            case 3:
                value = (value & ~0xF000) | 0xD000;
                break;
        }
        
        result ^= temp + value;
        COMPILER_BARRIER();
    }
    
    return result;
}

/* Test 6: Mixed operations to ensure MEM_P path is also taken */
volatile unsigned int test6_mixed_operations(void) {
    volatile unsigned int array[16];
    volatile unsigned int regs[4] = {0x11111111, 0x22222222, 0x33333333, 0x44444444};
    volatile unsigned int result = 0;
    
    /* Initialize array */
    for (int i = 0; i < 16; i++) {
        array[i] = i * 0x01010101;
    }
    
    for (int i = 0; i < 60; ++i) {
        /* Memory operations (MEM_P) */
        unsigned int idx = i & 0xF;
        unsigned int mem_val = array[idx];
        
        /* Bitfield extraction from memory value */
        unsigned int extracted = (mem_val >> (i % 24)) & ((1U << 8) - 1);
        
        /* Partial store to register */
        regs[i % 4] = (regs[i % 4] & ~0xFF00) | ((extracted << 8) & 0xFF00);
        
        /* Complex update with bitfields */
        array[idx] = (array[idx] & ~0xF0F0) | 
                    ((regs[0] & 0xF) << 4) | 
                    ((regs[1] & 0xF0) << 8);
        
        result += mem_val + extracted + regs[i % 4];
        COMPILER_BARRIER();
    }
    
    return result;
}

/* Main driver function */
int main(int argc, char *argv[]) {
    volatile unsigned int final_result = 0;
    int run_all = 1;
    int test_to_run = -1;
    
    /* Parse command line arguments */
    if (argc > 1) {
        run_all = 0;
        test_to_run = atoi(argv[1]);
    }
    
    /* Run tests based on arguments */
    if (run_all || test_to_run == 1) {
        final_result += test1_bitfield_extract();
    }
    if (run_all || test_to_run == 2) {
        final_result += test2_packed_struct();
    }
    if (run_all || test_to_run == 3) {
        final_result += test3_complex_bitfields();
    }
    if (run_all || test_to_run == 4) {
        final_result += test4_strict_low_part();
    }
    if (run_all || test_to_run == 5) {
        final_result += test5_asm_partial_stores();
    }
    if (run_all || test_to_run == 6) {
        final_result += test6_mixed_operations();
    }
    
    /* Print result to prevent dead code elimination */
    printf("Result: 0x%08X\n", final_result);
    
    return 0;
}
