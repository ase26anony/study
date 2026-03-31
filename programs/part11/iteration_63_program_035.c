/* test_resource_patterns.c */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* External functions to prevent optimization */
extern void use(int);
extern void sink(void*);
extern int volatile_input(void);

/* Prevent inlining and IPA */
#define NOINLINE __attribute__((noinline, noipa))

/* Global variables for memory patterns */
int global_array[256];
volatile int volatile_index = 0;
volatile int volatile_value = 0;

/* Pattern 1: ZERO_EXTRACT destination */
NOINLINE void test_zero_extract(void) {
    volatile int src = volatile_input();
    
    /* Using union with bitfields - explicit bitfield store */
    union {
        unsigned int full;
        struct {
            unsigned int low:8;
            unsigned int mid:8;
            unsigned int high:8;
            unsigned int top:8;
        } parts;
    } data;
    
    data.full = 0xFFFFFFFF;
    /* This assignment to bitfield may generate ZERO_EXTRACT */
    data.parts.mid = src & 0xFF;
    
    /* Force usage */
    use(data.full);
    
    /* Another pattern: manual masking */
    unsigned int val = 0x12345678;
    unsigned int mask = 0x0000FF00;
    /* Store into specific bits - may generate ZERO_EXTRACT */
    val = (val & ~mask) | ((src << 8) & mask);
    use(val);
}

/* Pattern 2: STRICT_LOW_PART destination */
NOINLINE void test_strict_low_part(void) {
    volatile short s = volatile_input() & 0xFFFF;
    volatile int i = volatile_input();
    
    /* Assignment to low part of larger integer */
    /* May generate STRICT_LOW_PART */
    i = (i & ~0xFFFF) | (s & 0xFFFF);
    use(i);
    
    /* Using pointer cast to access low part */
    long long big = 0x123456789ABCDEF0LL;
    int *p = (int*)&big;
    *p = s;  /* Store into low 32 bits of 64-bit value */
    use(big);
    
    /* Another pattern with explicit masking */
    unsigned int reg = 0x87654321;
    unsigned short low = s;
    reg = (reg & 0xFFFF0000) | low;
    use(reg);
}

/* Pattern 3: SUBREG destination */
NOINLINE void test_subreg(void) {
    volatile int idx = volatile_index % 4;
    volatile int val = volatile_value;
    
    /* Array access with type punning */
    int array[4] = {0, 0, 0, 0};
    short *ps = (short*)&array[idx];
    *ps = val & 0xFFFF;  /* May generate SUBREG */
    use(array[0]);
    use(array[1]);
    
    /* Access different parts of larger type */
    long long big_val = 0;
    int *p_int = (int*)&big_val;
    p_int[0] = val;      /* Low 32 bits */
    p_int[1] = val + 1;  /* High 32 bits */
    use(big_val);
    
    /* Structure with mixed types */
    struct mixed {
        char c;
        short s;
        int i;
        long long ll;
    } m;
    
    m.i = val;
    m.s = val >> 16;
    use(m.i + m.s);
}

/* Pattern 4: Complex MEM destination */
NOINLINE void test_complex_mem(void) {
    volatile int off = volatile_index;
    volatile int val = volatile_value;
    
    /* Complex addressing modes */
    struct S {
        int a;
        int b[4];
        int c;
    } s;
    
    /* Pointer arithmetic with volatile offset */
    int *ptr = &s.a + off;
    *ptr = val;  /* Complex MEM destination */
    use(s.a);
    
    /* Array with computed index */
    int *arr_ptr = &global_array[off % 256];
    *arr_ptr = val;
    use(global_array[0]);
    
    /* Nested structure access */
    struct Inner {
        int x;
        int y;
    };
    
    struct Outer {
        struct Inner inner[3];
        int extra;
    } outer;
    
    struct Inner *inner_ptr = &outer.inner[off % 3];
    inner_ptr->x = val;
    inner_ptr->y = val + 1;
    use(outer.inner[0].x);
}

/* Pattern 5: Combined patterns in loops */
NOINLINE void test_combined_patterns(void) {
    volatile int limit = volatile_index + 1;
    volatile int seed = volatile_value;
    int result = 0;
    
    for (int i = 0; i < limit; i++) {
        volatile int temp = seed + i;
        
        /* Mix different patterns in loop */
        if (temp & 1) {
            /* ZERO_EXTRACT pattern */
            unsigned int x = result;
            x = (x & ~0xFF00) | ((temp << 8) & 0xFF00);
            result ^= x;
        } else if (temp & 2) {
            /* STRICT_LOW_PART pattern */
            short s = temp & 0x7FFF;
            result = (result & 0xFFFF0000) | s;
        } else if (temp & 4) {
            /* SUBREG pattern */
            int arr[2] = {result, temp};
            short *ps = (short*)arr;
            ps[1] = temp & 0xFFFF;
            result += arr[0];
        } else {
            /* MEM pattern */
            global_array[i % 256] = temp;
            result += global_array[(i + 1) % 256];
        }
    }
    
    use(result);
}

/* Pattern 6: Nested control flow with patterns */
NOINLINE void test_nested_control_flow(void) {
    volatile int cond1 = volatile_index & 1;
    volatile int cond2 = volatile_index & 2;
    volatile int val = volatile_value;
    
    if (cond1) {
        /* Inside if: ZERO_EXTRACT */
        union {
            unsigned int full;
            struct {
                unsigned int a:4;
                unsigned int b:4;
                unsigned int c:8;
                unsigned int d:16;
            } bits;
        } u;
        
        u.full = 0;
        u.bits.c = val & 0xFF;
        use(u.full);
        
        if (cond2) {
            /* Nested if: STRICT_LOW_PART */
            int x = 0x12345678;
            short s = val & 0xABCD;
            x = (x & 0xFFFF0000) | s;
            use(x);
        }
    } else {
        /* Else branch: SUBREG and MEM */
        long long buffer[2];
        int *p = (int*)buffer;
        
        for (int i = 0; i < 2; i++) {
            p[i] = val + i;
        }
        
        short *ps = (short*)&buffer[1];
        *ps = val >> 16;
        
        use(buffer[0]);
        use(buffer[1]);
    }
}

/* Main function that drives all tests */
int main(int argc, char **argv) {
    /* Initialize volatile seed from command line or timer */
    unsigned int seed;
    if (argc > 1) {
        seed = atoi(argv[1]);
    } else {
        seed = time(NULL);
    }
    
    srand(seed);
    
    /* Set volatile globals */
    volatile_index = rand() % 100;
    volatile_value = rand();
    
    /* Initialize global array */
    for (int i = 0; i < 256; i++) {
        global_array[i] = rand();
    }
    
    /* Run all pattern tests */
    test_zero_extract();
    test_strict_low_part();
    test_subreg();
    test_complex_mem();
    test_combined_patterns();
    test_nested_control_flow();
    
    /* Create checksum from results */
    int checksum = 0;
    for (int i = 0; i < 256; i++) {
        checksum ^= global_array[i];
    }
    
    printf("Checksum: %d (seed: %u)\n", checksum, seed);
    return 0;
}

/* Dummy definitions for external functions */
void use(int x) {
    /* Prevent optimization */
    asm volatile("" : : "r"(x));
}

int volatile_input(void) {
    return rand();
}
