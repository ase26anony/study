/* test-annotated-loops.c */
#include <stdio.h>
#include <stdlib.h>

/* Dummy function to prevent optimization */
extern void use(int val);
void use(int val) {
    volatile int sink = val;
    (void)sink;
}

/* Function to create potential dependencies */
int maybe_dependent(int i, int n) {
    return (i * 7) % n;
}

int main(void) {
    const int N = 100;
    int a[N], b[N], c[N];
    int sum = 0, prod = 1;
    volatile int prevent_opt = 0;
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        a[i] = i + 1;
        b[i] = 0;
        c[i] = 0;
    }
    
    /* 1. Loop with #pragma GCC ivdep for annot_expr_no_vector_kind */
    /* This pragma tells GCC to ignore vector dependencies */
    #pragma GCC ivdep
    for (int i = 1; i < N; i++) {
        /* Potential dependency that ivdep overrides */
        a[i] = a[maybe_dependent(i, N)] + i;
        use(a[i]); /* Prevent dead code elimination */
    }
    
    /* 2. Loop with #pragma GCC vector for annot_expr_vector_kind */
    /* Hint that this loop should be vectorized */
    #pragma GCC vector
    for (int i = 0; i < N; i++) {
        b[i] = a[i] * 2 + 3;
        sum += b[i]; /* Reduction to prevent elimination */
    }
    
    /* 3. Loop with #pragma GCC parallel for annot_expr_parallel_kind */
    /* Hint that loop iterations are independent and can be parallelized */
    #pragma GCC parallel
    for (int i = 0; i < N; i++) {
        /* Independent computation - each iteration writes to different location */
        c[i] = a[i] * b[i];
        prevent_opt += c[i]; /* Use volatile to prevent optimization */
    }
    
    /* 4. Nested loops with pragma on inner loop */
    {
        int temp_sum = 0;
        for (int outer = 0; outer < 10; outer++) {
            /* #pragma GCC unroll for annot_expr_maybe_infinite_kind */
            /* No count specified - implies maybe infinite */
            #pragma GCC unroll
            for (int inner = 0; inner < 5; inner++) {
                temp_sum += a[outer * 10 + inner] + inner;
            }
        }
        sum += temp_sum;
    }
    
    /* 5. Loop with pragma inside conditional */
    if (sum > 0) {
        int local_arr[20];
        for (int i = 0; i < 20; i++) local_arr[i] = i;
        
        #pragma GCC ivdep
        for (int i = 1; i < 20; i++) {
            local_arr[i] = local_arr[i-1] + local_arr[i];
            use(local_arr[i]);
        }
    }
    
    /* 6. Another vector loop with different structure */
    {
        float fa[N], fb[N];
        for (int i = 0; i < N; i++) fa[i] = i * 1.5f;
        
        #pragma GCC vector
        for (int i = 0; i < N; i++) {
            fb[i] = fa[i] * 2.0f;
            prevent_opt += (int)fb[i];
        }
    }
    
    /* 7. While loop with pragma */
    {
        int count = 50;
        int acc = 0;
        
        #pragma GCC unroll
        while (count-- > 0) {
            acc += count;
            use(acc);
        }
        sum += acc;
    }
    
    /* Compute final result using all loops' outputs */
    int final_result = sum + prevent_opt;
    for (int i = 0; i < N; i++) {
        final_result += c[i] % 256;
    }
    
    printf("Result: %d\n", final_result);
    
    /* Additional test: loop with multiple pragmas in sequence */
    {
        int arr[10] = {0};
        
        /* This should generate separate ANNOTATE_EXPR nodes */
        #pragma GCC ivdep
        #pragma GCC vector
        for (int i = 0; i < 10; i++) {
            arr[i] = i * i;
        }
        
        int check = 0;
        for (int i = 0; i < 10; i++) check += arr[i];
        printf("Check: %d\n", check);
    }
    
    return final_result > 0 ? 0 : 1;
}
