/* test_resource_coverage.c
 * Designed to trigger uncovered lines 282-290 in resource.cc
 * Compile with: gcc -O2 -m32 -fno-strict-aliasing -fdump-rtl-all test_resource_coverage.c
 */

#include <stddef.h>

/* Function A: Focus on ZERO_EXTRACT and MEM patterns */
static void __attribute__((noinline)) 
pattern_zero_extract_mem(volatile int *counter) {
    /* Struct with volatile bit-field for ZERO_EXTRACT */
    struct S { 
        volatile unsigned int f1:5; 
        volatile unsigned int f2:3;
        volatile unsigned int pad:24;
    } s;
    
    /* Array with complex addressing for MEM pattern */
    int arr[10][10];
    volatile int idx1 = *counter % 10;
    volatile int idx2 = (*counter + 1) % 10;
    
    /* ZERO_EXTRACT: Assignment to volatile bit-field */
    s.f1 = idx1 & 0x1F;  /* 5-bit field */
    s.f2 = idx2 & 0x07;  /* 3-bit field */
    
    /* MEM: Complex addressing with pointer arithmetic */
    volatile int *ptr = &arr[idx1][idx2];
    volatile int v = *(ptr + (idx1 * idx2) % 5);
    
    /* Prevent dead code elimination */
    *counter += s.f1 + v;
}

/* Function B: Focus on STRICT_LOW_PART and SUBREG patterns */
static void __attribute__((noinline)) 
pattern_strict_low_part_subreg(volatile int *counter) {
    volatile char c = (*counter) & 0xFF;
    volatile short s = (*counter) & 0xFFFF;
    
    /* STRICT_LOW_PART: Inline assembly modifying byte-sized part */
    asm volatile (
        "addb $1, %0\n\t"
        : "=q"(c)        /* =q constraint for byte-addressable register */
        : "0"(c)         /* Matching input constraint */
        : "cc"
    );
    
    /* Another STRICT_LOW_PART variant with different operation */
    asm volatile (
        "orb $0x10, %0\n\t"
        : "=q"(c)
        : "0"(c)
        : "cc"
    );
    
    /* SUBREG: Type punning with mixed-size accesses */
    int i = *counter;
    short *ps = (short*)&i;  /* Cast to smaller type pointer */
    *ps = s;                 /* SUBREG access in RTL */
    
    /* More SUBREG: Access different parts of larger type */
    char *pc = (char*)&i;
    pc[1] = c;
    pc[3] = (*counter >> 8) & 0xFF;
    
    /* Prevent dead code elimination */
    *counter += i + c;
}

/* Function C: Complex expression mixing patterns */
static void __attribute__((noinline)) 
pattern_mixed_complex(volatile int *counter) {
    /* Struct with volatile bit-fields and regular members */
    struct Mixed {
        volatile unsigned int bf1:4;
        volatile unsigned int bf2:4;
        int regular;
        volatile unsigned int bf3:8;
    } m;
    
    /* Array for MEM patterns */
    volatile int array[20];
    for (volatile int i = 0; i < 5; i++) {
        array[i] = (*counter + i) * 2;
    }
    
    /* Complex addressing with ternary operator */
    volatile int idx = *counter % 20;
    volatile int *addr = (idx > 10) ? &array[idx - 5] : &array[idx + 5];
    
    /* ZERO_EXTRACT with computed value */
    m.bf1 = (*addr) & 0x0F;
    m.bf2 = (idx * 3) & 0x0F;
    
    /* SUBREG through pointer casting */
    long long big_val = *counter * 100LL;
    int *p_int = (int*)&big_val;
    m.regular = p_int[0] + p_int[1];  /* Accesses halves of long long */
    
    /* More MEM with complex addressing */
    volatile int v = addr[(idx * 7) % 5] + array[(idx * 3) % 10];
    
    /* STRICT_LOW_PART on char extracted from int */
    volatile char c = m.regular & 0xFF;
    asm volatile (
        "subb $2, %0\n\t"
        : "=q"(c)
        : "0"(c)
        : "cc"
    );
    
    /* Update counter to prevent elimination */
    *counter += m.bf1 + m.bf2 + v + c;
}

/* Helper function with loop to increase RTL complexity */
static void __attribute__((noinline))
pattern_loop_helper(volatile int *counter, int iterations) {
    struct LoopStruct {
        volatile unsigned int bits:6;
        int data[4];
    } ls;
    
    volatile int temp[8];
    
    for (volatile int i = 0; i < iterations; i++) {
        /* ZERO_EXTRACT in loop */
        ls.bits = (i + *counter) & 0x3F;
        
        /* MEM with indexing */
        temp[i % 8] = ls.data[i % 4] + i;
        
        /* SUBREG access */
        short *ps = (short*)&ls.data[i % 4];
        ps[0] = (i * 2) & 0xFFFF;
        
        /* Complex addressing */
        volatile int *ptr = &temp[(i * 3) % 8];
        *ptr += ls.bits;
    }
    
    /* STRICT_LOW_PART final adjustment */
    volatile char final_c = *counter & 0xFF;
    asm volatile (
        "incb %0\n\t"
        : "=q"(final_c)
        : "0"(final_c)
        : "cc"
    );
    
    *counter += final_c;
}

int main(int argc, char *argv[]) {
    /* Use argc to bound loops, preventing infinite loops in analysis */
    volatile int iterations = (argc > 1) ? 10 : 5;
    volatile int counter = 0;
    
    /* Initialize some volatile arrays */
    volatile int init_array[20];
    for (volatile int i = 0; i < 20; i++) {
        init_array[i] = i * i;
    }
    
    /* Main loop calling pattern functions */
    for (volatile int i = 0; i < iterations; i++) {
        pattern_zero_extract_mem(&counter);
        pattern_strict_low_part_subreg(&counter);
        pattern_mixed_complex(&counter);
        pattern_loop_helper(&counter, 3);
        
        /* Complex MEM addressing using initialized array */
        volatile int idx = counter % 20;
        volatile int *ptr1 = &init_array[idx];
        volatile int *ptr2 = &init_array[(idx * 7) % 20];
        counter += ptr1[(idx + 1) % 5] - ptr2[(idx + 3) % 5];
    }
    
    /* Final dummy operation to prevent elimination */
    volatile int result = 0;
    for (volatile int i = 0; i < counter % 10; i++) {
        result += init_array[i];
    }
    
    /* The program doesn't need correct runtime semantics,
     * but return something to make compiler happy */
    return result != 0;
}
