/* Test program to exercise GCC reload pass switch cases */
#include <stdio.h>
#include <stdint.h>

/* Volatile indices to prevent constant folding */
volatile int vi1 = 3, vi2 = 7, vi3 = 11;
volatile long vl1 = 5, vl2 = 9;

/* Structure for value passing */
struct SmallStruct {
    int a;
    int b;
    int c;
};

struct LargeStruct {
    int data[8];
    struct SmallStruct nested;
};

/* Function to pass/return structures by value */
struct SmallStruct __attribute__((noinline)) process_struct(struct SmallStruct s, int idx) {
    s.a += idx;
    s.b ^= idx;
    s.c *= (idx + 1);
    return s;
}

struct LargeStruct __attribute__((noinline)) process_large(struct LargeStruct ls, int* idx) {
    for (int i = 0; i < 8; i++) {
        ls.data[i] += idx[i % 3];
    }
    ls.nested = process_struct(ls.nested, *idx);
    return ls;
}

/* Vector type for SSE/MMX stress */
typedef int v4si __attribute__((vector_size(16)));
typedef long v2di __attribute__((vector_size(16)));

/* Complex addressing mode stress */
void complex_addressing(int n) {
    /* Large multi-dimensional array */
    int arr1[100][50];
    int arr2[75][60];
    
    /* Initialize with volatile indices */
    for (int i = 0; i < 50; i++) {
        for (int j = 0; j < 30; j++) {
            arr1[i][j] = i * j + vi1;
            arr2[i][j] = i + j * vi2;
        }
    }
    
    /* Complex addressing with pointer arithmetic */
    int* volatile ptr1 = &arr1[0][0];
    int* volatile ptr2 = &arr2[0][0];
    
    /* Stress RELOAD_FOR_INPUT_ADDRESS, RELOAD_FOR_OUTPUT_ADDRESS */
    for (int iter = 0; iter < n; iter++) {
        /* Multi-dimensional access with non-constant indices */
        arr1[vi1 + iter][vl1 % 20] = arr2[vi2 - iter][(vl2 * 2) % 30];
        
        /* Pointer arithmetic that can't be folded */
        *(ptr1 + vi1 * 8 + iter) = *(ptr2 + vi2 * 6 - iter);
        
        /* More complex addressing */
        arr1[iter][arr2[iter % 30][iter % 20] % 40] = 
            arr2[arr1[iter % 40][iter % 25] % 35][iter];
    }
}

/* Inline assembly with multiple constraints */
void asm_reload_stress(void) {
    int a = vi1, b = vi2, c = vi3;
    int d, e, f;
    long la = vl1, lb = vl2;
    
    /* Chain of asm blocks creating dependencies */
    /* RELOAD_FOR_INPUT, RELOAD_FOR_OUTPUT_ADDRESS */
    asm volatile (
        "mov %1, %0\n\t"
        "add %2, %0"
        : "=r"(d)
        : "r"(a), "r"(b)
        : "cc"
    );
    
    /* Memory constraint forcing address reloads */
    asm volatile (
        "imul %1, %0\n\t"
        "addl $1, %0"
        : "+r"(d)
        : "m"(c)
        : "cc"
    );
    
    /* Multiple outputs with different constraints */
    asm volatile (
        "lea (%1, %2, 2), %0\n\t"
        "mov %3, %4"
        : "=r"(e), "=r"(f)
        : "r"(d), "r"(la), "r"(lb)
        : "cc"
    );
    
    /* RELOAD_FOR_OPERAND_ADDRESS with offset */
    asm volatile (
        "mov (%1), %0\n\t"
        "add %2, %0"
        : "=r"(a)
        : "r"(&vi3), "i"(4)
        : "memory"
    );
}

/* Vector operations for register pressure */
void vector_reload_stress(void) {
    v4si vec1 = {vi1, vi2, vi3, vi1 + vi2};
    v4si vec2 = {vl1, vl2, vl1 * 2, vl2 / 2};
    v4si vec3, vec4;
    
    /* Vector operations */
    vec3 = vec1 + vec2;
    vec4 = vec1 * vec2;
    
    /* Shuffle operation - may need special handling */
    vec3 = __builtin_shuffle(vec3, vec4, (v4si){2, 3, 0, 1});
    
    /* Mixed scalar/vector access */
    int* vptr = (int*)&vec3;
    for (int i = 0; i < 4; i++) {
        vptr[i] += vi1 * i;
    }
}

/* Control flow to split live ranges */
void control_flow_reload(int n) {
    int x = vi1, y = vi2, z = vi3;
    int result = 0;
    
    /* Complex control flow with gotos */
    if (n > 0) {
        goto label1;
    } else {
        goto label2;
    }
    
label1:
    {
        int temp[100];
        for (int i = 0; i < 100; i++) {
            temp[i] = i * x;
        }
        
        /* Use in distant block */
        for (int i = 0; i < 50; i++) {
            result += temp[i] * y;
        }
        
        if (n % 2) {
            goto label3;
        }
    }
    
label2:
    {
        int temp2[100];
        for (int i = 0; i < 100; i++) {
            temp2[i] = i * z;
        }
        
        for (int i = 0; i < 50; i++) {
            result += temp2[i] * x;
        }
        
        goto label4;
    }
    
label3:
    {
        /* More operations */
        result += y * z;
        goto label4;
    }
    
label4:
    /* Use all variables in final computation */
    volatile int sink = result + x + y + z;
    (void)sink;
}

/* Main orchestrator */
int main(void) {
    int checksum = 0;
    
    /* Initialize indices */
    vi1 = 3; vi2 = 7; vi3 = 11;
    vl1 = 5; vl2 = 9;
    
    /* 1. Complex addressing mode stress */
    complex_addressing(10);
    
    /* 2. Structure passing chain */
    struct SmallStruct ss = {vi1, vi2, vi3};
    struct LargeStruct ls;
    
    for (int i = 0; i < 8; i++) {
        ls.data[i] = i * vi1;
    }
    ls.nested = ss;
    
    int idx_arr[3] = {vi1, vi2, vi3};
    ls = process_large(ls, idx_arr);
    
    checksum += ls.nested.a + ls.nested.b + ls.nested.c;
    
    /* 3. Inline assembly stress */
    asm_reload_stress();
    
    /* 4. Vector operations */
    vector_reload_stress();
    
    /* 5. Control flow with split live ranges */
    control_flow_reload(vi1);
    
    /* Final array computation for checksum */
    int final_arr[100][50];
    for (int i = 0; i < 50; i++) {
        for (int j = 0; j < 30; j++) {
            final_arr[i][j] = i * j + checksum;
            checksum = (checksum * 31 + final_arr[i][j]) & 0xFFFF;
        }
    }
    
    printf("Checksum: %d\n", checksum);
    return checksum != 0 ? 0 : 1;
}
