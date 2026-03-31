/* Test program to exercise GCC reload pass switch cases */
#include <stdio.h>
#include <stdint.h>

/* Volatile indices to prevent constant folding */
volatile int vi1 = 3, vi2 = 7, vi3 = 11;

/* Large structure to force spills */
struct LargeStruct {
    int data[32];
    long long more[16];
    char padding[128];
};

/* Vector type for SIMD operations */
typedef int v4si __attribute__((vector_size(16)));

/* Function passing/returning structure by value */
struct SmallStruct {
    int a, b, c;
};

/* Pattern 1: Complex addressing modes with multi-dimensional arrays */
void complex_addressing(int n) {
    int arr[20][20];
    int i, j;
    
    /* Initialize */
    for (i = 0; i < 20; i++)
        for (j = 0; j < 20; j++)
            arr[i][j] = i * 100 + j;
    
    /* Complex array access with volatile indices - triggers address reloads */
    for (i = 0; i < n; i++) {
        /* RELOAD_FOR_INPUT_ADDRESS, RELOAD_FOR_OUTPUT_ADDRESS */
        arr[vi1 + i][vi2 * 2] = arr[vi3][i + vi1] + arr[i][vi2];
        
        /* More complex addressing */
        arr[(i * vi1) % 20][(i + vi2) % 20] = 
            arr[(vi3 - i) % 20][(vi1 * i) % 20] * 2;
    }
}

/* Pattern 2: Inline assembly with multiple constraints */
void inline_asm_chain(int *p) {
    int a, b, c, d;
    volatile int v = 42;
    
    /* Chain of asm statements creating dependencies */
    asm volatile ("mov %1, %0\n\t"
                  "add $1, %0"
                  : "=r"(a) : "r"(v) : "cc");
    
    /* RELOAD_FOR_INPUT, RELOAD_FOR_OUTPUT_ADDRESS */
    asm volatile ("imul %2, %1\n\t"
                  "add %1, %0"
                  : "+r"(b), "=r"(c) : "r"(a), "m"(*p) : "cc");
    
    /* More complex with memory and register constraints */
    asm volatile ("lea (%1,%2,4), %0"
                  : "=r"(d) : "r"(b), "r"(c) :);
    
    *p = d;
}

/* Pattern 3: Structure passing by value */
struct SmallStruct process_struct(struct SmallStruct s1, 
                                  struct SmallStruct s2) {
    /* Mix with volatile to prevent optimizations */
    struct SmallStruct result;
    result.a = s1.a + s2.a + vi1;
    result.b = s1.b * s2.b - vi2;
    result.c = s1.c ^ s2.c ^ vi3;
    
    /* Inline asm to force specific register allocation */
    asm volatile ("# struct manipulation" : : "r"(result.a), "r"(result.b));
    
    return result;
}

struct SmallStruct create_struct(int base) {
    struct SmallStruct s;
    s.a = base + vi1;
    s.b = base * vi2;
    s.c = base ^ vi3;
    return s;
}

/* Pattern 4: Vector operations */
void vector_operations(v4si *v1, v4si *v2) {
    v4si a = *v1, b = *v2;
    
    /* Complex vector operations */
    v4si c = a + b;
    v4si d = a * b;
    
    /* Shuffle operation - may need special handling */
    v4si e = __builtin_shuffle(a, b, (v4si){0, 4, 1, 5});
    
    /* Mixed operations */
    *v1 = c + d;
    *v2 = e * a;
    
    /* Volatile memory access in between */
    volatile int barrier = vi1;
    (void)barrier;
}

/* Pattern 5: Control flow with split live ranges */
int control_flow_split(int x) {
    int a, b, c;
    
    /* Force register pressure */
    a = x * vi1;
    
    if (x > 100) {
        b = a + vi2;
        goto label1;
    } else {
        b = a - vi2;
        goto label2;
    }
    
label1:
    {
        /* Complex computation in separate basic block */
        c = b * vi3;
        /* Use inline asm to prevent optimization */
        asm volatile ("# label1 computation" : "+r"(c));
        return c;
    }
    
label2:
    {
        /* Different computation path */
        c = b / vi3;
        asm volatile ("# label2 computation" : "+r"(c));
        return c + 1;
    }
}

/* Pattern 6: Mixed addressing modes with pointers */
void pointer_arithmetic(struct LargeStruct *ls) {
    int *ptr1, *ptr2;
    volatile int offset = vi1;
    
    /* Complex pointer arithmetic */
    ptr1 = &ls->data[offset];
    ptr2 = &ls->data[vi2];
    
    /* Chain of operations */
    for (int i = 0; i < 8; i++) {
        /* Multiple addressing modes */
        ptr1[i * 2] = ptr2[i + offset] + ptr1[(i + vi3) % 32];
        
        /* More pointer arithmetic */
        *(ptr1 + i + vi1) = *(ptr2 - i + vi2) * 3;
    }
}

/* Main orchestrator */
int main() {
    int checksum = 0;
    int i;
    
    /* Initialize volatile indices */
    vi1 = 3; vi2 = 7; vi3 = 11;
    
    /* Pattern 1: Complex addressing */
    complex_addressing(10);
    
    /* Pattern 2: Inline assembly chain */
    int asm_var = 100;
    for (i = 0; i < 5; i++) {
        inline_asm_chain(&asm_var);
        checksum += asm_var;
    }
    
    /* Pattern 3: Structure passing */
    struct SmallStruct s1 = create_struct(10);
    struct SmallStruct s2 = create_struct(20);
    struct SmallStruct s3 = process_struct(s1, s2);
    checksum += s3.a + s3.b + s3.c;
    
    /* Pattern 4: Vector operations */
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    vector_operations(&vec1, &vec2);
    for (i = 0; i < 4; i++) checksum += vec1[i] + vec2[i];
    
    /* Pattern 5: Control flow */
    checksum += control_flow_split(50);
    checksum += control_flow_split(150);
    
    /* Pattern 6: Pointer arithmetic with large struct */
    struct LargeStruct ls;
    for (i = 0; i < 32; i++) ls.data[i] = i * 2;
    pointer_arithmetic(&ls);
    for (i = 0; i < 32; i++) checksum += ls.data[i];
    
    /* Final checksum output */
    printf("Checksum: %d\n", checksum);
    
    /* Additional: Nested loops with mixed operations */
    {
        int arr[100];
        volatile int idx = 0;
        
        for (i = 0; i < 100; i++) arr[i] = i;
        
        /* Complex loop with multiple array accesses */
        for (int j = 0; j < 10; j++) {
            idx = vi1 + j;
            arr[idx * 3] = arr[idx + vi2] + arr[vi3 - j];
            
            /* Force spill code with many live variables */
            int t1 = arr[j * 2];
            int t2 = arr[j * 3];
            int t3 = arr[j * 4];
            int t4 = arr[j * 5];
            int t5 = arr[j * 6];
            
            checksum += t1 + t2 + t3 + t4 + t5;
        }
    }
    
    printf("Final checksum: %d\n", checksum);
    return 0;
}
