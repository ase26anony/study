/* reload_stress_test.c
 * Compile with: gcc -O2 -fno-optimize-sibling-calls -mtune=generic -fno-inline reload_stress_test.c -o reload_test
 * Also try: gcc -O3 -fno-inline -fomit-frame-pointer reload_stress_test.c -o reload_test
 * And: gcc -Os -march=x86-64 -mno-sse reload_stress_test.c -o reload_test
 */

#include <stdio.h>
#include <stdint.h>

/* Technique 1: Complex addressing modes with volatile indices */
volatile int idx1 = 3, idx2 = 7, idx3 = 11;

/* Technique 3: Large local arrays and structures */
struct LargeStruct {
    int data[8];
    volatile int volatile_data[4];
    char padding[64];
};

/* Technique 5: Vector extensions */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Technique 4: Structure passing by value */
struct SmallStruct {
    int a, b, c, d;
};

/* Function prototypes for structure passing */
struct SmallStruct process_struct(struct SmallStruct s);
struct SmallStruct chain_struct(struct SmallStruct s, int iterations);

/* Technique 6: Control flow with goto */
static int use_goto_control_flow(int *arr, int size) {
    int sum = 0;
    int i = 0;
    
    loop_start:
    if (i >= size) goto loop_end;
    
    /* Complex addressing */
    arr[(i * idx1 + idx2) % size] += i;
    
    /* Split live ranges with goto */
    if (arr[i] & 1) goto odd_case;
    
    /* Even case */
    sum += arr[i] * 2;
    i++;
    goto loop_start;
    
    odd_case:
    sum += arr[i] * 3;
    i++;
    goto loop_start;
    
    loop_end:
    return sum;
}

/* Structure processing functions */
struct SmallStruct process_struct(struct SmallStruct s) {
    /* Force address calculations */
    s.a = s.a + s.b * idx1;
    s.b = s.b + s.c * idx2;
    s.c = s.c + s.d * idx3;
    s.d = s.d + s.a % 17;
    return s;
}

struct SmallStruct chain_struct(struct SmallStruct s, int iterations) {
    for (int i = 0; i < iterations; i++) {
        s = process_struct(s);
        
        /* More complex addressing */
        volatile int *ptr = &s.a;
        ptr[(i + idx1) % 4] += i;  /* This requires address reloads */
    }
    return s;
}

int main(void) {
    int checksum = 0;
    
    /* Technique 1: Multi-dimensional array with complex addressing */
    int arr[16][8];
    
    /* Initialize array */
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 8; j++) {
            arr[i][j] = i * 8 + j;
        }
    }
    
    /* Complex array access pattern */
    for (int i = 0; i < 8; i++) {
        /* Force RELOAD_FOR_INPUT_ADDRESS and RELOAD_FOR_OUTPUT_ADDRESS */
        arr[idx1 + i][(i * idx2 + 1) % 8] = 
            arr[(i + idx3) % 16][(idx1 * i * 2) % 8] + 
            arr[i][(idx2 + i) % 8];
    }
    
    /* Technique 3: Large local structure */
    struct LargeStruct big;
    for (int i = 0; i < 8; i++) {
        big.data[i] = i * 100;
    }
    for (int i = 0; i < 4; i++) {
        big.volatile_data[i] = i * 50;
    }
    
    /* Access structure with complex addressing */
    for (int i = 0; i < 4; i++) {
        checksum += big.data[(i + idx1) % 8];
        checksum += big.volatile_data[(i * idx2) % 4];
    }
    
    /* Technique 5: Vector operations */
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    v4si vec3;
    
    /* Vector operations that may need decomposition */
    vec3 = vec1 + vec2 * 3;
    
    /* Use __builtin_shuffle for complex access */
    v4si shuffled = __builtin_shuffle(vec3, vec1, (v4si){3, 2, 1, 0});
    
    /* Extract elements with complex addressing */
    int vec_array[4];
    for (int i = 0; i < 4; i++) {
        vec_array[i] = shuffled[i] + i * idx1;
    }
    
    /* Technique 2: Inline assembly with multiple operands */
    int asm_var1 = 100, asm_var2 = 200, asm_var3 = 300;
    
    /* Chain of inline asm statements creating dependencies */
    asm volatile (
        "mov %1, %0\n\t"
        "add $5, %0"
        : "=r"(asm_var1)
        : "r"(asm_var2)
        : "cc"
    );
    
    asm volatile (
        "imul %2, %1\n\t"
        "add %1, %0"
        : "+r"(asm_var1), "=&r"(asm_var2)
        : "r"(asm_var3), "m"(idx1)
        : "cc"
    );
    
    /* More complex asm with memory constraints */
    int mem_temp;
    asm volatile (
        "lea (%1, %2, 2), %0\n\t"
        "mov %0, %3"
        : "=&r"(asm_var3), "+m"(mem_temp)
        : "r"(asm_var1), "r"(asm_var2)
        : "cc"
    );
    
    checksum += asm_var1 + asm_var2 + asm_var3;
    
    /* Technique 4: Nested structure calls */
    struct SmallStruct s = {10, 20, 30, 40};
    s = chain_struct(s, 5);
    checksum += s.a + s.b + s.c + s.d;
    
    /* Technique 6: Control flow with goto */
    int linear_arr[32];
    for (int i = 0; i < 32; i++) {
        linear_arr[i] = i * 3;
    }
    checksum += use_goto_control_flow(linear_arr, 32);
    
    /* Final complex array computation */
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 8; j++) {
            /* Very complex addressing expression */
            int index_i = (i * idx1 + j * idx2 + idx3) % 16;
            int index_j = (j * idx3 + i * idx1) % 8;
            arr[index_i][index_j] += arr[i][j] + checksum;
            checksum += arr[i][j] % 256;
        }
    }
    
    /* Use all volatile indices to prevent optimization */
    checksum += idx1 + idx2 + idx3;
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
