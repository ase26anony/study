/* Compile with: gcc -O2 -fschedule-insns -fno-strict-aliasing -c this_file.c */
/* For RTL dumps add: -fdump-rtl-reload -fdump-rtl-all */

#include <stdio.h>
#include <stdlib.h>

/* Global volatile struct with bit-fields to force ZERO_EXTRACT/STRICT_LOW_PART */
volatile struct BitFieldStruct {
    unsigned int a : 4;
    unsigned int b : 8;
    unsigned int c : 20;
    unsigned int padding : 32;
} g_bfs = {0};

/* 2D array for complex addressing modes */
static int arr[100][100];

/* Non-inline function to force memory addressing */
__attribute__((noinline))
void modify_bitfields(struct BitFieldStruct *s, int x, int y) {
    /* Multiple bit-field assignments - may generate ZERO_EXTRACT/STRICT_LOW_PART */
    s->a = x & 0xF;
    s->b = (x >> 4) & 0xFF;
    s->c = y & 0xFFFFF;
    
    /* Additional assignment to force different patterns */
    s->padding = x + y;
}

/* Another noinline function for mixed-width operations */
__attribute__((noinline))
void mixed_width_ops(short *shorts, char *chars, int *ints, int n) {
    for (int i = 0; i < n; i++) {
        /* Operations that may generate SUBREG patterns */
        shorts[i] = ints[i] & 0xFFFF;           /* truncation to 16-bit */
        chars[i] = (ints[i] >> 16) & 0xFF;      /* truncation to 8-bit */
        
        /* Sign extension operations */
        ints[i] = (int)shorts[i] * (int)chars[i]; /* SUBREG may appear here */
    }
}

/* Function with complex array addressing */
__attribute__((noinline))
int complex_addressing(int i, int j, int mask) {
    /* Complex addressing with bitwise operation */
    return arr[i][j] & mask;  /* May generate ZERO_EXTRACT for the mask operation */
}

int main(int argc, char **argv) {
    /* Force argc to be at least 1 */
    if (argc < 1) return 1;
    
    /* 1. Bit-field operations on volatile struct */
    modify_bitfields((struct BitFieldStruct*)&g_bfs, argc, argc * 2);
    
    /* 2. Mixed-width local variables for SUBREG generation */
    volatile short vs[10];
    volatile char vc[10];
    volatile int vi[10];
    
    /* Initialize with argc-dependent values */
    for (int i = 0; i < 10; i++) {
        vi[i] = argc + i * 17;
    }
    
    /* Perform mixed-width operations */
    for (int i = 0; i < 10; i++) {
        /* These assignments may generate SUBREG in SET_DEST */
        vs[i] = vi[i];                     /* int -> short truncation */
        vc[i] = vi[i] >> 8;                /* int -> char truncation */
        
        /* More complex expression with mixing */
        vi[i] = (int)vs[i] + (int)vc[i];   /* short/char -> int extension */
    }
    
    /* 3. Complex array addressing with volatile indices */
    volatile int idx_i = argc % 50;
    volatile int idx_j = (argc * 3) % 50;
    
    /* Initialize some array elements */
    for (int i = 0; i < 50; i++) {
        for (int j = 0; j < 50; j++) {
            arr[i][j] = i * 100 + j;
        }
    }
    
    /* Access with volatile indices - prevents constant propagation */
    int val1 = complex_addressing(idx_i, idx_j, 0xFF);
    int val2 = complex_addressing(idx_j, idx_i, 0xFFFF);
    
    /* 4. Inline assembly to increase register pressure */
    asm volatile ("" 
                  : /* no outputs */
                  : /* no inputs */
                  : "r0", "r1", "r2", "r3", "r4", "r5", "memory");
    
    /* 5. More bit-field manipulation */
    struct BitFieldStruct local_bfs;
    
    /* Take address and manipulate */
    struct BitFieldStruct *p = &local_bfs;
    p->a = argc & 0xF;
    p->b = (argc >> 4) & 0xFF;
    p->c = argc & 0xFFFFF;
    
    /* 6. Additional mixed-type operations in loop */
    unsigned char buffer[100];
    unsigned short *short_ptr = (unsigned short*)buffer;
    unsigned int *int_ptr = (unsigned int*)buffer;
    
    for (int i = 0; i < 25; i++) {  /* 25 * 4 = 100 bytes */
        /* Store with different widths - may generate SUBREG */
        short_ptr[i] = argc + i;          /* 16-bit store */
        buffer[i] = argc & 0xFF;          /* 8-bit store */
        
        /* Load with extension */
        int val = buffer[i];              /* 8-bit load extended to int */
        int_ptr[i] = val * 2;             /* 32-bit store */
    }
    
    /* 7. Compute checksum to prevent dead code elimination */
    int checksum = g_bfs.a + g_bfs.b + g_bfs.c;
    for (int i = 0; i < 10; i++) {
        checksum += vs[i] + vc[i] + vi[i];
    }
    checksum += val1 + val2;
    checksum += local_bfs.a + local_bfs.b + local_bfs.c;
    
    /* Use checksum so it can't be optimized away */
    if (checksum > 0) {
        printf("Checksum: %d\n", checksum);
    } else {
        printf("Zero checksum\n");
    }
    
    return 0;
}
