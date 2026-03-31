/* resource_coverage.c
 * 
 * This program is designed to trigger specific RTL patterns in GCC's
 * resource tracking logic, particularly targeting the uncovered lines
 * in resource.cc that handle ZERO_EXTRACT, STRICT_LOW_PART, SUBREG, and MEM.
 * 
 * Compilation recommendations:
 *   gcc -O2 -m32 -fno-strict-aliasing -c resource_coverage.c
 *   gcc -O3 -funroll-loops -m32 -fno-strict-aliasing -c resource_coverage.c
 *   gcc -Os -m32 -fno-strict-aliasing -c resource_coverage.c
 */

#include <stddef.h>

/* Prevent inlining to ensure functions generate separate RTL */
#define NOINLINE __attribute__((noinline))

/* Function A: Focus on ZERO_EXTRACT and MEM patterns */
NOINLINE static void func_a(volatile int *counter) {
    /* Struct with volatile bit-field to generate ZERO_EXTRACT */
    struct bitfield_struct {
        volatile unsigned int field1 : 5;
        volatile unsigned int field2 : 3;
        volatile unsigned int field3 : 8;
    } bs;
    
    /* Array with complex indexing for MEM patterns */
    volatile int arr[10][10];
    
    /* Initialize with volatile counter to prevent optimization */
    int i = *counter % 10;
    int j = (*counter + 1) % 10;
    
    /* ZERO_EXTRACT: Assignment to volatile bit-field */
    bs.field1 = i & 0x1F;
    bs.field2 = j & 0x07;
    
    /* MEM: Complex array access with pointer arithmetic */
    volatile int *ptr = &arr[i][j];
    ptr += (i * j) % 5;
    
    /* Combined: Bit-field access through pointer */
    struct bitfield_struct *bps = (struct bitfield_struct *)ptr;
    bps->field3 = (i * j) & 0xFF;
    
    /* MEM with addressing mode */
    volatile int value = arr[*counter % 10][(*counter + 3) % 10];
    (void)value; /* Use to prevent elimination */
}

/* Function B: Focus on STRICT_LOW_PART and SUBREG patterns */
NOINLINE static void func_b(volatile int *counter) {
    /* Use char/short types to encourage byte operations */
    volatile char c = *counter & 0xFF;
    volatile short s = *counter & 0xFFFF;
    volatile int i = *counter;
    
    /* STRICT_LOW_PART: Inline assembly modifying only part of register */
    /* Modify low byte of integer */
    asm volatile (
        "addb $1, %0"
        : "=q"(c)      /* =q constraint for byte-addressable register */
        : "0"(c)       /* Matching constraint for input */
        : "cc"         /* Clobbers condition codes */
    );
    
    /* Another STRICT_LOW_PART pattern with short */
    asm volatile (
        "incw %0"
        : "=r"(s)      /* Word-sized operation */
        : "0"(s)
        : "cc"
    );
    
    /* SUBREG: Type punning through pointer casts */
    /* Access int as two shorts (SUBREG of HI mode inside SI) */
    short *ps = (short *)&i;
    ps[0] = s;          /* Low part */
    ps[1] = s + 1;      /* High part (platform-dependent endianness) */
    
    /* More SUBREG: Mixed-size accesses */
    char *pc = (char *)&i;
    pc[2] = c;          /* Third byte */
    
    /* Use results to prevent elimination */
    *counter += (int)c + (int)s + i;
}

/* Function C: Complex expression mixing multiple patterns */
NOINLINE static void func_c(volatile int *counter, volatile int *arr, size_t size) {
    if (size == 0) return;
    
    /* Ternary operator selecting between bit-field addresses */
    struct {
        volatile unsigned int low : 4;
        volatile unsigned int high : 4;
    } bf1, bf2;
    
    /* ZERO_EXTRACT with conditional */
    volatile unsigned int *target = (*counter & 1) ? &bf1.low : &bf2.high;
    (void)target; /* Use would generate ZERO_EXTRACT */
    
    /* Actually assign to force RTL generation */
    if (*counter & 1) {
        bf1.low = (*counter >> 4) & 0x0F;
    } else {
        bf2.high = (*counter >> 8) & 0x0F;
    }
    
    /* MEM with complex addressing and SUBREG */
    volatile int *int_ptr = arr + (*counter % (size / sizeof(int)));
    volatile short *short_ptr = (volatile short *)int_ptr;
    
    /* Mixed-size access through pointer */
    *short_ptr = (*counter) & 0xFFFF;
    
    /* Additional MEM pattern with scaled index */
    volatile int idx = (*counter * 3) % (size / sizeof(int));
    volatile int val = arr[idx] + arr[idx + 1];
    
    /* Use result */
    *counter += val;
}

/* Helper function to create more complex MEM addressing */
NOINLINE static void mem_pattern_helper(volatile int *base, int offset1, int offset2) {
    /* Multi-dimensional addressing simulation */
    volatile int *ptr = base + offset1 * 10 + offset2;
    volatile int value = *ptr;
    
    /* Chain of MEM accesses */
    volatile int **pptr = (volatile int **)ptr;
    (void)pptr;
    
    /* Force MEM reference */
    *base = value;
}

/* Main function that drives all patterns */
int main(int argc, char **argv) {
    volatile int counter = 0;
    volatile int iterations = 10;
    
    /* Use argc to bound iterations if provided */
    if (argc > 1) {
        iterations = 100; /* More iterations for better coverage */
    }
    
    /* Initialize arrays with volatile elements */
    volatile int array[100];
    for (int i = 0; i < 100; i++) {
        array[i] = i;
    }
    
    /* Main loop to repeatedly trigger patterns */
    for (volatile int i = 0; i < iterations; i++) {
        /* Call each pattern function */
        func_a(&counter);
        func_b(&counter);
        func_c(&counter, array, sizeof(array));
        
        /* Complex MEM addressing in loop */
        mem_pattern_helper(array, i % 10, (i + 1) % 10);
        
        /* Update counter with volatile semantics */
        counter += i;
        
        /* Prevent loop unrolling from simplifying too much */
        if (argv[0]) { /* Always true, but compiler doesn't know */
            counter += (int)argv[0][0];
        }
    }
    
    /* Final dummy operation using all results */
    volatile int sum = counter;
    
    /* Additional patterns in cleanup */
    {
        /* One more STRICT_LOW_PART before exit */
        volatile char final_byte = sum & 0xFF;
        asm volatile (
            "orb $0x80, %0"
            : "=q"(final_byte)
            : "0"(final_byte)
            : "cc"
        );
        sum += final_byte;
    }
    
    /* Return something based on the computations */
    return sum & 1;
}
