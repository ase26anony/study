/* test_complex_types.c - Complex type declarations to exercise gengtype parser */

/* 1. Complex function declarators with nested parentheses */
typedef int (*fn1)(void);
typedef fn1 (*fn2)(int);
typedef int (*(*fn3)(double))(char);

/* Function pointer returning pointer to array */
int (*(*global_func_ptr)(int (*)(double)))(char);

/* 2. Multi-dimensional and complex array declarations */
int (*(*array_of_func_ptrs[5])(int))[10];
char (*strings[(2+3)])[20];
int (*matrix[3])[4][5];

/* 3. Nested aggregate initializers and type definitions */
struct Inner {
    char *p;
    int (*func)(int, int);
};

struct Nested {
    int a[2][3];
    struct Inner inner;
    union {
        long x;
        double y;
    } u;
};

/* Global struct with complex initializer */
char global_char = 'A';
struct Nested global_nested = { 
    {{1,2,3},{4,5,6}}, 
    { &global_char, NULL },
    { .x = 42 }
};

/* Array with designated initializers */
int arr[2][3] = { [0] = {1,2,3}, [1] = {4,5,6} };

/* 4. Combine constructs in single declarations */
int (*ptr)[2] = (int[][2]){ {1,2}, {3,4} };

struct Table {
    int rows;
    int cols;
};

void process(int (*table[])[5], struct Table param);

/* 5. More global declarations with mixed delimiters */
int (*(*global_var)(void))[3];

struct Data { 
    int (*func)(int);
    struct {
        int (*nested_func)(int, int);
    } inner;
} global_data = { NULL, { NULL } };

/* Complex typedef with all delimiters */
typedef struct {
    int (*methods[5])(void);
    union {
        struct {
            int x;
            int y;
        } point;
        int array[2];
    } data;
} ComplexType;

ComplexType global_complex = {
    .methods = { NULL, NULL, NULL, NULL, NULL },
    .data = { .point = { 10, 20 } }
};

/* Helper functions for function pointers */
int add(int a, int b) {
    return a + b;
}

int simple_func(double d) {
    return (int)d;
}

int (*get_array_ptr(void))[3] {
    static int arr[2][3] = {{1,2,3},{4,5,6}};
    return arr;
}

/* 6. Main function to use all constructs */
int main(void) {
    /* Use complex typedef */
    fn2 complex_fn = NULL;
    
    /* Assign to function pointer */
    global_data.func = add;
    
    /* Access nested array elements */
    int sum = 0;
    sum += global_nested.a[0][0];
    sum += global_nested.a[1][2];
    sum += arr[0][1];
    sum += arr[1][2];
    
    /* Use pointer to array */
    int val = (*ptr)[0] + (*ptr)[1];
    sum += val;
    
    /* Access complex global struct */
    sum += global_complex.data.point.x;
    sum += global_complex.data.point.y;
    
    /* Call through function pointer */
    if (global_data.func) {
        sum += global_data.func(3, 4);
    }
    
    /* Use array of function pointers (simulated) */
    int (*local_func)(int, int) = add;
    sum += local_func(1, 2);
    
    /* Print result to prevent optimization */
    printf("Result: %d\n", sum);
    
    return sum > 0 ? 0 : 1;
}

/* Additional complex prototype to ensure parser sees it */
void unused_complex_function(
    int (*(*callback)(struct Nested))[2],
    char (*(*names[]))(void),
    struct { 
        union { 
            int a; 
            long b; 
        } u; 
        int arr[3][2]; 
    } param
) {
    /* Empty but complex signature */
}
