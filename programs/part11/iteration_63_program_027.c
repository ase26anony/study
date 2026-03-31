/* test_resource_patterns.c */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* External functions to prevent optimization */
extern void use_int(int);
extern void use_long(long);
extern void use_ptr(void*);
extern void sink(int);

/* Volatile seed to prevent compile-time optimization */
static volatile int seed = 0;

/* Pattern 1: Generate SET with ZERO_EXTRACT destination */
__attribute__((noinline, noipa))
void test_zero_extract(volatile int input) {
    /* Using bitfields to encourage ZERO_EXTRACT */
    union {
        unsigned int full;
        struct {
            unsigned int low:8;
            unsigned int mid:8;
            unsigned int high:16;
        } bits;
    } data;
    
    data.full = 0xFFFFFFFF;
    /* Store into specific bitfield - may generate ZERO_EXTRACT */
    data.bits.mid = input & 0xFF;
    
    /* Complex bitfield manipulation */
    struct {
        unsigned int a:4;
        unsigned int b:4;
        unsigned int c:24;
    } bf;
    
    bf.a = (input >> 4) & 0xF;
    bf.b = input & 0xF;
    bf.c = (input << 8) & 0xFFFFFF;
    
    /* Combine results to prevent dead code */
    int result = data.full + bf.a + bf.b + bf.c;
    sink(result);
}

/* Pattern 2: Generate SET with STRICT_LOW_PART destination */
__attribute__((noinline, noipa))
void test_strict_low_part(volatile short s_input, volatile int i_input) {
    int val = i_input;
    
    /* Assign short to part of int - may generate STRICT_LOW_PART */
    *(short*)&val = s_input;
    
    /* Another pattern: preserve high bits while modifying low */
    long long big = 0x123456789ABCDEF0LL;
    int *p_int = (int*)&big;
    *p_int = s_input;  /* Only modifies low 32 bits of big */
    
    /* Using explicit masking */
    int x = i_input;
    x = (x & ~0xFFFF) | (s_input & 0xFFFF);
    
    /* Complex control flow to keep all patterns */
    if (seed & 1) {
        val += x;
    } else {
        val += (int)big;
    }
    
    sink(val);
}

/* Pattern 3: Generate SET with SUBREG destination */
__attribute__((noinline, noipa))
void test_subreg(volatile int idx, volatile int val) {
    /* Array access with type punning */
    int array[8];
    for (int i = 0; i < 8; i++) {
        array[i] = i * 100 + seed;
    }
    
    /* Access sub-word through pointer cast */
    short *ps = (short*)&array[idx % 8];
    *ps = val & 0xFFFF;
    
    /* Different size accesses */
    long long big_array[4];
    big_array[0] = 0x1122334455667788LL;
    int *p = (int*)&big_array[1];
    *p = val;  /* SUBREG access into long long */
    
    /* Structure with mixed types */
    struct mixed {
        char c;
        short s;
        int i;
        long long ll;
    } m;
    
    m.i = val;
    m.s = (short)(val >> 8);
    
    /* Pointer arithmetic with different types */
    char *base = (char*)array;
    int *aliased = (int*)(base + 2);  /* Possibly unaligned */
    if ((seed & 3) == 0) {
        *aliased = val;
    }
    
    sink(array[0] + *p + m.i);
}

/* Pattern 4: Generate SET with complex MEM destination */
__attribute__((noinline, noipa))
void test_complex_mem(volatile int offset, volatile int value) {
    /* Global-like storage */
    static int storage[256];
    
    /* Complex addressing modes */
    int *ptr1 = &storage[offset % 256];
    *ptr1 = value;
    
    /* Pointer arithmetic */
    int *ptr2 = ptr1 + (offset % 16);
    *ptr2 = value * 2;
    
    /* Structure with pointer chasing */
    struct node {
        int data;
        struct node *next;
    } nodes[10];
    
    for (int i = 0; i < 9; i++) {
        nodes[i].data = i + value;
        nodes[i].next = &nodes[i + 1];
    }
    nodes[9].data = 9 + value;
    nodes[9].next = NULL;
    
    /* Complex memory access through structure */
    struct node *current = &nodes[offset % 10];
    for (int i = 0; i < 3 && current; i++) {
        current->data += value;
        current = current->next;
    }
    
    /* Multi-dimensional array with computed index */
    int matrix[16][16];
    int row = offset % 16;
    int col = (offset * 7) % 16;
    matrix[row][col] = value;
    
    /* Indirect through function pointer table */
    int (*funcs[4])(int) = { NULL, NULL, NULL, NULL };
    volatile int *mem_loc = (int*)&funcs[offset % 4];
    *mem_loc = value;
    
    sink(storage[0] + nodes[0].data + matrix[0][0] + *mem_loc);
}

/* Pattern 5: Combined patterns in loop */
__attribute__((noinline, noipa))
void test_combined(volatile int iterations) {
    int sum = 0;
    
    for (int i = 0; i < iterations && i < 100; i++) {
        /* Mix different patterns */
        if (i & 1) {
            /* ZERO_EXTRACT-like */
            union {
                int full;
                struct {
                    unsigned low:12;
                    unsigned high:20;
                } parts;
            } u;
            u.full = sum;
            u.parts.low = (seed + i) & 0xFFF;
            sum = u.full;
        } else if (i & 2) {
            /* STRICT_LOW_PART-like */
            long temp = sum;
            *(short*)&temp = (short)(i * 3);
            sum += (int)temp;
        } else {
            /* SUBREG-like */
            int array[2] = { sum, i };
            char *byte_ptr = (char*)array;
            int *word_ptr = (int*)(byte_ptr + 1);
            *word_ptr = *word_ptr + 1;
            sum = array[0];
        }
        
        /* Complex MEM access */
        static int counters[10];
        counters[i % 10] += sum;
    }
    
    sink(sum);
}

/* Main driver */
int main(int argc, char **argv) {
    /* Initialize volatile seed */
    seed = (argc > 1) ? atoi(argv[1]) : time(NULL);
    
    /* Call pattern generators with volatile inputs */
    volatile int input1 = seed;
    volatile short input2 = seed & 0xFFFF;
    volatile int input3 = seed * 3;
    volatile int input4 = seed % 100;
    volatile int input5 = (seed % 20) + 5;
    
    test_zero_extract(input1);
    test_strict_low_part(input2, input3);
    test_subreg(input4, input5);
    test_complex_mem(input3, input4);
    test_combined(input5);
    
    /* Create checksum to prevent complete optimization */
    int checksum = input1 + input2 + input3 + input4 + input5;
    printf("Result: %d\n", checksum);
    
    return 0;
}

/* Dummy definitions to satisfy linker */
void use_int(int x) { seed += x; }
void use_long(long x) { seed += (int)x; }
void use_ptr(void* p) { seed += (int)(long)p; }
void sink(int x) { seed ^= x; }
