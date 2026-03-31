/* reload_coverage.c - Stress GCC's reload pass for coverage testing */
#include <stdio.h>
#include <stdint.h>

/* Volatile indices to prevent constant folding */
static volatile int vi1 = 7, vi2 = 13, vi3 = 42;

/* 1. Complex addressing mode stress */
void complex_addressing(int n) {
    /* Large multi-dimensional array */
    int arr[100][50];
    int i, j;
    
    /* Initialize with pattern */
    for (i = 0; i < 100; i++)
        for (j = 0; j < 50; j++)
            arr[i][j] = i * 100 + j;
    
    /* Complex addressing with volatile indices */
    /* Forces RELOAD_FOR_INPUT_ADDRESS, RELOAD_FOR_OUTPUT_ADDRESS */
    for (i = 0; i < n; i++) {
        arr[vi1 + i][vi2 % 50] = arr[vi3 % 100][(vi1 * i + vi2) % 50];
        arr[(i * vi2) % 100][(vi1 + i) % 50] = arr[vi3][(vi2 * i) % 50];
    }
    
    /* Pointer arithmetic with non-constant offsets */
    int *ptr1 = &arr[0][0];
    int *ptr2 = &arr[vi1][vi2];
    for (i = 0; i < n * 10; i++) {
        ptr1[vi1 + i] = ptr2[vi2 * i];
        ptr2[i] = ptr1[vi3 - i];
    }
}

/* 2. Structure passing for address reloads */
struct SmallStruct {
    int a, b, c, d;
};

struct SmallStruct process_struct(struct SmallStruct s1, struct SmallStruct s2) {
    /* Mix operations to prevent optimization */
    struct SmallStruct result;
    result.a = s1.a + s2.b + vi1;
    result.b = s1.b - s2.c * vi2;
    result.c = s1.c ^ s2.d;
    result.d = s1.d | s2.a;
    return result;
}

struct SmallStruct chain_struct_calls(struct SmallStruct s) {
    struct SmallStruct temp1 = {vi1, vi2, vi3, 1};
    struct SmallStruct temp2 = {vi2, vi3, vi1, 2};
    
    /* Chain of calls forcing address reloads */
    /* May trigger RELOAD_FOR_INPADDR_ADDRESS, RELOAD_FOR_OUTADDR_ADDRESS */
    struct SmallStruct r1 = process_struct(s, temp1);
    struct SmallStruct r2 = process_struct(r1, temp2);
    struct SmallStruct r3 = process_struct(r2, s);
    
    return process_struct(r3, r1);
}

/* 3. Vector extensions for complex reload patterns */
typedef int v4si __attribute__((vector_size(16)));
typedef short v8hi __attribute__((vector_size(16)));

void vector_operations(int n) {
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {vi1, vi2, vi3, 5};
    v8hi vec3 = {1, 2, 3, 4, 5, 6, 7, 8};
    
    /* Vector operations that may need decomposition */
    for (int i = 0; i < n; i++) {
        vec1 = vec1 + vec2 * i;
        vec2 = __builtin_shuffle(vec1, vec2, (v4si){1, 3, 0, 2});
        
        /* Store to memory with complex addressing */
        int *ptr = (int*)&vec1;
        ptr[vi1 % 4] = ptr[vi2 % 4] + i;
    }
}

/* 4. Inline assembly with multiple constraints */
void inline_asm_stress(int n) {
    int a = vi1, b = vi2, c = vi3;
    int d, e, f;
    
    /* Chain of asm blocks creating dependencies */
    /* May trigger RELOAD_FOR_INPUT, RELOAD_FOR_OUTPUT_ADDRESS, RELOAD_OTHER */
    for (int i = 0; i < n; i++) {
        /* First asm: output used as input in next */
        asm volatile (
            "mov %[in1], %[out1]\n\t"
            "add %[in2], %[out1]\n\t"
            : [out1] "=r" (d)
            : [in1] "r" (a), [in2] "r" (b)
            : "cc"
        );
        
        /* Second asm: memory operand */
        asm volatile (
            "mov %[in1], %%eax\n\t"
            "imul %[in2], %%eax\n\t"
            "mov %%eax, %[out1]\n\t"
            : [out1] "=m" (*(int*)&e)
            : [in1] "r" (d), [in2] "r" (c)
            : "eax", "cc"
        );
        
        /* Third asm: multiple outputs */
        asm volatile (
            "lea (%[in1], %[in2], 2), %[out1]\n\t"
            "mov %[in2], %[out2]\n\t"
            : [out1] "=r" (f), [out2] "=r" (a)
            : [in1] "r" (e), [in2] "r" (b)
            : "cc"
        );
        
        /* Rotate values */
        b = c;
        c = f;
    }
}

/* 5. Control flow with split live ranges */
int control_flow_split(int n) {
    int x = vi1, y = vi2, z = vi3;
    int result = 0;
    
    /* Complex control flow splitting live ranges */
    /* May trigger RELOAD_FOR_OTHER_ADDRESS for spill code */
    for (int i = 0; i < n; i++) {
        if (i % 3 == 0) {
            x = y * z + i;
            goto label1;
        } else if (i % 3 == 1) {
            y = x - z * i;
            goto label2;
        } else {
            z = x ^ y ^ i;
            goto label3;
        }
        
    label1:
        result += x;
        continue;
        
    label2:
        result += y;
        continue;
        
    label3:
        result += z;
        continue;
    }
    
    return result;
}

/* 6. Mixed patterns for RELOAD_FOR_OPERAND_ADDRESS */
void mixed_operand_address(int n) {
    /* Large local structure */
    struct LargeStruct {
        int data[100];
        int more_data[50];
        volatile int volatile_data[20];
    } ls;
    
    /* Initialize */
    for (int i = 0; i < 100; i++) ls.data[i] = i * vi1;
    for (int i = 0; i < 50; i++) ls.more_data[i] = i * vi2;
    for (int i = 0; i < 20; i++) ls.volatile_data[i] = i * vi3;
    
    /* Operations mixing addressable and non-addressable parts */
    int *ptr_array[10];
    for (int i = 0; i < 10; i++) {
        ptr_array[i] = &ls.data[i * 10];
    }
    
    /* Complex addressing through pointer array */
    for (int i = 0; i < n; i++) {
        int idx = (vi1 * i + vi2) % 10;
        ptr_array[idx][vi3 % 10] = 
            ls.volatile_data[i % 20] + 
            ptr_array[(idx + 1) % 10][(vi1 + i) % 10];
    }
}

/* Main orchestrator */
int main() {
    int checksum = 0;
    
    /* Initialize volatile indices */
    vi1 = 7; vi2 = 13; vi3 = 42;
    
    printf("Starting reload stress test...\n");
    
    /* Execute all patterns */
    complex_addressing(100);
    
    struct SmallStruct ss = {1, 2, 3, 4};
    struct SmallStruct result = chain_struct_calls(ss);
    checksum += result.a + result.b + result.c + result.d;
    
    vector_operations(50);
    
    inline_asm_stress(25);
    
    checksum += control_flow_split(100);
    
    mixed_operand_address(50);
    
    /* Final checksum to prevent optimization */
    checksum += vi1 + vi2 + vi3;
    
    printf("Checksum: %d\n", checksum);
    printf("Test completed.\n");
    
    return 0;
}
