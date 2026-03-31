/* reload_coverage.c - Test program to trigger various reload types in GCC reload1.cc */

#include <stdint.h>
#include <stdlib.h>

/* Force frame pointer usage and prevent optimizations */
#pragma GCC optimize ("O0")
#pragma GCC push_options

/* Structure with many members to force complex addressing */
struct BigStruct {
    int a, b, c, d, e, f, g, h;
    int arr[8][8];
    struct {
        int x, y, z;
    } nested;
    volatile int vol;
};

/* Global arrays to create addressing complexity */
static int global_array[256][256];
static volatile int volatile_global[100];

/* Function 1: Complex array addressing with multiple index computations */
static __attribute__((noinline)) 
int test_complex_addressing(int a, int b, int c, int d, int e, int f, int g, int h) {
    /* Many local variables to consume registers */
    int i1, i2, i3, i4, i5, i6, i7, i8, i9, i10;
    int j1, j2, j3, j4, j5, j6, j7, j8, j9, j10;
    int k1, k2, k3, k4, k5, k6, k7, k8, k9, k10;
    int l1, l2, l3, l4, l5, l6, l7, l8, l9, l10;
    
    /* Complex array indexing - forces address reloads */
    i1 = global_array[a + b][c + d] + global_array[e + f][g + h];
    i2 = global_array[b + c][d + e] + global_array[f + g][h + a];
    
    /* Multi-level computation with many intermediate values */
    i3 = ((a * b) + (c * d) - (e * f) + (g * h)) & 0xFF;
    i4 = ((b * c) + (d * e) - (f * g) + (h * a)) & 0xFF;
    
    /* Nested array accesses with volatile */
    volatile_global[i3] = i1;
    i5 = volatile_global[i4] + i2;
    
    /* More computations to keep values live */
    i6 = i1 * i2 + i3 * i4 - i5;
    i7 = i2 * i3 + i4 * i5 - i6;
    i8 = i3 * i4 + i5 * i6 - i7;
    i9 = i4 * i5 + i6 * i7 - i8;
    i10 = i5 * i6 + i7 * i8 - i9;
    
    /* Complex addressing with multiple base registers */
    j1 = global_array[i1 + i2][i3 + i4] + global_array[i5 + i6][i7 + i8];
    j2 = global_array[i2 + i3][i4 + i5] + global_array[i6 + i7][i8 + i9];
    
    /* Manual loop unrolling to increase register pressure */
    #pragma GCC unroll 4
    for (int x = 0; x < 4; x++) {
        j3 += global_array[a + x][b + x] * global_array[c + x][d + x];
        j4 += global_array[e + x][f + x] * global_array[g + x][h + x];
        j5 += global_array[b + x][c + x] * global_array[d + x][e + x];
        j6 += global_array[f + x][g + x] * global_array[h + x][a + x];
    }
    
    return i10 + j1 + j2 + j3 + j4 + j5 + j6;
}

/* Function 2: Structure member accesses with pointer arithmetic */
static __attribute__((noinline))
int test_structure_access(struct BigStruct *s1, struct BigStruct *s2, 
                          struct BigStruct *s3, struct BigStruct *s4) {
    int sum = 0;
    
    /* Complex structure member accesses */
    sum += s1->a + s1->b + s1->c + s1->d;
    sum += s1->arr[s1->nested.x][s1->nested.y];
    sum += s1->vol;  /* volatile access forces memory reload */
    
    /* Pointer arithmetic with structure offsets */
    int *ptr1 = &s1->a;
    int *ptr2 = &s2->b;
    int *ptr3 = &s3->c;
    int *ptr4 = &s4->d;
    
    /* Complex pointer expressions */
    sum += *(ptr1 + s1->nested.x) + *(ptr2 + s2->nested.y);
    sum += *(ptr3 - s3->nested.z) + *(ptr4 + s4->a);
    
    /* Array accesses through structure pointers */
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            sum += s1->arr[i][j] * s2->arr[j][i];
            sum += s3->arr[i][j] - s4->arr[j][i];
        }
    }
    
    /* Inline assembly with multiple outputs to clobber registers */
    int out1, out2, out3, out4;
    asm volatile (
        "movl %[in1], %[out1]\n\t"
        "movl %[in2], %[out2]\n\t"
        "addl %[in3], %[out1]\n\t"
        "addl %[in4], %[out2]\n\t"
        "movl %[out1], %[out3]\n\t"
        "movl %[out2], %[out4]"
        : [out1] "=&r" (out1), [out2] "=&r" (out2),
          [out3] "=&r" (out3), [out4] "=&r" (out4)
        : [in1] "r" (s1->a), [in2] "r" (s1->b),
          [in3] "r" (s1->c), [in4] "r" (s1->d)
        : "cc"
    );
    
    sum += out1 + out2 + out3 + out4;
    return sum;
}

/* Function 3: Mixed addressing modes with inline assembly constraints */
static __attribute__((noinline))
int test_mixed_addressing(int *base1, int *base2, int idx1, int idx2, int scale) {
    int result = 0;
    
    /* Various addressing modes */
    result += base1[idx1];                     /* simple indexing */
    result += *(base1 + idx1 * scale + idx2);  /* scaled indexing */
    result += base2[idx1 * 2 + idx2 * 3];      /* complex index computation */
    
    /* Address computations that need their own registers */
    int *addr1 = base1 + idx1 * scale;
    int *addr2 = base2 + idx2 * scale;
    int *addr3 = addr1 + idx2;
    int *addr4 = addr2 + idx1;
    
    /* Use computed addresses */
    result += *addr1 + *addr2 + *addr3 + *addr4;
    
    /* Inline assembly with memory output operand */
    int temp;
    asm volatile (
        "movl (%[addr]), %%eax\n\t"
        "addl %%eax, %[temp]\n\t"
        "movl %[temp], (%[out])"
        : [temp] "=r" (temp), [out] "=m" (*addr1)
        : [addr] "r" (addr2), "0" (result)
        : "%eax", "cc", "memory"
    );
    
    /* More complex pointer chasing */
    int **ptr_ptr = &addr1;
    result += **ptr_ptr;
    ptr_ptr = &addr2;
    result += **ptr_ptr;
    
    /* Volatile pointer access */
    volatile int *vol_ptr = (volatile int *)addr3;
    result += *vol_ptr;
    
    return result + temp;
}

/* Function 4: Extreme register pressure with many live values */
static __attribute__((noinline))
int test_register_pressure(int p1, int p2, int p3, int p4, 
                           int p5, int p6, int p7, int p8) {
    /* Declare many local variables to exhaust registers */
    int v1 = p1, v2 = p2, v3 = p3, v4 = p4, v5 = p5, v6 = p6, v7 = p7, v8 = p8;
    int w1, w2, w3, w4, w5, w6, w7, w8, w9, w10;
    int x1, x2, x3, x4, x5, x6, x7, x8, x9, x10;
    int y1, y2, y3, y4, y5, y6, y7, y8, y9, y10;
    int z1, z2, z3, z4, z5, z6, z7, z8, z9, z10;
    
    /* Complex expression chain keeping many values live */
    w1 = v1 + v2; w2 = v3 + v4; w3 = v5 + v6; w4 = v7 + v8;
    w5 = v1 * v2; w6 = v3 * v4; w7 = v5 * v6; w8 = v7 * v8;
    w9 = w1 + w2 + w3 + w4;
    w10 = w5 + w6 + w7 + w8;
    
    /* More computations with data dependencies */
    x1 = w1 * w2 - w3 + w4;
    x2 = w5 * w6 - w7 + w8;
    x3 = w9 * w10;
    x4 = x1 + x2 + x3;
    x5 = x1 * x2 * x3;
    x6 = x4 + x5;
    x7 = x6 * 2 - x5;
    x8 = x7 / 3 + x6;
    x9 = x8 * x7 - x6;
    x10 = x9 + x8 + x7 + x6;
    
    /* Use inline assembly to force specific register usage */
    asm volatile (
        "movl %0, %%eax\n\t"
        "movl %1, %%ebx\n\t"
        "movl %2, %%ecx\n\t"
        "movl %3, %%edx\n\t"
        "addl %%eax, %%ebx\n\t"
        "addl %%ecx, %%edx\n\t"
        "movl %%ebx, %0\n\t"
        "movl %%edx, %1"
        : "+r" (y1), "+r" (y2)
        : "r" (x1), "r" (x2)
        : "%eax", "%ebx", "%ecx", "%edx", "cc"
    );
    
    /* More variables to increase pressure */
    y3 = x3 + x4; y4 = x5 + x6; y5 = x7 + x8; y6 = x9 + x10;
    y7 = y1 * y2; y8 = y3 * y4; y9 = y5 * y6;
    y10 = y7 + y8 + y9;
    
    /* Final computation using all variables */
    z1 = v1 + w1 + x1 + y1;
    z2 = v2 + w2 + x2 + y2;
    z3 = v3 + w3 + x3 + y3;
    z4 = v4 + w4 + x4 + y4;
    z5 = v5 + w5 + x5 + y5;
    z6 = v6 + w6 + x6 + y6;
    z7 = v7 + w7 + x7 + y7;
    z8 = v8 + w8 + x8 + y8;
    z9 = w9 + x9 + y9;
    z10 = w10 + x10 + y10;
    
    return z1 + z2 + z3 + z4 + z5 + z6 + z7 + z8 + z9 + z10;
}

/* Main driver function */
int main(void) {
    int total = 0;
    
    /* Initialize global array */
    for (int i = 0; i < 256; i++) {
        for (int j = 0; j < 256; j++) {
            global_array[i][j] = i * j;
        }
    }
    
    /* Initialize volatile global */
    for (int i = 0; i < 100; i++) {
        volatile_global[i] = i * 2;
    }
    
    /* Test 1: Complex addressing */
    total += test_complex_addressing(1, 2, 3, 4, 5, 6, 7, 8);
    total += test_complex_addressing(8, 7, 6, 5, 4, 3, 2, 1);
    
    /* Test 2: Structure accesses */
    struct BigStruct s1, s2, s3, s4;
    
    /* Initialize structures */
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            s1.arr[i][j] = i + j;
            s2.arr[i][j] = i * j;
            s3.arr[i][j] = i - j;
            s4.arr[i][j] = j - i;
        }
    }
    
    s1.a = 1; s1.b = 2; s1.c = 3; s1.d = 4;
    s1.nested.x = 1; s1.nested.y = 2; s1.nested.z = 3;
    s1.vol = 100;
    
    s2.a = 5; s2.b = 6; s2.c = 7; s2.d = 8;
    s2.nested.x = 4; s2.nested.y = 5; s2.nested.z = 6;
    s2.vol = 200;
    
    s3.a = 9; s3.b = 10; s3.c = 11; s3.d = 12;
    s3.nested.x = 7; s3.nested.y = 8; s3.nested.z = 9;
    s3.vol = 300;
    
    s4.a = 13; s4.b = 14; s4.c = 15; s4.d = 16;
    s4.nested.x = 10; s4.nested.y = 11; s4.nested.z = 12;
    s4.vol = 400;
    
    total += test_structure_access(&s1, &s2, &s3, &s4);
    
    /* Test 3: Mixed addressing modes */
    int array1[100], array2[100];
    for (int i = 0; i < 100; i++) {
        array1[i] = i * 3;
        array2[i] = i * 5;
    }
    
    total += test_mixed_addressing(array1, array2, 10, 20, 4);
    total += test_mixed_addressing(array2, array1, 5, 15, 2);
    
    /* Test 4: Extreme register pressure */
    total += test_register_pressure(1, 2, 3, 4, 5, 6, 7, 8);
    total += test_register_pressure(9, 10, 11, 12, 13, 14, 15, 16);
    
    /* Use result to prevent dead code elimination */
    volatile int use_result = total;
    
    return use_result > 0 ? 0 : 1;
}

#pragma GCC pop_options
