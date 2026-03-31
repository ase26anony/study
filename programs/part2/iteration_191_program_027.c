/* resource_patterns.c - Generate RTL patterns for resource.cc coverage */

#include <stdio.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
volatile int g_volatile_flag = 1;
volatile unsigned int g_volatile_counter = 0;

/* ===== ZERO_EXTRACT patterns ===== */

/* Pattern 1: Bit-field extraction using shift and mask */
int extract_bits_ze1(volatile unsigned int *p) {
    /* Should generate ZERO_EXTRACT: extract bits 8-15 */
    return (*p >> 8) & 0xFF;
}

/* Pattern 2: Struct with bit-fields */
struct BitFieldStruct {
    unsigned int low8 : 8;
    unsigned int mid8 : 8;
    unsigned int high16 : 16;
};

int extract_bits_ze2(struct BitFieldStruct *bfs) {
    /* Accessing bit-field members can generate ZERO_EXTRACT */
    unsigned int val = bfs->mid8;
    return val + bfs->low8;
}

/* Pattern 3: Multiple extractions in sequence */
unsigned int extract_multiple_ze(volatile unsigned int *p) {
    unsigned int x = *p;
    /* Series of extractions that might persist in RTL */
    unsigned int a = (x >> 0) & 0xF;
    unsigned int b = (x >> 4) & 0xF;
    unsigned int c = (x >> 8) & 0xF;
    unsigned int d = (x >> 12) & 0xF;
    return a + b + c + d;
}

/* ===== STRICT_LOW_PART patterns ===== */

/* Pattern 1: Writing only low byte of a word */
void set_low_byte_slp1(volatile unsigned int *p, unsigned char v) {
    /* Writing only low part while preserving high bits */
    *p = (*p & ~0xFF) | v;
}

/* Pattern 2: Cast to smaller type assignment */
void set_low_part_slp2(volatile uint32_t *p) {
    /* Write to low 16 bits via pointer cast */
    uint16_t *hp = (uint16_t *)p;
    hp[0] = 0x1234;  /* Should generate STRICT_LOW_PART */
}

/* Pattern 3: Inline assembly that suggests low-part operation */
void set_low_asm_slp3(uint32_t *p) {
    /* Assembly that hints at low-part operation */
    uint32_t val = *p;
    asm volatile("" : "+r"(val) : : "cc");
    /* Follow with low-part write */
    *(uint16_t *)&val = 0xABCD;
    *p = val;
}

/* ===== SUBREG patterns ===== */

/* Pattern 1: Union for type punning */
union SubregUnion {
    int32_t full;
    int16_t halves[2];
    int8_t bytes[4];
};

int32_t subreg_via_union1(union SubregUnion *u) {
    /* Access parts through different types */
    u->halves[0] = 100;
    u->bytes[2] = 50;
    return u->full;
}

/* Pattern 2: Pointer casting between different sizes */
int32_t subreg_via_cast2(volatile int64_t *llp) {
    /* Access 64-bit as 32-bit */
    int32_t low = *(int32_t *)llp;
    int32_t high = *((int32_t *)llp + 1);
    return low + high;
}

/* Pattern 3: Mixed-size operations in expression */
int32_t subreg_mixed_ops3(int64_t big) {
    /* Operations that require mode changes */
    int32_t small = (int32_t)big;
    int16_t smaller = (int16_t)small;
    return smaller * 2;
}

/* ===== Complex MEM patterns ===== */

/* Pattern 1: Array with complex indexing */
struct MemStruct {
    int arr[100];
    int pad;
    int arr2[50];
};

int complex_mem1(struct MemStruct *ms, int i, int j) {
    /* Complex address calculation */
    return ms->arr[i + j * 4] + ms->arr2[j * 2];
}

/* Pattern 2: Pointer arithmetic with multiple bases */
int complex_mem2(int *base1, int *base2, int offset) {
    /* Multiple memory references with arithmetic */
    int a = base1[offset * 3];
    int b = base2[offset * 2];
    int c = *(base1 + offset + 5);
    return a + b + c;
}

/* Pattern 3: Nested struct access */
struct Inner {
    int x;
    int y;
};

struct Outer {
    struct Inner inner[10];
    int data[20];
};

int complex_mem3(struct Outer *o, int idx) {
    /* Complex addressing through nested structures */
    return o->inner[idx].x + o->data[idx * 2];
}

/* ===== Combined function with control flow ===== */

int combined_patterns(volatile int flag) {
    int result = 0;
    
    /* Local variables for patterns */
    volatile unsigned int extract_target = 0x12345678;
    uint32_t lowpart_target = 0xFFFFFFFF;
    union SubregUnion subreg_u;
    struct MemStruct ms;
    int array[100];
    
    /* Initialize */
    for (int i = 0; i < 100; i++) {
        ms.arr[i] = i;
        array[i] = i * 2;
    }
    
    /* Complex control flow with volatile condition */
    if (flag & 0x1) {
        /* ZERO_EXTRACT patterns */
        result += extract_bits_ze1(&extract_target);
        result += extract_multiple_ze(&extract_target);
    }
    
    if (flag & 0x2) {
        /* STRICT_LOW_PART patterns */
        set_low_byte_slp1(&lowpart_target, 0xAA);
        set_low_part_slp2(&lowpart_target);
        result += lowpart_target & 0xFFFF;
    }
    
    if (flag & 0x4) {
        /* SUBREG patterns */
        result += subreg_via_union1(&subreg_u);
        result += subreg_mixed_ops3(0x123456789ABCDEFLL);
    }
    
    if (flag & 0x8) {
        /* Complex MEM patterns */
        for (int i = 0; i < 10; i++) {
            result += complex_mem1(&ms, i, i+1);
            result += complex_mem2(array, ms.arr, i);
        }
    }
    
    /* Loop with mixed operations */
    for (int i = 0; i < 5; i++) {
        /* Alternate between pattern types */
        switch (i % 4) {
            case 0:
                result += (extract_target >> (i * 4)) & 0xF;  /* ZERO_EXTRACT-like */
                break;
            case 1:
                *(uint16_t *)&lowpart_target = i * 100;  /* STRICT_LOW_PART-like */
                break;
            case 2:
                subreg_u.halves[i % 2] = result & 0xFFFF;  /* SUBREG-like */
                break;
            case 3:
                result += ms.arr[array[i] % 50];  /* Complex MEM */
                break;
        }
    }
    
    return result;
}

/* ===== Main driver ===== */

int main(void) {
    int total_result = 0;
    
    printf("Starting pattern generation for resource tracking...\n");
    
    /* Call individual pattern functions */
    volatile unsigned int extract_src = 0x89ABCDEF;
    total_result += extract_bits_ze1(&extract_src);
    
    uint32_t lowpart_var = 0x11223344;
    set_low_byte_slp1(&lowpart_var, 0x77);
    total_result += lowpart_var;
    
    union SubregUnion u = {0};
    total_result += subreg_via_union1(&u);
    
    struct MemStruct ms;
    for (int i = 0; i < 100; i++) ms.arr[i] = i * 3;
    total_result += complex_mem1(&ms, 10, 20);
    
    /* Call combined function multiple times with different flags */
    for (int i = 0; i < 8; i++) {
        total_result += combined_patterns(g_volatile_flag + i);
    }
    
    /* Additional complex memory patterns */
    int big_array[200];
    for (int i = 0; i < 200; i++) {
        big_array[i] = i * i;
    }
    
    struct Outer outer;
    for (int i = 0; i < 10; i++) {
        outer.inner[i].x = i * 10;
        outer.inner[i].y = i * 20;
    }
    
    /* More pattern mixing in a loop */
    for (volatile int i = 0; i < 100; i++) {
        if (g_volatile_counter++ % 3 == 0) {
            /* Bit-field struct for ZERO_EXTRACT */
            struct BitFieldStruct bfs = {1, 2, 3};
            total_result += extract_bits_ze2(&bfs);
        }
        
        if (g_volatile_counter % 5 == 0) {
            /* SUBREG via cast */
            int64_t big_val = 0x1234567890ABCDEFLL;
            total_result += subreg_via_cast2(&big_val);
        }
        
        if (g_volatile_counter % 7 == 0) {
            /* Complex memory with pointer arithmetic */
            total_result += complex_mem2(big_array, ms.arr, i % 50);
        }
    }
    
    printf("Final result: %d\n", total_result);
    
    /* Use result to prevent dead code elimination */
    if (total_result > 1000000) {
        printf("Result is large\n");
    }
    
    return total_result & 0xFF;  /* Return non-zero to indicate success */
}
