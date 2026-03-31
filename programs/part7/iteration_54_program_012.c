/* reload_coverage.c - Program to exercise GCC's reload pass */
#include <stdio.h>
#include <stdint.h>

/* Volatile indices to prevent constant folding */
static volatile int vi1 = 1, vi2 = 2, vi3 = 3, vi4 = 4;

/* Structure for passing by value */
struct SmallStruct {
    int a, b, c, d;
};

/* Large local arrays to increase register pressure */
#define ARR_SIZE 100
static int global_arr[ARR_SIZE][ARR_SIZE];

/* Vector type for GCC extensions */
typedef int v4si __attribute__((vector_size(16)));

/* Function passing/returning structures by value */
struct SmallStruct func1(struct SmallStruct s) {
    s.a += vi1;
    s.b += vi2;
    return s;
}

struct SmallStruct func2(struct SmallStruct s) {
    s.c += vi3;
    s.d += vi4;
    return s;
}

/* Function with complex addressing */
void complex_addressing(int n) {
    int local_arr[ARR_SIZE][ARR_SIZE];
    volatile int v_idx = vi1;
    
    /* Initialize */
    for (int i = 0; i < ARR_SIZE; i++) {
        for (int j = 0; j < ARR_SIZE; j++) {
            local_arr[i][j] = i * 100 + j;
            global_arr[i][j] = 0;
        }
    }
    
    /* Complex array access with volatile indices - stresses address reloads */
    for (int i = 0; i < n; i++) {
        /* RELOAD_FOR_INPUT_ADDRESS, RELOAD_FOR_OUTPUT_ADDRESS */
        global_arr[v_idx + i][vi2 * i] = local_arr[vi3 + i][v_idx * 2];
        
        /* More complex addressing */
        global_arr[local_arr[i][v_idx] % ARR_SIZE][(vi4 * i) % ARR_SIZE] =
            local_arr[(v_idx * i) % ARR_SIZE][global_arr[i][0] % ARR_SIZE];
    }
}

/* Function with inline assembly chains */
void asm_reload_chain(int iterations) {
    int a = vi1, b = vi2, c = vi3, d = vi4;
    int tmp1, tmp2, tmp3;
    
    for (int i = 0; i < iterations; i++) {
        /* Chain of asm blocks creating dependencies */
        
        /* First asm: output used as input in next */
        asm volatile (
            "mov %1, %0\n\t"
            "add %2, %0"
            : "=r" (tmp1)
            : "r" (a), "r" (b)
            : "cc"
        );
        
        /* Second asm: uses previous output, produces new output */
        asm volatile (
            "imul %1, %0\n\t"
            "sub %2, %0"
            : "=r" (tmp2)
            : "r" (tmp1), "r" (c)
            : "cc"
        );
        
        /* Third asm: memory operand */
        asm volatile (
            "add %1, %0\n\t"
            "mov %0, %2"
            : "=r" (tmp3)
            : "r" (tmp2), "m" (d)
            : "cc", "memory"
        );
        
        /* Cycle values */
        a = tmp3;
        b = (b + 1) % 10;
        c = (c * 2) % 100;
    }
}

/* Function with vector operations */
void vector_ops(void) {
    v4si v1 = {vi1, vi2, vi3, vi4};
    v4si v2 = {vi4, vi3, vi2, vi1};
    v4si v3, v4;
    
    /* Vector operations that may need decomposition */
    v3 = v1 + v2;
    v4 = v1 * v2;
    
    /* Shuffle operation */
    v4 = __builtin_shuffle(v3, v4, (v4si){3, 2, 1, 0});
    
    /* Use the results to prevent optimization */
    global_arr[0][0] = v3[0] + v4[3];
}

/* Function with goto-based control flow */
void complex_control_flow(int x) {
    int a = vi1, b = vi2, c = vi3;
    int result = 0;
    
    /* Jump to different blocks to split live ranges */
    if (x < 0) goto block1;
    if (x == 0) goto block2;
    
block1:
    a = x * 2;
    b = a + vi1;
    /* Force spill by using many temporaries */
    {
        int t1 = a + b;
        int t2 = t1 * vi2;
        int t3 = t2 - vi3;
        int t4 = t3 / vi4;
        int t5 = t4 ^ a;
        int t6 = t5 | b;
        result = t6;
    }
    goto block3;
    
block2:
    a = x * 3;
    c = a - vi2;
    {
        int t1 = c * a;
        int t2 = t1 + vi4;
        int t3 = t2 ^ vi1;
        result = t3;
    }
    /* Fall through */
    
block3:
    /* Use all variables in complex expression */
    global_arr[1][1] = result + a + b + c;
    
    /* Loop with variable bounds */
    for (int i = 0; i < (x & 0xF); i++) {
        /* Vary register pressure inside loop */
        int temp = a * i + b * (i + 1) + c * (i + 2);
        global_arr[2][i % ARR_SIZE] = temp % 1000;
    }
}

/* Main orchestrator */
int main(void) {
    int checksum = 0;
    
    /* Initialize global array */
    for (int i = 0; i < ARR_SIZE; i++) {
        for (int j = 0; j < ARR_SIZE; j++) {
            global_arr[i][j] = (i * 97 + j * 31) % 1000;
        }
    }
    
    /* 1. Complex addressing patterns */
    complex_addressing(50);
    
    /* 2. Structure passing chain */
    struct SmallStruct s = {vi1, vi2, vi3, vi4};
    for (int i = 0; i < 10; i++) {
        s = func1(s);
        s = func2(s);
    }
    checksum += s.a + s.b + s.c + s.d;
    
    /* 3. Inline assembly chains */
    asm_reload_chain(5);
    
    /* 4. Vector operations */
    vector_ops();
    
    /* 5. Complex control flow */
    complex_control_flow(vi1);
    
    /* Compute final checksum from modified arrays */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            checksum = (checksum * 31 + global_arr[i][j]) % 1000000;
        }
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
