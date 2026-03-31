/* Compile with: gcc -O3 -fno-inline -fomit-frame-pointer -march=x86-64 -mno-sse -o test_reload test_reload.c */

#include <stdio.h>
#include <stdint.h>

/* Pattern 1: Complex addressing modes with volatile indices */
volatile int volatile_idx1 = 3;
volatile int volatile_idx2 = 7;
volatile int volatile_idx3 = 11;

/* Pattern 2: Large structures and arrays */
struct LargeStruct {
    int data[8];
    long long more_data[4];
    char padding[32];
};

/* Pattern 3: Vector extensions */
typedef int v4si __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));

/* Pattern 4: Structure passing functions */
struct SmallStruct {
    int a, b, c, d;
};

/* Function to force structure passing reloads */
struct SmallStruct __attribute__((noinline)) 
process_struct(struct SmallStruct s1, struct SmallStruct s2) {
    struct SmallStruct result;
    result.a = s1.a + s2.b;
    result.b = s1.b - s2.a;
    result.c = s1.c * s2.d;
    result.d = s1.d / (s2.c ? s2.c : 1);
    return result;
}

/* Another level to create chains */
struct SmallStruct __attribute__((noinline))
chain_struct(struct SmallStruct s) {
    struct SmallStruct temp = {s.b, s.c, s.d, s.a};
    return process_struct(s, temp);
}

/* Pattern 5: Complex control flow with goto */
void complex_control_flow(int *arr, int size, volatile int *trigger) {
    int i = 0;
    
    if (*trigger > 100) {
        goto middle;
    }
    
start:
    for (; i < size; i++) {
        if (i % 2 == 0) {
            arr[i] = i * 2;
        } else {
            arr[i] = i * 3;
        }
        
        if (i == size/2) {
            goto middle;
        }
    }
    goto end;
    
middle:
    /* Force spill by using many temporaries */
    int t1 = arr[0], t2 = arr[1], t3 = arr[2], t4 = arr[3];
    int t5 = arr[4], t6 = arr[5], t7 = arr[6], t8 = arr[7];
    int t9 = arr[8], t10 = arr[9], t11 = arr[10], t12 = arr[11];
    
    /* Use all temporaries in computation */
    arr[0] = t1 + t2 - t3 + t4 - t5 + t6 - t7 + t8 - t9 + t10 - t11 + t12;
    
    if (*trigger < 50) {
        i = size/2 + 1;
        goto start;
    }
    
end:
    return;
}

int main() {
    int checksum = 0;
    
    /* Pattern 1: Multi-dimensional array with volatile indices */
    int arr1[20][20];
    int arr2[20][20];
    
    /* Initialize arrays */
    for (int i = 0; i < 20; i++) {
        for (int j = 0; j < 20; j++) {
            arr1[i][j] = i * 100 + j;
            arr2[i][j] = 0;
        }
    }
    
    /* Complex addressing with volatile indices - triggers address reloads */
    for (int i = 0; i < 10; i++) {
        /* RELOAD_FOR_INPUT_ADDRESS, RELOAD_FOR_OUTPUT_ADDRESS */
        arr2[volatile_idx1 + i][i + 1] = arr1[i][volatile_idx2 * 2];
        
        /* More complex addressing */
        arr2[i][volatile_idx3] = arr1[volatile_idx1 + i][volatile_idx2 - i];
    }
    
    /* Pattern 2: Large local structures */
    struct LargeStruct big1, big2;
    for (int i = 0; i < 8; i++) {
        big1.data[i] = i * 10;
        big2.data[i] = i * 20;
    }
    
    /* Force spills with structure operations */
    for (int i = 0; i < 8; i++) {
        big1.data[i] = big2.data[7 - i] + volatile_idx1;
        checksum += big1.data[i];
    }
    
    /* Pattern 3: Vector operations */
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    v4si vec3;
    
    /* Vector operations that might need decomposition */
    for (int i = 0; i < 100; i++) {
        vec3 = vec1 + vec2;
        vec1 = vec3 * vec2;
        vec2 = __builtin_shuffle(vec1, vec2, (v4si){3, 2, 1, 0});
        
        /* Use volatile to prevent optimization */
        if (volatile_idx1 > 0) {
            vec1[0] += i;
        }
    }
    
    /* Extract and use vector elements */
    int vec_sum = vec1[0] + vec1[1] + vec1[2] + vec1[3];
    checksum += vec_sum;
    
    /* Pattern 4: Structure passing chain */
    struct SmallStruct s1 = {100, 200, 300, 400};
    struct SmallStruct s2 = {500, 600, 700, 800};
    
    for (int i = 0; i < 10; i++) {
        s1 = process_struct(s1, s2);
        s2 = chain_struct(s1);
        checksum += s1.a + s1.b + s1.c + s1.d;
    }
    
    /* Pattern 5: Complex control flow */
    int flow_arr[50];
    volatile int trigger = 75;
    complex_control_flow(flow_arr, 50, &trigger);
    
    for (int i = 0; i < 50; i++) {
        checksum += flow_arr[i];
    }
    
    /* Pattern 6: Inline assembly with multiple constraints */
    int asm_var1 = 1000, asm_var2 = 2000, asm_var3 = 3000;
    int asm_result1, asm_result2, asm_result3;
    
    /* Chain of inline asm statements creating dependencies */
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl %2, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=r" (asm_result1)
        : "r" (asm_var1), "r" (asm_var2)
        : "%eax"
    );
    
    asm volatile (
        "imull %1, %0\n\t"
        "addl $100, %0\n\t"
        : "+r" (asm_result1)
        : "r" (asm_var3)
        : "cc"
    );
    
    /* Memory constraint to force address reloads */
    asm volatile (
        "movl (%1), %%eax\n\t"
        "addl %%eax, %0\n\t"
        : "+r" (asm_result2)
        : "m" (arr1[5][5])
        : "%eax"
    );
    
    /* Multiple output constraints */
    asm volatile (
        "leal (%1, %2, 2), %0\n\t"
        "leal (%1, %2, 4), %3\n\t"
        : "=r" (asm_result1), "=r" (asm_result3)
        : "r" (asm_var1), "r" (asm_var2)
        : 
    );
    
    checksum += asm_result1 + asm_result2 + asm_result3;
    
    /* Final checksum computation to prevent optimization */
    printf("Checksum: %d\n", checksum);
    
    /* Additional complex pattern: Pointer arithmetic with volatile */
    int *ptr1 = &arr1[0][0];
    int *ptr2 = &arr2[0][0];
    
    for (int i = 0; i < 100; i++) {
        /* Complex pointer arithmetic */
        *(ptr1 + volatile_idx1 * i + volatile_idx2) = 
            *(ptr2 + volatile_idx3 * (i % 10) + (i / 10));
        
        /* More pointer chasing */
        ptr1 += (volatile_idx1 % 3) + 1;
        ptr2 += (volatile_idx2 % 5) + 1;
    }
    
    /* Final array checksum */
    for (int i = 0; i < 20; i++) {
        for (int j = 0; j < 20; j++) {
            checksum += arr1[i][j] + arr2[i][j];
        }
    }
    
    printf("Final checksum: %d\n", checksum);
    
    return 0;
}
