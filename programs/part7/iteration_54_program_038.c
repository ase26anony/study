/* reload_coverage.c - Program to exercise GCC's reload pass */
#include <stdio.h>
#include <stdint.h>

/* Pattern 1: Complex addressing modes with volatile indices */
volatile int idx1 = 3, idx2 = 7, idx3 = 11;
int large_array[100][50][20];

/* Pattern 2: Structure passing by value */
struct SmallStruct {
    int a;
    int b;
    int c;
    int d;
};

struct MediumStruct {
    int data[8];
    struct SmallStruct nested;
};

/* Pattern 3: Vector extensions */
typedef int v4si __attribute__((vector_size(16)));
typedef short v8hi __attribute__((vector_size(16)));

/* Pattern 4: Function to force structure passing reloads */
struct SmallStruct process_struct(struct SmallStruct s1, struct SmallStruct s2) {
    struct SmallStruct result;
    result.a = s1.a + s2.b;
    result.b = s1.b - s2.a;
    result.c = s1.c * s2.c;
    result.d = s1.d / (s2.d ? s2.d : 1);
    return result;
}

struct MediumStruct process_medium(struct MediumStruct m, int idx) {
    struct MediumStruct result;
    for (int i = 0; i < 8; i++) {
        result.data[i] = m.data[(i + idx) % 8] + m.nested.a;
    }
    result.nested = process_struct(m.nested, (struct SmallStruct){idx, idx+1, idx+2, idx+3});
    return result;
}

/* Pattern 5: Complex control flow with gotos */
int complex_control_flow(int n) {
    int sum = 0;
    int i = 0;
    
    loop_start:
    if (i >= n) goto loop_end;
    
    /* Force register pressure with many live variables */
    int a = i * 2;
    int b = i * 3;
    int c = i * 5;
    int d = i * 7;
    int e = i * 11;
    int f = i * 13;
    int g = i * 17;
    int h = i * 19;
    
    /* Use all variables in computation */
    sum += a + b + c + d + e + f + g + h;
    
    /* Complex addressing within the loop */
    large_array[i][a % 50][b % 20] = sum;
    
    i++;
    goto loop_start;
    
    loop_end:
    return sum;
}

int main() {
    int checksum = 0;
    
    /* Pattern 1: Complex array addressing with volatile indices */
    for (int i = 0; i < 10; i++) {
        /* RELOAD_FOR_INPUT_ADDRESS, RELOAD_FOR_OUTPUT_ADDRESS */
        large_array[idx1 + i][idx2 % 50][idx3 % 20] = 
            large_array[idx2 - i][(idx1 * 2) % 50][(idx3 / 2) % 20] + i;
        
        /* More complex addressing */
        large_array[(idx1 * i) % 100][(idx2 + i) % 50][(idx3 - i) % 20] += 
            large_array[(idx3 - i) % 100][(idx1 + i) % 50][(idx2 * i) % 20];
    }
    
    /* Pattern 2: Structure passing chain */
    struct MediumStruct ms1 = {{1, 2, 3, 4, 5, 6, 7, 8}, {10, 20, 30, 40}};
    struct MediumStruct ms2 = {{9, 8, 7, 6, 5, 4, 3, 2}, {50, 60, 70, 80}};
    
    /* Chain of structure operations - forces address reloads for temporaries */
    struct MediumStruct ms3 = process_medium(ms1, idx1);
    struct MediumStruct ms4 = process_medium(ms2, idx2);
    struct MediumStruct ms5 = process_medium(ms3, idx3);
    
    /* Use structure elements in checksum */
    for (int i = 0; i < 8; i++) {
        checksum += ms3.data[i] + ms4.data[i] + ms5.data[i];
    }
    checksum += ms3.nested.a + ms4.nested.b + ms5.nested.c;
    
    /* Pattern 3: Vector operations */
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    v4si vec3 = {9, 10, 11, 12};
    
    /* Complex vector operations */
    v4si vec_result = vec1 * vec2 + vec3;
    v4si vec_shuffle = __builtin_shuffle(vec1, vec2, (v4si){3, 2, 1, 0});
    v4si vec_combined = vec_result + vec_shuffle * 2;
    
    /* Extract elements - forces register pressure */
    int vec_array[4];
    for (int i = 0; i < 4; i++) {
        vec_array[i] = vec_combined[i];
        checksum += vec_array[i];
    }
    
    /* Pattern 4: Inline assembly with multiple constraints */
    int asm_var1 = 100, asm_var2 = 200, asm_var3 = 300;
    int asm_result1, asm_result2, asm_result3;
    
    /* Chain of asm blocks creating dependencies */
    asm volatile (
        "mov %[in1], %[out1]\n\t"
        "add $1, %[out1]\n\t"
        : [out1] "=r" (asm_result1)
        : [in1] "r" (asm_var1)
        : "cc"
    );
    
    asm volatile (
        "imul %[in1], %[in2], %[out1]\n\t"
        "add %[in3], %[out1]\n\t"
        : [out1] "=r" (asm_result2)
        : [in1] "r" (asm_result1), [in2] "r" (asm_var2), [in3] "m" (asm_var3)
        : "cc"
    );
    
    asm volatile (
        "lea (%[in1], %[in2], 4), %[out1]\n\t"
        : [out1] "=r" (asm_result3)
        : [in1] "r" (asm_result2), [in2] "r" (idx1)
        : "cc"
    );
    
    checksum += asm_result1 + asm_result2 + asm_result3;
    
    /* Pattern 5: Complex control flow */
    checksum += complex_control_flow(50);
    
    /* Pattern 6: Mixed operations with volatile and non-addressable variables */
    volatile int vol1 = 42, vol2 = 84;
    int nonaddr_array[1000]; /* Large array to increase register pressure */
    
    for (int i = 0; i < 200; i++) {
        /* Mix volatile and non-volatile accesses */
        nonaddr_array[i] = vol1 * i + vol2;
        vol1 = (vol1 + 1) % 100;
        
        /* Complex addressing with mixed types */
        if (i % 3 == 0) {
            large_array[vol1 % 100][i % 50][vol2 % 20] = nonaddr_array[i];
        } else if (i % 3 == 1) {
            nonaddr_array[i] += large_array[vol2 % 100][(i * 2) % 50][vol1 % 20];
        } else {
            /* Pointer arithmetic that can't be easily folded */
            int *ptr = &nonaddr_array[i];
            for (int j = 0; j < 5; j++) {
                *(ptr + j * (vol1 % 10)) = j * i;
            }
        }
        
        checksum += nonaddr_array[i] % 256;
    }
    
    /* Final checksum computation using all patterns */
    printf("Checksum: %d\n", checksum);
    
    /* Verify with expected value for basic correctness check */
    if (checksum == 130841522) { /* This will vary - adjust based on actual run */
        printf("SUCCESS: All patterns executed\n");
    } else {
        printf("Result computed: %d\n", checksum);
    }
    
    return 0;
}
