#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Pattern 1: Simple pointer dereference with post-increment in loop */
int sum_array_int(const int *arr, int n) {
    const int *p = arr;
    int sum = 0;
    
    /* This should generate (mem (reg)) patterns */
    while (n-- > 0) {
        sum += *p;      /* mem access via register */
        p = p + 1;      /* increment in separate statement */
    }
    return sum;
}

/* Pattern 2: Char pointer with restrict qualifier */
void copy_buffer_char(char *restrict dst, const char *restrict src, int n) {
    /* Classic memcpy pattern - should trigger auto-inc-dec */
    while (n-- > 0) {
        *dst = *src;    /* Two (mem (reg)) accesses */
        dst = dst + 1;  /* Separate increments */
        src = src + 1;
    }
}

/* Pattern 3: Mixed operations in loop */
void fill_alternating(short *arr, int n, short val1, short val2) {
    short *p = arr;
    int i = 0;
    
    for (i = 0; i < n; i++) {
        *p = (i & 1) ? val2 : val1;  /* Simple store */
        p = p + 1;                   /* Separate increment */
    }
}

/* Pattern 4: Pointer traversal with multiple dereferences */
int sum_pairs(const int *arr, int n) {
    const int *p = arr;
    int sum = 0;
    int i;
    
    for (i = 0; i < n/2; i++) {
        int a = *p;      /* First dereference */
        p = p + 1;       /* Increment */
        int b = *p;      /* Second dereference */
        p = p + 1;       /* Another increment */
        sum += a + b;
    }
    return sum;
}

/* Pattern 5: Simple while loop with post-increment */
int count_zeros(const char *buf, int n) {
    const char *p = buf;
    int count = 0;
    
    while (n-- > 0) {
        if (*p == 0) {   /* Dereference for comparison */
            count++;
        }
        p = p + 1;       /* Separate increment */
    }
    return count;
}

/* Pattern 6: Local pointer in main - direct traversal */
void process_buffer_local(void) {
    int buffer[16];
    int *p = buffer;
    int i;
    
    /* Initialize */
    for (i = 0; i < 16; i++) {
        buffer[i] = i * 2;
    }
    
    /* Process with pointer */
    p = buffer;
    for (i = 0; i < 16; i++) {
        int val = *p;    /* Dereference */
        *p = val + 1;    /* Store back */
        p = p + 1;       /* Increment separately */
    }
}

/* Pattern 7: Nested pointer operations */
void reverse_copy(int *restrict dst, const int *restrict src, int n) {
    const int *s = src;
    int *d = dst + n - 1;
    
    while (n-- > 0) {
        *d = *s;        /* Two memory accesses */
        s = s + 1;      /* Increment source */
        d = d - 1;      /* Decrement destination */
    }
}

/* Pattern 8: Multiple basic blocks with same pattern */
int sum_even_odd(const int *arr, int n) {
    const int *p = arr;
    int sum_even = 0, sum_odd = 0;
    int i;
    
    for (i = 0; i < n; i++) {
        int val = *p;           /* Dereference */
        p = p + 1;              /* Increment */
        
        if (i & 1) {
            sum_odd += val;
        } else {
            sum_even += val;
        }
    }
    return sum_even + sum_odd;
}

int main(void) {
    int int_array[100];
    char char_buffer[200];
    short short_array[50];
    int i;
    int total_sum = 0;
    
    /* Initialize test data */
    for (i = 0; i < 100; i++) {
        int_array[i] = i + 1;
    }
    
    memset(char_buffer, 'A', sizeof(char_buffer));
    char_buffer[50] = 0;  /* One zero for counting */
    
    for (i = 0; i < 50; i++) {
        short_array[i] = (short)(i * 3);
    }
    
    /* Test Pattern 1: Simple sum */
    total_sum += sum_array_int(int_array, 100);
    
    /* Test Pattern 2: Char copy */
    char dest_buffer[200];
    copy_buffer_char(dest_buffer, char_buffer, 200);
    
    /* Test Pattern 3: Fill alternating */
    fill_alternating(short_array, 50, 1, 2);
    
    /* Test Pattern 4: Sum pairs */
    total_sum += sum_pairs(int_array, 100);
    
    /* Test Pattern 5: Count zeros */
    total_sum += count_zeros(char_buffer, 200);
    
    /* Test Pattern 6: Local processing */
    process_buffer_local();
    
    /* Test Pattern 7: Reverse copy */
    int src_array[20], dst_array[20];
    for (i = 0; i < 20; i++) {
        src_array[i] = i * 5;
    }
    reverse_copy(dst_array, src_array, 20);
    
    /* Test Pattern 8: Even/odd sum */
    total_sum += sum_even_odd(int_array, 100);
    
    /* Additional pattern in main itself */
    {
        const int *ptr = int_array;
        int local_sum = 0;
        
        /* This loop in main should also generate the pattern */
        for (i = 0; i < 10; i++) {
            local_sum += *ptr;   /* Dereference */
            ptr = ptr + 1;       /* Separate increment */
        }
        total_sum += local_sum;
    }
    
    /* Verify the copy worked */
    if (memcmp(char_buffer, dest_buffer, 200) != 0) {
        printf("Error: Copy verification failed\n");
        return 1;
    }
    
    printf("Checksum: %d\n", total_sum);
    printf("All patterns executed successfully\n");
    
    return 0;
}
