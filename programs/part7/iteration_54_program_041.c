/* Test program to exercise GCC reload pass switch cases */
#include <stdio.h>
#include <stdint.h>

/* Volatile indices to prevent constant folding */
volatile int vi1 = 7, vi2 = 13, vi3 = 42;

/* Structure for passing by value */
struct SmallStruct {
    int a, b, c, d;
};

/* Vector type to stress register allocation */
typedef int v4si __attribute__((vector_size(16)));

/* Function to pass/return structures by value */
struct SmallStruct process_struct(struct SmallStruct s1, struct SmallStruct s2) {
    struct SmallStruct result;
    result.a = s1.a + s2.b;
    result.b = s1.b - s2.c;
    result.c = s1.c * s2.d;
    result.d = s1.d + s2.a;
    return result;
}

/* Another function to create call chain */
struct SmallStruct chain_struct(struct SmallStruct s) {
    struct SmallStruct local = {vi1, vi2, vi3, 1};
    return process_struct(s, local);
}

int main(void) {
    int checksum = 0;
    
    /* Large arrays to increase register pressure */
    int big_array1[100][50];
    int big_array2[100][50];
    
    /* Initialize arrays */
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 50; j++) {
            big_array1[i][j] = i * 100 + j;
            big_array2[i][j] = 0;
        }
    }
    
    /* Pattern 1: Complex addressing modes with volatile indices */
    /* Should trigger: RELOAD_FOR_INPUT_ADDRESS, RELOAD_FOR_OUTPUT_ADDRESS */
    for (int i = 0; i < 10; i++) {
        /* Non-constant indices with arithmetic */
        int idx1 = (vi1 * i) % 100;
        int idx2 = (vi2 + i * 3) % 50;
        int idx3 = (vi3 - i * 2) % 100;
        int idx4 = (i * 7) % 50;
        
        /* Complex array access pattern */
        big_array2[idx1][idx2] = big_array1[idx3][idx4] + 
                                 big_array1[idx1][idx2] * 
                                 big_array1[idx3][idx4];
    }
    
    /* Pattern 2: Inline assembly with multiple constraints */
    /* Should trigger: RELOAD_FOR_INPUT, RELOAD_FOR_OUTPUT_ADDRESS, RELOAD_OTHER */
    {
        int a = vi1, b = vi2, c = vi3;
        int d, e, f;
        
        /* Chain of asm blocks creating dependencies */
        asm volatile (
            "movl %1, %0\n\t"
            "addl %2, %0"
            : "=r" (d)
            : "r" (a), "r" (b)
            : "cc"
        );
        
        asm volatile (
            "imull %1, %0\n\t"
            "subl %2, %0"
            : "+r" (d)
            : "r" (c), "m" (big_array1[0][0])
            : "cc"
        );
        
        asm volatile (
            "leal (%1, %2, 4), %0"
            : "=r" (e)
            : "r" (d), "r" (a)
        );
        
        asm volatile (
            "movl %1, %0\n\t"
            "rorl $8, %0"
            : "=r" (f)
            : "m" (big_array2[1][1])
            : "cc"
        );
        
        checksum += d + e + f;
    }
    
    /* Pattern 3: Structure passing by value */
    /* Should trigger: RELOAD_FOR_INPADDR_ADDRESS, RELOAD_FOR_OUTADDR_ADDRESS */
    {
        struct SmallStruct s1 = {1, 2, 3, 4};
        struct SmallStruct s2 = {5, 6, 7, 8};
        struct SmallStruct s3, s4;
        
        /* Chain of structure operations */
        s3 = process_struct(s1, s2);
        s4 = chain_struct(s3);
        
        checksum += s4.a + s4.b + s4.c + s4.d;
    }
    
    /* Pattern 4: Vector operations */
    /* Should trigger: RELOAD_FOR_OPERAND_ADDRESS, RELOAD_FOR_OPADDR_ADDR */
    {
        v4si v1 = {1, 2, 3, 4};
        v4si v2 = {5, 6, 7, 8};
        v4si v3, v4;
        
        /* Vector operations that may need decomposition */
        v3 = v1 + v2;
        v4 = v1 * v2;
        
        /* Shuffle to create complex pattern */
        v4 = __builtin_shuffle(v3, v4, (v4si){0, 2, 1, 3});
        
        /* Access elements to force computation */
        int* vp = (int*)&v4;
        checksum += vp[0] + vp[1] + vp[2] + vp[3];
    }
    
    /* Pattern 5: Complex control flow with goto */
    /* Should trigger: RELOAD_FOR_OTHER_ADDRESS */
    {
        int x = vi1, y = vi2;
        int result = 0;
        
        /* Create split live ranges with goto */
        if (x > 0) {
            int temp = x * 2;
            y = temp + 1;
            goto label1;
        } else {
            int temp = x * 3;
            y = temp - 1;
            goto label2;
        }
        
    label1:
        {
            int z = y * 3;
            result += z;
            goto label3;
        }
        
    label2:
        {
            int z = y * 5;
            result += z;
            /* Fall through */
        }
        
    label3:
        /* Use result in complex addressing */
        big_array2[result % 100][0] += result;
        checksum += result;
    }
    
    /* Pattern 6: Mixed addressing modes in loops */
    /* Should trigger multiple reload types */
    {
        int* ptr1 = &big_array1[0][0];
        int* ptr2 = &big_array2[0][0];
        
        for (int i = 0; i < 1000; i++) {
            /* Pointer arithmetic with volatile */
            int offset = (vi1 * i) % 5000;
            
            /* Complex addressing */
            ptr1[offset] = ptr2[offset * 2 % 5000] + 
                          ptr1[(offset + vi2) % 5000];
            
            /* More pointer arithmetic */
            if (i % 7 == 0) {
                ptr2[offset] = ptr1[offset] * vi3;
            }
        }
        
        /* Compute checksum from modified arrays */
        for (int i = 0; i < 100; i += 3) {
            for (int j = 0; j < 50; j += 2) {
                checksum += big_array1[i][j];
                checksum += big_array2[i][j];
            }
        }
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
