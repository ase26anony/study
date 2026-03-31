#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Pattern 1: Simple pointer dereference followed by increment in loop */
int sum_array_int(const int *arr, int n) {
    const int *p = arr;
    const int *end = arr + n;
    int sum = 0;
    
    while (p < end) {
        /* This should generate (mem (reg)) pattern */
        int val = *p;
        p = p + 1;  /* Separate increment to match pattern */
        sum += val;
    }
    return sum;
}

/* Pattern 2: Char pointer with restrict to help alias analysis */
void copy_buffer_char(char *restrict dst, const char *restrict src, int n) {
    const char *s = src;
    char *d = dst;
    
    while (n-- > 0) {
        /* Direct dereference of pointer registers */
        char c = *s;
        s = s + 1;  /* Increment after dereference */
        *d = c;
        d = d + 1;  /* Separate increment for dst */
    }
}

/* Pattern 3: Mixed statements in loop - dereference and increment separated */
void fill_buffer_short(short *buf, short value, int count) {
    short *p = buf;
    
    for (int i = 0; i < count; i++) {
        /* Store then increment */
        *p = value;
        p = p + 1;
    }
}

/* Pattern 4: Post-increment in expression (common idiom) */
int sum_array_postinc(const int *arr, int n) {
    const int *p = arr;
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Post-increment in expression - may generate desired pattern */
        sum += *p++;
    }
    return sum;
}

/* Pattern 5: Multiple dereferences with increments */
void swap_and_advance(int *restrict a, int *restrict b, int n) {
    while (n-- > 0) {
        int temp = *a;  /* First dereference */
        *a = *b;        /* Second dereference */
        *b = temp;
        a = a + 1;      /* Separate increments */
        b = b + 1;
    }
}

/* Pattern 6: Simple while loop with pointer traversal */
int count_chars(const char *str) {
    const char *p = str;
    int count = 0;
    
    while (*p != '\0') {
        /* Dereference check, then increment */
        if (*p == 'X') count++;
        p = p + 1;
    }
    return count;
}

/* Pattern 7: Local pointer in main - direct memory access */
void process_local_buffer(void) {
    int buffer[16];
    int *p = buffer;
    
    /* Initialize */
    for (int i = 0; i < 16; i++) {
        *p = i * 2;
        p = p + 1;
    }
    
    /* Process */
    p = buffer;
    int sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += *p;
        p = p + 1;
    }
    
    printf("Local buffer sum: %d\n", sum);
}

int main(void) {
    /* Test data */
    int int_array[100];
    char char_buffer[200];
    short short_buffer[50];
    
    /* Initialize test data */
    for (int i = 0; i < 100; i++) {
        int_array[i] = i + 1;
    }
    
    for (int i = 0; i < 200; i++) {
        char_buffer[i] = (char)(i % 26 + 'A');
    }
    
    for (int i = 0; i < 50; i++) {
        short_buffer[i] = (short)(i * 3);
    }
    
    /* Call pattern functions */
    int sum1 = sum_array_int(int_array, 100);
    printf("Sum of int array: %d\n", sum1);
    
    char dest_buffer[200];
    copy_buffer_char(dest_buffer, char_buffer, 200);
    
    fill_buffer_short(short_buffer, 42, 50);
    
    int sum2 = sum_array_postinc(int_array, 100);
    printf("Sum with postinc: %d\n", sum2);
    
    /* Test swap function */
    int a[10], b[10];
    for (int i = 0; i < 10; i++) {
        a[i] = i;
        b[i] = i + 100;
    }
    swap_and_advance(a, b, 10);
    
    /* Test char counting */
    const char *test_str = "TESTXSTRINGXWITHXXMULTIPLEX";
    int x_count = count_chars(test_str);
    printf("Count of 'X' chars: %d\n", x_count);
    
    /* Process local buffer */
    process_local_buffer();
    
    /* Additional pattern in main itself */
    int *ptr = int_array;
    int checksum = 0;
    for (int i = 0; i < 10; i++) {  /* Small loop in main */
        int val = *ptr;  /* Should generate (mem (reg)) */
        ptr = ptr + 1;   /* Separate increment */
        checksum ^= val; /* Simple computation */
    }
    printf("Final checksum: %d\n", checksum);
    
    /* Verify copy worked */
    if (memcmp(char_buffer, dest_buffer, 200) == 0) {
        printf("Copy verification: PASSED\n");
    } else {
        printf("Copy verification: FAILED\n");
    }
    
    return 0;
}
