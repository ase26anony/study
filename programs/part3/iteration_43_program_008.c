/* resource_patterns.c - Generate RTL patterns for GCC resource tracking coverage */

#include <stdint.h>
#include <string.h>

/* Force noinline to prevent optimization from removing patterns */
#define NOINLINE __attribute__((noinline))

/* Pattern 1: ZERO_EXTRACT + MEM */
NOINLINE static void pattern_zero_extract_mem(volatile int *counter) {
    /* Struct with volatile bit-field for ZERO_EXTRACT */
    struct bitfield_struct {
        volatile unsigned int field1 : 5;
        volatile unsigned int field2 : 7;
        volatile unsigned int field3 : 3;
    } bf;
    
    /* Array with complex addressing for MEM */
    volatile int arr[16][8];
    volatile int *ptr;
    
    /* ZERO_EXTRACT: Assignment to volatile bit-field */
    bf.field1 = (*counter) & 0x1F;
    bf.field2 = ((*counter) >> 5) & 0x7F;
    bf.field3 = ((*counter) >> 12) & 0x7;
    
    /* MEM: Complex addressing with pointer arithmetic */
    ptr = &arr[0][0];
    ptr += (*counter) & 0x7F;  /* Create non-constant offset */
    
    /* Combine: Use bit-field value in memory access */
    arr[((*counter) & 0xF)][((*counter) >> 4) & 0x7] = bf.field1 + bf.field2;
    
    /* More complex MEM addressing */
    volatile int val = *(ptr + ((*counter) & 0x3));
    (void)val; /* Prevent unused warning */
}

/* Pattern 2: STRICT_LOW_PART + SUBREG */
NOINLINE static void pattern_strict_low_part_subreg(volatile int *counter) {
    volatile short s_val;
    volatile char c_val;
    volatile int i_val;
    
    /* Initialize values */
    s_val = (*counter) & 0xFFFF;
    c_val = (*counter) & 0xFF;
    i_val = *counter;
    
    /* STRICT_LOW_PART: Inline assembly modifying only low part */
    /* Modify low byte of s_val */
    asm volatile (
        "addb %1, %0\n\t"
        : "=q"(c_val)
        : "q"(c_val), "0"(c_val)
        : "cc"
    );
    
    /* Another STRICT_LOW_PART pattern */
    asm volatile (
        "incb %0\n\t"
        : "=q"(c_val)
        : "0"(c_val)
        : "cc"
    );
    
    /* SUBREG: Type punning through different sized accesses */
    /* Cast int to short pointer for SUBREG access */
    short *ps = (short *)&i_val;
    *ps = s_val;  /* This generates SUBREG store */
    
    /* More SUBREG: Access different parts of int */
    char *pc = (char *)&i_val;
    pc[1] = c_val;  /* Another SUBREG */
    
    /* Mixed size operations */
    i_val = (i_val & 0xFFFF0000) | (s_val & 0xFFFF);
}

/* Pattern 3: Complex expression mixing patterns */
NOINLINE static void pattern_complex_mix(volatile int *counter, volatile int *arr) {
    /* Struct with bit-fields at different positions */
    struct mixed_struct {
        volatile unsigned int a : 3;
        volatile unsigned int b : 9;
        volatile unsigned int c : 4;
    } ms;
    
    /* ZERO_EXTRACT assignments */
    ms.a = (*counter) & 0x7;
    ms.b = ((*counter) >> 3) & 0x1FF;
    ms.c = ((*counter) >> 12) & 0xF;
    
    /* Complex addressing with ternary operator */
    volatile int *ptr;
    int idx = *counter;
    
    /* Ternary creates complex control flow for resource tracking */
    ptr = (idx & 1) ? &arr[idx % 64] : &arr[(idx * 3) % 64];
    
    /* MEM access with computed address */
    volatile int temp = *ptr;
    
    /* Combine with bit-field value */
    *ptr = temp + ms.a + ms.b;
    
    /* SUBREG through pointer casting */
    if (idx & 2) {
        short *sptr = (short *)ptr;
        *sptr = (short)(ms.c * 2);  /* SUBREG store */
    }
}

/* Pattern 4: Loop-based pattern generation */
NOINLINE static void pattern_loop_based(volatile int *counter) {
    volatile int buffer[32];
    volatile int temp = *counter;
    
    /* Initialize buffer with pattern */
    for (int i = 0; i < 32; i++) {
        buffer[i] = (temp + i) & 0xFF;
    }
    
    /* Mixed operations in loop */
    for (int i = 0; i < 16; i++) {
        /* ZERO_EXTRACT-like via bit operations */
        volatile int val = buffer[i];
        val = (val & 0x0F) | ((val << 4) & 0xF0);
        
        /* MEM with complex addressing */
        buffer[i + 16] = buffer[(i * 3) % 16] + val;
        
        /* SUBREG access */
        if (i & 1) {
            char *cptr = (char *)&buffer[i];
            cptr[0] = (char)(val & 0xFF);
        }
    }
    
    /* Final STRICT_LOW_PART */
    volatile char last_byte;
    asm volatile (
        "movb %%al, %0\n\t"
        : "=q"(last_byte)
        :
        : "%al"
    );
}

/* Main function that drives all patterns */
int main(int argc, char **argv) {
    volatile int counter = 0;
    volatile int iterations = 10;
    volatile int dummy_sum = 0;
    
    /* Use argc to bound iterations if provided */
    if (argc > 1) {
        iterations = (argv[1][0] - '0') % 20;
        if (iterations < 1) iterations = 5;
    }
    
    /* Array for MEM patterns */
    volatile int mem_array[128];
    
    /* Initialize array */
    for (int i = 0; i < 128; i++) {
        mem_array[i] = i * 3;
    }
    
    /* Main loop calling pattern functions */
    for (counter = 0; counter < iterations; counter++) {
        /* Call each pattern function */
        pattern_zero_extract_mem(&counter);
        pattern_strict_low_part_subreg(&counter);
        pattern_complex_mix(&counter, mem_array);
        pattern_loop_based(&counter);
        
        /* Prevent dead code elimination */
        dummy_sum += counter;
        
        /* Additional volatile operations to force RTL generation */
        volatile int temp = counter;
        asm volatile ("" : "+r"(temp) : : "memory");
    }
    
    /* Use dummy_sum to prevent optimization */
    volatile int result = dummy_sum;
    
    /* Final complex expression that might generate interesting RTL */
    if (result > 100) {
        /* This block creates additional control flow */
        struct { volatile unsigned int f:8; } s;
        s.f = result & 0xFF;  /* ZERO_EXTRACT */
        
        /* More SUBREG patterns */
        int x = result;
        short *sp = (short *)&x;
        *sp = (short)(result >> 8);
    }
    
    return 0;
}
