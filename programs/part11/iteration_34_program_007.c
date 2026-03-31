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
        unsigned int mask1 = (1U << 8) - 1;
        result = (source >> 4) & mask1;
        
        /* Extract bits 16-23 with different width */
        unsigned int mask2 = (1U << 8) - 1;
        result ^= (source >> 16) & mask2;
        
        /* Extract bits 0-7 and combine */
        unsigned int mask3 = 0xFF;
        result |= (source & mask3);
        
        /* Modify source slightly each iteration */
        source = source * 1103515245U + 12345U;
    }
    
    COMPILER_BARRIER();
    printf("Test1 result: %u\n", result);
}

/* Test 2: Packed struct with bitfields */
struct __attribute__((packed)) packed_bitfields {
    unsigned int field_a : 5;
    unsigned int field_b : 11;
    unsigned int field_c : 7;
    unsigned int field_d : 9;
};

void test_zero_extract_struct(void) {
    volatile struct packed_bitfields s = {0};
    volatile unsigned int results[4] = {0};
    
    /* Initialize with pattern */
    s.field_a = 0x1F;
    s.field_b = 0x7FF;
    s.field_c = 0x7F;
    s.field_d = 0x1FF;
    
    for (int i = 0; i < 50; ++i) {
        /* Read bitfields - should generate ZERO_EXTRACT */
        results[0] = s.field_a;
        results[1] = s.field_b;
        results[2] = s.field_c;
        results[3] = s.field_d;
        
        /* Write with arithmetic - may generate complex patterns */
        s.field_a = (s.field_b + i) & 0x1F;
        s.field_c = (s.field_d - s.field_a) & 0x7F;
        
        /* Cross-field operations */
        s.field_b = (s.field_a << 6) | (s.field_c & 0x3F);
    }
    
    COMPILER_BARRIER();
    printf("Test2 results: %u %u %u %u\n", 
           results[0], results[1], results[2], results[3]);
}

/* Test 3: Mixed bitfield operations with arrays */
void test_zero_extract_array(void) {
    volatile unsigned int data[16];
    volatile unsigned int output[16];
    
    /* Initialize array */
    for (int i = 0; i < 16; ++i) {
        data[i] = 0x12345678 ^ (i * 0x11111111);
    }
    
    for (int i = 0; i < 100; ++i) {
        int idx = i & 0xF;
        
        /* Various bitfield extractions */
        unsigned int val = data[idx];
        
        /* Extract different bit ranges */
        output[idx] = (val >> 0) & 0xF;          /* bits 0-3 */
        output[(idx + 1) & 0xF] = (val >> 4) & 0xFF;   /* bits 4-11 */
        output[(idx + 2) & 0xF] = (val >> 12) & 0x7FF; /* bits 12-22 */
        output[(idx + 3) & 0xF] = (val >> 23) & 0x1FF; /* bits 23-31 */
        
        /* Update source with extracted bits */
        data[idx] = (output[idx] << 24) | (output[(idx + 1) & 0xF] << 16) |
                    (output[(idx + 2) & 0xF] << 8) | output[(idx + 3) & 0xF];
    }
    
    COMPILER_BARRIER();
    unsigned int sum = 0;
    for (int i = 0; i < 16; ++i) sum += output[i];
    printf("Test3 sum: %u\n", sum);
}

/* Test 4: STRICT_LOW_PART via conditional narrow stores */
void test_strict_low_part_conditional(void) {
    volatile unsigned int reg = 0x87654321;
    volatile unsigned char flag = 1;
    
    for (int i = 0; i < 200; ++i) {
        /* Conditional update of low byte - may generate STRICT_LOW_PART */
        if (flag) {
            /* Update only low 8 bits */
            reg = (reg & ~0xFF) | ((i & 0xFF) & 0xFF);
        }
        
        /* Conditional update of low 16 bits */
        if (i % 3 == 0) {
            reg = (reg & ~0xFFFF) | ((i * 0x1234) & 0xFFFF);
        }
        
        /* Update low 4 bits based on condition */
        if (i % 5 == 0) {
            reg = (reg & ~0xF) | ((i + 0xA) & 0xF);
        }
        
        /* Toggle flag */
        flag = !flag;
    }
    
    COMPILER_BARRIER();
    printf("Test4 reg: 0x%08x\n", reg);
}

/* Test 5: STRICT_LOW_PART via inline assembly and pointer casts */
void test_strict_low_part_asm(void) {
    volatile unsigned int value = 0xABCD1234;
    volatile unsigned int result = 0;
    
    for (int i = 0; i < 100; ++i) {
        /* Inline assembly that operates on low bits */
        asm volatile (
            "and %0, %1, %2\n\t"
            : "=r"(result)
            : "r"(value), "i"(0x00FFFFFF)  /* Keep only low 24 bits */
        );
        
        /* Pointer cast for partial update */
        volatile unsigned char *byte_ptr = (volatile unsigned char*)&value;
        byte_ptr[1] = (i * 7) & 0xFF;  /* Update second byte only */
        
        /* Another inline asm with mask */
        unsigned int temp;
        asm volatile (
            "bic %0, %1, %2\n\t"  /* Bit clear */
            : "=r"(temp)
            : "r"(value), "i"(0xFF000000)
        );
        value = temp | ((i << 24) & 0xFF000000);
    }
    
    COMPILER_BARRIER();
    printf("Test5 value: 0x%08x, result: 0x%08x\n", value, result);
}

/* Test 6: Complex mixed pattern with switch */
void test_mixed_pattern_switch(void) {
    volatile unsigned int state = 0;
    volatile unsigned int accum = 0;
    
    struct __attribute__((packed)) control_reg {
        unsigned int mode : 3;
        unsigned int enable : 1;
        unsigned int count : 10;
        unsigned int reserved : 18;
    } reg = {0};
    
    for (int i = 0; i < 300; ++i) {
        /* Update bitfields */
        reg.count = (reg.count + 1) & 0x3FF;
        reg.mode = (reg.mode + (i & 0x7)) & 0x7;
        reg.enable = (i % 50) > 25;
        
        /* Switch on extracted bitfield - creates control flow */
        switch (reg.mode) {
            case 0:
                /* STRICT_LOW_PART style update */
                state = (state & ~0xFFF) | (i & 0xFFF);
                break;
            case 1:
                /* ZERO_EXTRACT from bitfield */
                accum += reg.count;
                break;
            case 2:
                /* Partial update */
                state = (state & ~0xF0000) | ((i << 16) & 0xF0000);
                break;
            case 3:
                /* Bitfield extraction and store */
                reg.count = (state >> 8) & 0x3FF;
                break;
            default:
                /* Mixed operation */
                state = (state << 1) | (reg.enable & 1);
                break;
        }
        
        /* Array access to create MEM_P references */
        volatile unsigned int mem_buffer[8];
        for (int j = 0; j < 8; ++j) {
            mem_buffer[j] = state + j;
        }
        accum += mem_buffer[i & 0x7];
    }
    
    COMPILER_BARRIER();
    printf("Test6 state: 0x%08x, accum: %u\n", state, accum);
}

/* Main driver */
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
        test_zero_extract_array();
    }
    if (test_to_run == 0 || test_to_run == 4) {
        test_strict_low_part_conditional();
    }
    if (test_to_run == 0 || test_to_run == 5) {
        test_strict_low_part_asm();
    }
    if (test_to_run == 0 || test_to_run == 6) {
        test_mixed_pattern_switch();
    }
    
    return 0;
}
