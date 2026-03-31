/* Test program to cover specific RTL patterns in GCC's resource.cc */
#include <stdint.h>
#include <assert.h>

/* Ensure optimization is enabled for RTL generation */
#ifndef __OPTIMIZE__
#error "Compile with -O2 or -O3 for RTL pattern generation"
#endif

/* Prevent inlining to ensure separate RTL generation */
#define NOINLINE __attribute__((noinline))

/* For STRICT_LOW_PART - x86 inline assembly */
#if defined(__i386__) || defined(__x86_64__)
#define HAS_X86_ASM 1
#else
#define HAS_X86_ASM 0
#endif

/* Function 1: Generate ZERO_EXTRACT through bit-field operations */
NOINLINE static int bitfield_ops(volatile unsigned int *p) {
    /* Bit-field extraction that may generate ZERO_EXTRACT */
    struct bitfield {
        unsigned int a : 4;
        unsigned int b : 8;
        unsigned int c : 4;
    } bf;
    
    /* Force memory access and bit extraction */
    unsigned int val = *p;
    bf.a = (val >> 0) & 0xF;   /* Potential ZERO_EXTRACT */
    bf.b = (val >> 4) & 0xFF;  /* Potential ZERO_EXTRACT */
    bf.c = (val >> 12) & 0xF;  /* Potential ZERO_EXTRACT */
    
    /* Combine with more operations */
    return (bf.a << 16) | (bf.b << 8) | bf.c;
}

/* Function 2: Generate STRICT_LOW_PART via inline assembly (x86) */
NOINLINE static int strict_low_part_asm(int x) {
#if HAS_X86_ASM
    int result;
    /* Byte operation that may generate STRICT_LOW_PART */
    asm volatile (
        "movb %1, %0\n\t"
        : "=q"(result)          /* =q constraint for byte register */
        : "r"(x)
        : "cc"
    );
    return result;
#else
    /* Fallback: type punning that might generate similar patterns */
    union {
        int32_t full;
        int8_t parts[4];
    } u;
    u.full = x;
    return u.parts[0];  /* Access low byte */
#endif
}

/* Function 3: Generate SUBREG through type conversions */
NOINLINE static int subreg_conversions(void) {
    /* Operations on different-sized types */
    long long big = 0x123456789ABCDEF0LL;
    int medium = (int)big;           /* Truncation - may use SUBREG */
    short small = (short)medium;     /* Further truncation */
    char tiny = (char)small;         /* More truncation */
    
    /* Mix operations to force register moves */
    volatile int mix = 0;
    mix += (int)small;               /* Promote short to int */
    mix += (int)tiny << 8;           /* Promote char to int with shift */
    
    return mix + (big & 0xFFFF);
}

/* Function 4: Generate MEM_P with complex addressing */
NOINLINE static int complex_mem_access(int idx1, int idx2) {
    /* Multi-dimensional array with variable indices */
    int arr[16][16];
    static int init = 0;
    
    /* Initialize once */
    if (!init) {
        for (int i = 0; i < 16; i++) {
            for (int j = 0; j < 16; j++) {
                arr[i][j] = i * 100 + j;
            }
        }
        init = 1;
    }
    
    /* Complex addressing modes */
    int *ptr1 = &arr[idx1][0];
    int *ptr2 = ptr1 + idx2;
    
    /* Multiple memory accesses with addressing arithmetic */
    int sum = 0;
    sum += arr[idx1][idx2];              /* Base + offset */
    sum += *(ptr2 + 3);                  /* Pointer arithmetic */
    sum += arr[idx2 % 8][idx1 % 8];      /* More complex indices */
    
    /* Structure with pointer chasing */
    struct node {
        int value;
        struct node *next;
    } nodes[4];
    
    for (int i = 0; i < 3; i++) {
        nodes[i].value = i * 10;
        nodes[i].next = &nodes[i + 1];
    }
    nodes[3].value = 30;
    nodes[3].next = &nodes[0];
    
    /* Pointer chasing creates complex MEM addresses */
    struct node *current = &nodes[0];
    for (int i = 0; i < 4; i++) {
        sum += current->value;
        current = current->next;
    }
    
    return sum;
}

/* Function 5: Mixed operations in a loop to engage scheduling passes */
NOINLINE static int mixed_operations_loop(int iterations) {
    volatile int counter = 0;
    int array[64];
    
    /* Initialize array */
    for (int i = 0; i < 64; i++) {
        array[i] = i * 3;
    }
    
    /* Loop with mixed operations that should generate various RTL patterns */
    for (int i = 0; i < iterations; i++) {
        /* Bit-field extraction (ZERO_EXTRACT potential) */
        int val = array[i % 64];
        int bits = (val >> (i % 8)) & ((1 << 4) - 1);
        
        /* Type conversions (SUBREG potential) */
        short short_val = (short)val;
        int promoted = (int)short_val * 2;
        
        /* Complex memory addressing (MEM_P potential) */
        int *ptr = array + (i % 32);
        int mem_val = ptr[0] + ptr[1] + ptr[2];
        
        /* Combine results */
        counter += bits + promoted + mem_val;
        
        /* Inline assembly on x86 (STRICT_LOW_PART potential) */
#if HAS_X86_ASM
        int asm_result;
        asm volatile (
            "movw %1, %0\n\t"
            : "=r"(asm_result)
            : "r"(counter)
            : "cc"
        );
        counter = asm_result & 0x7FFF;
#endif
    }
    
    return counter;
}

/* Main function that exercises all patterns */
int main(void) {
    volatile unsigned int bitfield_source = 0xDEADBEEF;
    int result = 0;
    
    /* Static assert to ensure optimization */
    _Static_assert(__OPTIMIZE__, "Optimization required for RTL pattern generation");
    
    /* Exercise each pattern multiple times */
    for (int i = 0; i < 10; i++) {
        result ^= bitfield_ops(&bitfield_source);
        result += strict_low_part_asm(i * 17);
        result += subreg_conversions();
        result += complex_mem_access(i % 8, (i * 3) % 8);
        result += mixed_operations_loop(50);
        
        /* Modify source to vary patterns */
        bitfield_source = (bitfield_source << 1) | (bitfield_source >> 31);
    }
    
    /* Return deterministic result for test validation */
    return (result & 0x7FFFFFFF) == 0 ? 1 : 0;
}
