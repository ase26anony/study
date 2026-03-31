/* Test parentheses in function pointer declarations */
typedef int (*simple_func_ptr)(int, char);

/* Complex function pointer declaration */
void (*(*complex_fp)(void))(int);

/* Function pointer as struct member */
struct Callbacks {
    int (*handler)(const char*);
    void (*cleanup)(void);
};

/* GTY-marked function pointer */
static GTY(()) int (*global_handler)(int) = NULL;

/* Function pointer in typedef with GTY */
typedef GTY(()) void (*gty_func_ptr)(void*);
