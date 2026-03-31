/* reload_coverage.c - Test program to exercise GCC's reload pass */
#include <stdio.h>
#include <stdint.h>

/* Volatile indices to prevent constant folding */
static volatile int vi1 = 1, vi2 = 2, vi3 = 3, vi4 = 4;

/* Structure for testing structure passing reloads */
struct SmallStruct {
    int a;
    int b;
    int c;
    int d;
};

/* Vector type for vector extension reloads */
typedef int v4si __attribute__((vector_size(16)));

/* Function to create structure passing reloads */
struct SmallStruct __attribute__((noinline)) 
process_struct(struct SmallStruct s1, struct SmallStruct s2) {
    struct SmallStruct result;
    result.a = s1.a + s2.b;
    result.b = s1.b + s2.c;
    result.c = s1.c + s2.d;
    result.d = s1.d + s2.a;
    
    /* Complex array access to trigger address reloads */
    volatile int arr[10][10];
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            /* RELOAD_FOR_INPUT_ADDRESS, RELOAD_FOR_OUTPUT_ADDRESS */
            arr[i][j] = arr[j][i] + vi1;
        }
    }
    
    return result;
}

/* Another function to chain structure passing */
struct SmallStruct __attribute__((noinline))
chain_struct(struct SmallStruct s) {
    /* Use inline asm to force register constraints */
    int a_val, b_val;
    
    /* RELOAD_FOR_INPUT, RELOAD_FOR_OUTPUT_ADDRESS */
    asm volatile ("mov %1, %0\n\t"
                  "add $1, %0"
                  : "=r"(a_val)
                  : "r"(s.a)
                  : "cc");
    
    /* RELOAD_FOR_OPERAND_ADDRESS, RELOAD_FOR_OPADDR_ADDR */
    asm volatile ("lea (%1,%2,2), %0"
                  : "=r"(b_val)
                  : "r"(s.b), "r"(vi2)
                  :);
    
    struct SmallStruct result = {a_val, b_val, s.c + vi3, s.d + vi4};
    return result;
}

/* Function with complex control flow */
int __attribute__((noinline))
complex_control_flow(int n) {
    int result = 0;
    int arr[100];
    
    /* Initialize array */
    for (int i = 0; i < 100; i++) {
        arr[i] = i * vi1;
    }
    
    /* Complex control flow with gotos */
    int i = 0;
    int j = 0;
    
loop_start:
    if (i >= n) goto loop_end;
    
    /* Multi-dimensional array access with volatile indices */
    /* RELOAD_FOR_INPUT_ADDRESS, RELOAD_FOR_OUTPUT_ADDRESS */
    int idx1 = (i * vi2) % 10;
    int idx2 = (j * vi3) % 10;
    
    /* Force address calculation reloads */
    int temp[10][10];
    temp[idx1][idx2] = arr[i] + arr[j];
    
    /* RELOAD_FOR_INPADDR_ADDRESS, RELOAD_FOR_OUTADDR_ADDRESS */
    asm volatile ("# Complex addressing"
                  : 
                  : "r"(&temp[idx1][idx2]), "r"(&arr[i]), "r"(&arr[j])
                  : "memory");
    
    result += temp[idx1][idx2];
    
    i++;
    j = (j + 1) % n;
    goto loop_start;
    
loop_end:
    return result;
}

/* Function using vector extensions */
void __attribute__((noinline))
vector_operations(v4si *a, v4si *b, v4si *c) {
    /* Vector operations that may need decomposition */
    v4si v1 = *a;
    v4si v2 = *b;
    
    /* Shuffle operation - may need special handling */
    v4si v3 = __builtin_shuffle(v1, v2, 
        (v4si){0, 2, 1, 3});
    
    /* Complex vector expression */
    *c = v1 + v2 * v3;
    
    /* Inline asm with multiple constraints */
    int temp;
    asm volatile ("pmulld %1, %0\n\t"
                  "paddd %2, %0"
                  : "+x"(v1)
                  : "x"(v2), "x"(v3)
                  :);
    
    /* Store with complex addressing */
    c[vi1] = v1;
}

/* Main function orchestrating all patterns */
int main() {
    int checksum = 0;
    
    /* Test 1: Complex array addressing */
    {
        int arr[20][20];
        
        /* Initialize */
        for (int i = 0; i < 20; i++) {
            for (int j = 0; j < 20; j++) {
                arr[i][j] = i * 20 + j;
            }
        }
        
        /* Complex access pattern - forces address reloads */
        for (int i = 0; i < 10; i++) {
            for (int j = 0; j < 10; j++) {
                /* RELOAD_FOR_INPUT_ADDRESS, RELOAD_FOR_OUTPUT_ADDRESS */
                arr[vi1 + i][vi2 + j] = 
                    arr[vi3 + j][vi4 + i] + 
                    arr[i * 2][j * 3];
                checksum += arr[vi1 + i][vi2 + j];
            }
        }
    }
    
    /* Test 2: Structure passing chain */
    {
        struct SmallStruct s1 = {1, 2, 3, 4};
        struct SmallStruct s2 = {5, 6, 7, 8};
        
        for (int i = 0; i < 10; i++) {
            struct SmallStruct s3 = process_struct(s1, s2);
            struct SmallStruct s4 = chain_struct(s3);
            
            checksum += s4.a + s4.b + s4.c + s4.d;
            
            /* Modify for next iteration */
            s1.a += vi1;
            s2.b += vi2;
        }
    }
    
    /* Test 3: Complex control flow */
    checksum += complex_control_flow(15);
    
    /* Test 4: Vector operations */
    {
        v4si vec1 = {1, 2, 3, 4};
        v4si vec2 = {5, 6, 7, 8};
        v4si vec3;
        v4si vec_array[4];
        
        vector_operations(&vec1, &vec2, &vec3);
        vector_operations(&vec2, &vec3, vec_array);
        
        /* Extract elements for checksum */
        int *p = (int*)&vec3;
        for (int i = 0; i < 4; i++) {
            checksum += p[i];
        }
    }
    
    /* Test 5: Inline assembly with complex constraints */
    {
        int a = 100, b = 200, c = 300, d = 400;
        int result1, result2, result3;
        
        /* Chain of asm blocks creating dependencies */
        /* RELOAD_FOR_INPUT, RELOAD_FOR_OUTPUT_ADDRESS, RELOAD_OTHER */
        asm volatile ("imul %2, %1\n\t"
                      "add %3, %1"
                      : "=r"(result1), "+r"(a)
                      : "r"(b), "r"(vi1)
                      : "cc");
        
        asm volatile ("lea (%1, %2, 4), %0"
                      : "=r"(result2)
                      : "r"(result1), "r"(c)
                      :);
        
        /* RELOAD_FOR_OTHER_ADDRESS */
        asm volatile ("mov %1, (%0)\n\t"
                      "add $1, (%0)"
                      : 
                      : "r"(&result3), "r"(result2)
                      : "memory");
        
        checksum += result1 + result2 + result3;
    }
    
    /* Test 6: Large local arrays causing register pressure */
    {
        /* Large array to increase register pressure */
        int big_array[1000];
        int another_array[500];
        
        /* Initialize with volatile indices */
        for (int i = 0; i < 1000; i++) {
            big_array[i] = i * vi1;
            if (i < 500) {
                another_array[i] = i * vi2;
            }
        }
        
        /* Complex computation mixing arrays */
        for (int i = 0; i < 100; i++) {
            int idx1 = (i * vi3) % 1000;
            int idx2 = (i * vi4) % 500;
            
            /* Multiple array accesses in one expression */
            big_array[idx1] = 
                another_array[idx2] + 
                big_array[(idx1 + 1) % 1000] * 2 -
                big_array[(idx1 + 2) % 1000] / 3;
            
            checksum += big_array[idx1];
        }
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
