/* test_resource.c - Program to trigger ZERO_EXTRACT, STRICT_LOW_PART, and SUBREG RTL expressions */

#include <stdio.h>
#include <stdlib.h>

/* Structure with bit-fields to potentially generate ZERO_EXTRACT/STRICT_LOW_PART */
struct bitfield_pack {
    unsigned int flag1 : 1;
    unsigned int value : 12;
    unsigned int : 3;  /* padding */
    unsigned int flag2 : 3;
    unsigned int count : 5;
    unsigned int : 8;  /* more padding */
};

/* Union for accessing same memory as different types */
union mixed_access {
    struct bitfield_pack bits;
    unsigned int raw;
    volatile unsigned int volatile_raw;
};

/* Function to force complex addressing */
static int complex_index(int i, int stride, int offset) {
    return i * stride + offset;
}

/* Function with inline assembly to manipulate low parts */
static long long asm_lowpart_op(long long x, int y) {
    long long result;
    /* Inline asm that operates on low 32 bits */
    __asm__ volatile (
        "movl %2, %%eax\n\t"
        "addl %3, %%eax\n\t"
        "movl %%eax, %0\n\t"
        "movl %4, %%edx\n\t"
        "movl %%edx, %1"
        : "=r" (((unsigned int*)&result)[0]), "=r" (((unsigned int*)&result)[1])
        : "r" (((const unsigned int*)&x)[0]), "r" (y), "r" (((const unsigned int*)&x)[1])
        : "%eax", "%edx", "memory"
    );
    return result;
}

int main(int argc, char *argv[]) {
    volatile int i, limit;
    volatile long long ll_var = 0x123456789ABCDEF0LL;
    volatile double dbl_var = 3.141592653589793;
    volatile int results_sum = 0;
    
    /* Use argc to make loop count non-constant */
    limit = (argc > 1) ? atoi(argv[1]) : 100;
    if (limit <= 0) limit = 50;
    
    /* Volatile array with complex access patterns */
    volatile int array[256];
    for (i = 0; i < 256; i++) {
        array[i] = i * 3;
    }
    
    /* Volatile structure with bit-fields */
    volatile union mixed_access data;
    data.raw = 0;
    
    /* Main loop with mixed operations */
    for (i = 0; i < limit; i++) {
        int idx;
        long long temp_ll;
        double temp_dbl;
        
        /* 1. Bit-field operations (potential ZERO_EXTRACT/STRICT_LOW_PART) */
        data.bits.flag1 = i & 1;                    /* Could generate STRICT_LOW_PART */
        data.bits.value = (i * 7) & 0xFFF;          /* 12-bit field */
        data.bits.flag2 = (i >> 2) & 0x7;           /* 3-bit field */
        data.bits.count = (data.bits.value >> 3) & 0x1F; /* Extract from another field */
        
        /* Extract bit-field using mask and shift (potential ZERO_EXTRACT) */
        unsigned int extracted = (data.volatile_raw >> 13) & 0x7; /* Extract flag2 */
        
        /* 2. Multi-word operations (potential SUBREG generation) */
        /* Operations on long long on 32-bit targets need SUBREGs */
        ll_var = ll_var + (long long)(i * 0x10001LL);
        ll_var = ll_var ^ (0xF0F0F0F0F0F0F0F0LL >> (i & 0x3F));
        
        /* Force split operations by accessing halves separately */
        unsigned int low_part = ((unsigned int*)&ll_var)[0];
        unsigned int high_part = ((unsigned int*)&ll_var)[1];
        low_part = low_part + extracted;
        high_part = high_part ^ (i << 16);
        ((unsigned int*)&ll_var)[0] = low_part;
        ((unsigned int*)&ll_var)[1] = high_part;
        
        /* Double operations also need multiple registers on some archs */
        dbl_var = dbl_var * 1.0001 + (i & 0xF);
        
        /* 3. Complex memory addressing with bit-field derived index */
        idx = complex_index(i, data.bits.value, data.bits.flag2 * 4);
        idx = idx & 0xFF;  /* Ensure in bounds */
        
        /* Read-modify-write with bit manipulation */
        array[idx] = (array[idx] & ~0xFF) | (data.volatile_raw & 0xFF);
        array[(idx + 128) & 0xFF] = array[(idx + 128) & 0xFF] ^ (data.volatile_raw >> 8);
        
        /* 4. Control flow based on bit-field and multi-word comparisons */
        if (data.bits.flag1) {
            /* Branch 1: More bit-field manipulation */
            data.bits.value = (data.bits.value << 1) | data.bits.flag2;
            
            /* Use inline assembly for low-part operations */
            temp_ll = asm_lowpart_op(ll_var, i);
            ll_var = temp_ll + (ll_var & 0xFFFFFFFF00000000LL);
        } else {
            /* Branch 2: Different operations */
            if (low_part > high_part) {
                /* Swap halves when low > high */
                ((unsigned int*)&ll_var)[0] = high_part;
                ((unsigned int*)&ll_var)[1] = low_part;
            }
            
            /* Access misaligned data through pointer casting */
            char *byte_ptr = (char*)&array[idx];
            unsigned int word = *(unsigned int*)(byte_ptr + 1); /* Potential unaligned */
            array[idx] = word & 0x7FFFFFFF;
        }
        
        /* Conditional based on parity of extracted bit-field */
        switch (extracted & 0x3) {
            case 0:
                dbl_var = dbl_var / 1.5;
                break;
            case 1:
                dbl_var = dbl_var * 1.5;
                break;
            case 2:
                /* Force memory operation with complex address */
                volatile int *ptr = &array[complex_index(i, 3, data.bits.count)];
                *ptr = *ptr + 1;
                break;
            case 3:
                /* Mixed operation */
                ll_var = ll_var - (long long)(dbl_var * 1000.0);
                break;
        }
        
        /* Accumulate results to prevent elimination */
        results_sum += data.bits.value + (low_part & 0xFF) + (array[idx] & 0xFF);
    }
    
    /* Final aggregation */
    int final_result = results_sum;
    for (i = 0; i < 256; i += 16) {
        final_result ^= array[i];
    }
    
    final_result += ((unsigned int*)&ll_var)[0] ^ ((unsigned int*)&ll_var)[1];
    final_result += (int)(dbl_var * 100.0);
    
    /* Print to prevent dead code elimination */
    printf("Result: %d (limit=%d)\n", final_result, limit);
    
    return final_result & 0xFF;
}
