/* Compile with: gcc -O3 -fno-inline -fomit-frame-pointer -march=x86-64 -mno-sse -S -o test.s test.c */

#include <stdio.h>
#include <stdint.h>

/* Pattern 1: Complex addressing modes with volatile indices */
void complex_addressing(int arr[][100], volatile int idx1, volatile int idx2) {
    int i, j;
    for (i = 0; i < 10; i++) {
        for (j = 0; j < 10; j++) {
            /* Multiple dimensions with volatile indices */
            arr[idx1 + i][j + 1] = arr[j][idx2 * 2 + i] + 
                                   arr[i + idx2][j + idx1];
        }
    }
}

/* Pattern 2: Structure passing by value */
struct small_struct {
    int a, b, c, d;
};

struct small_struct struct_ops(struct small_struct s1, struct small_struct s2) {
    struct small_struct result;
    result.a = s1.a + s2.b;
    result.b = s1.b - s2.c;
    result.c = s1.c * s2.d;
    result.d = s1.d / (s2.a ? s2.a : 1);
    return result;
}

struct small_struct struct_chain(struct small_struct s) {
    struct small_struct tmp1, tmp2;
    tmp1 = struct_ops(s, (struct small_struct){1, 2, 3, 4});
    tmp2 = struct_ops(tmp1, (struct small_struct){5, 6, 7, 8});
    return struct_ops(tmp2, s);
}

/* Pattern 3: Vector extensions */
typedef int v4si __attribute__((vector_size(16)));
typedef short v8hi __attribute__((vector_size(16)));

void vector_ops(v4si *a, v4si *b, v4si *c) {
    v4si v1 = *a;
    v4si v2 = *b;
    v4si v3;
    
    /* Complex vector operations */
    v3 = v1 + v2;
    v3 = __builtin_shuffle(v3, v1, (v4si){3, 2, 1, 0});
    v3 = v3 * (v1 > v2 ? v1 : v2);
    *c = v3;
}

/* Pattern 4: Large local arrays forcing spills */
void large_local_arrays(volatile int trigger) {
    /* Large stack usage */
    int big1[1000];
    int big2[1000];
    struct { int a[500]; char b[500]; } big_struct;
    
    for (int i = 0; i < 1000; i++) {
        /* Complex addressing with pointer arithmetic */
        int *p1 = &big1[i];
        int *p2 = &big2[999 - i];
        *p2 = *p1 + trigger;
        
        /* Structure member access with addressing */
        big_struct.a[i % 500] = big_struct.b[i % 500] + i;
    }
}

/* Pattern 5: Inline assembly with register constraints */
void asm_chains(volatile int *input, volatile int *output) {
    int tmp1, tmp2, tmp3;
    
    /* Chain of asm blocks creating dependencies */
    asm volatile (
        "movl %1, %0\n\t"
        "addl $1, %0"
        : "=r"(tmp1)
        : "m"(*input)
        : "cc"
    );
    
    asm volatile (
        "imull %1, %0\n\t"
        "subl $5, %0"
        : "+r"(tmp1)
        : "r"(tmp2)
        : "cc"
    );
    
    asm volatile (
        "leal (%1, %2, 2), %0"
        : "=r"(tmp3)
        : "r"(tmp1), "r"(tmp2)
        : "cc"
    );
    
    asm volatile (
        "movl %1, %0"
        : "=m"(*output)
        : "r"(tmp3)
        : "memory"
    );
}

/* Pattern 6: Control flow splitting live ranges */
int control_flow_live_ranges(int n, volatile int *arr) {
    int x = 0, y = 0, z = 0;
    int i = 0;
    
    /* Complex control flow with goto */
    if (n > 100) {
        x = arr[0];
        goto label1;
    } else {
        y = arr[1];
        goto label2;
    }
    
label1:
    for (; i < n; i++) {
        if (i % 2) {
            x += arr[i];
            goto label3;
        } else {
            y -= arr[i];
        }
    }
    goto end;
    
label2:
    while (i < n) {
        z = x * y + arr[i];
        i++;
        if (z > 1000) break;
    }
    
label3:
    x = y + z;
    
end:
    return x + y + z;
}

int main() {
    volatile int idx1 = 5, idx2 = 3;
    int array[100][100];
    int checksum = 0;
    
    /* Initialize array */
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 100; j++) {
            array[i][j] = i * 100 + j;
        }
    }
    
    /* Pattern 1: Complex addressing */
    complex_addressing(array, idx1, idx2);
    
    /* Pattern 2: Structure passing */
    struct small_struct s1 = {10, 20, 30, 40};
    struct small_struct s2 = struct_chain(s1);
    checksum += s2.a + s2.b + s2.c + s2.d;
    
    /* Pattern 3: Vector operations */
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    v4si vec3;
    vector_ops(&vec1, &vec2, &vec3);
    checksum += vec3[0] + vec3[1] + vec3[2] + vec3[3];
    
    /* Pattern 4: Large local arrays */
    large_local_arrays(idx1);
    
    /* Pattern 5: Inline assembly chains */
    volatile int asm_in = 42;
    volatile int asm_out;
    asm_chains(&asm_in, &asm_out);
    checksum += asm_out;
    
    /* Pattern 6: Control flow */
    volatile int flow_arr[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    checksum += control_flow_live_ranges(10, flow_arr);
    
    /* Final checksum from array */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            checksum += array[i][j];
        }
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
