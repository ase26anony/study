/* Target: resource.cc lines 282-290 coverage */
#include <stdio.h>
#include <stdlib.h>

/* Force compiler to generate specific RTL patterns */

/* Global volatile struct with bit-fields for ZERO_EXTRACT/STRICT_LOW_PART */
volatile struct BitFieldStruct {
    unsigned int a : 4;    /* Will generate ZERO_EXTRACT for 4-bit field */
    unsigned int b : 8;    /* 8-bit field */
    unsigned int c : 20;   /* 20-bit field */
    unsigned int pad : 32; /* Full 32-bit field for contrast */
} g_bfs = {0};

/* Helper to prevent optimization */
__attribute__((noinline, optimize("O0")))
void modify_bitfields(struct BitFieldStruct *s, int x, int y, int z) {
    /* Multiple bit-field assignments to force RTL patterns */
    s->a = x & 0xF;        /* ZERO_EXTRACT for 4-bit assignment */
    s->b = y & 0xFF;       /* ZERO_EXTRACT for 8-bit assignment */
    s->c = z & 0xFFFFF;    /* ZERO_EXTRACT for 20-bit assignment */
    
    /* Mixed assignment to full field for comparison */
    s->pad = (x << 16) | (y << 8) | z;
}

/* Another noinline function for SUBREG patterns */
__attribute__((noinline, optimize("O0")))
void mixed_width_operations(volatile short *shorts, volatile char *chars, 
                           volatile int *ints, int count) {
    for (int i = 0; i < count; i++) {
        /* Operations that generate SUBREG patterns */
        shorts[i] = ints[i] & 0xFFFF;          /* SUBREG: truncate 32->16 bits */
        chars[i] = (ints[i] >> 8) & 0xFF;      /* SUBREG: 32->8 bits */
        
        /* Complex expression with SUBREG */
        int temp = shorts[i] * chars[i];       /* SUBREG in multiplication */
        ints[i] = temp + (chars[i] << 16);     /* Mixed-width operation */
    }
}

/* Function to create complex addressing modes */
__attribute__((noinline, optimize("O0")))
int complex_addressing(int arr[100][100], volatile int *idx1, volatile int *idx2) {
    /* Volatile indices prevent constant propagation */
    int i = *idx1 % 100;
    int j = *idx2 % 100;
    
    /* Complex memory access with bit manipulation */
    int val = arr[i][j];
    
    /* Bit-field like operation on memory value */
    val = (val & ~0xFF) | ((val + 1) & 0xFF);  /* May generate ZERO_EXTRACT */
    
    /* Store back with modification */
    arr[i][j] = val;
    
    return val;
}

/* Main function with register pressure */
int main(int argc, char *argv[]) {
    /* Force non-constant loop bounds */
    int loop_count = argc > 1 ? atoi(argv[1]) : 10;
    if (loop_count > 100) loop_count = 100;
    
    /* 1. Bit-field operations for ZERO_EXTRACT/STRICT_LOW_PART */
    modify_bitfields((struct BitFieldStruct *)&g_bfs, 
                     argc, argc * 2, argc * 3);
    
    /* 2. Mixed-width operations for SUBREG patterns */
    volatile short short_array[100];
    volatile char char_array[100];
    volatile int int_array[100];
    
    /* Initialize arrays */
    for (int i = 0; i < loop_count; i++) {
        int_array[i] = i * 3 + argc;
    }
    
    /* Perform mixed-width operations */
    mixed_width_operations(short_array, char_array, int_array, loop_count);
    
    /* 3. Complex addressing with 2D array */
    int matrix[100][100];
    volatile int idx1 = argc * 7;
    volatile int idx2 = argc * 13;
    
    /* Initialize matrix */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            matrix[i][j] = i * 100 + j + argc;
        }
    }
    
    /* Access with complex addressing */
    int matrix_val = complex_addressing(matrix, &idx1, &idx2);
    
    /* 4. Increase register pressure with inline assembly */
    /* Clobber multiple registers to force spilling */
    asm volatile (
        "/* Clobber registers to increase pressure */"
        :
        : 
        : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", 
          "r8", "r9", "r10", "r11", "r12", "memory"
    );
    
    /* 5. More operations to create additional RTL patterns */
    /* Pointer casting for SUBREG patterns */
    {
        volatile int *int_ptr = &int_array[0];
        volatile short *short_ptr = (volatile short *)int_ptr;
        volatile char *char_ptr = (volatile char *)int_ptr;
        
        /* Mixed accesses through different type pointers */
        for (int i = 0; i < loop_count; i++) {
            /* These generate SUBREG MEM accesses */
            short_ptr[i] = int_ptr[i] >> 8;
            char_ptr[i * 2] = int_ptr[i] & 0xFF;
        }
    }
    
    /* 6. Compute checksum to prevent dead code elimination */
    unsigned int checksum = 0;
    
    /* Include bit-field values */
    checksum += g_bfs.a;
    checksum += g_bfs.b;
    checksum += g_bfs.c;
    checksum += g_bfs.pad;
    
    /* Include array values */
    for (int i = 0; i < loop_count && i < 10; i++) {
        checksum += short_array[i];
        checksum += char_array[i];
        checksum += int_array[i];
    }
    
    /* Include matrix value */
    checksum += matrix_val;
    
    /* Final inline assembly to ensure all operations complete */
    asm volatile (
        "/* Memory barrier */"
        :
        :
        : "memory"
    );
    
    printf("Checksum: %u\n", checksum);
    
    return (checksum > 0) ? 0 : 1;
}

/* Additional global to prevent optimization */
volatile int global_dummy = 0;

/* Another function to create more patterns */
__attribute__((noinline, optimize("O0")))
void extra_patterns(void) {
    /* STRICT_LOW_PART pattern through byte operations */
    volatile struct {
        unsigned char low;
        unsigned char high;
    } pair = {0, 0};
    
    volatile int *as_int = (volatile int *)&pair;
    
    /* This may generate STRICT_LOW_PART */
    *as_int = (*as_int & 0xFFFFFF00) | (global_dummy & 0xFF);
    
    /* More SUBREG patterns */
    volatile long long big_val = 0x123456789ABCDEF0LL;
    volatile int *half = (volatile int *)&big_val;
    
    /* Access halves of long long */
    half[0] += argc;  /* if argc is accessible here */
    half[1] += argc * 2;
}
