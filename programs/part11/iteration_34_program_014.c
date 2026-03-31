/* test_resource.c - Test program for GCC RTL resource coverage */
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
        /* Extract bits 4-11 (8 bits) */
        unsigned int extracted = (source >> 4) & 0xFF;
        result += extracted;
        
        /* Extract bits 12-19 with different width */
        extracted = (source >> 12) & 0x7F;  /* 7 bits */
        result += extracted;
        
        /* Extract bits 20-31 (12 bits) */
        extracted = (source >> 20) & 0xFFF;
        result += extracted;
        
        /* Modify source to create variation */
        source = source * 1103515245 + 12345;
        COMPILER_BARRIER();
    }
    
    /* Use result to prevent dead code elimination */
    volatile unsigned int sink = result;
    (void)sink;
}

/* Test 2: Packed struct with bitfields */
void test_zero_extract_packed_struct(void) {
    /* Packed struct with various bitfield widths */
    struct __attribute__((packed)) BitFieldStruct {
        unsigned int field1 : 3;
        unsigned int field2 : 7;
        unsigned int field3 : 11;
        unsigned int field4 : 5;
        unsigned int field5 : 6;
    };
    
    volatile struct BitFieldStruct s = {0};
    volatile unsigned int accumulator = 0;
    
    /* Initialize with pattern */
    s.field1 = 0x7;
    s.field2 = 0x7F;
    s.field3 = 0x7FF;
    s.field4 = 0x1F;
    s.field5 = 0x3F;
    
    /* Complex bitfield operations in loop */
    for (int i = 0; i < 50; ++i) {
        /* Read bitfields (should generate ZERO_EXTRACT) */
        unsigned int val1 = s.field1;
        unsigned int val2 = s.field2;
        unsigned int val3 = s.field3;
        
        /* Write bitfields with arithmetic */
        s.field1 = (val2 + i) & 0x7;
        s.field2 = (val3 ^ val1) & 0x7F;
        s.field3 = (s.field3 * 3 + s.field4) & 0x7FF;
        
        /* Cross-field operations */
        s.field4 = (s.field5 - s.field2) & 0x1F;
        s.field5 = (s.field1 | s.field3) & 0x3F;
        
        accumulator += s.field1 + s.field2 + s.field3 + s.field4 + s.field5;
        COMPILER_BARRIER();
    }
    
    volatile unsigned int sink = accumulator;
    (void)sink;
}

/* Test 3: STRICT_LOW_PART via inline assembly */
void test_strict_low_part_asm(void) {
    volatile unsigned int data = 0x12345678;
    volatile unsigned int result = 0;
    
    for (int i = 0; i < 100; ++i) {
        unsigned int temp = data;
        
        /* Inline assembly that modifies only low parts */
        /* Mask low 8 bits */
        asm volatile (
            "and %0, %1, %2"
            : "=r" (temp)
            : "r" (temp), "i" (0xFF)
        );
        
        /* Mask low 16 bits */
        asm volatile (
            "and %0, %1, %2"
            : "+r" (temp)
            : "i" (0xFFFF)
        );
        
        /* Extract and set low 4 bits */
        unsigned int low_bits = temp & 0xF;
        asm volatile (
            "orr %0, %1, %2"
            : "=r" (temp)
            : "r" (temp & ~0xF), "r" (low_bits)
        );
        
        result += temp;
        data = data * 1664525 + 1013904223;
        COMPILER_BARRIER();
    }
    
    volatile unsigned int sink = result;
    (void)sink;
}

/* Test 4: STRICT_LOW_PART via conditional merge operations */
void test_strict_low_part_conditional(void) {
    volatile unsigned int var = 0x87654321;
    volatile unsigned int mask = 0;
    volatile unsigned int result = 0;
    
    for (int i = 0; i < 100; ++i) {
        /* Conditional update of low byte */
        if (i & 1) {
            var = (var & ~0xFF) | ((i * 7) & 0xFF);
        }
        
        /* Conditional update of low 16 bits */
        if (i & 2) {
            var = (var & ~0xFFFF) | ((i * 13) & 0xFFFF);
        }
        
        /* Complex conditional merge */
        mask = (i * 17) & 0xF;
        if (mask) {
            var = (var & ~0xF) | (mask & 0xF);
        }
        
        /* Update low 12 bits based on condition */
        unsigned int new_val = (i * 23) & 0xFFF;
        var = (var & ~0xFFF) | (new_val & 0xFFF);
        
        result += var;
        COMPILER_BARRIER();
    }
    
    volatile unsigned int sink = result;
    (void)sink;
}

/* Test 5: Mixed operations with memory references */
void test_mixed_operations(void) {
    /* Array for memory operations */
    volatile unsigned int array[64];
    for (int i = 0; i < 64; ++i) {
        array[i] = i * 0x01010101;
    }
    
    /* Packed struct for bitfields */
    struct __attribute__((packed)) MixedStruct {
        unsigned int header : 8;
        unsigned int data : 16;
        unsigned int footer : 8;
    };
    
    volatile struct MixedStruct ms = {0};
    volatile unsigned int sum = 0;
    
    /* Use register variables to increase allocation pressure */
    register int i;
    
    for (i = 0; i < 100; ++i) {
        /* Bitfield extraction */
        ms.header = (array[i % 64] >> 24) & 0xFF;
        ms.data = (array[i % 64] >> 8) & 0xFFFF;
        ms.footer = array[i % 64] & 0xFF;
        
        /* STRICT_LOW_PART-like operation */
        unsigned int temp = array[(i + 1) % 64];
        if (ms.header & 1) {
            temp = (temp & ~0xFF) | (ms.footer & 0xFF);
        }
        
        /* Switch based on bitfield to create control flow */
        switch (ms.data & 0x7) {
            case 0:
                temp &= 0xFFFFFF00;
                break;
            case 1:
                temp |= 0x000000FF;
                break;
            case 2:
                temp = (temp & 0xFFFF0000) | (ms.data & 0xFFFF);
                break;
            default:
                temp ^= 0x00FF00FF;
                break;
        }
        
        /* Store back with partial update */
        array[i % 64] = (array[i % 64] & 0xFF000000) | (temp & 0x00FFFFFF);
        
        /* Extract from struct */
        sum += ms.header + ms.data + ms.footer;
        COMPILER_BARRIER();
    }
    
    volatile unsigned int sink = sum;
    (void)sink;
}

/* Test 6: Complex nested bitfield operations */
void test_complex_nested(void) {
    /* Nested packed structs */
    struct __attribute__((packed)) Inner {
        unsigned int a : 4;
        unsigned int b : 12;
        unsigned int c : 8;
    };
    
    struct __attribute__((packed)) Outer {
        struct Inner inner;
        unsigned int d : 20;
        unsigned int e : 12;
    };
    
    volatile struct Outer outer = {0};
    volatile unsigned int counter = 0;
    
    /* Initialize */
    outer.inner.a = 0xF;
    outer.inner.b = 0xFFF;
    outer.inner.c = 0xFF;
    outer.d = 0xFFFFF;
    outer.e = 0xFFF;
    
    for (int i = 0; i < 75; ++i) {
        /* Complex bitfield extraction chain */
        unsigned int val_a = outer.inner.a;
        unsigned int val_b = outer.inner.b;
        unsigned int val_c = outer.inner.c;
        
        /* Cross-field operations that may generate ZERO_EXTRACT */
        outer.inner.a = (val_b >> 4) & 0xF;
        outer.inner.b = ((val_c << 4) | val_a) & 0xFFF;
        outer.inner.c = (val_b ^ val_a) & 0xFF;
        
        /* Update outer fields with partial results */
        outer.d = (outer.d & 0xFFF00) | ((val_c << 8) & 0xFFF00);
        outer.e = (outer.e & 0xF00) | (val_a & 0xFF);
        
        /* Conditional partial store */
        if (i & 1) {
            outer.d = (outer.d & ~0xFFF) | (i & 0xFFF);
        }
        
        counter += outer.inner.a + outer.inner.b + outer.inner.c + outer.d + outer.e;
        COMPILER_BARRIER();
    }
    
    volatile unsigned int sink = counter;
    (void)sink;
}

/* Main driver function */
int main(int argc, char *argv[]) {
    volatile unsigned int total_result = 0;
    
    /* Default: run all tests if no arguments */
    int run_all = (argc <= 1);
    
    /* Run tests based on arguments */
    if (run_all || strstr(argv[0], "test1") || (argc > 1 && atoi(argv[1]) == 1)) {
        test_zero_extract_volatile();
        total_result += 1;
    }
    
    if (run_all || strstr(argv[0], "test2") || (argc > 1 && atoi(argv[1]) == 2)) {
        test_zero_extract_packed_struct();
        total_result += 2;
    }
    
    if (run_all || strstr(argv[0], "test3") || (argc > 1 && atoi(argv[1]) == 3)) {
        test_strict_low_part_asm();
        total_result += 3;
    }
    
    if (run_all || strstr(argv[0], "test4") || (argc > 1 && atoi(argv[1]) == 4)) {
        test_strict_low_part_conditional();
        total_result += 4;
    }
    
    if (run_all || strstr(argv[0], "test5") || (argc > 1 && atoi(argv[1]) == 5)) {
        test_mixed_operations();
        total_result += 5;
    }
    
    if (run_all || strstr(argv[0], "test6") || (argc > 1 && atoi(argv[1]) == 6)) {
        test_complex_nested();
        total_result += 6;
    }
    
    /* Print result to prevent dead code elimination */
    printf("Total result marker: %u\n", total_result);
    
    return 0;
}
