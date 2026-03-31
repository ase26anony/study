/* test_resource.c - Generate ZERO_EXTRACT and STRICT_LOW_PART RTL patterns */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Compiler barrier to prevent optimization */
#define COMPILER_BARRIER() asm volatile("" ::: "memory")

/* Test 1: Bitfield extraction from volatile integer (ZERO_EXTRACT) */
void test_bitfield_extract_volatile(void) {
    volatile unsigned int source = 0xDEADBEEF;
    volatile unsigned int result = 0;
    
    /* Multiple extraction patterns that may generate ZERO_EXTRACT */
    for (int i = 0; i < 100; ++i) {
        /* Extract bits 4-11 */
        unsigned int extract1 = (source >> 4) & ((1U << 8) - 1);
        
        /* Extract bits 12-23 with variable shift */
        unsigned int shift = i & 0xF;
        unsigned int extract2 = (source >> (12 + shift)) & ((1U << 12) - 1);
        
        /* Combine extractions */
        result = extract1 + extract2;
        
        /* Modify source to create different patterns */
        source = source ^ (result << 3);
        
        COMPILER_BARRIER();
    }
    
    volatile unsigned int sink __attribute__((unused)) = result;
}

/* Test 2: Packed struct with bitfields (ZERO_EXTRACT) */
struct packed_bitfields {
    unsigned int field_a : 5;
    unsigned int field_b : 11;
    unsigned int field_c : 7;
    unsigned int field_d : 9;
} __attribute__((packed));

void test_packed_struct_bitfields(void) {
    volatile struct packed_bitfields data;
    volatile unsigned int accumulator = 0;
    
    /* Initialize with pattern */
    *(volatile unsigned int*)&data = 0x12345678;
    
    for (int i = 0; i < 100; ++i) {
        /* Read bitfields - may generate ZERO_EXTRACT */
        unsigned int val_a = data.field_a;
        unsigned int val_b = data.field_b;
        unsigned int val_c = data.field_c;
        unsigned int val_d = data.field_d;
        
        /* Write bitfields - may generate both read and write patterns */
        data.field_a = (val_b + i) & 0x1F;      /* 5 bits */
        data.field_b = (val_c ^ val_d) & 0x7FF; /* 11 bits */
        data.field_c = (val_a * 3) & 0x7F;      /* 7 bits */
        data.field_d = (val_b - val_c) & 0x1FF; /* 9 bits */
        
        /* Accumulate results */
        accumulator += val_a + val_b + val_c + val_d;
        
        COMPILER_BARRIER();
    }
    
    volatile unsigned int sink __attribute__((unused)) = accumulator;
}

/* Test 3: Conditional narrow store (STRICT_LOW_PART) */
void test_conditional_narrow_store(void) {
    volatile unsigned int data = 0x87654321;
    volatile unsigned int mask_low = 0x000000FF;
    volatile unsigned int mask_word = 0x0000FFFF;
    volatile unsigned int result = 0;
    
    for (int i = 0; i < 100; ++i) {
        /* Conditional update of low byte */
        if (i & 1) {
            /* Pattern that may generate STRICT_LOW_PART */
            data = (data & ~mask_low) | ((i * 37) & mask_low);
        }
        
        /* Conditional update of low word */
        if (i & 2) {
            /* Another potential STRICT_LOW_PART pattern */
            unsigned int new_word = (data >> 8) + i;
            data = (data & ~mask_word) | (new_word & mask_word);
        }
        
        /* Merge operation with mask */
        unsigned int temp = data;
        temp = (temp & ~0x00FF00FF) | ((temp + i) & 0x00FF00FF);
        result += temp;
        
        COMPILER_BARRIER();
    }
    
    volatile unsigned int sink __attribute__((unused)) = result;
}

/* Test 4: Inline assembly for partial register updates (STRICT_LOW_PART) */
void test_inline_asm_partial_store(void) {
    volatile unsigned int reg_var = 0x12345678;
    volatile unsigned int output = 0;
    
    for (int i = 0; i < 50; ++i) {
        unsigned int input = reg_var + i;
        unsigned int result;
        
        /* Inline asm that operates on low parts */
        asm volatile (
            "and %0, %1, %2\n\t"
            "orr %0, %0, %3"
            : "=r" (result)
            : "r" (input), "i" (0x0000FFFF), "r" (i << 16)
            : /* no clobbers */
        );
        
        /* Another asm pattern with byte operation */
        unsigned int byte_result;
        asm volatile (
            "and %0, %1, #0xFF\n\t"
            "lsl %0, %0, #8"
            : "=r" (byte_result)
            : "r" (result)
            : /* no clobbers */
        );
        
        reg_var = result ^ byte_result;
        output += reg_var;
        
        COMPILER_BARRIER();
    }
    
    volatile unsigned int sink __attribute__((unused)) = output;
}

/* Test 5: Mixed operations with memory references */
void test_mixed_operations_with_memory(void) {
    volatile unsigned int array[64];
    volatile unsigned int temp;
    register unsigned int reg1 asm ("r12"); /* Suggest register */
    register unsigned int reg2 asm ("r11"); /* Suggest another register */
    
    /* Initialize array */
    for (int i = 0; i < 64; ++i) {
        array[i] = i * 0x01010101;
    }
    
    reg1 = 0;
    reg2 = 0;
    
    for (int i = 0; i < 100; ++i) {
        /* Memory access that may generate MEM_P patterns */
        unsigned int idx = i & 63;
        temp = array[idx];
        
        /* Bitfield extraction from memory value */
        unsigned int low_bits = (temp >> 8) & 0xFFF; /* 12 bits */
        unsigned int high_bits = (temp >> 20) & 0xFFF; /* 12 bits */
        
        /* Conditional partial store back to memory */
        if (low_bits > high_bits) {
            /* Update only part of the memory word */
            array[idx] = (array[idx] & ~0x00000FFF) | (low_bits & 0x00000FFF);
        }
        
        /* Switch based on extracted bits to create control flow */
        switch (low_bits & 0x7) {
            case 0: reg1 += temp; break;
            case 1: reg1 -= temp; break;
            case 2: reg1 ^= temp; break;
            case 3: reg1 |= temp; break;
            case 4: reg1 &= temp; break;
            case 5: reg1 = reg1 << (temp & 0x3); break;
            case 6: reg1 = reg1 >> (temp & 0x3); break;
            default: reg1 = ~temp; break;
        }
        
        /* Partial update of register variable */
        reg2 = (reg2 & ~0xFFFF) | (reg1 & 0xFFFF);
        
        COMPILER_BARRIER();
    }
    
    volatile unsigned int sink __attribute__((unused)) = reg1 + reg2;
}

/* Test 6: Complex nested bitfield operations */
struct nested_bitfields {
    struct {
        unsigned int a : 3;
        unsigned int b : 5;
        unsigned int c : 4;
    } inner;
    unsigned int d : 20;
} __attribute__((packed));

void test_nested_bitfield_operations(void) {
    volatile struct nested_bitfields nb;
    volatile unsigned int *ptr = (volatile unsigned int*)&nb;
    *ptr = 0x89ABCDEF;
    
    volatile unsigned int sum = 0;
    
    for (int i = 0; i < 100; ++i) {
        /* Nested bitfield accesses */
        unsigned int val_a = nb.inner.a;
        unsigned int val_b = nb.inner.b;
        unsigned int val_c = nb.inner.c;
        unsigned int val_d = nb.d;
        
        /* Complex transformations */
        nb.inner.a = (val_b + val_c) & 0x7;
        nb.inner.b = (val_c ^ val_d) & 0x1F;
        nb.inner.c = (val_a * val_b) & 0xF;
        nb.d = (val_d + (val_a << val_b)) & 0xFFFFF;
        
        /* Extract and combine */
        unsigned int combined = (nb.inner.a << 0) |
                                (nb.inner.b << 3) |
                                (nb.inner.c << 8) |
                                (nb.d << 12);
        
        /* Partial store with mask */
        if (i & 1) {
            *ptr = (*ptr & ~0x000FFFFF) | (combined & 0x000FFFFF);
        }
        
        sum += combined;
        COMPILER_BARRIER();
    }
    
    volatile unsigned int sink __attribute__((unused)) = sum;
}

/* Main driver function */
int main(int argc, char *argv[]) {
    volatile unsigned int total_result = 0;
    
    /* Run tests based on command line arguments or all by default */
    int run_all = (argc <= 1);
    
    if (run_all || strstr(argv[0], "test1") || (argc > 1 && atoi(argv[1]) == 1)) {
        test_bitfield_extract_volatile();
        total_result += 1;
    }
    
    if (run_all || strstr(argv[0], "test2") || (argc > 1 && atoi(argv[1]) == 2)) {
        test_packed_struct_bitfields();
        total_result += 2;
    }
    
    if (run_all || strstr(argv[0], "test3") || (argc > 1 && atoi(argv[1]) == 3)) {
        test_conditional_narrow_store();
        total_result += 3;
    }
    
    if (run_all || strstr(argv[0], "test4") || (argc > 1 && atoi(argv[1]) == 4)) {
        test_inline_asm_partial_store();
        total_result += 4;
    }
    
    if (run_all || strstr(argv[0], "test5") || (argc > 1 && atoi(argv[1]) == 5)) {
        test_mixed_operations_with_memory();
        total_result += 5;
    }
    
    if (run_all || strstr(argv[0], "test6") || (argc > 1 && atoi(argv[1]) == 6)) {
        test_nested_bitfield_operations();
        total_result += 6;
    }
    
    /* Print result to prevent dead code elimination */
    printf("Total result marker: %u\n", total_result);
    
    return 0;
}
