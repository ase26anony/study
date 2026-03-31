/* Compile with: gcc -O3 -fno-inline -fomit-frame-pointer -march=x86-64 -mno-sse -S -o test.s test.c */

#include <stdio.h>
#include <stdint.h>

/* Pattern 1: Complex addressing modes with volatile indices */
volatile int v_idx1 = 3, v_idx2 = 7, v_idx3 = 11;

/* Pattern 2: Vector extensions */
typedef int v4si __attribute__((vector_size(16)));
typedef short v8hi __attribute__((vector_size(16)));

/* Pattern 3: Structure for value passing */
struct SmallStruct {
    int a, b, c, d;
};

/* Pattern 4: Large local arrays */
#define ARR_SIZE 512

/* Helper function for structure passing */
struct SmallStruct process_struct(struct SmallStruct s1, struct SmallStruct s2) {
    struct SmallStruct result;
    result.a = s1.a + s2.b;
    result.b = s1.b - s2.c;
    result.c = s1.c * s2.d;
    result.d = s1.d / (s2.a ? s2.a : 1);
    return result;
}

/* Another function to create call chain */
struct SmallStruct chain_struct(struct SmallStruct s, int iterations) {
    for (int i = 0; i < iterations; i++) {
        s = process_struct(s, s);
    }
    return s;
}

int main(void) {
    int checksum = 0;
    
    /* Pattern 1: Complex array addressing with volatile indices */
    int arr1[ARR_SIZE][ARR_SIZE];
    int arr2[ARR_SIZE][ARR_SIZE];
    
    /* Initialize arrays */
    for (int i = 0; i < ARR_SIZE; i++) {
        for (int j = 0; j < ARR_SIZE; j++) {
            arr1[i][j] = i * 1000 + j;
            arr2[i][j] = i * 2000 + j;
        }
    }
    
    /* Complex addressing that requires multiple reload types */
    for (int i = 0; i < 100; i++) {
        /* RELOAD_FOR_INPUT_ADDRESS, RELOAD_FOR_OUTPUT_ADDRESS */
        arr1[v_idx1 + i][v_idx2 * 2] = arr2[i + v_idx3][v_idx1 * v_idx2];
        
        /* More complex addressing with pointer arithmetic */
        int *ptr1 = &arr1[i][0];
        int *ptr2 = &arr2[0][i];
        
        /* RELOAD_FOR_OPERAND_ADDRESS, RELOAD_FOR_OPADDR_ADDR */
        *(ptr1 + v_idx1) = *(ptr2 + v_idx2) + *(ptr1 + v_idx3);
    }
    
    /* Pattern 2: Vector operations */
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    v8hi vec3 = {1, 2, 3, 4, 5, 6, 7, 8};
    
    for (int i = 0; i < 100; i++) {
        /* Vector operations that may need decomposition */
        vec1 = vec1 + vec2;
        vec2 = vec2 * vec1;
        
        /* Using __builtin_shuffle for complex patterns */
        v4si shuffled = __builtin_shuffle(vec1, vec2, (v4si){3, 2, 1, 0});
        vec1 = shuffled;
        
        /* Mix vector and scalar operations */
        checksum += vec1[0] + vec2[i % 4];
    }
    
    /* Pattern 3: Structure passing */
    struct SmallStruct s1 = {100, 200, 300, 400};
    struct SmallStruct s2 = {500, 600, 700, 800};
    
    /* Chain of structure operations - may trigger address reloads for temporaries */
    for (int i = 0; i < 50; i++) {
        s1 = process_struct(s1, s2);
        s2 = chain_struct(s2, 2);
        
        /* Use volatile to prevent optimization */
        volatile struct SmallStruct vs = s1;
        checksum += vs.a + vs.b;
    }
    
    /* Pattern 4: Inline assembly with multiple constraints */
    int asm_var1 = 1234, asm_var2 = 5678, asm_var3 = 0;
    
    for (int i = 0; i < 100; i++) {
        /* First asm: output constraint */
        asm volatile (
            "movl %1, %%eax\n\t"
            "addl $1, %%eax\n\t"
            "movl %%eax, %0"
            : "=r" (asm_var1)
            : "r" (asm_var2)
            : "%eax"
        );
        
        /* Second asm: memory constraint with address reloads */
        asm volatile (
            "movl (%1), %%ebx\n\t"
            "imull %2, %%ebx\n\t"
            "movl %%ebx, (%0)"
            : 
            : "r" (&arr1[i % 10][0]), "r" (&asm_var1), "r" (asm_var2)
            : "%ebx", "memory"
        );
        
        /* Third asm: multiple outputs */
        asm volatile (
            "leal (%1, %2), %%ecx\n\t"
            "movl %%ecx, %0\n\t"
            "movl %%ecx, %3"
            : "=r" (asm_var2), "=m" (arr2[0][i % 10])
            : "r" (asm_var1), "r" (i)
            : "%ecx"
        );
        
        checksum += asm_var1 + asm_var2;
    }
    
    /* Pattern 5: Control flow splitting live ranges */
    int flow_var1 = 0, flow_var2 = 0, flow_var3 = 0;
    
    for (int i = 0; i < 1000; i++) {
        /* Complex control flow with goto */
        if (i & 1) {
            flow_var1 = i * 2;
            goto label1;
        } else {
            flow_var2 = i * 3;
            goto label2;
        }
        
    label1:
        /* Use variables defined in other basic blocks */
        flow_var3 = flow_var1 + arr1[flow_var1 % 10][0];
        continue;
        
    label2:
        flow_var3 = flow_var2 - arr2[0][flow_var2 % 10];
        
        /* More complex addressing in loop */
        volatile int *volatile_ptr = &arr1[i % ARR_SIZE][i % ARR_SIZE];
        *volatile_ptr = flow_var3;
    }
    
    /* Final checksum computation using array elements */
    for (int i = 0; i < 100; i++) {
        checksum += arr1[i][i] + arr2[i][i];
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
