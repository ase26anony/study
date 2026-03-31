/* test_resource_patterns.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* External functions to prevent optimization */
extern void use(int);
extern void sink(void*);
extern int volatile_input(void);

/* Prevent inlining and IPA */
#define NOINLINE __attribute__((noinline, noipa))

/* Global variables for memory patterns */
int glob_array[100];
volatile int volatile_idx = 0;

/* Pattern 1: ZERO_EXTRACT destination via bitfields */
NOINLINE void test_zero_extract(volatile int seed) {
    /* Method 1: Union with bitfields */
    union {
        unsigned int full;
        struct {
            unsigned int low: 8;
            unsigned int mid: 8;
            unsigned int high: 16;
        } bits;
    } u;
    
    u.full = seed;
    /* This assignment to bitfield may generate ZERO_EXTRACT */
    u.bits.mid = volatile_input() & 0xFF;
    use(u.full);
    
    /* Method 2: Explicit bit masking */
    unsigned int val = seed;
    unsigned int mask = 0xFF00;
    unsigned int insert = (volatile_input() & 0xFF) << 8;
    /* May generate: (set (zero_extract:SI (reg:SI X) (const_int 8) (const_int 8))
     *                (and:SI (reg:SI Y) (const_int 255))) */
    val = (val & ~mask) | insert;
    use(val);
}

/* Pattern 2: STRICT_LOW_PART destination */
NOINLINE void test_strict_low_part(volatile int seed) {
    /* Method 1: Short to int assignment */
    int i = seed;
    short s = volatile_input() & 0xFFFF;
    
    /* This may generate STRICT_LOW_PART */
    i = (i & ~0xFFFF) | (s & 0xFFFF);
    use(i);
    
    /* Method 2: Pointer casting */
    long long big = seed;
    int *p = (int*)&big;
    volatile int v = volatile_input();
    
    /* Store to low part of 64-bit via 32-bit pointer */
    *p = v;
    use(big);
    
    /* Method 3: Explicit masking preserving high bits */
    unsigned int x = seed;
    unsigned short low = volatile_input() & 0xFFFF;
    x = (x & 0xFFFF0000) | low;
    use(x);
}

/* Pattern 3: SUBREG destination */
NOINLINE void test_subreg(volatile int seed) {
    /* Method 1: Array with sub-word access */
    int array[4] = {seed, seed + 1, seed + 2, seed + 3};
    volatile int idx = volatile_idx % 4;
    
    /* Access via short pointer - may generate SUBREG */
    short *ps = (short*)&array[idx];
    *ps = volatile_input() & 0xFFFF;
    use(array[idx]);
    
    /* Method 2: Type punning between different sizes */
    long long big_val = seed;
    /* Access different parts */
    char *pc = (char*)&big_val;
    pc[2] = volatile_input() & 0xFF;
    pc[5] = volatile_input() & 0xFF;
    use(big_val);
    
    /* Method 3: Structure with mixed types */
    struct mixed {
        char c;
        short s;
        int i;
        long long ll;
    } m;
    m.i = seed;
    
    /* Access subparts */
    short *sp = (short*)&m.i;
    sp[1] = volatile_input() & 0xFFFF;  /* High 16 bits of int */
    use(m.i);
}

/* Pattern 4: Complex MEM destinations */
NOINLINE void test_complex_mem(volatile int seed) {
    /* Method 1: Structure with offset */
    struct S {
        int a;
        int b;
        int c[10];
    } s;
    
    volatile int off = volatile_idx % 12;
    int *ptr = &s.a + off;
    *ptr = volatile_input();
    use(s.a + s.b);
    
    /* Method 2: Global array with computed index */
    int idx = (volatile_input() + seed) % 100;
    glob_array[idx] = volatile_input();
    use(glob_array[idx]);
    
    /* Method 3: Pointer arithmetic with scaling */
    int *base = &glob_array[50];
    int offset = (volatile_input() % 20) * 2;
    base[offset] = volatile_input();
    use(base[offset]);
    
    /* Method 4: Nested addressing */
    struct Node {
        int value;
        struct Node *next;
    } nodes[10];
    
    for (int i = 0; i < 9; i++) {
        nodes[i].value = seed + i;
        nodes[i].next = &nodes[i + 1];
    }
    nodes[9].next = NULL;
    
    volatile int steps = volatile_idx % 10;
    struct Node *current = &nodes[0];
    for (int i = 0; i < steps && current; i++) {
        current = current->next;
    }
    if (current) {
        current->value = volatile_input();
        use(current->value);
    }
}

/* Pattern 5: Combined patterns in loops */
NOINLINE void test_combined(volatile int seed) {
    unsigned int checksum = 0;
    volatile int limit = (volatile_input() % 10) + 5;
    
    for (int i = 0; i < limit; i++) {
        /* Mix different patterns in loop */
        unsigned int val = seed + i;
        
        /* ZERO_EXTRACT-like */
        unsigned int field = (volatile_input() + i) & 0xF;
        val = (val & ~0xF0) | (field << 4);
        
        /* STRICT_LOW_PART-like */
        short low = (volatile_input() + i * 2) & 0x7FFF;
        val = (val & 0xFFFF8000) | low;
        
        /* Store to memory with complex address */
        int idx = (val + i) % 100;
        glob_array[idx] = val;
        
        checksum += val + glob_array[idx];
    }
    
    use(checksum);
}

/* Main driver with volatile control flow */
int main(int argc, char **argv) {
    /* Use argv for volatile seed */
    volatile int seed = 0;
    if (argc > 1) {
        seed = atoi(argv[1]);
    } else {
        seed = 12345;
    }
    
    /* Call all test functions with volatile control */
    volatile int mode = volatile_idx % 5;
    
    if (mode == 0 || mode == 4) {
        test_zero_extract(seed);
    }
    if (mode == 1 || mode == 4) {
        test_strict_low_part(seed + 1);
    }
    if (mode == 2 || mode == 4) {
        test_subreg(seed + 2);
    }
    if (mode == 3 || mode == 4) {
        test_complex_mem(seed + 3);
    }
    
    /* Always run combined test */
    test_combined(seed + 4);
    
    /* Create checksum from globals to prevent elimination */
    int final_checksum = 0;
    for (int i = 0; i < 100; i++) {
        final_checksum += glob_array[i];
    }
    
    printf("Result: %d\n", final_checksum);
    return final_checksum != 0 ? 0 : 1;
}

/* Dummy definitions to satisfy extern declarations */
void use(int x) {
    /* Empty but prevents dead code elimination */
    volatile static int sink;
    sink = x;
}

void sink(void *p) {
    volatile static void *vsink;
    vsink = p;
}

int volatile_input(void) {
    static volatile int counter = 0;
    return ++counter;
}
