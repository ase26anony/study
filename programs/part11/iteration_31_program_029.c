/* test_resource.c - Program to trigger ZERO_EXTRACT, STRICT_LOW_PART, and SUBREG RTL patterns */

#include <stdio.h>
#include <stdlib.h>

/* Structure with bit-fields to generate ZERO_EXTRACT/STRICT_LOW_PART */
struct bitfield_struct {
    unsigned int a : 1;    /* 1-bit field */
    unsigned int b : 3;    /* 3-bit field */
    unsigned int c : 12;   /* 12-bit field */
    unsigned int d : 16;   /* 16-bit field */
    unsigned int pad : 0;  /* Force alignment */
} __attribute__((packed));

/* Union for accessing bit-fields in different ways */
union bitfield_union {
    struct bitfield_struct bits;
    volatile unsigned int raw;
    volatile long long extended;
};

/* Function to force complex addressing modes */
static int complex_index(int i, int stride, int offset) {
    return i * stride + offset;
}

/* Function with inline assembly to mimic STRICT_LOW_PART */
static long long asm_low_part(long long val) {
    long long result;
    /* Operate on low 32 bits, high bits implicitly constrained */
    __asm__ volatile (
        "movl %1, %%eax\n\t"
        "addl $0x1234, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=r" (result)
        : "r" ((int)val)
        : "%eax", "cc"
    );
    return result;
}

int main(int argc, char *argv[]) {
    volatile union bitfield_union bf_var = {0};
    volatile long long ll_var = 0x123456789ABCDEF0LL;
    volatile double dbl_var = 3.141592653589793;
    volatile int array[256];
    volatile int limit = (argc > 1) ? atoi(argv[1]) : 100;
    volatile int stride = 7;
    volatile int offset = 3;
    
    /* Initialize array with pattern */
    for (int i = 0; i < 256; i++) {
        array[i] = i * i;
    }
    
    int sum = 0;
    
    /* Main loop with mixed operations */
    for (volatile int i = 0; i < limit; i++) {
        /* 1. Bit-field operations (ZERO_EXTRACT/STRICT_LOW_PART candidates) */
        bf_var.bits.a = (i & 1);                    /* Single bit assignment */
        bf_var.bits.b = (i & 0x7);                  /* 3-bit field */
        bf_var.bits.c = (i * 3) & 0xFFF;            /* 12-bit field */
        
        /* Extract bit-field using mask and shift (ZERO_EXTRACT pattern) */
        unsigned int extracted = (bf_var.raw >> 4) & 0xFFF;  /* Extract 12 bits */
        
        /* Combine bit-fields (may generate STRICT_LOW_PART) */
        bf_var.bits.d = (bf_var.bits.c << 4) | bf_var.bits.b;
        
        /* 2. Multi-word operations (SUBREG candidates on 32-bit targets) */
        ll_var += (long long)extracted * 0x10001LL;
        ll_var ^= 0x5555555555555555LL;
        
        /* Double operations that may use multiple registers */
        dbl_var *= 1.0001;
        dbl_var += (double)(i & 0xF) * 0.001;
        
        /* 3. Complex memory addressing with bit-field derived index */
        int idx = complex_index(i, stride, offset);
        if (idx >= 0 && idx < 256) {
            /* Read-modify-write with bit-field mask */
            array[idx] = (array[idx] & ~0xFF) | (bf_var.bits.c & 0xFF);
            
            /* Access with pointer arithmetic and casting */
            volatile char *byte_ptr = (volatile char *)&array[idx];
            byte_ptr[1] = bf_var.bits.b << 5;  /* Misaligned access */
        }
        
        /* 4. Control flow based on bit-field and multi-word results */
        if (bf_var.bits.a) {
            /* When LSB is 1 */
            ll_var = asm_low_part(ll_var);  /* Inline assembly for low-part ops */
            
            /* Compare high vs low word of long long */
            unsigned int low_word = (unsigned int)ll_var;
            unsigned int high_word = (unsigned int)(ll_var >> 32);
            
            if (low_word > high_word) {
                /* Swap operations using SUBREG-like patterns */
                long long temp = ll_var;
                ll_var = ((long long)high_word << 32) | low_word;
                sum += (int)temp;
            }
        } else {
            /* When LSB is 0 */
            /* Force SUBREG generation through type punning */
            volatile int *int_ptr = (volatile int *)&ll_var;
            int_ptr[0] += int_ptr[1];  /* Add high and low words */
            int_ptr[1] ^= extracted;
        }
        
        /* 5. Additional bit-field manipulation with volatile */
        volatile struct bitfield_struct *bf_ptr = &bf_var.bits;
        bf_ptr->c = (bf_ptr->c * 2) & 0xFFF;  /* Keep within 12 bits */
        
        /* Use switch based on bit-field value */
        switch (bf_ptr->b) {
            case 0: sum += 1; break;
            case 1: sum += array[i % 256]; break;
            case 2: sum += (int)(ll_var & 0xFFFFFFFF); break;
            case 3: sum += (int)(dbl_var * 1000); break;
            default: sum -= 1; break;
        }
        
        /* Prevent loop unrolling */
        asm volatile ("" : : "r"(i) : "memory");
    }
    
    /* Aggregate results to prevent dead code elimination */
    int final_result = sum;
    final_result += (int)ll_var;
    final_result += (int)(ll_var >> 32);
    final_result += (int)(dbl_var * 100);
    final_result += bf_var.bits.c;
    
    /* Access array with complex pattern one more time */
    for (int i = 0; i < 16; i++) {
        int idx = complex_index(i, 13, bf_var.bits.b);
        if (idx >= 0 && idx < 256) {
            final_result ^= array[idx];
        }
    }
    
    printf("Result: %d\n", final_result);
    return final_result & 0xFF;
}
