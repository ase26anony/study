/* Test program to exercise GCC reload pass switch cases */
#include <stdio.h>
#include <stdint.h>

/* Volatile indices to prevent constant folding */
volatile int vi1 = 1, vi2 = 2, vi3 = 3, vi4 = 4;

/* Structure for passing by value */
struct SmallStruct {
    int a, b, c, d;
};

/* Function to create structure passing reloads */
struct SmallStruct __attribute__((noinline)) 
process_struct(struct SmallStruct s1, struct SmallStruct s2) {
    struct SmallStruct result;
    result.a = s1.a + s2.b;
    result.b = s1.b + s2.c;
    result.c = s1.c + s2.d;
    result.d = s1.d + s2.a;
    return result;
}

/* Another function to chain structure passing */
struct SmallStruct __attribute__((noinline))
chain_struct(struct SmallStruct s) {
    struct SmallStruct temp = {vi1, vi2, vi3, vi4};
    return process_struct(s, temp);
}

/* Vector type for vector extension reloads */
typedef int v4si __attribute__((vector_size(16)));

/* Function using vector extensions */
v4si __attribute__((noinline))
vector_ops(v4si a, v4si b) {
    /* Complex shuffle pattern */
    v4si c = __builtin_shuffle(a, b, (v4si){3, 1, 7, 5});
    /* Element-wise operations */
    v4si d = a * b + c;
    /* Non-contiguous access pattern */
    int* p = (int*)&d;
    p[vi1] = p[vi2] + p[vi3];
    return d;
}

int main() {
    int checksum = 0;
    
    /* Large arrays to increase register pressure */
    int big_array1[100][50];
    int big_array2[100][50];
    
    /* Initialize arrays */
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 50; j++) {
            big_array1[i][j] = i * 100 + j;
            big_array2[i][j] = (i + j) * 2;
        }
    }
    
    /* Pattern 1: Complex addressing modes with volatile indices */
    /* This stresses RELOAD_FOR_INPUT_ADDRESS, RELOAD_FOR_OUTPUT_ADDRESS */
    for (int i = 0; i < 10; i++) {
        /* Multi-dimensional array with non-constant indices */
        big_array1[vi1 + i][vi2 * 2] = big_array2[vi3][i + vi4];
        big_array2[i][vi1 * vi2] = big_array1[vi4][vi3 + i] + big_array2[vi2][i];
        
        /* Pointer arithmetic that can't be folded */
        int* ptr1 = &big_array1[vi1][i];
        int* ptr2 = &big_array2[i][vi2];
        *ptr1 = *ptr2 + *(ptr2 + vi3);
    }
    
    /* Pattern 2: Inline assembly with multiple operands */
    /* This stresses RELOAD_FOR_INPUT, RELOAD_FOR_OUTPUT_ADDRESS, RELOAD_OTHER */
    {
        int asm_var1 = vi1, asm_var2 = vi2, asm_var3 = vi3;
        
        /* Chain of asm blocks creating dependencies */
        asm volatile (
            "addl %1, %0\n\t"
            "movl %0, %2\n\t"
            : "+r"(asm_var1), "+m"(big_array1[5][5])
            : "r"(asm_var2)
            : "cc"
        );
        
        asm volatile (
            "imull %1, %0\n\t"
            "addl %%eax, %2\n\t"
            : "+r"(asm_var2), "+m"(big_array2[10][10])
            : "r"(asm_var3)
            : "eax", "cc"
        );
        
        asm volatile (
            "leal (%1, %2, 4), %0\n\t"
            : "=r"(asm_var3)
            : "r"(asm_var1), "r"(asm_var2)
            : "cc"
        );
        
        checksum += asm_var1 + asm_var2 + asm_var3;
    }
    
    /* Pattern 3: Structure passing by value */
    /* This stresses RELOAD_FOR_INPADDR_ADDRESS, RELOAD_FOR_OUTADDR_ADDRESS */
    {
        struct SmallStruct s1 = {1, 2, 3, 4};
        struct SmallStruct s2 = {5, 6, 7, 8};
        
        /* Chain of structure passing calls */
        struct SmallStruct s3 = process_struct(s1, s2);
        struct SmallStruct s4 = chain_struct(s3);
        
        checksum += s4.a + s4.b + s4.c + s4.d;
    }
    
    /* Pattern 4: Vector extensions */
    /* This stresses various reload types for vector decomposition */
    {
        v4si v1 = {1, 2, 3, 4};
        v4si v2 = {5, 6, 7, 8};
        
        v4si v3 = vector_ops(v1, v2);
        
        /* Extract elements for checksum */
        int* vp = (int*)&v3;
        checksum += vp[0] + vp[1] + vp[2] + vp[3];
    }
    
    /* Pattern 5: Complex control flow splitting live ranges */
    /* This stresses RELOAD_FOR_OTHER_ADDRESS */
    {
        int x = 0, y = 0, z = 0;
        
        /* Use goto to create non-trivial CFG */
        if (vi1 > 0) {
            x = vi1 * 10;
            goto label1;
        } else {
            x = vi2 * 20;
        }
        
        y = x + vi3;
        
    label1:
        z = y + vi4;
        
        /* Loop with complex addressing */
        for (int i = 0; i < 20; i++) {
            if (i & 1) {
                big_array1[x % 100][i % 50] = big_array2[z % 100][(i + x) % 50];
                goto label2;
            }
            big_array2[y % 100][i % 50] = big_array1[i % 100][z % 50];
        label2:
            checksum += i;
        }
        
        checksum += x + y + z;
    }
    
    /* Pattern 6: Mixed operations with volatile and addressing */
    /* This stresses RELOAD_FOR_OPERAND_ADDRESS, RELOAD_FOR_OPADDR_ADDR */
    {
        volatile int vol_array[100];
        int* restrict ptr = (int*)&big_array1[0][0];
        
        /* Complex addressing with volatile */
        for (int i = 0; i < 50; i++) {
            vol_array[vi1 + i] = ptr[vi2 * i] + vol_array[vi3 - i];
            ptr[vi4 + i] = vol_array[i] * ptr[i + vi1];
        }
        
        /* Compute final checksum from modified arrays */
        for (int i = 0; i < 10; i++) {
            for (int j = 0; j < 10; j++) {
                checksum += big_array1[i][j] + big_array2[i][j];
            }
        }
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
