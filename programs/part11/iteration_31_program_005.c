/* test_resource.c - Program to trigger ZERO_EXTRACT, STRICT_LOW_PART, and SUBREG RTL patterns */

#include <stdio.h>
#include <stdlib.h>

/* Structure with bit-fields to potentially generate ZERO_EXTRACT/STRICT_LOW_PART */
struct bitfield_pack {
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    unsigned int value : 12;
    unsigned int pad   : 16;
} __attribute__((packed));

/* Union for accessing same memory as different types */
union mixed_access {
    struct bitfield_pack bf;
    unsigned int raw;
    unsigned short parts[2];
};

/* Function with inline assembly to force specific register usage */
static inline unsigned long long asm_ll_op(unsigned long long a, unsigned long long b) {
    unsigned long long result;
    /* Inline asm that operates on low 32 bits, potentially creating STRICT_LOW_PART */
    __asm__ volatile (
        "addl %2, %k0\n\t"          /* Add lower 32 bits, %k0 = 32-bit register name */
        "adcl $0, %H0"              /* Add carry to high 32 bits */
        : "=r"(result)
        : "0"(a), "r"(b)
        : "cc"
    );
    return result;
}

/* Function to access memory with complex addressing */
static volatile int* complex_index(volatile int* arr, int idx, int stride) {
    return &arr[idx * stride + (idx & 7)];  /* Non-trivial addressing */
}

int main(int argc, char** argv) {
    volatile int loop_limit = (argc > 1) ? atoi(argv[1]) : 100;
    if (loop_limit <= 0) loop_limit = 100;
    
    /* Variables that may generate SUBREG operations on 32-bit targets */
    volatile long long ll_var = 0x123456789ABCDEF0ULL;
    volatile double dbl_var = 3.141592653589793;
    
    /* Bit-field structure with volatile pointer */
    volatile union mixed_access data;
    data.raw = 0;
    
    /* Array with volatile elements for complex memory addressing */
    volatile int arr[256];
    for (int i = 0; i < 256; i++) {
        arr[i] = i;
    }
    
    /* Results accumulator */
    unsigned int total_result = 0;
    
    /* Main loop with mixed operations */
    for (volatile int i = 0; i < loop_limit; i++) {
        /* 1. Bit-field operations - may generate ZERO_EXTRACT/STRICT_LOW_PART */
        data.bf.flag1 = i & 1;
        data.bf.flag2 = (i >> 1) & 0x7;
        data.bf.value = (data.bf.value + i) & 0xFFF;
        
        /* Extract using shift/mask (alternative to direct bit-field access) */
        unsigned int extracted = (data.raw >> 4) & 0xFFF;  /* Could become ZERO_EXTRACT */
        
        /* 2. Multi-word operations - may generate SUBREG */
        ll_var = ll_var + (extracted * 0x10001ULL);
        
        /* Force split operations on 64-bit values */
        unsigned int low_part = (unsigned int)(ll_var & 0xFFFFFFFF);
        unsigned int high_part = (unsigned int)(ll_var >> 32);
        
        /* Operation that might use STRICT_LOW_PART */
        low_part = (low_part + high_part) | 0x1;
        
        /* Recombine */
        ll_var = ((unsigned long long)high_part << 32) | low_part;
        
        /* 3. Double operations (uses multiple registers on 32-bit) */
        dbl_var = dbl_var * 1.01 + (double)(i & 0xF);
        
        /* 4. Complex memory addressing with bit-field derived index */
        int idx = (data.bf.value * i) & 0xFF;
        volatile int* ptr = complex_index(arr, idx, 3);
        
        /* Read-modify-write with masking */
        *ptr = (*ptr & 0xFFFF0000) | (extracted & 0xFFFF);
        
        /* 5. Conditional based on bit-field and multi-word comparisons */
        if (data.bf.flag1 || (low_part < high_part)) {
            /* Different operation path */
            ll_var = asm_ll_op(ll_var, 0x100000001ULL);
            
            /* Another potential ZERO_EXTRACT from memory */
            unsigned short mem_low = *(volatile unsigned short*)ptr;
            total_result += mem_low;
        } else {
            /* Alternative: access misaligned 32-bit value */
            if ((idx & 1) && idx < 254) {
                unsigned int misaligned;
                /* Cast through char* to avoid strict-aliasing issues */
                unsigned char* bytes = (unsigned char*)&arr[idx];
                misaligned = bytes[0] | (bytes[1] << 8) | 
                            (bytes[2] << 16) | (bytes[3] << 24);
                total_result ^= misaligned;
            }
        }
        
        /* Switch based on bit-field value */
        switch (data.bf.flag2) {
            case 0:
                arr[idx] += 1;
                break;
            case 1:
                arr[idx] -= extracted;
                break;
            case 2:
                arr[idx] ^= 0x5555;
                break;
            case 3:
                /* Nested bit-field operation */
                data.bf.value = (data.bf.value << 1) | data.bf.flag1;
                break;
            default:
                /* Complex operation mixing types */
                ll_var = ll_var ^ ((unsigned long long)arr[idx] << 16);
                break;
        }
        
        /* Prevent loop unrolling */
        asm volatile("" : "+r"(i) : : "memory");
    }
    
    /* Final aggregation to prevent dead code elimination */
    total_result += (unsigned int)(ll_var & 0xFFFFFFFF);
    total_result += (unsigned int)(ll_var >> 32);
    total_result += data.raw;
    
    /* Use array elements */
    for (int i = 0; i < 16; i++) {
        total_result += arr[i * 7];
    }
    
    /* Print result to prevent optimization */
    printf("Result: %u\n", total_result);
    
    return (int)(total_result & 0xFF);
}
