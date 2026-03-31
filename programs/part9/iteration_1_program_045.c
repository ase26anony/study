/* auto_inc_dec_test.c
 * Test program to trigger GCC's auto-increment/decrement optimization
 * Specifically targets the (mem (reg)) pattern in find_address_incs()
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Pattern 1: Simple pointer dereference followed by increment */
int sum_array_int(const int *restrict arr, int n) {
    const int *restrict p = arr;
    int sum = 0;
    
    /* This should generate (mem (reg)) pattern */
    while (n-- > 0) {
        sum += *p;      /* mem access via register */
        p = p + 1;      /* increment in separate statement */
    }
    return sum;
}

/* Pattern 2: Char pointer with post-increment in same statement */
int count_chars(const char *restrict str) {
    const char *restrict p = str;
    int count = 0;
    
    /* Direct dereference with post-increment */
    while (*p != '\0') {
        if (*p++ == 'a')  /* *p then p++ combined */
            count++;
    }
    return count;
}

/* Pattern 3: Memory copy with separate load/store and increments */
void copy_buffer(char *restrict dst, const char *restrict src, int n) {
    char *restrict d = dst;
    const char *restrict s = src;
    
    /* Separate operations to encourage pattern matching */
    while (n-- > 0) {
        char temp = *s;     /* Load: (mem (reg)) */
        s = s + 1;          /* Increment source */
        *d = temp;          /* Store: (mem (reg)) */
        d = d + 1;          /* Increment destination */
    }
}

/* Pattern 4: Fill memory with value using pointer */
void fill_sequence(int *restrict arr, int n, int start) {
    int *restrict p = arr;
    int value = start;
    
    /* Store and increment in loop */
    for (int i = 0; i < n; i++) {
        *p = value++;   /* Store then increment pointer */
        p = p + 1;
    }
}

/* Pattern 5: Double dereference pattern */
int sum_indirect(const int **restrict ptrs, int n) {
    const int **restrict p = ptrs;
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        sum += **p;     /* Double dereference */
        p = p + 1;
    }
    return sum;
}

/* Pattern 6: Mixed types to test different modes */
short sum_shorts(const short *restrict arr, int n) {
    const short *restrict p = arr;
    short sum = 0;
    
    while (n-- > 0) {
        sum += *p;
        p = p + 1;
    }
    return sum;
}

/* Pattern 7: Pointer in main function itself */
int process_buffer(int *restrict buf, int size) {
    int *restrict p = buf;
    int result = 0;
    
    /* Simple pattern in its own basic block */
    for (int i = 0; i < size; i++) {
        result ^= *p;   /* XOR instead of add for variety */
        p = p + 1;
    }
    
    /* Additional pattern: post-increment in expression */
    p = buf;
    for (int i = 0; i < size; i++) {
        result += *p++;
    }
    
    return result;
}

/* Helper to verify results */
int verify_copy(const char *a, const char *b, int n) {
    for (int i = 0; i < n; i++) {
        if (a[i] != b[i]) return 0;
    }
    return 1;
}

int main(void) {
    printf("Testing auto-increment/decrement patterns...\n");
    
    /* Test 1: Integer array sum */
    int int_arr[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    int sum1 = sum_array_int(int_arr, 10);
    printf("Sum of ints: %d (expected: 55)\n", sum1);
    
    /* Test 2: Char counting */
    const char *str = "auto_increment_test_string";
    int count = count_chars(str);
    printf("Count of 'a': %d\n", count);
    
    /* Test 3: Buffer copy */
    char src[50], dst[50];
    for (int i = 0; i < 50; i++) src[i] = (char)('A' + (i % 26));
    copy_buffer(dst, src, 50);
    int copy_ok = verify_copy(dst, src, 50);
    printf("Copy verified: %s\n", copy_ok ? "YES" : "NO");
    
    /* Test 4: Sequence fill */
    int seq_arr[20];
    fill_sequence(seq_arr, 20, 100);
    printf("Sequence[0]=%d, [19]=%d\n", seq_arr[0], seq_arr[19]);
    
    /* Test 5: Indirect access */
    const int *ptrs[5];
    int vals[5] = {10, 20, 30, 40, 50};
    for (int i = 0; i < 5; i++) ptrs[i] = &vals[i];
    int indirect_sum = sum_indirect(ptrs, 5);
    printf("Indirect sum: %d (expected: 150)\n", indirect_sum);
    
    /* Test 6: Short array */
    short short_arr[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    short short_sum = sum_shorts(short_arr, 8);
    printf("Sum of shorts: %d (expected: 36)\n", short_sum);
    
    /* Test 7: Main function pattern */
    int main_buf[15];
    for (int i = 0; i < 15; i++) main_buf[i] = i * 2;
    int main_result = process_buffer(main_buf, 15);
    printf("Main buffer result: %d\n", main_result);
    
    /* Additional pattern in main with char pointer */
    char msg[] = "HelloWorld";
    char *p = msg;
    int char_sum = 0;
    while (*p) {
        char_sum += *p;  /* Simple (mem (reg)) access */
        p = p + 1;       /* Separate increment */
    }
    printf("Char sum in main: %d\n", char_sum);
    
    /* Final checksum for verification */
    int final_checksum = sum1 + count + copy_ok + seq_arr[0] + 
                        indirect_sum + short_sum + main_result + char_sum;
    printf("\nFinal checksum: %d\n", final_checksum);
    
    return 0;
}
