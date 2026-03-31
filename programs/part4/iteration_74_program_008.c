#include <stdio.h>
#include <stdlib.h>

/* Opaque functions to prevent optimization */
static int __attribute__((noinline, noipa)) get_value(int x) {
    volatile int v = x;
    return v + 1;
}

static void __attribute__((noinline, noipa)) sink(int val) {
    volatile int sink_var = val;
    (void)sink_var;
}

/* Test 1: Simple loop with register and memory dependencies */
static int __attribute__((noinline, noipa)) 
test1_loop_carried_deps(int n, int* arr1, int* arr2) {
    int acc = 0;
    int prev = arr1[0];
    
    /* Loop with RAW, WAR, and WAW dependencies */
    for (int i = 1; i < n; ++i) {
        /* RAW: Read arr1[i] before writing arr2[i] */
        int temp = arr1[i] + prev;  
        
        /* WAR: Read arr2[i-1] before writing arr2[i] */
        temp += arr2[i-1];          
        
        /* WAW: Multiple writes to arr2[i] */
        arr2[i] = temp * 2;         
        arr2[i] = arr2[i] + 1;      /* Second write creates WAW */
        
        /* Loop-carried register dependency (distance 1) */
        prev = arr2[i] - arr1[i];   
        
        /* Anti-dependency (WAR) on acc */
        acc = acc + prev;           
        
        /* Output dependency (WAW) on temp */
        temp = get_value(i);        
    }
    
    /* Complex recurrence chain within single iteration */
    int x = acc;
    int y = x + 1;
    x = y * 2;      /* Creates cycle: x -> y -> x */
    y = x / 3;
    
    return acc + x + y;
}

/* Test 2: Nested loops for SCC formation */
static int __attribute__((noinline, noipa))
test2_nested_matrix(int n, int m, int mat_a[], int mat_b[], int mat_c[]) {
    int sum = 0;
    
    /* Outer loop */
    for (int i = 0; i < n; ++i) {
        int row_acc = 0;
        
        /* Inner loop with loop-carried dependency */
        for (int j = 0; j < m; ++j) {
            /* Memory dependency with distance 1 in inner loop */
            int prev = (j > 0) ? mat_c[i*m + j-1] : 0;
            
            /* RAW on mat_a, anti on mat_b */
            int val = mat_a[i*m + j] * 2;
            mat_b[i*m + j] = val + prev;  /* Write mat_b */
            
            /* Read mat_b immediately after write (WAR) */
            row_acc += mat_b[i*m + j] - mat_a[i*m + j];
            
            /* Control dependency */
            if (row_acc > 1000) {
                mat_c[i*m + j] = row_acc / 2;
                row_acc = row_acc % 1000;
            } else {
                mat_c[i*m + j] = row_acc;
            }
            
            /* Loop-carried in inner loop */
            mat_a[i*m + j] = mat_c[i*m + j] + i;
        }
        
        /* Loop-carried in outer loop */
        sum += row_acc;
        
        /* Cross-iteration dependency in outer loop */
        if (i > 0) {
            mat_b[i*m] += mat_b[(i-1)*m + m-1];
        }
    }
    
    return sum;
}

/* Test 3: Complex pointer arithmetic with aliasing */
static int __attribute__((noinline, noipa))
test3_pointer_aliasing(int n, int* base) {
    int* ptr1 = base;
    int* ptr2 = base + n/2;
    int result = 0;
    
    for (int i = 0; i < n/2; ++i) {
        /* Pointer-based accesses creating potential aliasing */
        int val1 = *ptr1;
        int val2 = *ptr2;
        
        /* RAW through pointers */
        *ptr1 = val1 + val2;
        
        /* WAR: Read after write through different pointer */
        result += *ptr1 - val2;
        
        /* Output dependency through same pointer */
        *ptr1 = *ptr1 * 3;
        
        /* Anti-dependency through second pointer */
        *ptr2 = result + i;
        
        /* Update pointers with loop-carried dependency */
        ptr1++;
        ptr2--;
        
        /* Complex recurrence within iteration */
        int x = result;
        int y = x + *ptr1;
        result = y - *ptr2;
        x = result / 2;  /* Creates small dependency cycle */
    }
    
    return result;
}

/* Test 4: Conditional dependencies and mixed patterns */
static int __attribute__((noinline, noipa))
test4_conditional_deps(int n, int* data, int* flags) {
    int acc1 = 0, acc2 = 0;
    int prev1 = data[0], prev2 = 0;
    
    for (int i = 1; i < n; ++i) {
        /* Multiple loop-carried dependencies */
        int curr = data[i];
        
        /* True dependency chain with distance 2 */
        if (i >= 2) {
            curr += data[i-2];  /* Distance 2 RAW */
        }
        
        /* Control dependency based on loop-variant value */
        if (curr > prev1) {
            acc1 += curr * 2;
            flags[i] = 1;
            prev2 = acc1 - prev1;  /* Cross-iteration in conditional path */
        } else {
            acc2 += curr / 2;
            flags[i] = 0;
            prev2 = acc2 + prev1;  /* Different cross-iteration in else path */
        }
        
        /* Output dependency on data[i] */
        data[i] = acc1 + acc2;
        
        /* Anti-dependency on prev1 */
        prev1 = curr + i;
        
        /* WAR on acc1/acc2 */
        int temp = (i % 2) ? acc1 : acc2;
        acc1 = acc1 - temp / 4;
        acc2 = acc2 + temp / 4;
    }
    
    /* Final recurrence to create small SCC */
    int x = acc1;
    int y = acc2;
    for (int i = 0; i < 3; ++i) {
        x = y + x;
        y = x - y;
    }
    
    return x + y;
}

/* Test 5: Reduction with multiple dependency types */
static int __attribute__((noinline, noipa))
test5_mixed_reductions(int n, int* arr) {
    int sum = 0;
    int prod = 1;
    int xor_val = 0;
    int prev_sum = 0;
    
    volatile int* varr = (volatile int*)arr;
    
    for (int i = 0; i < n; ++i) {
        /* Volatile read to prevent optimization */
        int val = varr[i];
        
        /* Multiple reductions with loop-carried dependencies */
        prev_sum = sum;
        sum = sum + val + (i > 0 ? arr[i-1] : 0);  /* RAW on sum, arr[i-1] */
        
        /* WAR on prod */
        xor_val = xor_val ^ prod;
        prod = prod * (val + 1);  /* Write after read of prod */
        
        /* WAW on arr[i] */
        arr[i] = sum;
        arr[i] = arr[i] + prod;  /* Second write */
        
        /* Complex dependency web */
        int temp = xor_val;
        xor_val = prev_sum ^ temp;
        prev_sum = temp + i;
        
        /* Control dependency affecting reduction */
        if (prod > 1000000) {
            prod = prod / 2;
            sum = sum - val;
        }
    }
    
    return sum + prod + xor_val;
}

int main(int argc, char* argv[]) {
    /* Use volatile or arguments to make bounds unknown at compile time */
    int n = 1000;
    int m = 100;
    
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n < 10) n = 1000;
        m = n / 10;
    }
    
    /* Allocate and initialize arrays */
    int* arr1 = (int*)malloc(n * sizeof(int));
    int* arr2 = (int*)malloc(n * sizeof(int));
    int* mat_a = (int*)malloc(n * m * sizeof(int));
    int* mat_b = (int*)malloc(n * m * sizeof(int));
    int* mat_c = (int*)malloc(n * m * sizeof(int));
    int* data = (int*)malloc(n * sizeof(int));
    int* flags = (int*)malloc(n * sizeof(int));
    
    /* Initialize with non-constant patterns */
    for (int i = 0; i < n; ++i) {
        arr1[i] = i * 3 + 1;
        arr2[i] = i * 2 - 1;
        data[i] = i * 5 % 100;
        flags[i] = i % 2;
    }
    
    for (int i = 0; i < n * m; ++i) {
        mat_a[i] = (i * 7) % 50;
        mat_b[i] = (i * 11) % 40;
        mat_c[i] = (i * 13) % 30;
    }
    
    /* Run all test cases */
    int result1 = test1_loop_carried_deps(n, arr1, arr2);
    int result2 = test2_nested_matrix(n/10, m, mat_a, mat_b, mat_c);
    int result3 = test3_pointer_aliasing(n, arr1);
    int result4 = test4_conditional_deps(n, data, flags);
    int result5 = test5_mixed_reductions(n, arr2);
    
    /* Aggregate results with volatile sink */
    volatile int final_sink = 0;
    final_sink += result1;
    final_sink += result2;
    final_sink += result3;
    final_sink += result4;
    final_sink += result5;
    
    /* Print checksum to ensure execution */
    printf("Checksum: %d\n", final_sink);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(mat_a);
    free(mat_b);
    free(mat_c);
    free(data);
    free(flags);
    
    return 0;
}
