/* test_resource_coverage.c
 * 
 * This program is designed to trigger specific RTL patterns in GCC's
 * resource tracking logic (mark_referenced_resources function).
 * The goal is to cover lines 282-290 in resource.cc which handle
 * ZERO_EXTRACT, STRICT_LOW_PART, SUBREG, and MEM expressions.
 */

#include <stddef.h>

/* Force functions to not be inlined to ensure they generate separate RTL */
#define NOINLINE __attribute__((noinline))

/* Function A: Focus on ZERO_EXTRACT and MEM patterns */
NOINLINE static void func_a(volatile int *counter) {
    /* Struct with volatile bit-field for ZERO_EXTRACT */
    struct bitfield_struct {
        volatile unsigned int field1 : 5;
        volatile unsigned int field2 : 7;
        volatile unsigned int field3 : 3;
    } bf;
    
    /* Array with complex addressing for MEM patterns */
    volatile int arr[10][10];
    
    /* Use loop counter to create non-constant indices */
    int i = *counter % 10;
    int j = (*counter + 1) % 10;
    
    /* ZERO_EXTRACT: Assignment to volatile bit-field */
    bf.field1 = i & 0x1F;
    bf.field2 = j & 0x7F;
    
    /* MEM: Complex array addressing with pointer arithmetic */
    volatile int *ptr = &arr[0][0];
    ptr += i * 10 + j;  /* Create complex address calculation */
    
    /* Another MEM access through the calculated pointer */
    volatile int val = *ptr;
    
    /* Combine both: MEM of ZERO_EXTRACT address */
    bf.field3 = val & 0x7;
    
    /* Prevent dead code elimination */
    *counter += bf.field1 + bf.field2 + bf.field3;
}

/* Function B: Focus on STRICT_LOW_PART and SUBREG patterns */
NOINLINE static void func_b(volatile int *counter) {
    /* Use char/short types for STRICT_LOW_PART */
    volatile char c = *counter & 0xFF;
    volatile short s = *counter & 0xFFFF;
    
    /* STRICT_LOW_PART: Inline assembly modifying only part of register */
    /* Modify low byte of char */
    asm volatile (
        "addb $1, %0"
        : "=q"(c)      /* =q constraint for byte-addressable register */
        : "0"(c)       /* Same as output */
        : "cc"
    );
    
    /* Modify low word of short */
    asm volatile (
        "addw $2, %0"
        : "=r"(s)      /* Word-sized register */
        : "0"(s)
        : "cc"
    );
    
    /* SUBREG: Type punning between different sized types */
    int int_val = *counter;
    
    /* Access int as short (SUBREG from SImode to HImode) */
    short *short_ptr = (short*)&int_val;
    *short_ptr = s;  /* This generates SUBREG store */
    
    /* Access int as char (SUBREG from SImode to QImode) */
    char *char_ptr = (char*)&int_val;
    char_ptr[1] = c;  /* Another SUBREG access */
    
    /* Mixed-size operations that may create SUBREG in RTL */
    long long big_val = (long long)int_val * s;
    int_val = (int)(big_val >> 16);  /* Potential SUBREG extraction */
    
    /* Prevent dead code elimination */
    *counter += int_val + c + s;
}

/* Function C: Complex expression mixing multiple patterns */
NOINLINE static void func_c(volatile int *counter, volatile int *result) {
    /* Nested struct with bit-fields for ZERO_EXTRACT */
    struct nested {
        struct {
            volatile unsigned int a : 4;
            volatile unsigned int b : 4;
        } inner;
        volatile int array[5];
    } ns;
    
    /* Initialize with counter-dependent values */
    int idx = *counter % 5;
    
    /* Complex ternary selecting address (MEM pattern) */
    volatile int *select_ptr = (idx & 1) ? 
                              &ns.array[idx] : 
                              &ns.array[4 - idx];
    
    /* ZERO_EXTRACT on selected value */
    ns.inner.a = (*select_ptr) & 0xF;
    
    /* More complex addressing with multiple indices */
    volatile int *ptr2 = &ns.array[0];
    ptr2 += (idx * 3) % 5;  /* Non-linear addressing */
    
    /* SUBREG access through pointer casting */
    short *short_view = (short*)ptr2;
    *short_view = (short)(*counter);  /* SUBREG store */
    
    /* ZERO_EXTRACT from the SUBREG-modified value */
    ns.inner.b = (*ptr2 >> 4) & 0xF;
    
    /* Final assignment that could involve multiple RTL transformations */
    *result = ns.inner.a + ns.inner.b + *select_ptr;
}

/* Main function that drives the pattern generation */
int main(int argc, char **argv) {
    volatile int counter = 0;
    volatile int result = 0;
    
    /* Use argc to bound loops - prevents infinite loops during analysis */
    int iterations = (argc > 1) ? 10 : 5;  /* Reduced for compilation speed */
    
    /* Initialize some volatile arrays */
    volatile int data[20];
    for (int i = 0; i < 20; i++) {
        data[i] = i * 2;
    }
    
    /* Main loop to repeatedly trigger the patterns */
    for (int i = 0; i < iterations; i++) {
        /* Update counter in volatile way */
        counter += i + (int)((size_t)argv % 16);  /* Use argv to prevent const propagation */
        
        /* Call each pattern-generating function */
        func_a(&counter);
        func_b(&counter);
        func_c(&counter, &result);
        
        /* Complex MEM access with multiple indices */
        volatile int *ptr = &data[0];
        ptr += (counter % 10) * 2;
        result += *ptr;
        
        /* Additional SUBREG pattern via union type punning */
        union {
            int i;
            short s[2];
            char c[4];
        } u;
        u.i = counter;
        u.s[1] = (short)result;  /* SUBREG access */
        result += u.i;
    }
    
    /* Final dummy operation to prevent elimination */
    asm volatile ("" : : "r"(result), "r"(counter) : "memory");
    
    return 0;
}
