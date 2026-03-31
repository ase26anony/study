/* Test program to trigger selective scheduler debug dumps in GCC */
#include <stdio.h>
#include <stdlib.h>

/* Function 1: Inner loop with conditional branch and memory write */
void test_inner_loop(int *arr, int n) {
    volatile int sum = 0;  /* Prevent dead code elimination */
    int i, j;
    
    for (i = 0; i < n; i++) {
        /* Create data dependencies */
        int temp = arr[i];
        
        /* Conditional store with dependency chain */
        if (temp > 0) {
            for (j = 0; j < 3; j++) {
                temp = temp * 2 + j;
                /* Inline asm to create unschedulable dependency */
                asm volatile ("" : "+r" (temp));
            }
            arr[i] = temp;
        } else {
            /* Different path with arithmetic */
            temp = temp - i * 7;
            arr[i] = temp % 256;
        }
        
        sum += arr[i];
    }
    
    /* Use sum to prevent optimization */
    asm volatile ("" : : "r" (sum));
}

/* Function 2: Nested loops with different iteration counts */
void test_nested_loops(int *matrix, int rows, int cols) {
    volatile int checksum = 0;
    int i, j, k;
    
    /* Outer loop with variable bounds */
    for (i = 0; i < rows; i++) {
        /* Middle loop */
        for (j = 0; j < cols; j++) {
            int idx = i * cols + j;
            int val = matrix[idx];
            
            /* Inner loop with small iteration count */
            for (k = 0; k < 2; k++) {
                /* Complex dependency chain */
                val = (val * 1103515245 + 12345) & 0x7fffffff;
                if (val & 1) {
                    val ^= 0x55555555;
                }
            }
            
            matrix[idx] = val;
            checksum ^= val;
        }
        
        /* Conditional break to create control flow */
        if (checksum > 1000000) {
            /* Early exit with side effect */
            matrix[0] = checksum;
            break;
        }
    }
    
    /* Prevent dead code elimination */
    asm volatile ("" : : "r" (checksum));
}

/* Function 3: Switch statement with computed goto */
void test_switch_complex(int mode, int *data, int size) {
    static void *labels[] = { &&case0, &&case1, &&case2, &&case3 };
    volatile int result = 0;
    int i;
    
    if (mode < 0 || mode > 3) return;
    
    /* Jump to label */
    goto *labels[mode];
    
case0:
    /* Simple processing */
    for (i = 0; i < size; i++) {
        data[i] = data[i] + i;
    }
    goto end;
    
case1:
    /* More complex with branches */
    for (i = 0; i < size; i++) {
        if (i % 2 == 0) {
            data[i] = data[i] * 3;
        } else {
            data[i] = data[i] / 2;
        }
    }
    goto end;
    
case2:
    /* Nested loops in switch case */
    for (i = 0; i < size; i++) {
        int j;
        for (j = 0; j < 4; j++) {
            data[i] = (data[i] << j) | (data[i] >> (32 - j));
        }
    }
    goto end;
    
case3:
    /* Mixed operations */
    for (i = 0; i < size; i++) {
        data[i] = (data[i] ^ 0xAA) + (i * 11);
        /* Memory barrier-like asm */
        asm volatile ("" : : "r" (data[i]));
    }
    
end:
    /* Compute result to prevent optimization */
    for (i = 0; i < size; i++) {
        result += data[i];
    }
    asm volatile ("" : : "r" (result));
}

/* Function 4: Complex control flow with function calls */
int helper1(int x) {
    return (x * 13 + 7) % 256;
}

int helper2(int x, int y) {
    return (x ^ y) + (x & y) * 2;
}

void test_function_calls(int *arr, int n) {
    volatile int total = 0;
    int i;
    
    for (i = 0; i < n; i++) {
        int val = arr[i];
        
        /* Conditional function calls */
        if (val % 3 == 0) {
            val = helper1(val);
        } else if (val % 3 == 1) {
            val = helper2(val, i);
        } else {
            val = helper1(helper2(val, val >> 2));
        }
        
        /* Additional branching */
        switch (val % 4) {
            case 0: val += 1; break;
            case 1: val *= 2; break;
            case 2: val ^= 0xFF; break;
            case 3: val = ~val; break;
        }
        
        arr[i] = val;
        total += val;
        
        /* Early exit condition */
        if (total > 10000 && i > n/2) {
            break;
        }
    }
    
    asm volatile ("" : : "r" (total));
}

/* Function 5: Pointer chasing with dependencies */
void test_pointer_chasing(int **ptr_array, int count) {
    volatile int hash = 0;
    int i;
    
    for (i = 0; i < count; i++) {
        int *ptr = ptr_array[i];
        if (ptr) {
            /* Dereference and modify */
            int val = *ptr;
            
            /* Create long dependency chain */
            val = ((val << 5) | (val >> 27)) + i;
            val = val ^ (val * 31);
            val = (val + 0x12345678) & 0xFFFFFFFF;
            
            *ptr = val;
            hash ^= val;
            
            /* Conditional store to different location */
            if (val % 7 == 0) {
                ptr_array[(i + 1) % count] = &val;
            }
        }
    }
    
    asm volatile ("" : : "r" (hash));
}

/* Main driver function */
int main() {
    /* Initialize test data */
    int arr1[100];
    int arr2[10][10];
    int arr3[50];
    int *ptr_arr[20];
    int i, j;
    
    /* Initialize arrays with pseudo-random data */
    for (i = 0; i < 100; i++) {
        arr1[i] = (i * 37 + 123) % 1000;
    }
    
    for (i = 0; i < 10; i++) {
        for (j = 0; j < 10; j++) {
            arr2[i][j] = (i * 11 + j * 7) % 256;
        }
    }
    
    for (i = 0; i < 50; i++) {
        arr3[i] = (i * 19 + 456) % 512;
    }
    
    for (i = 0; i < 20; i++) {
        ptr_arr[i] = &arr1[i * 5];
    }
    
    /* Call all test functions to ensure they're compiled */
    test_inner_loop(arr1, 100);
    test_nested_loops((int *)arr2, 10, 10);
    test_switch_complex(2, arr3, 50);
    test_function_calls(arr1, 100);
    test_pointer_chasing(ptr_arr, 20);
    
    /* Compute a simple result to return */
    int result = 0;
    for (i = 0; i < 100; i++) {
        result += arr1[i];
    }
    
    return result % 256;
}
