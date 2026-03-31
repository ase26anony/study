/* test_resource.c - Generate RTL patterns for ZERO_EXTRACT and STRICT_LOW_PART coverage */

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
        /* Extract bits 8-15 (8 bits starting at bit 8) */
        unsigned int extracted = (source >> 8) & 0xFF;
        
        /* Combine with arithmetic to create complex pattern */
        extracted = (extracted * 3 + 7) & 0xFF;
        
        /* Store result to volatile to prevent elimination */
        result = extracted;
        
        /* Modify source to vary pattern */
        source = source * 1103515245 + 12345;
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
    s.field_a = 0x1F;  /* 5 bits max */
    s.field_b = 0x7FF; /* 11 bits max */
    s.field_c = 0x7F;  /* 7 bits max */
    s.field_d = 0x1FF; /* 9 bits max */
    
    for (int i = 0; i < 50; ++i) {
        /* Multiple bitfield reads - should generate ZERO_EXTRACT */
        unsigned int a = s.field_a;
        unsigned int b = s.field_b;
        unsigned int c = s.field_c;
        unsigned int d = s.field_d;
        
        /* Complex operation combining bitfields */
        unsigned int combined = (a << 16) | (b << 5) | (c << 12) | d;
        
        /* Write back to different fields */
        s.field_a = (combined >> 16) & 0x1F;
        s.field_b = (combined >> 5) & 0x7FF;
        s.field_c = (combined >> 12) & 0x7F;
        s.field_d = combined & 0x1FF;
        
        /* Store results */
        results[i % 4] = combined;
    }
    
    COMPILER_BARRIER();
    printf("Test2 results: %u %u %u %u\n", 
           results[0], results[1], results[2], results[3]);
}

/* Test 3: STRICT_LOW_PART via conditional narrow stores */
void test_strict_low_part_conditional(void) {
    volatile unsigned int data = 0x12345678;
    volatile unsigned char flag = 1;
    
    for (int i = 0; i < 100; ++i) {
        /* Conditional update of low byte only */
        if (flag) {
            /* This pattern often generates STRICT_LOW_PART:
               (data & ~0xFF) | (new_value & 0xFF) */
            unsigned char new_low = (i * 3) & 0xFF;
            data = (data & ~0xFF) | new_low;
        } else {
            /* Update high byte */
            unsigned char new_high = (i * 5) & 0xFF;
            data = (data & 0x00FFFFFF) | (new_high << 24);
        }
        
        /* Toggle flag */
        flag = !flag;
        
        /* Additional arithmetic to create scheduling pressure */
        data = data * 1664525 + 1013904223;
    }
    
    COMPILER_BARRIER();
    printf("Test3 result: 0x%08X\n", data);
}

/* Test 4: STRICT_LOW_PART via inline assembly hints */
void test_strict_low_part_asm(void) {
    volatile unsigned int value = 0xABCD1234;
    volatile unsigned int mask = 0x0000FFFF;
    
    for (int i = 0; i < 75; ++i) {
        unsigned int temp = value;
        
        /* Inline assembly that hints at partial register update */
        asm volatile (
            "and %0, %1, %2\n\t"
            : "=r" (temp)
            : "r" (temp), "r" (mask)
        );
        
        /* Additional operations to ensure RTL complexity */
        temp = temp ^ (temp >> 16);
        temp = temp * 0x5BD1E995;
        
        /* Store back through volatile pointer */
        volatile unsigned int *ptr = &value;
        *ptr = temp;
        
        /* Modify mask */
        mask = (mask << 1) | (mask >> 31);
    }
    
    COMPILER_BARRIER();
    printf("Test4 result: 0x%08X (mask: 0x%08X)\n", value, mask);
}

/* Test 5: Mixed operations with memory and control flow */
void test_mixed_operations(void) {
    volatile unsigned int array[16];
    volatile struct packed_bitfields structs[4];
    
    /* Initialize arrays */
    for (int i = 0; i < 16; ++i) {
        array[i] = i * 0x11111111;
    }
    
    for (int i = 0; i < 4; ++i) {
        structs[i].field_a = (i * 3) & 0x1F;
        structs[i].field_b = (i * 7) & 0x7FF;
        structs[i].field_c = (i * 11) & 0x7F;
        structs[i].field_d = (i * 13) & 0x1FF;
    }
    
    volatile unsigned int result = 0;
    
    /* Nested loops for scheduling complexity */
    for (int outer = 0; outer < 10; ++outer) {
        for (int inner = 0; inner < 8; ++inner) {
            /* Mix bitfield access with memory operations */
            unsigned int idx = (outer + inner) & 0xF;
            
            /* Bitfield extraction from struct */
            unsigned int bf_val = structs[idx % 4].field_b;
            
            /* Memory access */
            unsigned int mem_val = array[idx];
            
            /* Conditional partial update - potential STRICT_LOW_PART */
            if (bf_val & 1) {
                mem_val = (mem_val & ~0xFFFF) | (bf_val & 0xFFFF);
            }
            
            /* Switch statement for control flow complexity */
            switch (mem_val & 0x7) {
                case 0:
                    result += bf_val;
                    break;
                case 1:
                    result += mem_val;
                    break;
                case 2:
                    result ^= bf_val;
                    break;
                case 3:
                    result |= mem_val;
                    break;
                default:
                    result = result * 3 + 1;
                    break;
            }
            
            /* Update array with partial store */
            array[idx] = (array[idx] & 0xFFFF0000) | (result & 0xFFFF);
        }
    }
    
    COMPILER_BARRIER();
    printf("Test5 result: 0x%08X\n", result);
}

/* Test 6: Pointer-based partial updates */
void test_pointer_partial_updates(void) {
    volatile unsigned int data = 0x87654321;
    volatile unsigned char *byte_ptr = (volatile unsigned char *)&data;
    
    for (int i = 0; i < 100; ++i) {
        /* Update individual bytes - may generate partial store patterns */
        byte_ptr[i % 4] = (i * 7) & 0xFF;
        
        /* Every 10 iterations, do a full word update */
        if (i % 10 == 0) {
            /* This creates a mix of full and partial updates */
            data = data ^ 0xAAAAAAAA;
        }
        
        /* Bitfield-like extraction using pointer arithmetic */
        unsigned int extracted = (data >> 8) & 0xFF;
        extracted = extracted * 11;
        
        /* Conditional merge back */
        if (extracted & 1) {
            data = (data & ~0xFF00) | ((extracted & 0xFF) << 8);
        }
    }
    
    COMPILER_BARRIER();
    printf("Test6 result: 0x%08X\n", data);
}

int main(int argc, char *argv[]) {
    int test_to_run = 0;
    
    /* Parse command line argument to select test */
    if (argc > 1) {
        test_to_run = atoi(argv[1]);
    }
    
    /* Run selected test or all tests */
    switch (test_to_run) {
        case 1:
            test_zero_extract_volatile();
            break;
        case 2:
            test_zero_extract_struct();
            break;
        case 3:
            test_strict_low_part_conditional();
            break;
        case 4:
            test_strict_low_part_asm();
            break;
        case 5:
            test_mixed_operations();
            break;
        case 6:
            test_pointer_partial_updates();
            break;
        default:
            /* Run all tests */
            printf("Running all tests...\n");
            test_zero_extract_volatile();
            test_zero_extract_struct();
            test_strict_low_part_conditional();
            test_strict_low_part_asm();
            test_mixed_operations();
            test_pointer_partial_updates();
            break;
    }
    
    return 0;
}
