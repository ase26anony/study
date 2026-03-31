#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* Include GCC's GTY marker macros */
#ifndef GTY
#define GTY(x) 
#endif

/* Forward declaration for TYPE_UNDEFINED case */
struct opaque;  /* TYPE_UNDEFINED */

/* TYPE_SCALAR: Fundamental scalar types and enums */
typedef enum GTY(()) color {
    RED,
    GREEN,
    BLUE
} color_t;

typedef int GTY(()) scalar_int;
typedef double GTY(()) scalar_double;
typedef float GTY(()) scalar_float;
typedef long GTY(()) scalar_long;
typedef char GTY(()) scalar_char;

/* TYPE_STRUCT: Basic struct with various members */
struct GTY(()) basic_struct {
    int GTY(()) id;
    char GTY(()) name[32];
    double GTY(()) value;
    
    /* Anonymous struct inside */
    struct GTY(()) {
        int GTY(()) x;
        int GTY(()) y;
    } position;
    
    /* Bit-fields */
    unsigned GTY(()) flags : 4;
    unsigned GTY(()) status : 2;
};

/* TYPE_UNION: Tagged union */
union GTY(()) data_union {
    int GTY(()) int_value;
    double GTY(()) double_value;
    char GTY(()) char_value;
    float GTY(()) float_value;
};

/* TYPE_USER_STRUCT: User-defined struct with special handling */
struct GTY((user)) user_struct {
    int GTY(()) custom_id;
    void* GTY(()) user_data;
};

/* TYPE_POINTER: Various pointer types */
typedef struct basic_struct* GTY(()) struct_ptr;
typedef union data_union* GTY(()) union_ptr;
typedef void (*GTY(()) func_ptr)(int, double);  /* Function pointer */
typedef void* GTY(()) void_ptr;
typedef int** GTY(()) ptr_to_ptr;

/* TYPE_ARRAY: Various array types */
typedef int GTY(()) int_array[10];
typedef struct basic_struct GTY(()) struct_array[5];
typedef union data_union GTY(()) union_array[8];
typedef char* GTY(()) string_array[3];

/* TYPE_STRING: String types */
typedef char* GTY(()) string_ptr;
typedef const char* GTY(()) const_string_ptr;

/* TYPE_CALLBACK: Callback function type */
typedef int (*GTY(()) callback_func)(const char*, int);

/* Complex struct with all type dependencies */
struct GTY(()) complex_struct {
    /* TYPE_STRUCT nested */
    struct GTY(()) nested {
        int GTY(()) depth;
        struct nested* GTY(()) next;  /* TYPE_POINTER to struct */
    } *GTY(()) first;
    
    /* TYPE_UNION */
    union GTY(()) {
        int GTY(()) tag;
        double GTY(()) data;
    } variant;
    
    /* TYPE_ARRAY */
    int GTY(()) matrix[3][3];  /* Multi-dimensional array */
    
    /* TYPE_POINTER to union */
    union data_union* GTY(()) data_ptr;
    
    /* TYPE_SCALAR */
    color_t GTY(()) color;
    
    /* TYPE_STRING */
    char* GTY(()) description;
    
    /* TYPE_CALLBACK */
    callback_func GTY(()) handler;
    
    /* Chain pointers for GTY options */
    struct complex_struct* GTY((chain_next("next_complex"))) next;
    struct complex_struct* GTY((chain_prev("prev_complex"))) prev;
    
    /* Length field for array */
    int GTY(()) count;
    struct basic_struct* GTY(()) items GTY((length("count")));
    
    /* Descriminator union */
    enum GTY(()) kind { KIND_A, KIND_B, KIND_C } kind;
    union GTY((desc ("kind"))) {
        struct GTY((tag ("KIND_A"))) {
            int GTY(()) a_value;
        } kind_a;
        struct GTY((tag ("KIND_B"))) {
            double GTY(()) b_value;
        } kind_b;
        struct GTY((tag ("KIND_C"))) {
            char GTY(()) c_value[10];
        } kind_c;
    } u;
};

/* Another struct for circular references */
struct GTY(()) node {
    int GTY(()) value;
    struct node* GTY(()) left;   /* TYPE_POINTER */
    struct node* GTY(()) right;  /* TYPE_POINTER */
    struct node* GTY(()) parent; /* TYPE_POINTER */
};

/* Global variables declaration */
extern struct basic_struct GTY(()) global_struct;
extern union data_union GTY(()) global_union;
extern struct complex_struct GTY(()) global_complex;
extern int_array GTY(()) global_int_array;
extern string_ptr GTY(()) global_string;

#endif /* TEST_TYPES_H */
