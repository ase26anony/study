/* Complex type definitions to exercise gengtype parser's balanced delimiter handling */

/* 1. Complex function declarators with nested parentheses */
typedef int (*fn_ptr_simple)(void);
typedef fn_ptr_simple (*fn_ptr_returning_fn_ptr)(int, double);
typedef int (*(*fn_ptr_complex)(int (*)(double)))(char);

/* 2. Multi-dimensional and complex array declarations */
int (*(*array_of_func_ptrs[5])(int))[10];
char (*strings[(2+3)])[20];
int (*multi_dim_array[2][3][4])(float);

/* 3. Nested aggregate initializers and type definitions */
struct InnerStruct {
    char *p;
    int (*func)(int);
};

struct Nested {
    int a[2][3];
    struct InnerStruct inner;
    union {
        long l;
        double d;
    } u;
};

struct Container {
    struct Nested items[2];
    int (*process[3])(struct Nested *);
};

/* Global variables with complex types and initializers */
int (*(*global_func_ptr)(void))[3] = NULL;

struct Nested global_nested = { 
    {{1, 2, 3}, {4, 5, 6}}, 
    { (char*)0x1000, NULL }, 
    { .l = 42 } 
};

int arr[2][3] = { [0] = {1, 2, 3}, [1] = {4, 5, 6} };

struct Container global_container = {
    .items = {
        { {{7, 8, 9}, {10, 11, 12}}, { NULL, NULL }, { .d = 3.14 } },
        { {{13, 14, 15}, {16, 17, 18}}, { (char*)0x2000, NULL }, { .l = 99 } }
    },
    .process = { NULL, NULL, NULL }
};

/* 4. Combined constructs in single declarations */
int (*combined_ptr)[2] = (int[][2]){ {1, 2}, {3, 4} };

struct {
    int (*table[3])[5];
    struct { int x; } param;
} combined_struct = { 
    .table = { NULL, NULL, NULL }, 
    .param = { .x = 100 } 
};

/* Function prototypes with complex parameters */
void process_table(int (*table[])[5], struct { int x; } param);
int (*register_callback(int (*cb)(int, int)))(void);

/* Simple functions to assign to function pointers */
int simple_func(int x, int y) { return x + y; }
int another_func(int x) { return x * 2; }
int func_for_array(float f) { return (int)f; }

/* Main function to use the complex types */
int main(void) {
    /* Local variable using complex typedef */
    fn_ptr_returning_fn_ptr local_fn_ptr = NULL;
    
    /* Assign address to complex function pointer */
    /* Note: We'll just use NULL for demonstration since we don't have
       actual compatible functions, but the type syntax is what matters
       for parser coverage */
    
    /* Access elements from nested array */
    int sum = 0;
    sum += global_nested.a[0][0];  /* 1 */
    sum += global_nested.a[1][2];  /* 6 */
    sum += arr[0][1];              /* 2 */
    sum += arr[1][0];              /* 4 */
    
    /* Access nested structure */
    sum += global_container.items[0].a[0][1];  /* 8 */
    sum += global_container.items[1].u.l;      /* 99 */
    
    /* Use combined construct */
    if (combined_ptr) {
        /* combined_ptr points to (int[][2]){ {1,2}, {3,4} } */
        sum += ((int(*)[2])combined_ptr)[0][0];  /* 1 */
    }
    
    /* Complex local declaration mixing all delimiters */
    int (*(*local_complex)(int (*)(double)))(char) = NULL;
    struct {
        int (*arr[2])(float);
        struct { char c; } s;
    } local_mixed = { .arr = { func_for_array, NULL }, .s = { 'A' } };
    
    /* Add the character value from local mixed struct */
    sum += local_mixed.s.c;  /* 'A' = 65 */
    
    /* Print result */
    printf("Result: %d\n", sum);  /* 1+6+2+4+8+99+1+65 = 186 */
    
    return 0;
}

/* Function definitions */
void process_table(int (*table[])[5], struct { int x; } param) {
    /* Empty implementation - exists for prototype parsing */
    (void)table;
    (void)param;
}

int (*register_callback(int (*cb)(int, int)))(void) {
    /* Return NULL for demonstration */
    (void)cb;
    return NULL;
}
