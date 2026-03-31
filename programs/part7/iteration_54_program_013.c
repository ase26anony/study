/* reload_coverage.c - Comprehensive test to trigger various reload types in GCC */
#include <stdio.h>
#include <stdint.h>

/* Volatile indices to prevent constant folding */
volatile int vi1 = 7, vi2 = 3, vi3 = 11, vi4 = 5;

/* Structure for passing by value */
struct SmallStruct {
    int a;
    int b;
    int c;
    int d;
};

/* Vector type for SIMD operations */
typedef int v4si __attribute__((vector_size(16)));

/* Large arrays to increase register pressure */
int large_array1[1000];
int large_array2[1000];
int large_array3[1000];

/* Multi-dimensional array for complex addressing */
int md_array[50][50];

/* Function to pass/return structures by value */
struct SmallStruct process_struct(struct SmallStruct s1, struct SmallStruct s2) {
    struct SmallStruct result;
    result.a = s1.a + s2.b;
    result.b = s1.b - s2.a;
    result.c = s1.c * s2.d;
    result.d = s1.d / (s2.c ? s2.c : 1);
    return result;
}

/* Chain of structure-passing functions */
struct SmallStruct chain_struct(struct SmallStruct s, int depth) {
    if (depth <= 0) return s;
    
    struct SmallStruct temp;
    temp.a = s.b + depth;
    temp.b = s.c - depth;
    temp.c = s.d * depth;
    temp.d = s.a / (depth ? depth : 1);
    
    /* Recursive call with different structure */
    return process_struct(temp, chain_struct(s, depth - 1));
}

/* Function with complex control flow */
int complex_control_flow(int *arr, int n) {
    int sum = 0;
    int i = 0;
    
    /* Use goto to split live ranges */
    if (n <= 0) goto end;
    
loop_start:
    /* Complex addressing with volatile indices */
    sum += arr[i + vi1] * arr[vi2 * i];
    
    /* More complex addressing */
    if (i % 2 == 0) {
        sum -= arr[vi3 + i * 2];
    } else {
        sum += arr[vi4 * i / 2];
    }
    
    i++;
    if (i < n) goto loop_start;
    
end:
    return sum;
}

/* Function using vector extensions */
v4si vector_operations(v4si a, v4si b) {
    v4si result;
    
    /* Shuffle operation requiring special handling */
    result = __builtin_shuffle(a, b, (v4si){3, 2, 1, 0});
    
    /* Mixed operations */
    result = result + a * b;
    result = result - b / (a + (v4si){1, 1, 1, 1});
    
    return result;
}

int main(void) {
    int checksum = 0;
    int i, j;
    
    /* Initialize arrays */
    for (i = 0; i < 1000; i++) {
        large_array1[i] = i;
        large_array2[i] = i * 2;
        large_array3[i] = i * 3;
    }
    
    for (i = 0; i < 50; i++) {
        for (j = 0; j < 50; j++) {
            md_array[i][j] = i * 50 + j;
        }
    }
    
    /* Pattern 1: Complex addressing modes */
    /* This stresses RELOAD_FOR_INPUT_ADDRESS, RELOAD_FOR_OUTPUT_ADDRESS */
    for (i = 0; i < 10; i++) {
        /* Multi-dimensional array with volatile indices */
        md_array[vi1 + i][vi2 * i] = md_array[vi3 - i][vi4 + i * 2];
        
        /* Pointer arithmetic that can't be folded */
        int *ptr1 = &large_array1[vi1 * i + vi2];
        int *ptr2 = &large_array2[vi3 + i * vi4];
        *ptr1 = *ptr2 + md_array[i][i];
        
        checksum += md_array[i][i];
    }
    
    /* Pattern 2: Inline assembly with multiple operands */
    /* This stresses RELOAD_FOR_INPUT, RELOAD_FOR_OUTPUT_ADDRESS, RELOAD_OTHER */
    {
        int asm_var1 = 100, asm_var2 = 200, asm_var3 = 300;
        int asm_result1, asm_result2, asm_result3;
        
        /* Chain of asm blocks creating dependencies */
        asm volatile (
            "mov %[in1], %[out1]\n\t"
            "add %[in2], %[out1]\n\t"
            : [out1] "=r" (asm_result1)
            : [in1] "r" (asm_var1), [in2] "r" (asm_var2)
            : "cc"
        );
        
        asm volatile (
            "imul %[in1], %[out1]\n\t"
            "sub %[in2], %[out1]\n\t"
            : [out1] "=r" (asm_result2)
            : [in1] "r" (asm_result1), [in2] "m" (large_array3[vi1])
            : "cc"
        );
        
        asm volatile (
            "lea (%[in1], %[in2], 4), %[out1]\n\t"
            : [out1] "=r" (asm_result3)
            : [in1] "r" (asm_result2), [in2] "r" (vi3)
            : "cc"
        );
        
        checksum += asm_result3;
    }
    
    /* Pattern 3: Structure passing by value */
    /* This stresses RELOAD_FOR_INPADDR_ADDRESS, RELOAD_FOR_OUTADDR_ADDRESS */
    {
        struct SmallStruct s1 = {vi1, vi2, vi3, vi4};
        struct SmallStruct s2 = {vi2, vi3, vi4, vi1};
        
        for (i = 0; i < 5; i++) {
            struct SmallStruct result = chain_struct(s1, 3);
            checksum += result.a + result.b + result.c + result.d;
            
            /* Modify and pass again */
            s1.a += i;
            s2.b -= i;
        }
    }
    
    /* Pattern 4: Vector operations */
    /* This stresses various reload types for vector decomposition */
    {
        v4si vec1 = {vi1, vi2, vi3, vi4};
        v4si vec2 = {vi4, vi3, vi2, vi1};
        
        for (i = 0; i < 8; i++) {
            v4si result = vector_operations(vec1, vec2);
            
            /* Extract elements for checksum */
            int *p = (int*)&result;
            checksum += p[0] + p[1] + p[2] + p[3];
            
            /* Modify vectors */
            vec1 += (v4si){1, 2, 3, 4};
            vec2 -= (v4si){4, 3, 2, 1};
        }
    }
    
    /* Pattern 5: Complex control flow with volatile accesses */
    /* This stresses RELOAD_FOR_OTHER_ADDRESS and others */
    checksum += complex_control_flow(large_array1, 20);
    
    /* Pattern 6: Mixed operations with volatile and non-addressable variables */
    /* This stresses RELOAD_FOR_OPERAND_ADDRESS, RELOAD_FOR_OPADDR_ADDR */
    {
        volatile int vol1 = vi1, vol2 = vi2;
        int nonvol1 = vi3, nonvol2 = vi4;
        
        /* Complex expression mixing volatile and non-volatile */
        for (i = 0; i < 10; i++) {
            /* Force multiple reload types */
            int temp = large_array2[vol1 + i] * nonvol1;
            temp += large_array3[vol2 * i] / (nonvol2 ? nonvol2 : 1);
            
            /* Use in addressing calculation */
            large_array1[temp % 100] = md_array[i][vol1] + nonvol2;
            
            checksum += temp;
            
            /* Modify values to prevent optimization */
            vol1 += i % 3;
            nonvol1 -= i % 2;
        }
    }
    
    /* Final checksum output */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
