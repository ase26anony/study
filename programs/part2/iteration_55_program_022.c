/* { dg-do compile } */
/* { dg-options "-O1" } */

void test_func(void) {
    int arr[100];
    int *ptr = arr;
    int *end = arr + 100;
    int sum = 0;
    
    /* Loop with separate dereference and increment */
    while (ptr < end) {
        /* Dereference pointer with base+0 addressing */
        sum += *ptr;
        /* Separate increment statement (not *ptr++) */
        ptr += 1;
    }
    
    /* Prevent dead code elimination */
    asm volatile ("" : : "r"(sum));
}
