/* Test program to trigger various reload types in GCC's reload pass */
#include <stdio.h>
#include <stdint.h>

/* Volatile indices to prevent constant folding */
volatile int vi1 = 1, vi2 = 2, vi3 = 3, vi4 = 4;

/* Structure for passing by value */
struct SmallStruct {
    int a;
    int b;
    int c;
    int d;
};

/* Function to pass structure by value */
struct SmallStruct process_struct(struct SmallStruct s) {
    s.a += vi1;
    s.b *= vi2;
    s.c -= vi3;
    s.d /= (vi4 ? vi4 : 1);
    return s;
}

/* Chain of structure passing */
struct SmallStruct struct_chain(struct SmallStruct s1, struct SmallStruct s2) {
    struct SmallStruct temp = process_struct(s1);
    temp.a += s2.a;
    temp.b += s2.b;
    temp = process_struct(temp);
    return temp;
}

/* Vector type for SSE/AVX operations */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Complex array access with pointer arithmetic */
void complex_array_access(int size) {
    /* Large arrays to increase register pressure */
    int arr1[100][100];
    int arr2[100][100];
    int arr3[100][100];
    
    /* Initialize arrays */
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 100; j++) {
            arr1[i][j] = i * 100 + j;
            arr2[i][j] = (i + j) * 2;
            arr3[i][j] = 0;
        }
    }
    
    /* Complex addressing with volatile indices */
    for (int iter = 0; iter < 10; iter++) {
        /* RELOAD_FOR_INPUT_ADDRESS and RELOAD_FOR_OUTPUT_ADDRESS */
        arr3[vi1 + iter][vi2 * 2] = arr1[vi3][iter + vi4] + arr2[iter][vi1 * vi2];
        
        /* More complex addressing with pointer arithmetic */
        int *ptr1 = &arr1[vi1][vi2];
        int *ptr2 = &arr2[vi3][vi4];
        
        /* RELOAD_FOR_OPERAND_ADDRESS */
        for (int k = 0; k < 5; k++) {
            ptr1[k * vi1] = ptr2[k * vi2] + ptr1[(k + 1) * vi3];
        }
    }
}

/* Inline assembly with multiple constraints */
void inline_asm_chain(int *a, int *b, int *c) {
    int tmp1, tmp2, tmp3;
    
    /* First asm: output to register, input from memory */
    asm volatile (
        "movl %1, %0\n\t"
        "addl $1, %0"
        : "=r"(tmp1)          /* RELOAD_FOR_OUTPUT */
        : "m"(*a)             /* RELOAD_FOR_INPUT */
        : "cc"
    );
    
    /* Second asm: chain dependencies */
    asm volatile (
        "imull %1, %0\n\t"
        "addl %2, %0"
        : "+r"(tmp1)          /* RELOAD_FOR_INPUT and RELOAD_FOR_OUTPUT */
        : "r"(tmp1), "m"(*b)  /* Mixed constraints */
        : "cc"
    );
    
    /* Third asm: multiple outputs */
    asm volatile (
        "movl %1, %0\n\t"
        "movl %1, %2"
        : "=r"(tmp2), "=r"(tmp3)
        : "r"(tmp1)
        : /* empty clobber */
    );
    
    /* Store results */
    *c = tmp2 + tmp3;
}

/* Function with goto to split live ranges */
int control_flow_split(int x) {
    int a = x * 2;
    int b = x + vi1;
    
    if (x > 100) {
        goto complex_path;
    }
    
    /* Simple path */
    for (int i = 0; i < 10; i++) {
        a += i * vi2;
    }
    goto merge;
    
complex_path:
    /* Complex path with many variables */
    int c = x * 3;
    int d = x * 4;
    int e = x * 5;
    int f = x * 6;
    
    /* Use all variables to keep them live */
    for (int i = 0; i < 20; i++) {
        a += c * i;
        b += d / (i + 1);
        c += e - i;
        d += f % (i + 2);
    }
    
merge:
    return a + b;
}

/* Vector operations */
void vector_operations(void) {
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    v4si vec3 = {0};
    
    /* Vector operations that might need decomposition */
    vec3 = vec1 + vec2;
    vec3 = vec3 * vec1;
    
    /* Use builtin shuffle for complex pattern */
    vec3 = __builtin_shuffle(vec1, vec2, 
        (v4si){3, 2, 1, 0});  /* Reverse order */
    
    /* Store to memory with complex addressing */
    volatile v4si *vptr = (volatile v4si*)&vec3;
    v4si temp = *vptr;
    
    /* Prevent optimization */
    asm volatile ("" : "+x"(temp));
}

/* Main orchestrator */
int main(void) {
    int checksum = 0;
    
    /* 1. Structure passing */
    struct SmallStruct s1 = {10, 20, 30, 40};
    struct SmallStruct s2 = {50, 60, 70, 80};
    struct SmallStruct result = struct_chain(s1, s2);
    checksum += result.a + result.b + result.c + result.d;
    
    /* 2. Complex array access */
    complex_array_access(100);
    
    /* 3. Inline assembly chains */
    int asm_a = 100, asm_b = 200, asm_c = 0;
    for (int i = 0; i < 10; i++) {
        inline_asm_chain(&asm_a, &asm_b, &asm_c);
        checksum += asm_c;
        asm_a += i;
        asm_b += i * 2;
    }
    
    /* 4. Control flow with split live ranges */
    for (int i = 0; i < 50; i++) {
        checksum += control_flow_split(i);
    }
    
    /* 5. Vector operations */
    vector_operations();
    
    /* 6. Additional stress: mixed operations */
    {
        /* Large local array */
        double big_array[256];
        for (int i = 0; i < 256; i++) {
            big_array[i] = i * 1.5;
        }
        
        /* Complex pointer arithmetic */
        double *ptr = &big_array[0];
        for (int i = 0; i < 100; i++) {
            /* RELOAD_FOR_INPADDR_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
            *(ptr + vi1 + i) = *(ptr + vi2 * i) + *(ptr + vi3 * (i + 1));
            checksum += (int)*(ptr + i);
        }
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
