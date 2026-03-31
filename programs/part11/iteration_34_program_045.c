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
    
    /* This should generate ZERO_EXTRACT RTL */
    for (int i = 0; i < 100; ++i) {
        /* Extract bits 8-15 (8 bits wide) */
        unsigned int extracted = (source >> 8) & 0xFF;
        result += extracted;
        
        /* Extract bits 4-10 (7 bits wide) */
        extracted = (source >> 4) & 0x7F;
        result ^= extracted;
        
        /* Extract bits 16-23 with variable width */
        int width = 8;
        extracted = (source >> 16) & ((1U << width) - 1);
        result |= extracted;
        
        COMPILER_BARRIER();
    }
    
    /* Use result to prevent dead code elimination */
    volatile unsigned int sink = result;
    (void)sink;
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
        
        /* Write back with transformations */
        s.field1 = (val2 + i) & 0x1F;
        s.field2 = (val3 ^ val4) & 0x7FF;
        s.field3 = (val1 * 2) & 0x7F;
        s.field4 = (val2 >> 3) & 0x1FF;
        
        accumulator += s.field1 + s.field2 + s.field3 + s.field4;
        COMPILER_BARRIER();
    }
    
    volatile unsigned int sink = accumulator;
    (void)sink;
}

/* Test 3: Complex bitfield operations with arrays */
void test_zero_extract_array(void) {
    volatile unsigned int data[16];
    volatile unsigned int results[16];
    
    /* Initialize array */
    for (int i = 0; i < 16; ++i) {
        data[i] = 0x12345678 ^ (i * 0x11111111);
    }
    
    for (int iter = 0; iter < 25; ++iter) {
        for (int i = 0; i < 16; ++i) {
            /* Extract different bit ranges */
            unsigned int val = data[i];
            
            /* Multiple extractions */
            unsigned int low4 = val & 0xF;
            unsigned int mid8 = (val >> 8) & 0xFF;
            unsigned int high12 = (val >> 20) & 0xFFF;
            
            /* Combine and store */
            results[i] = (low4 << 24) | (mid8 << 12) | high12;
            
            /* Update source with rotated bits */
            data[(i + 1) % 16] = ((val << 4) | (val >> 28)) & 0xFFFFFFFF;
        }
        COMPILER_BARRIER();
    }
    
    /* Use results */
    volatile unsigned int sum = 0;
    for (int i = 0; i < 16; ++i) {
        sum += results[i];
    }
    (void)sum;
}

/* Test 4: STRICT_LOW_PART via inline assembly */
void test_strict_low_part_asm(void) {
    register unsigned int reg_var1 asm("r12") = 0x12345678;
    register unsigned int reg_var2 asm("r13") = 0x9ABCDEF0;
    volatile unsigned int output;
    
    for (int i = 0; i < 100; ++i) {
        /* Inline assembly that modifies only low parts */
        asm volatile (
            "and %0, %1, %2\n\t"
            "orr %0, %0, %3"
            : "=r" (output)
            : "r" (reg_var1), "i" (0x0000FFFF), "r" (reg_var2 & 0xFFFF0000)
            : /* No clobbers */
        );
        
        /* Conditional update of low byte */
        if (i & 1) {
            asm volatile (
                "and %0, %1, %2"
                : "+r" (output)
                : "r" (output), "i" (0xFFFFFF00)
            );
        }
        
        reg_var1 = output ^ 0x55555555;
        reg_var2 = output + 0x11111111;
        COMPILER_BARRIER();
    }
    
    volatile unsigned int sink = output;
    (void)sink;
}

/* Test 5: STRICT_LOW_PART via conditional merge operations */
void test_strict_low_part_merge(void) {
    volatile unsigned int var = 0x87654321;
    volatile unsigned char flag = 1;
    
    for (int i = 0; i < 200; ++i) {
        unsigned int new_val = i * 0x01010101;
        
        /* Conditional partial update - may generate STRICT_LOW_PART */
        if (flag) {
            /* Update only low 16 bits */
            var = (var & ~0xFFFF) | (new_val & 0xFFFF);
        } else {
            /* Update only bits 8-23 */
            var = (var & ~0x00FFFF00) | (new_val & 0x00FFFF00);
        }
        
        /* Toggle flag */
        flag ^= 1;
        
        /* Another pattern: merge based on condition */
        unsigned int mask = (i % 3 == 0) ? 0xFF : 0xFFFF;
        var = (var & ~mask) | (new_val & mask);
        
        COMPILER_BARRIER();
    }
    
    volatile unsigned int sink = var;
    (void)sink;
}

/* Test 6: Mixed operations with memory references */
void test_mixed_operations(void) {
    volatile unsigned int mem_buffer[64];
    volatile unsigned int *ptr = mem_buffer;
    register unsigned int reg1 asm("r10") = 0;
    register unsigned int reg2 asm("r11") = 0;
    
    /* Initialize memory */
    for (int i = 0; i < 64; ++i) {
        mem_buffer[i] = i * 0x03030303;
    }
    
    for (int i = 0; i < 100; ++i) {
        /* Memory load with bitfield extraction */
        unsigned int loaded = ptr[i % 64];
        unsigned int extracted = (loaded >> (i % 16)) & ((1U << 8) - 1);
        
        /* STRICT_LOW_PART-like update */
        reg1 = (reg1 & ~0xFF) | (extracted & 0xFF);
        
        /* Another extraction from register */
        extracted = (reg1 >> 4) & 0x0F;
        
        /* Store with partial update to memory */
        unsigned int old = ptr[(i + 1) % 64];
        ptr[(i + 1) % 64] = (old & ~0x0000FF00) | ((extracted << 8) & 0x0000FF00);
        
        /* Switch statement based on extracted value */
        switch (extracted & 0x7) {
            case 0: reg2 += 1; break;
            case 1: reg2 += loaded; break;
            case 2: reg2 ^= extracted; break;
            case 3: reg2 = reg2 >> 1; break;
            case 4: reg2 = reg2 << 1; break;
            case 5: reg2 = reg2 | 0x80000000; break;
            case 6: reg2 = reg2 & 0x7FFFFFFF; break;
            default: reg2 = ~reg2; break;
        }
        
        COMPILER_BARRIER();
    }
    
    /* Use results */
    volatile unsigned int sum = reg1 + reg2;
    for (int i = 0; i < 64; ++i) {
        sum += mem_buffer[i];
    }
    (void)sum;
}

/* Main driver */
int main(int argc, char *argv[]) {
    int test_to_run = 0;
    
    /* Parse command line argument */
    if (argc > 1) {
        test_to_run = atoi(argv[1]);
    }
    
    volatile unsigned int final_result = 0;
    
    /* Run selected test or all tests */
    if (test_to_run == 0 || test_to_run == 1) {
        test_zero_extract_volatile();
        final_result += 1;
    }
    if (test_to_run == 0 || test_to_run == 2) {
        test_zero_extract_struct();
        final_result += 2;
    }
    if (test_to_run == 0 || test_to_run == 3) {
        test_zero_extract_array();
        final_result += 3;
    }
    if (test_to_run == 0 || test_to_run == 4) {
        test_strict_low_part_asm();
        final_result += 4;
    }
    if (test_to_run == 0 || test_to_run == 5) {
        test_strict_low_part_merge();
        final_result += 5;
    }
    if (test_to_run == 0 || test_to_run == 6) {
        test_mixed_operations();
        final_result += 6;
    }
    
    /* Print result to prevent optimization */
    printf("Test result: %u\n", final_result);
    
    return 0;
}
