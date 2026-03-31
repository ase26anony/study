/* reload_coverage.c - Program to exercise GCC's reload pass */
#include <stdio.h>
#include <stdint.h>

/* Pattern 1: Complex addressing modes with volatile indices */
volatile int idx1 = 3, idx2 = 7, idx3 = 11;

/* Pattern 2: Large structures and arrays */
struct LargeStruct {
    int data[8];
    volatile int volatile_data[4];
    struct {
        int nested[3];
    } inner;
};

/* Pattern 3: Vector extensions */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Pattern 4: Structure passing functions */
struct SmallStruct {
    int a, b, c;
};

/* Function that returns structure by value - forces complex parameter passing */
struct SmallStruct __attribute__((noinline)) 
process_struct(struct SmallStruct s1, struct SmallStruct s2) {
    struct SmallStruct result;
    result.a = s1.a + s2.b;
    result.b = s1.b - s2.c;
    result.c = s1.c * s2.a;
    return result;
}

/* Function with complex control flow */
int __attribute__((noinline))
complex_control_flow(int *arr, int n) {
    int sum = 0;
    volatile int v = 0;
    
    /* goto to split live ranges */
    if (n > 0) goto compute;
    
    /* Dead code to create control flow complexity */
    for (int i = 0; i < 10; i++) {
        sum += i * 2;
    }
    
compute:
    /* Loop with volatile index forces address reloads */
    for (int i = 0; i < n; i++) {
        /* Multi-dimensional array-like access with pointer arithmetic */
        int *ptr = arr + i + v;
        sum += *ptr * (i + idx1);
        
        /* Another level of indirection */
        if (i % 2 == 0) {
            int **pptr = &ptr;
            sum += **pptr;
        }
    }
    
    return sum;
}

/* Main orchestration function */
int main(void) {
    int checksum = 0;
    
    /* Pattern 1: Complex array addressing */
    int arr1[20][15];
    int arr2[10][25];
    
    /* Initialize arrays */
    for (int i = 0; i < 20; i++) {
        for (int j = 0; j < 15; j++) {
            arr1[i][j] = i * 100 + j;
        }
    }
    
    /* Complex addressing with volatile indices - forces RELOAD_FOR_INPUT_ADDRESS,
       RELOAD_FOR_OUTPUT_ADDRESS, etc. */
    for (int i = 0; i < 5; i++) {
        /* arr1[volatile][expr] = arr2[expr][volatile*2] */
        arr1[idx1][i+1] = arr2[i][idx2*2];
        
        /* More complex: arr1[volatile+expr][volatile*expr] */
        arr1[idx1 + i][idx2 * i] = arr1[idx3][idx1];
        
        checksum += arr1[idx1][i+1];
    }
    
    /* Pattern 2: Large structures */
    struct LargeStruct ls1, ls2;
    
    /* Fill structure data */
    for (int i = 0; i < 8; i++) {
        ls1.data[i] = i * 10;
        ls2.data[i] = i * 20;
    }
    
    /* Access with complex addressing - forces RELOAD_FOR_OPERAND_ADDRESS */
    for (int i = 0; i < 4; i++) {
        ls1.volatile_data[i] = ls2.data[i + idx1];
        checksum += ls1.volatile_data[i];
    }
    
    /* Pattern 3: Inline assembly with multiple constraints */
    int asm_var1 = 123, asm_var2 = 456, asm_var3 = 789;
    
    /* Chain of asm blocks creating dependencies - forces RELOAD_FOR_INPUT,
       RELOAD_FOR_OUTPUT_ADDRESS, RELOAD_OTHER */
    asm volatile (
        "mov %[in1], %[out1]\n\t"
        "add $1, %[out1]"
        : [out1] "=r" (asm_var1)
        : [in1] "r" (asm_var2)
        : "cc"
    );
    
    asm volatile (
        "imul %[in2], %[out2]\n\t"
        "add %%eax, %[out2]"
        : [out2] "=r" (asm_var2)
        : [in2] "rm" (asm_var3), "[out2]" (asm_var1)
        : "eax", "cc"
    );
    
    /* Memory constraint asm - forces different reload types */
    int mem_buffer[10] = {1,2,3,4,5,6,7,8,9,10};
    asm volatile (
        "movl $0x1, (%[mem])"
        : 
        : [mem] "r" (&mem_buffer[idx1])
        : "memory"
    );
    
    checksum += asm_var1 + asm_var2 + mem_buffer[idx1];
    
    /* Pattern 4: Structure passing chain */
    struct SmallStruct ss1 = {1, 2, 3};
    struct SmallStruct ss2 = {4, 5, 6};
    struct SmallStruct ss3, ss4;
    
    /* Chain of structure returns - forces RELOAD_FOR_INPADDR_ADDRESS,
       RELOAD_FOR_OUTADDR_ADDRESS */
    ss3 = process_struct(ss1, ss2);
    ss4 = process_struct(ss3, ss1);
    
    checksum += ss4.a + ss4.b + ss4.c;
    
    /* Pattern 5: Vector extensions */
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    v4si vec3;
    
    /* Vector operations that might need decomposition */
    vec3 = vec1 + vec2;
    vec3 = __builtin_shuffle(vec1, vec2, (v4si){0, 2, 1, 3});
    
    /* Extract elements for checksum */
    int vec_elements[4];
    __builtin_memcpy(vec_elements, &vec3, sizeof(vec3));
    for (int i = 0; i < 4; i++) {
        checksum += vec_elements[i];
    }
    
    /* Pattern 6: Complex control flow with split live ranges */
    int dynamic_arr[50];
    for (int i = 0; i < 50; i++) {
        dynamic_arr[i] = i * 3;
    }
    
    checksum += complex_control_flow(dynamic_arr, 25);
    
    /* Final checksum output */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
