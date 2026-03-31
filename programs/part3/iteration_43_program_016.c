/* test_resource_coverage.c
 * Designed to trigger specific RTL patterns in GCC's resource tracking:
 * - ZERO_EXTRACT (volatile bit-field assignments)
 * - STRICT_LOW_PART (inline assembly with byte operations)
 * - SUBREG (type punning with mixed-size accesses)
 * - MEM (complex addressing modes)
 */

#include <stddef.h>
#include <string.h>

/* Prevent inlining to ensure separate functions are analyzed */
#define NOINLINE __attribute__((noinline))

/* Function A: Focus on ZERO_EXTRACT and MEM patterns */
NOINLINE static void func_a(volatile int *counter) {
    /* Struct with volatile bit-fields for ZERO_EXTRACT */
    struct bitfield_struct {
        volatile unsigned int f1 : 5;
        volatile unsigned int f2 : 7;
        volatile unsigned int f3 : 3;
        volatile unsigned int padding : 17;
    } bs;
    
    /* Array with complex addressing for MEM patterns */
    volatile int arr[16][8];
    volatile int *ptr_arr[4];
    
    /* Initialize pointer array */
    for (int i = 0; i < 4; i++) {
        ptr_arr[i] = (volatile int *)&arr[i * 2][0];
    }
    
    /* ZERO_EXTRACT: Assign to volatile bit-fields */
    bs.f1 = (*counter & 0x1F);        /* Should generate ZERO_EXTRACT */
    bs.f2 = ((*counter >> 5) & 0x7F); /* Another ZERO_EXTRACT */
    bs.f3 = 1;                        /* And another */
    
    /* MEM: Complex addressing with multiple indices */
    int idx1 = (*counter) & 0x7;
    int idx2 = ((*counter) >> 3) & 0x3;
    
    /* This should generate MEM with complex addressing */
    volatile int val = arr[idx1 * 2][idx2 * 2];
    
    /* More MEM: Pointer arithmetic with dereference */
    volatile int *ptr = ptr_arr[idx2] + idx1;
    volatile int val2 = *ptr;
    
    /* Use results to prevent elimination */
    bs.f1 = bs.f1 ^ (val & 0x1F);
    *counter += val2;
}

/* Function B: Focus on STRICT_LOW_PART and SUBREG patterns */
NOINLINE static void func_b(volatile int *counter) {
    volatile short s_val = (short)(*counter);
    volatile char c_val = (char)(*counter);
    volatile int i_val = *counter;
    
    /* STRICT_LOW_PART: Inline assembly modifying byte part */
    /* Using 'q' constraint for byte-addressable register */
    asm volatile (
        "addb $1, %0\n\t"
        : "=q"(c_val)   /* =q constraint for STRICT_LOW_PART */
        : "0"(c_val)    /* Matching input constraint */
        : "cc"
    );
    
    /* Another STRICT_LOW_PART with different operation */
    asm volatile (
        "orb $0x0F, %0\n\t"
        : "=q"(c_val)
        : "0"(c_val)
        : "cc"
    );
    
    /* SUBREG: Type punning with mixed-size accesses */
    /* Cast pointer to larger type to pointer to smaller type */
    short *ps = (short *)&i_val;
    *ps = (short)(*counter + 1);  /* Should generate SUBREG */
    
    /* More SUBREG: Access different parts of integer */
    char *pc = (char *)&i_val;
    pc[1] = c_val;                /* Another SUBREG */
    pc[3] = (char)(*counter);
    
    /* Mixed operations that may involve SUBREG */
    s_val = (short)i_val;
    i_val = (int)s_val + (int)c_val;
    
    /* Use result */
    *counter += i_val;
}

/* Function C: Complex expression mixing patterns */
NOINLINE static void func_c(volatile int *counter, volatile int *arr_base) {
    /* Struct with bit-field for ZERO_EXTRACT */
    struct mixed_struct {
        volatile unsigned int flag : 1;
        volatile unsigned int value : 15;
        volatile unsigned int index : 8;
    } ms;
    
    /* Initialize */
    ms.flag = (*counter) & 0x1;
    ms.value = (*counter) & 0x7FFF;
    ms.index = (*counter) & 0xFF;
    
    /* Complex addressing with ternary operator */
    volatile int *ptr;
    
    /* This complex expression may generate interesting RTL */
    ptr = (ms.flag) ? 
          (arr_base + ms.index) : 
          (arr_base + (ms.value & 0x3F));
    
    /* MEM with complex addressing */
    volatile int arr_val = ptr[(ms.index * 3) & 0x7];
    
    /* ZERO_EXTRACT assignment based on condition */
    if (arr_val > 0) {
        ms.value = (arr_val & 0x7FFF);  /* ZERO_EXTRACT */
    } else {
        ms.flag = 0;                     /* ZERO_EXTRACT */
    }
    
    /* SUBREG through type punning */
    unsigned char *byte_ptr = (unsigned char *)&arr_val;
    byte_ptr[2] = ms.index;              /* Likely SUBREG */
    
    /* Use results */
    *counter += ms.value + arr_val;
}

/* Function D: Additional patterns with loops */
NOINLINE static void func_d(volatile int *counter) {
    /* Array for MEM patterns */
    volatile int matrix[8][8];
    
    /* Initialize matrix */
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            matrix[i][j] = (i * 8) + j + *counter;
        }
    }
    
    /* Complex nested array access - should generate MEM with addressing */
    volatile int sum = 0;
    for (int i = 1; i < 7; i++) {
        for (int j = 1; j < 7; j++) {
            /* Complex addressing expression */
            sum += matrix[i-1][j] + matrix[i][j-1] + 
                   matrix[i+1][j] + matrix[i][j+1];
        }
    }
    
    /* Bit-field struct for ZERO_EXTRACT */
    struct {
        volatile unsigned int low : 4;
        volatile unsigned int high : 4;
    } bf;
    
    /* Multiple ZERO_EXTRACT assignments */
    bf.low = (sum & 0xF);
    bf.high = ((sum >> 4) & 0xF);
    
    /* STRICT_LOW_PART with inline assembly */
    volatile char byte_var = (char)sum;
    asm volatile (
        "subb $5, %0\n\t"
        : "=q"(byte_var)
        : "0"(byte_var)
        : "cc"
    );
    
    /* Update counter */
    *counter += sum + (bf.high << 4) + bf.low + byte_var;
}

/* Main function that drives everything */
int main(int argc, char **argv) {
    volatile int counter = 0;
    volatile int arr_storage[256];
    
    /* Initialize array */
    for (int i = 0; i < 256; i++) {
        arr_storage[i] = i * 3;
    }
    
    /* Use argc to bound loops for compilation (won't actually run) */
    int iterations = (argc > 1) ? 10 : 5;
    
    /* Main loop calling pattern functions */
    for (volatile int i = 0; i < iterations; i++) {
        /* Call each function with different patterns */
        func_a(&counter);
        func_b(&counter);
        func_c(&counter, arr_storage);
        func_d(&counter);
        
        /* Additional mixed operations */
        volatile int temp = counter;
        
        /* More type punning for SUBREG */
        short *sp = (short *)&temp;
        sp[0] = (short)(temp + i);
        sp[1] = (short)(temp - i);
        
        /* Update counter with modified value */
        counter = temp + i;
    }
    
    /* Final dummy operation to prevent elimination */
    volatile int result = counter;
    
    /* This would cause UB if run, but is valid for compilation */
    /* The cast and dereference are for SUBREG pattern generation */
    if (result > 1000) {
        char *cp = (char *)&result;
        cp[0] = 0;  /* Potential SUBREG */
    }
    
    return result & 0xFF;  /* Prevent elimination of result */
}
