/* Test program to trigger various reload types in GCC reload1.cc */
#include <stdio.h>
#include <stdint.h>

/* Volatile indices to prevent constant folding */
volatile int vi1 = 1, vi2 = 2, vi3 = 3, vi4 = 4;

/* Large structure to force spilling */
struct LargeStruct {
    int data[32];
    long long more[16];
    char padding[128];
};

/* Vector type for vector extensions */
typedef int v4si __attribute__((vector_size(16)));

/* Function to pass/return structures by value (triggers address reloads) */
struct SmallStruct {
    int a, b, c, d;
};

static struct SmallStruct __attribute__((noinline))
process_struct(struct SmallStruct s1, struct SmallStruct s2) {
    struct SmallStruct result;
    result.a = s1.a + s2.b;
    result.b = s1.b + s2.c;
    result.c = s1.c + s2.d;
    result.d = s1.d + s2.a;
    
    /* Force address calculation for structure fields */
    volatile int* ptr = &result.a;
    *ptr += vi1;
    *(ptr + 1) += vi2;
    
    return result;
}

/* Another level of indirection */
static struct SmallStruct __attribute__((noinline))
chain_struct(struct SmallStruct s) {
    struct SmallStruct temp = {vi1, vi2, vi3, vi4};
    return process_struct(s, temp);
}

int main(void) {
    int i, j, k;
    int checksum = 0;
    
    /* 1. Complex array addressing with volatile indices */
    int arr1[32][16];
    int arr2[24][20];
    
    /* Initialize arrays */
    for (i = 0; i < 32; i++) {
        for (j = 0; j < 16; j++) {
            arr1[i][j] = i * 100 + j;
        }
    }
    
    for (i = 0; i < 24; i++) {
        for (j = 0; j < 20; j++) {
            arr2[i][j] = i * 50 + j * 3;
        }
    }
    
    /* Complex addressing patterns - will trigger RELOAD_FOR_INPUT_ADDRESS,
       RELOAD_FOR_OUTPUT_ADDRESS, etc. */
    for (k = 0; k < 10; k++) {
        /* Multi-dimensional with non-constant indices */
        arr1[vi1 + k][vi2 + 1] = arr2[vi3 * 2][vi4 + k];
        
        /* Pointer arithmetic that can't be folded */
        int* ptr1 = &arr1[vi1][vi2];
        int* ptr2 = &arr2[vi3][vi4];
        ptr1[vi1 * 2] = ptr2[vi2 * 3];
        
        /* Nested array accesses with computation */
        arr1[arr2[k][vi1] % 16][arr1[vi2][k] % 8] = 
            arr2[arr1[k][vi3] % 12][arr2[vi4][k] % 10];
    }
    
    /* 2. Inline assembly with multiple operands and constraints */
    int asm_var1 = 1000, asm_var2 = 2000, asm_var3 = 3000;
    
    /* Chain of asm blocks creating dependencies */
    for (i = 0; i < 5; i++) {
        /* First asm: output used as input later */
        asm volatile (
            "addl %[in2], %[out1]\n\t"
            : [out1] "=r" (asm_var1)
            : [in2] "r" (asm_var2), "0" (asm_var1)
            : "cc"
        );
        
        /* Second asm: memory operand */
        asm volatile (
            "movl %[in], (%[mem])\n\t"
            :
            : [in] "r" (asm_var1), [mem] "r" (&asm_var3)
            : "memory"
        );
        
        /* Third asm: multiple outputs */
        int asm_out1, asm_out2;
        asm volatile (
            "imull %[in1], %[out1]\n\t"
            "addl %[in2], %[out2]\n\t"
            : [out1] "=r" (asm_out1), [out2] "=r" (asm_out2)
            : [in1] "r" (asm_var1), [in2] "r" (asm_var2)
            : "cc"
        );
        
        asm_var2 = asm_out1 + asm_out2;
    }
    
    checksum += asm_var1 + asm_var2 + asm_var3;
    
    /* 3. Large local structures and volatile variables */
    struct LargeStruct big1, big2;
    volatile int vol_idx = 5;
    
    /* Fill structures */
    for (i = 0; i < 32; i++) {
        big1.data[i] = i * 10 + vol_idx;
        big2.data[i] = i * 20 - vol_idx;
    }
    
    /* Operations forcing partial spilling */
    for (i = 0; i < 16; i++) {
        /* Mix of array and pointer access */
        big1.more[i] = big2.more[vol_idx + i % 8];
        big1.data[i * 2] = big2.data[vol_idx * 3 + i];
        
        /* Address calculations that need reloads */
        int* volatile ptr = &big1.data[vol_idx];
        ptr[i] = big2.data[i] + ptr[vol_idx];
    }
    
    /* 4. Structure passing chain */
    struct SmallStruct s1 = {100, 200, 300, 400};
    struct SmallStruct s2 = {500, 600, 700, 800};
    
    for (i = 0; i < 8; i++) {
        s1 = chain_struct(s1);
        s2 = process_struct(s2, s1);
        
        /* Use goto to split live ranges (triggers RELOAD_FOR_OTHER_ADDRESS) */
        if (i & 1) {
            goto process_part;
        } else {
            goto process_other;
        }
        
    process_part:
        checksum += s1.a + s1.c;
        continue;
        
    process_other:
        checksum += s2.b + s2.d;
        continue;
    }
    
    /* 5. Vector extensions with builtins */
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    v4si vec3 = {9, 10, 11, 12};
    
    /* Complex vector operations */
    for (i = 0; i < 10; i++) {
        vec1 = vec1 + vec2;
        vec2 = vec2 * vec3;
        
        /* Shuffle creates non-contiguous access pattern */
        vec3 = __builtin_shuffle(vec1, vec2, 
            (v4si){0, 2, 1, 3});
        
        /* Mix with scalar operations to increase pressure */
        int* vptr = (int*)&vec1;
        vptr[vi1 % 4] = vptr[vi2 % 4] + vptr[vi3 % 4];
    }
    
    /* Extract results from vectors */
    int* v1 = (int*)&vec1;
    int* v2 = (int*)&vec2;
    for (i = 0; i < 4; i++) {
        checksum += v1[i] + v2[i];
    }
    
    /* 6. Complex control flow with computed goto (increased pressure) */
    int cf_var1 = 100, cf_var2 = 200, cf_var3 = 300;
    
    void* labels[] = { &&label1, &&label2, &&label3, &&label4 };
    
    for (i = 0; i < 20; i++) {
        /* Use all variables in each block to keep them live */
        switch (i % 4) {
            case 0:
                cf_var1 = cf_var2 + arr1[vi1][i % 16];
                goto *labels[i % 4];
            case 1:
                cf_var2 = cf_var3 * arr2[vi2][i % 20];
                goto *labels[i % 4];
            case 2:
                cf_var3 = cf_var1 - big1.data[i % 32];
                goto *labels[i % 4];
            case 3:
                cf_var1 = cf_var2 + cf_var3;
                goto *labels[i % 4];
        }
        
    label1:
        checksum += cf_var1;
        continue;
    label2:
        checksum += cf_var2;
        continue;
    label3:
        checksum += cf_var3;
        continue;
    label4:
        checksum += cf_var1 + cf_var2;
        continue;
    }
    
    /* Final array checksum */
    for (i = 0; i < 32; i++) {
        for (j = 0; j < 16; j++) {
            checksum += arr1[i][j] % 256;
        }
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
