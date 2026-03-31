/* reload_coverage.c - Stress GCC's reload pass for coverage testing */
#include <stdio.h>
#include <stdint.h>

/* Volatile indices to prevent constant folding */
volatile int vi1 = 7, vi2 = 3, vi3 = 11;
volatile long vl1 = 5, vl2 = 13;

/* 1. Complex Addressing Mode Stress */
void complex_addressing(int n) {
    /* Large multi-dimensional array */
    int arr[100][50];
    int i, j, k;
    
    /* Initialize with volatile indices */
    for (i = 0; i < 100; i++)
        for (j = 0; j < 50; j++)
            arr[i][j] = i * 100 + j;
    
    /* Complex addressing with volatile indices */
    for (k = 0; k < n; k++) {
        /* Multiple non-constant indices with arithmetic */
        arr[vi1 + k][vi2 * 2] = arr[vi3 - k][(vi1 * vi2) % 50];
        arr[k * 2][(vi1 + vi2) % 50] = arr[(vi3 + k) % 100][vi2];
        
        /* Pointer arithmetic that can't be folded */
        int *ptr1 = &arr[vi1][vi2] + k;
        int *ptr2 = &arr[vi3][vi1] - k;
        *ptr1 = *ptr2 + arr[k][(k * vi2) % 50];
    }
}

/* 2. Structure passing for address reloads */
struct SmallStruct {
    int a, b, c, d;
};

struct SmallStruct process_struct(struct SmallStruct s1, struct SmallStruct s2) {
    /* Operations that might need address reloads */
    struct SmallStruct result;
    result.a = s1.a + s2.b;
    result.b = s1.b - s2.a;
    result.c = s1.c * s2.d;
    result.d = s1.d / (s2.c ? s2.c : 1);
    return result;
}

struct SmallStruct chain_struct_calls(struct SmallStruct s) {
    struct SmallStruct temp1, temp2, temp3;
    
    /* Chain of structure operations */
    temp1 = process_struct(s, (struct SmallStruct){vi1, vi2, vi3, 1});
    temp2 = process_struct(temp1, (struct SmallStruct){vl1, vl2, vi1, vi2});
    temp3 = process_struct(temp2, (struct SmallStruct){vi2, vi3, vl1, vl2});
    
    return temp3;
}

/* 3. Inline Assembly with multiple operand types */
void inline_asm_stress(void) {
    int a = vi1, b = vi2, c = vi3;
    int d, e, f;
    long la = vl1, lb = vl2;
    
    /* Chain of asm blocks creating dependencies */
    asm volatile (
        "mov %[input], %[output]\n\t"
        "add %[input2], %[output]"
        : [output] "=r" (d)
        : [input] "r" (a), [input2] "r" (b)
        : "cc"
    );
    
    asm volatile (
        "imul %[in1], %[in2]\n\t"
        "mov %%eax, %[out]"
        : [out] "=m" (*(int*)&e)
        : [in1] "r" (d), [in2] "r" (c)
        : "eax", "cc"
    );
    
    asm volatile (
        "lea (%[in1], %[in2], 4), %[out]"
        : [out] "=r" (f)
        : [in1] "r" (la), [in2] "r" (lb)
    );
    
    /* Use results to prevent optimization */
    vi1 = d + e + f;
}

/* 4. Vector extensions for complex reload patterns */
typedef int v4si __attribute__((vector_size(16)));
typedef long v2di __attribute__((vector_size(16)));

void vector_operations(void) {
    v4si vec1 = {vi1, vi2, vi3, 1};
    v4si vec2 = {vl1, vl2, vi1, vi2};
    v4si vec3, vec4;
    
    /* Vector operations that may need decomposition */
    vec3 = vec1 + vec2;
    vec4 = vec1 * vec2;
    
    /* Shuffle operation */
    vec3 = __builtin_shuffle(vec3, vec4, (v4si){2, 3, 0, 1});
    
    /* Store to volatile memory location */
    volatile v4si vresult = vec3;
    (void)vresult;
}

/* 5. Control flow with split live ranges */
int control_flow_stress(int iterations) {
    int i, sum = 0;
    int arr[100];
    
    /* Initialize array */
    for (i = 0; i < 100; i++) {
        arr[i] = i * vi1;
    }
    
    /* Complex control flow with goto */
    i = 0;
    volatile int *volatile_ptr = &vi2;
    
loop_start:
    if (i >= iterations) goto loop_end;
    
    /* Value defined here, used in distant block */
    int temp = arr[i] + *volatile_ptr;
    
    if (i % 3 == 0) {
        goto skip1;
    } else if (i % 3 == 1) {
        goto skip2;
    }
    
skip1:
    /* Use temp here */
    sum += temp * 2;
    goto continue_loop;
    
skip2:
    /* Different use of temp */
    sum += temp / 2;
    goto continue_loop;
    
continue_loop:
    i++;
    goto loop_start;
    
loop_end:
    return sum;
}

/* 6. Mixed operations in main */
int main(void) {
    int checksum = 0;
    
    /* Initialize volatile values */
    vi1 = 7; vi2 = 3; vi3 = 11;
    vl1 = 5; vl2 = 13;
    
    printf("Starting reload coverage test...\n");
    
    /* 1. Complex addressing */
    complex_addressing(10);
    checksum += vi1 + vi2;
    
    /* 2. Structure passing chain */
    struct SmallStruct ss = {1, 2, 3, 4};
    struct SmallStruct result = chain_struct_calls(ss);
    checksum += result.a + result.b + result.c + result.d;
    
    /* 3. Inline assembly stress */
    for (int i = 0; i < 5; i++) {
        inline_asm_stress();
        checksum += vi1;
    }
    
    /* 4. Vector operations */
    vector_operations();
    checksum += vi2;
    
    /* 5. Control flow with split ranges */
    checksum += control_flow_stress(20);
    
    /* Final array computation */
    {
        int final_arr[50];
        for (int i = 0; i < 50; i++) {
            final_arr[i] = i * (vi1 + vi2 + vi3);
            if (i % 2 == 0) {
                final_arr[i] += vl1;
            } else {
                final_arr[i] += vl2;
            }
            checksum += final_arr[i];
        }
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
