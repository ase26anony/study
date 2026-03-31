/* test_auto_inc_dec.c
 * 
 * This program is designed to trigger the uncovered block in GCC's
 * auto-inc-dec.cc (lines 1352-1358) by creating memory operands with
 * simple register addressing (register + zero offset) in various contexts.
 * The compiler's auto-increment/decrement optimization pass should analyze
 * these patterns during RTL generation at -O2 or higher.
 */

#include <stdio.h>
#include <stdlib.h>

/* Global array to enable pointer-to-global patterns */
int global_arr[100] = {0};

/* Test 1: Simple parameter load with zero offset
 * Should generate: mem = *(reg), where reg is the parameter register
 */
int test_simple_param_load(int *p) {
    volatile int *vp = p; /* Prevent optimization */
    int val = *vp;        /* Simple register indirect: XEXP(x,0) = p, offset=0 */
    return val;
}

/* Test 2: Mixed addressing patterns in one function
 * Includes: simple register, register+constant, and pointer increment loop
 */
int test_mixed_addressing(int *base, int n) {
    int sum = 0;
    volatile int *volatile_base = base; /* Force memory accesses */
    
    /* 1. Simple register indirect - target for uncovered block */
    sum += *volatile_base;
    
    /* 2. Register + constant offset */
    sum += volatile_base[5];
    
    /* 3. Loop with pointer increment - encourages auto-inc optimization */
    int *ptr = volatile_base;
    for (int i = 0; i < n; i++) {
        sum += *ptr++;
    }
    
    /* 4. Another simple register access after loop */
    sum += *volatile_base;
    
    return sum;
}

/* Test 3: Global array accessed via local pointer with zero offset */
int test_global_access(void) {
    int *p = &global_arr[0];
    volatile int *vp = p;
    
    /* Multiple simple register accesses */
    int a = vp[0];  /* p[0] is *(p + 0) -> simple register */
    int b = *vp;    /* *p is same pattern */
    
    /* Also include offset access to ensure variety */
    int c = vp[10];
    
    return a + b + c;
}

/* Test 4: Conditional simple access inside loop */
int test_conditional_simple_access(int *data, int n, int threshold) {
    int sum = 0;
    volatile int *vp = data;
    
    for (int i = 0; i < n; i++) {
        /* Complex addressing in loop */
        sum += vp[i];
        
        /* Conditional simple register access */
        if (vp[i] > threshold) {
            int val = *vp;  /* Simple register indirect inside loop */
            sum += val;
        }
    }
    
    /* Final simple access */
    sum += *vp;
    
    return sum;
}

/* Test 5: Struct access with pointer */
struct Point {
    int x, y, z;
};

int test_struct_access(struct Point *pts, int n) {
    int sum = 0;
    volatile struct Point *vpts = pts;
    
    /* Access struct member via pointer + 0 offset (after struct address calc) */
    sum += vpts[0].x;
    
    /* Simple access to first element */
    sum += vpts->x;  /* Equivalent to (*vpts).x */
    
    /* Loop with pointer increment through struct array */
    for (int i = 0; i < n; i++) {
        sum += vpts[i].y;
    }
    
    return sum;
}

/* Test 6: Multiple simple pointers in same function */
int test_multiple_pointers(int *a, int *b, int *c) {
    volatile int *va = a;
    volatile int *vb = b;
    volatile int *vc = c;
    
    /* Three separate simple register accesses */
    int sum = *va + *vb + *vc;
    
    /* Also use offset accesses */
    sum += va[1] + vb[2] + vc[3];
    
    return sum;
}

/* Driver function to call all tests and ensure code isn't eliminated */
int main(void) {
    int result = 0;
    
    /* Initialize test data */
    int local_arr[50];
    for (int i = 0; i < 50; i++) {
        local_arr[i] = i * 3 + 1;
    }
    
    for (int i = 0; i < 100; i++) {
        global_arr[i] = i * 2;
    }
    
    struct Point pts[10];
    for (int i = 0; i < 10; i++) {
        pts[i].x = i;
        pts[i].y = i * 2;
        pts[i].z = i * 3;
    }
    
    /* Run all tests */
    result += test_simple_param_load(local_arr);
    result += test_mixed_addressing(local_arr, 10);
    result += test_global_access();
    result += test_conditional_simple_access(local_arr, 20, 25);
    result += test_struct_access(pts, 5);
    
    int *p1 = &local_arr[0];
    int *p2 = &local_arr[10];
    int *p3 = &local_arr[20];
    result += test_multiple_pointers(p1, p2, p3);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    return (result > 0) ? 0 : 1;
}
