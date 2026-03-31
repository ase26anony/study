/* This program is designed to trigger specific RTL patterns in GCC's
   resource tracking pass during optimization. It creates operations
   that generate ZERO_EXTRACT, STRICT_LOW_PART, SUBREG, and complex
   MEM expressions in the RTL representation. */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Volatile variables to prevent optimization and create unpredictable
   control flow */
static volatile int v_flag1 = 1;
static volatile int v_flag2 = 0;
static volatile int v_counter = 0;

/* Global arrays and structs for memory operand patterns */
static int global_array[256];
static volatile int *volatile_ptr = &global_array[0];

/* Struct with bit-fields for ZERO_EXTRACT patterns */
struct bitfield_struct {
    unsigned int low8 : 8;
    unsigned int mid8 : 8;
    unsigned int high16 : 16;
};

/* Union for SUBREG patterns */
union mixed_types {
    int32_t full;
    int16_t halves[2];
    int8_t bytes[4];
};

/* Complex struct for memory addressing */
struct nested_struct {
    int data[64];
    struct bitfield_struct bf;
    union mixed_types u;
    int padding[8];
};

/* Global instances */
static struct bitfield_struct g_bf = {0};
static union mixed_types g_union = {0};
static struct nested_struct g_nested = {0};

/* ========== ZERO_EXTRACT PATTERNS ========== */

/* Function 1: Direct bit-field extraction */
unsigned int extract_bitfield_1(struct bitfield_struct *s) {
    /* Accessing bit-field members often generates ZERO_EXTRACT */
    unsigned int val = s->mid8;  /* 8-bit extract from middle */
    val += s->high16;            /* 16-bit extract from high part */
    return val;
}

/* Function 2: Manual bit extraction with volatile */
unsigned int extract_bitfield_2(volatile unsigned int *p) {
    /* Complex shift/mask pattern that may generate ZERO_EXTRACT */
    unsigned int temp = *p;
    /* Extract bits 8-15 */
    unsigned int extracted = (temp >> 8) & 0xFF;
    /* Extract bits 16-31 with conditional */
    if (v_flag1) {
        extracted |= (temp >> 16) & 0xFFFF;
    }
    return extracted;
}

/* Function 3: Multiple extractions in loop */
unsigned int extract_bitfield_3(volatile unsigned int *arr, int n) {
    unsigned int sum = 0;
    for (int i = 0; i < n; i++) {
        /* Each iteration does a different extract */
        unsigned int val = arr[i];
        switch (i % 3) {
            case 0: sum += (val >> 0) & 0xF; break;    /* bits 0-3 */
            case 1: sum += (val >> 4) & 0xFF; break;   /* bits 4-11 */
            case 2: sum += (val >> 12) & 0xFFFF; break;/* bits 12-27 */
        }
    }
    return sum;
}

/* ========== STRICT_LOW_PART PATTERNS ========== */

/* Function 4: Writing to low part of larger variable */
void write_low_part_1(volatile unsigned int *p, unsigned char b) {
    /* This pattern may generate STRICT_LOW_PART */
    *p = (*p & ~0xFF) | b;  /* Only modify low 8 bits */
}

/* Function 5: Cast to smaller type assignment */
void write_low_part_2(int32_t *dest, int16_t value) {
    /* Casting pointer may create STRICT_LOW_PART */
    *(int16_t*)dest = value;  /* Write only low 16 bits */
}

/* Function 6: Conditional low-part write */
void write_low_part_3(union mixed_types *u) {
    /* Conditional write to partial register */
    if (v_flag2) {
        u->halves[0] = 0x1234;  /* Write to low half */
    } else {
        u->bytes[1] = 0xAB;     /* Write to second byte */
    }
}

/* ========== SUBREG PATTERNS ========== */

/* Function 7: Union-based subreg access */
int32_t subreg_access_1(union mixed_types *u) {
    /* Access different views of same storage */
    int32_t result = u->full;
    result += u->halves[0];  /* SUBREG from 32-bit to 16-bit */
    result += u->bytes[3];   /* SUBREG from 32-bit to 8-bit */
    return result;
}

/* Function 8: Pointer casting for subreg */
int32_t subreg_access_2(int64_t *large) {
    /* Cast between different pointer sizes */
    int32_t *medium = (int32_t*)large;
    int16_t *small = (int16_t*)large;
    
    /* Mixed-size accesses to same memory */
    int32_t val1 = *medium;
    int16_t val2 = *small;
    return val1 + val2;
}

/* Function 9: Array with mixed types */
int32_t subreg_access_3(void) {
    /* Create aliasing through arrays */
    int32_t data32[4] = {0};
    int16_t *data16 = (int16_t*)data32;
    
    /* Write through 16-bit view, read through 32-bit view */
    data16[1] = 0x5678;
    data16[3] = 0x9ABC;
    return data32[0] + data32[1];  /* SUBREG accesses */
}

/* ========== COMPLEX MEM PATTERNS ========== */

/* Function 10: Complex addressing mode */
int mem_access_1(struct nested_struct *ns, int idx1, int idx2) {
    /* Complex address calculation */
    return ns->data[idx1 * 8 + idx2 * 2];
}

/* Function 11: Pointer arithmetic with multiple indices */
int mem_access_2(int *base, int i, int j, int k) {
    /* Non-trivial addressing */
    return base[(i * j) + (k << 2) + 16];
}

/* Function 12: Struct with multiple field accesses */
int mem_access_3(struct nested_struct *ns) {
    /* Multiple memory accesses with different offsets */
    int sum = ns->data[0];
    sum += ns->bf.mid8;          /* Bit-field access */
    sum += ns->u.halves[1];      /* Union access */
    sum += ns->padding[3];       /* Array access */
    return sum;
}

/* ========== MAIN FUNCTION ========== */

int main(void) {
    /* Initialize data */
    for (int i = 0; i < 256; i++) {
        global_array[i] = i * 3 + 1;
    }
    
    g_bf.low8 = 0x12;
    g_bf.mid8 = 0x34;
    g_bf.high16 = 0x5678;
    
    g_union.full = 0x89ABCDEF;
    
    for (int i = 0; i < 64; i++) {
        g_nested.data[i] = i * 5;
    }
    g_nested.bf = g_bf;
    g_nested.u = g_union;
    
    /* Result accumulator */
    unsigned int result = 0;
    
    /* Main loop with mixed operations */
    for (v_counter = 0; v_counter < 10; v_counter++) {
        /* Vary flags to create different control flow paths */
        v_flag1 = (v_counter % 3) == 0;
        v_flag2 = (v_counter % 5) == 0;
        
        /* Execute all pattern functions in mixed order */
        if (v_flag1) {
            result += extract_bitfield_1(&g_bf);
            result += extract_bitfield_2((volatile unsigned int*)&global_array[v_counter]);
        }
        
        if (v_flag2) {
            write_low_part_1((volatile unsigned int*)&g_union.full, v_counter);
            write_low_part_2(&g_nested.u.full, v_counter * 2);
        } else {
            write_low_part_3(&g_union);
        }
        
        /* Always do SUBREG accesses */
        result += subreg_access_1(&g_union);
        result += subreg_access_2((int64_t*)&global_array[0]);
        result += subreg_access_3();
        
        /* Complex memory accesses */
        int idx = v_counter % 8;
        result += mem_access_1(&g_nested, idx, idx * 2);
        result += mem_access_2(&global_array[32], idx, idx + 1, idx + 2);
        result += mem_access_3(&g_nested);
        
        /* Bit-field extraction in loop */
        result += extract_bitfield_3((volatile unsigned int*)&global_array[64], 4);
    }
    
    /* Use volatile pointer to force memory operations */
    *volatile_ptr = result;
    
    /* Print result to prevent optimization */
    printf("Result: %u\n", result);
    
    return (int)(result & 0x7FFFFFFF);
}
