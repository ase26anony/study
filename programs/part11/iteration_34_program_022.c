/* test_resource.c - Generate ZERO_EXTRACT and STRICT_LOW_PART RTL patterns */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Compiler barrier to prevent optimization */
#define COMPILER_BARRIER() asm volatile("" ::: "memory")

/* Test 1: Bitfield extraction from volatile integer (ZERO_EXTRACT) */
volatile unsigned int test1_bitfield_extract(void) {
    volatile unsigned int result = 0;
    volatile unsigned int source = 0xDEADBEEF;
    
    /* Multiple extraction patterns to increase chances */
    for (int i = 0; i < 100; ++i) {
        /* Extract bits 4-11 (8 bits) */
        unsigned int extracted = (source >> 4) & 0xFF;
        result += extracted;
        
        /* Extract bits 16-23 */
        extracted = (source >> 16) & 0xFF;
        result ^= extracted;
        
        /* Extract variable-width field */
        int width = (i % 8) + 1;
        int start = (i % 16);
        unsigned int mask = (1U << width) - 1;
        extracted = (source >> start) & mask;
        result |= extracted;
        
        COMPILER_BARRIER();
    }
    
    return result;
}

/* Test 2: Packed struct with bitfields (ZERO_EXTRACT) */
struct __attribute__((packed)) packed_bitfields {
    unsigned int header : 4;
    unsigned int data   : 20;
    unsigned int footer : 8;
};

volatile unsigned int test2_packed_struct(void) {
    volatile struct packed_bitfields s = {0};
    volatile unsigned int result = 0;
    
    /* Initialize with pattern */
    s.header = 0xA;
    s.data = 0x12345;
    s.footer = 0xBC;
    
    for (int i = 0; i < 50; ++i) {
        /* Read bitfields - should generate ZERO_EXTRACT */
        unsigned int header_val = s.header;
        unsigned int data_val = s.data;
        unsigned int footer_val = s.footer;
        
        /* Complex bitfield manipulation */
        s.data = (s.data + header_val) & 0xFFFFF;
        s.footer = (s.footer ^ data_val) & 0xFF;
        
        result += header_val + data_val + footer_val;
        
        /* Nested loop for scheduling complexity */
        for (int j = 0; j < 10; ++j) {
            s.header = (s.header + j) & 0xF;
            COMPILER_BARRIER();
        }
    }
    
    return result;
}

/* Test 3: Inline assembly for partial register updates (STRICT_LOW_PART) */
volatile unsigned int test3_asm_partial_store(void) {
    volatile unsigned int value = 0x12345678;
    volatile unsigned int result = 0;
    
    for (int i = 0; i < 100; ++i) {
        /* Inline asm that operates on low parts */
        unsigned int temp;
        
        /* Mask lower 8 bits */
        asm volatile (
            "and %0, %1, %2"
            : "=r"(temp)
            : "r"(value), "i"(0xFF)
        );
        result += temp;
        
        /* Mask lower 16 bits */
        asm volatile (
            "and %0, %1, %2"
            : "=r"(temp)
            : "r"(value), "i"(0xFFFF)
        );
        result ^= temp;
        
        /* Update only low byte */
        unsigned char low_byte = i & 0xFF;
        asm volatile (
            "mov %0, %1"
            : "=r"(temp)
            : "r"((unsigned int)low_byte)
        );
        value = (value & ~0xFF) | temp;
        
        COMPILER_BARRIER();
    }
    
    return result;
}

/* Test 4: Conditional merge operations (STRICT_LOW_PART) */
volatile unsigned int test4_conditional_merge(void) {
    volatile unsigned int value = 0;
    volatile unsigned int result = 0;
    volatile int condition = 1;
    
    for (int i = 0; i < 200; ++i) {
        /* Conditional update of low byte */
        if (condition) {
            unsigned int new_byte = (i * 7) & 0xFF;
            value = (value & ~0xFF) | (new_byte & 0xFF);
        }
        
        /* Conditional update of low 16 bits */
        if (i % 3 == 0) {
            unsigned int new_word = (i * 13) & 0xFFFF;
            value = (value & ~0xFFFF) | (new_word & 0xFFFF);
        }
        
        /* Switch statement for control flow complexity */
        switch (i % 4) {
            case 0:
                value = (value & ~0xF) | 0x1;
                break;
            case 1:
                value = (value & ~0xF0) | 0x20;
                break;
            case 2:
                value = (value & ~0xF00) | 0x300;
                break;
            case 3:
                value = (value & ~0xF000) | 0x4000;
                break;
        }
        
        result += value;
        condition = !condition;
        COMPILER_BARRIER();
    }
    
    return result;
}

/* Test 5: Mixed operations with memory references */
volatile unsigned int test5_mixed_operations(void) {
    volatile unsigned int array[256];
    volatile unsigned int result = 0;
    
    /* Initialize array */
    for (int i = 0; i < 256; ++i) {
        array[i] = i * 0x01010101;
    }
    
    /* Use register variables for allocation pressure */
    register unsigned int r1 asm ("r12") = 0;
    register unsigned int r2 asm ("r13") = 0;
    
    for (int i = 0; i < 100; ++i) {
        /* Memory access combined with bitfield ops */
        volatile unsigned int* ptr = &array[i % 256];
        
        /* Read and extract bits */
        unsigned int val = *ptr;
        unsigned int extracted = (val >> 8) & 0xFF;  /* ZERO_EXTRACT */
        
        /* Partial store back */
        *ptr = (*ptr & ~0xFF) | (extracted & 0xFF);  /* STRICT_LOW_PART pattern */
        
        /* Use register variables */
        r1 = (r1 << 1) | (extracted & 1);
        r2 = (r2 >> 1) | ((extracted & 0x80) << 24);
        
        /* Pointer arithmetic */
        ptr += (extracted & 0x3);
        if (ptr >= &array[256]) ptr = &array[0];
        
        result += *ptr + r1 + r2;
        COMPILER_BARRIER();
    }
    
    return result;
}

/* Test 6: Complex nested bitfield operations */
struct __attribute__((packed)) nested_bitfields {
    struct {
        unsigned int a : 3;
        unsigned int b : 5;
        unsigned int c : 12;
    } inner;
    unsigned int d : 8;
    unsigned int e : 4;
};

volatile unsigned int test6_nested_bitfields(void) {
    volatile struct nested_bitfields nb = {{0}};
    volatile unsigned int result = 0;
    
    for (int i = 0; i < 150; ++i) {
        /* Complex bitfield chain */
        nb.inner.a = (i >> 0) & 0x7;
        nb.inner.b = (i >> 3) & 0x1F;
        nb.inner.c = (i >> 8) & 0xFFF;
        nb.d = (i >> 20) & 0xFF;
        nb.e = (i >> 28) & 0xF;
        
        /* Extract and combine */
        unsigned int combined = 
            (nb.inner.a << 0) |
            (nb.inner.b << 3) |
            (nb.inner.c << 8) |
            (nb.d << 20) |
            (nb.e << 28);
        
        /* Conditional partial update */
        if (combined & 1) {
            nb.inner.c = (nb.inner.c + 1) & 0xFFF;
        }
        
        /* Nested loop with memory barrier */
        for (int j = 0; j < 5; ++j) {
            nb.d = (nb.d + j) & 0xFF;
            COMPILER_BARRIER();
        }
        
        result += combined;
    }
    
    return result;
}

/* Main driver */
int main(int argc, char *argv[]) {
    volatile unsigned int final_result = 0;
    
    /* Run tests based on command line or all by default */
    int run_all = (argc == 1);
    
    if (run_all || strstr(argv[0], "1") || (argc > 1 && atoi(argv[1]) == 1)) {
        final_result += test1_bitfield_extract();
    }
    
    if (run_all || strstr(argv[0], "2") || (argc > 1 && atoi(argv[1]) == 2)) {
        final_result += test2_packed_struct();
    }
    
    if (run_all || strstr(argv[0], "3") || (argc > 1 && atoi(argv[1]) == 3)) {
        final_result += test3_asm_partial_store();
    }
    
    if (run_all || strstr(argv[0], "4") || (argc > 1 && atoi(argv[1]) == 4)) {
        final_result += test4_conditional_merge();
    }
    
    if (run_all || strstr(argv[0], "5") || (argc > 1 && atoi(argv[1]) == 5)) {
        final_result += test5_mixed_operations();
    }
    
    if (run_all || strstr(argv[0], "6") || (argc > 1 && atoi(argv[1]) == 6)) {
        final_result += test6_nested_bitfields();
    }
    
    /* Print result to prevent dead code elimination */
    printf("Result: %u\n", final_result);
    
    return 0;
}
