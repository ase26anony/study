/* Test case for early rematerialization pass coverage */
#include <stdio.h>
#include <stdlib.h>

/* Volatile variables to create dataflow barriers */
volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
volatile int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;

/* Mixed types for mode conversions */
typedef struct {
    short a;
    int b;
    long c;
    char d;
} mixed_t;

/* Packed structure for sub-register accesses */
struct __attribute__((packed)) packed_struct {
    int x : 12;
    int y : 8;
    int z : 12;
};

/* Function with high register pressure and complex dataflow */
int __attribute__((noinline)) compute_heavy(int base1, int base2, int base3, 
                                           int base4, int base5, int base6) {
    /* Many local variables to create register pressure */
    int t1, t2, t3, t4, t5, t6, t7, t8, t9, t10;
    int t11, t12, t13, t14, t15, t16, t17, t18, t19, t20;
    int t21, t22, t23, t24, t25, t26, t27, t28, t29, t30;
    short s1, s2, s3, s4, s5;
    long l1, l2, l3;
    
    /* Complex expression chain with many intermediate values */
    t1 = base1 + v1;
    t2 = base2 ^ v2;
    t3 = t1 * t2;
    t4 = base3 - v3;
    t5 = t3 | t4;
    t6 = base4 & v4;
    t7 = t5 ^ t6;
    t8 = base5 + v5;
    t9 = t7 * t8;
    t10 = base6 - v6;
    t11 = t9 | t10;
    t12 = t1 + t2;
    t13 = t3 - t4;
    t14 = t5 * t6;
    t15 = t7 ^ t8;
    t16 = t9 & t10;
    t17 = t11 | t12;
    t18 = t13 ^ t14;
    t19 = t15 * t16;
    t20 = t17 - t18;
    
    /* Mode conversions to trigger different register modes */
    s1 = (short)t1;
    s2 = (short)t2;
    s3 = (short)t3;
    s4 = (short)t4;
    s5 = (short)t5;
    
    l1 = (long)t6;
    l2 = (long)t7;
    l3 = (long)t8;
    
    /* More arithmetic with mixed types */
    t21 = (int)s1 + (int)s2;
    t22 = (int)s3 * (int)s4;
    t23 = t21 ^ t22;
    t24 = (int)(l1 & 0xFFFF);
    t25 = (int)(l2 | 0xFFFF);
    t26 = t23 + t24;
    t27 = t25 * t26;
    t28 = t20 ^ t27;
    t29 = t19 & t28;
    t30 = t18 | t29;
    
    /* Complex switch with different variable usage patterns */
    switch (t30 & 0x7) {
        case 0:
            /* Use subset 1 */
            t1 = t2 + t3;
            t4 = t5 * t6;
            s1 = (short)(t1 & 0xFFFF);
            break;
        case 1:
            /* Use subset 2 */
            t7 = t8 ^ t9;
            t10 = t11 | t12;
            s2 = (short)(t7 & 0xFFFF);
            break;
        case 2:
            /* Use subset 3 */
            t13 = t14 - t15;
            t16 = t17 * t18;
            s3 = (short)(t13 & 0xFFFF);
            break;
        case 3:
            /* Use subset 4 */
            t19 = t20 ^ t21;
            t22 = t23 | t24;
            s4 = (short)(t19 & 0xFFFF);
            break;
        case 4:
            /* Use subset 5 */
            t25 = t26 - t27;
            t28 = t29 * t30;
            s5 = (short)(t25 & 0xFFFF);
            break;
        case 5:
            /* Use all variables */
            t1 = t2 + t3 + t4 + t5;
            t6 = t7 * t8 * t9 * t10;
            l1 = (long)t1 * (long)t6;
            break;
        case 6:
            /* Mixed mode operations */
            t11 = (int)s1 + (int)s2 + (int)s3;
            t12 = (int)(l1 & 0xFFFFFFFF) + (int)(l2 & 0xFFFFFFFF);
            break;
        default:
            /* Complex expression chain */
            t13 = t14 ^ t15 ^ t16 ^ t17;
            t18 = t19 | t20 | t21 | t22;
            t23 = t13 * t18;
            break;
    }
    
    /* Final aggregation */
    int result = t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9 + t10 +
                 t11 + t12 + t13 + t14 + t15 + t16 + t17 + t18 + t19 + t20 +
                 t21 + t22 + t23 + t24 + t25 + t26 + t27 + t28 + t29 + t30 +
                 (int)s1 + (int)s2 + (int)s3 + (int)s4 + (int)s5 +
                 (int)(l1 & 0xFFFFFFFF) + (int)(l2 & 0xFFFFFFFF) + (int)(l3 & 0xFFFFFFFF);
    
    return result;
}

/* Main function with loops to increase pressure */
int main() {
    int i, j;
    int total = 0;
    
    /* Array for address calculations (potential rematerialization) */
    int array[100];
    for (i = 0; i < 100; i++) {
        array[i] = i * 3;
    }
    
    /* Multiple loops with different induction variables */
    for (i = 0; i < 1000; i++) {
        /* Create many live values across loop iterations */
        int a = i + v1;
        int b = i * v2;
        int c = i ^ v3;
        int d = i - v4;
        int e = i & v5;
        
        /* Complex expression using array address calculations */
        int *ptr1 = &array[i % 100];
        int *ptr2 = &array[(i + 1) % 100];
        int *ptr3 = &array[(i + 2) % 100];
        
        /* Use the pointers in computations (address rematerialization candidates) */
        int f = *ptr1 + a;
        int g = *ptr2 * b;
        int h = *ptr3 ^ c;
        
        /* Inline assembly to create complex dataflow */
        int asm_out1, asm_out2;
        asm volatile (
            "movl %1, %0\n\t"
            "addl %2, %0\n\t"
            : "=r" (asm_out1)
            : "r" (f), "r" (g)
            : "cc"
        );
        
        asm volatile (
            "xorl %1, %0\n\t"
            "orl %2, %0\n\t"
            : "=r" (asm_out2)
            : "r" (h), "r" (asm_out1)
            : "cc"
        );
        
        /* Call compute_heavy with many arguments */
        int heavy_result = compute_heavy(a, b, c, d, e, f);
        
        /* Use packed structure for sub-register mode changes */
        struct packed_struct ps;
        ps.x = a & 0xFFF;
        ps.y = b & 0xFF;
        ps.z = c & 0xFFF;
        
        int packed_val = ps.x + ps.y + ps.z;
        
        /* Conditional with __builtin_expect */
        if (__builtin_expect((i & 0xF) == 0, 0)) {
            /* Merge many live values */
            total += heavy_result + asm_out1 + asm_out2 + packed_val + 
                     g + h + *ptr1 + *ptr2 + *ptr3;
        } else {
            total += heavy_result + asm_out2;
        }
        
        /* Nested loop for additional pressure */
        for (j = 0; j < 10; j++) {
            int temp = (i * j) & 0xFF;
            total ^= temp;
            
            /* More mode conversions */
            short stemp = (short)temp;
            long ltemp = (long)temp * 1000;
            
            total += (int)stemp + (int)(ltemp & 0xFFFF);
        }
    }
    
    printf("Result: %d\n", total);
    return 0;
}
