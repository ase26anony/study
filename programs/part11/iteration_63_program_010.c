/* test_resource_patterns.c
 * Designed to generate RTL SET destinations with:
 * - ZERO_EXTRACT
 * - STRICT_LOW_PART  
 * - SUBREG
 * - Complex MEM patterns
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* External functions to prevent optimization */
extern void use(int);
extern void sink(void*);
extern int volatile_input(void);

/* Prevent inlining and IPA */
#define NOINLINE __attribute__((noinline, noipa))

/* Global variables for memory patterns */
int global_array[1024];
struct compound {
    int a;
    int b;
    long long c;
    char d[8];
} global_struct;

/* Pattern 1: ZERO_EXTRACT destinations via bitfields */
NOINLINE void test_zero_extract(volatile int seed) {
    /* Union with bitfields - explicit bitfield store */
    union {
        unsigned int full;
        struct {
            unsigned int low : 8;
            unsigned int mid : 8;
            unsigned int high : 16;
        } bits;
    } u;
    
    /* Force data flow with volatile */
    volatile unsigned int v = seed;
    
    /* Multiple bitfield assignments that may generate ZERO_EXTRACT */
    u.bits.low = v & 0xFF;
    use(u.full);
    
    u.bits.mid = (v >> 8) & 0xFF;
    use(u.full);
    
    u.bits.high = (v >> 16) & 0xFFFF;
    use(u.full);
    
    /* Another approach: masking operations */
    unsigned int dest = 0;
    dest = (dest & ~0xFF) | (v & 0xFF);           /* Low 8 bits */
    dest = (dest & ~0xFF00) | ((v & 0xFF00));     /* Next 8 bits */
    use(dest);
    
    /* Bitfield in struct */
    struct {
        unsigned int field1 : 4;
        unsigned int field2 : 12;
        unsigned int field3 : 16;
    } s;
    
    s.field1 = v & 0xF;
    s.field2 = (v >> 4) & 0xFFF;
    s.field3 = (v >> 16) & 0xFFFF;
    use(*(int*)&s);
}

/* Pattern 2: STRICT_LOW_PART destinations */
NOINLINE void test_strict_low_part(volatile int seed) {
    volatile short vs = seed;
    volatile char vc = seed;
    
    /* Assigning smaller types to larger ones */
    int i = 0;
    i = (i & ~0xFFFF) | (vs & 0xFFFF);  /* Only low 16 bits changed */
    use(i);
    
    long long ll = 0;
    ll = (ll & ~0xFFFFFFFFLL) | (i & 0xFFFFFFFFLL);
    use(ll);
    
    /* Pointer casting approach */
    int val = 0x12345678;
    *(short*)&val = vs;  /* Modify only low 16 bits */
    use(val);
    
    /* With arithmetic to prevent constant folding */
    for (int j = 0; j < (seed & 3); j++) {
        int temp = j * 1000;
        *(char*)&temp = vc + j;
        use(temp);
    }
    
    /* Array element with type punning */
    int arr[4] = {0};
    for (volatile int k = 0; k < 2; k++) {
        *(short*)(&arr[k]) = vs + k;
        use(arr[k]);
    }
}

/* Pattern 3: SUBREG destinations */
NOINLINE void test_subreg(volatile int seed) {
    volatile int idx = seed & 3;
    
    /* Type punning with different sizes */
    long long big_var = 0;
    int* p_int = (int*)&big_var;
    *p_int = seed;                     /* Write to first 32 bits */
    use(big_var);
    
    p_int = (int*)((char*)&big_var + 4);
    *p_int = seed * 2;                 /* Write to second 32 bits */
    use(big_var);
    
    /* Array with sub-word access */
    int array[8];
    short* ps = (short*)array;
    for (int i = 0; i < (seed & 7); i++) {
        ps[i] = (short)(seed + i);     /* SUBREG for short store into int array */
        use(array[i/2]);
    }
    
    /* Structure with mixed types */
    struct mixed {
        char c;
        short s;
        int i;
        long long ll;
    } m;
    
    m.c = seed;
    m.s = seed * 2;
    m.i = seed * 3;
    m.ll = seed * 4;
    
    /* Access through different type pointers */
    short* s_ptr = (short*)&m.i;
    *s_ptr = seed;                     /* SUBREG access to part of int */
    use(m.i);
    
    /* Union for type punning */
    union {
        int i;
        short s[2];
        char c[4];
    } pun;
    
    pun.i = 0;
    for (int j = 0; j < 2; j++) {
        pun.s[j] = seed + j;           /* SUBREG stores */
        use(pun.i);
    }
}

/* Pattern 4: Complex MEM destinations */
NOINLINE void test_complex_mem(volatile int seed) {
    volatile int offset = seed & 1023;
    volatile int index = seed & 7;
    
    /* Pointer arithmetic with volatile offset */
    int* ptr = global_array + offset;
    *ptr = seed;                       /* MEM with offset addressing */
    use(*ptr);
    
    ptr = &global_array[index * 2];
    *ptr = seed * 2;                   /* MEM with scaled index */
    use(*ptr);
    
    /* Structure member with computed offset */
    struct compound* sptr = &global_struct;
    int* member_ptr = &sptr->a + index;
    *member_ptr = seed * 3;            /* MEM with structure offset */
    use(*member_ptr);
    
    /* Two-dimensional indexing */
    int matrix[8][8];
    for (int i = 0; i < (seed & 7); i++) {
        for (int j = 0; j < (seed & 7); j++) {
            matrix[i][j] = seed + i * 8 + j;  /* MEM with complex address */
            use(matrix[i][j]);
        }
    }
    
    /* Pointer chasing */
    int* chain[8];
    for (int i = 0; i < 8; i++) {
        chain[i] = &global_array[i * 4];
    }
    
    int** pp = chain + (seed & 7);
    **pp = seed * 4;                   /* MEM through pointer indirection */
    use(**pp);
    
    /* Global variable with computed address */
    extern int extern_global;
    int* global_ptr = &extern_global + (seed & 3);
    *global_ptr = seed * 5;
    use(*global_ptr);
}

/* Pattern 5: Combined patterns in loops */
NOINLINE void test_combined(volatile int seed) {
    volatile int limit = (seed & 7) + 1;
    
    for (int i = 0; i < limit; i++) {
        volatile int v = seed + i;
        
        /* Mix different patterns in loop */
        if (v & 1) {
            /* ZERO_EXTRACT pattern */
            union {
                unsigned int val;
                struct {
                    unsigned int a : 3;
                    unsigned int b : 5;
                    unsigned int c : 24;
                } bits;
            } u;
            u.bits.a = v & 0x7;
            u.bits.b = (v >> 3) & 0x1F;
            use(u.val);
        }
        
        if (v & 2) {
            /* STRICT_LOW_PART pattern */
            int x = i * 1000;
            *(short*)&x = v & 0xFFFF;
            use(x);
        }
        
        if (v & 4) {
            /* SUBREG pattern */
            long long ll = v;
            int* p = (int*)&ll + (i & 1);
            *p = v * 2;
            use(ll);
        }
        
        if (v & 8) {
            /* MEM pattern */
            global_array[(v + i) & 1023] = v * 3;
            use(global_array[(v + i) & 1023]);
        }
    }
}

/* Main driver */
int main(int argc, char** argv) {
    /* Use argument or time as volatile seed */
    volatile int seed = 0;
    if (argc > 1) {
        seed = atoi(argv[1]);
    } else {
        seed = time(NULL);
    }
    
    /* Initialize globals */
    memset(global_array, 0, sizeof(global_array));
    memset(&global_struct, 0, sizeof(global_struct));
    
    /* Call pattern generators */
    test_zero_extract(seed);
    test_strict_low_part(seed);
    test_subreg(seed);
    test_complex_mem(seed);
    test_combined(seed);
    
    /* Create checksum to prevent optimization */
    int checksum = 0;
    for (int i = 0; i < 256; i++) {
        checksum += global_array[i];
    }
    checksum += *(int*)&global_struct;
    
    printf("Result: %d\n", checksum);
    return checksum & 0xFF;
}

/* Dummy definitions to satisfy extern declarations */
void use(int x) {
    /* Empty but prevents dead code elimination */
    static volatile int sink;
    sink = x;
}

void sink(void* p) {
    static volatile void* vsink;
    vsink = p;
}

int extern_global = 0;
