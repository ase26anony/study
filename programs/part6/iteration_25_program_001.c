/* Complex type declarations to exercise gengtype parser's balanced delimiter handling */

/* 1. Complex function pointers with nested parentheses */
typedef int (*fn_simple)(void);
typedef fn_simple (*fn_returning_fn)(int);
typedef int (*(*complex_func_ptr)(int (*)(double)))(char);

/* 2. Multi-dimensional and complex array declarations */
int (*(*array_of_func_ptrs[5])(int))[10];
char (*strings[(2+3)])[20];
int (*matrix_ptr)[3][4];

/* 3. Nested structures with arrays and function pointers */
struct Inner {
    int (*calc)(int, int);
    char data[2][5];
};

struct Outer {
    struct Inner inner[3];
    int (*(*func_table)[2])(void);
    union {
        long l;
        double d;
    } value;
};

/* 4. Combined construct in single declaration */
struct Container {
    int (*process)(int (*array[])[5], struct Inner *param);
    void (*cleanup)(void);
};

/* 5. Global variables with initializers */
int (*(*global_var)(void))[3] = NULL;

struct Data {
    int (*func)(int);
    int values[2][2];
} global_data = { 
    NULL, 
    { {1, 2}, {3, 4} } 
};

struct Outer global_outer = {
    .inner = {
        [0] = { NULL, { {'a','b','c','d','\0'}, {'e','f','g','h','\0'} } },
        [1] = { NULL, { {'i','j','k','l','\0'}, {'m','n','o','p','\0'} } },
        [2] = { NULL, { {'q','r','s','t','\0'}, {'u','v','w','x','\0'} } }
    },
    .func_table = NULL,
    .value = { .l = 42 }
};

/* 6. Compound literal in global scope */
int (*global_ptr)[2] = (int[][2]){ {1,2}, {3,4}, {5,6} };

/* Helper functions for function pointers */
int simple_func(void) { return 42; }
int calc_sum(int a, int b) { return a + b; }
int process_data(int (*array[])[5], struct Inner *param) { return 0; }
void cleanup_resources(void) { }

/* Main function to use all constructs */
int main(void) {
    /* Use complex typedef */
    fn_returning_fn fn_var = NULL;
    
    /* Access nested array from global struct */
    int sum = 0;
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 2; j++) {
            sum += global_data.values[i][j];
        }
    }
    
    /* Access compound literal through pointer */
    int matrix_sum = 0;
    for (int i = 0; i < 3; i++) {
        matrix_sum += (*global_ptr)[i];
    }
    
    /* Initialize function pointers */
    struct Outer local_outer = global_outer;
    local_outer.inner[0].calc = calc_sum;
    
    /* Use designated initializer with nested braces */
    int arr[2][3] = { [0] = {1,2,3}, [1] = {4,5,6} };
    int arr_sum = 0;
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 3; j++) {
            arr_sum += arr[i][j];
        }
    }
    
    /* Complex declaration with all delimiters in main's scope */
    void (*signal_handler(int sig, void (*handler)(int)))(int) = NULL;
    
    /* Nested structure with array of function pointers */
    struct {
        int (*(*callbacks[3])(char *))[2];
        struct {
            int depth;
            char *path;
        } config;
    } app_state = {
        .callbacks = { NULL, NULL, NULL },
        .config = { 5, NULL }
    };
    
    /* Calculate final result using all accessed data */
    int result = sum + matrix_sum + arr_sum + local_outer.value.l;
    
    printf("Result: %d\n", result);
    
    return 0;
}
