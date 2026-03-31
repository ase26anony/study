/* test_resource.c - Program to trigger ZERO_EXTRACT, STRICT_LOW_PART, and SUBREG RTL patterns */

#include <stdio.h>
#include <stdint.h>

/* Structure with bit-fields to generate ZERO_EXTRACT/STRICT_LOW_PART */
struct bitfield_struct {
    volatile unsigned int field1 : 1;
    volatile unsigned int field2 : 3;
    volatile unsigned int field3 : 12;
    volatile unsigned int field4 : 16;
    volatile unsigned int padding : 32;
};

/* Union for accessing same memory as different types */
union mixed_access {
    volatile uint32_t word;
    volatile struct bitfield_struct bits;
    volatile uint8_t bytes[4];
};

/* Function to force complex addressing modes */
static int complex_index(int idx, int stride) {
    return (idx * stride + 7) & 0xFF;
}

/* Main function with operations designed to generate target RTL patterns */
int main(int argc, char *argv[]) {
    volatile int i, j, k;
    volatile long long ll_var = 0x123456789ABCDEF0LL;
    volatile double dbl_var = 3.141592653589793;
    volatile union mixed_access data[4];
    volatile uint32_t array[256];
    volatile int loop_limit = (argc > 1) ? 10 : 5;
    volatile uint32_t result = 0;
    
    /* Initialize data */
    for (i = 0; i < 4; i++) {
        data[i].word = 0;
        data[i].bits.field1 = i & 1;
        data[i].bits.field2 = (i * 3) & 0x7;
        data[i].bits.field3 = (i * 100) & 0xFFF;
        data[i].bits.field4 = (i * 1000) & 0xFFFF;
    }
    
    for (i = 0; i < 256; i++) {
        array[i] = i;
    }
    
    /* Main loop with operations designed to trigger target RTL patterns */
    for (i = 0; i < loop_limit; i++) {
        /* 1. Bit-field operations for ZERO_EXTRACT/STRICT_LOW_PART */
        volatile struct bitfield_struct *bf_ptr = &data[i % 4].bits;
        
        /* Extract and manipulate bit-fields */
        volatile unsigned int extracted = bf_ptr->field3;
        bf_ptr->field2 = (bf_ptr->field2 + 1) & 0x7;  /* STRICT_LOW_PART candidate */
        
        /* Manual bit extraction using shifts and masks */
        volatile uint32_t mask = (1 << 12) - 1;
        volatile uint32_t bits = (data[i % 4].word >> 4) & mask;  /* ZERO_EXTRACT candidate */
        
        /* 2. Multi-word operations for SUBREG generation */
        ll_var = ll_var + 0x100000001LL;  /* Forces 64-bit arithmetic on 32-bit targets */
        dbl_var = dbl_var * 1.01;         /* Double operations may use multiple registers */
        
        /* Split 64-bit operations */
        volatile uint32_t ll_low = (uint32_t)(ll_var & 0xFFFFFFFF);
        volatile uint32_t ll_high = (uint32_t)(ll_var >> 32);
        
        /* 3. Complex memory addressing with bit-field derived index */
        volatile int idx = complex_index(i, extracted & 0xF);
        array[idx] = array[idx] + bits;
        
        /* Array access with pointer arithmetic */
        volatile uint32_t *ptr = array + ((i * 13 + 7) & 0xFF);
        *ptr = *ptr ^ data[i % 4].word;
        
        /* 4. Conditional control flow based on bit-field and multi-word results */
        if (bf_ptr->field1) {
            /* When field1 is 1, use different operations */
            ll_var = ll_var | 0x5555555555555555LL;
            
            /* Access misaligned data through byte pointers */
            volatile uint8_t *byte_ptr = (volatile uint8_t *)&ll_var;
            for (j = 0; j < 4; j++) {
                byte_ptr[j] = byte_ptr[j] + i;
            }
        } else {
            /* When field1 is 0, compare high and low words */
            if (ll_high > ll_low) {
                dbl_var = dbl_var / 2.0;
            }
        }
        
        /* Switch based on bit-field value */
        switch (bf_ptr->field2) {
            case 0:
                array[i % 256] = array[i % 256] << 1;
                break;
            case 1:
                array[i % 256] = array[i % 256] >> 1;
                break;
            case 2:
                array[i % 256] = ~array[i % 256];
                break;
            default:
                array[i % 256] = array[i % 256] + ll_low;
                break;
        }
        
        /* Inline assembly to force specific register constraints */
        /* This asm operates on low 32 bits of ll_var, potentially creating STRICT_LOW_PART */
        asm volatile (
            "addl $1, %0"
            : "+r" (ll_low)
            :
            : "cc"
        );
        
        /* Recombine after assembly operation */
        ll_var = ((long long)ll_high << 32) | ll_low;
    }
    
    /* Aggregate results to prevent optimization */
    for (i = 0; i < 256; i++) {
        result += array[i];
    }
    
    result += data[0].word + data[1].word + data[2].word + data[3].word;
    result += (uint32_t)ll_var + (uint32_t)(ll_var >> 32);
    result += (uint32_t)dbl_var;
    
    printf("Result: %u\n", (unsigned int)result);
    
    /* Additional test with packed structure for misaligned access */
    struct __attribute__((packed)) packed_struct {
        char a;
        int b;
        char c;
    } packed;
    
    packed.a = 'A';
    packed.b = 0xDEADBEEF;
    packed.c = 'Z';
    
    /* Force reload of misaligned field */
    volatile int misaligned_load = packed.b;
    result += misaligned_load;
    
    return (int)(result & 0x7FFFFFFF);
}
