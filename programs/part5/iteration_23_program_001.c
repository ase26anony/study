#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* Include GCC's GTY marker macros */
#ifndef GTY
#define GTY(x) 
#endif

/* TYPE_UNDEFINED: Forward declaration without definition */
struct GTY(()) opaque_struct;

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

/* TYPE_STRUCT: Regular struct with various members */
struct GTY(()) my_struct {
    int GTY(()) id;
    char GTY(()) name[32];  /* TYPE_ARRAY within struct */
    struct my_struct* GTY((chain_next)) next;  /* TYPE_POINTER with chain_next */
    struct my_struct* GTY((chain_prev)) prev;  /* TYPE_POINTER with chain_prev */
    union my_union* GTY(()) data;  /* TYPE_POINTER to union */
    color_t GTY(()) color;  /* TYPE_SCALAR (enum) */
    
    /* Anonymous struct */
    struct GTY(()) {
        int GTY(()) x;
        int GTY(()) y;
    } position;
    
    /* Bit-fields */
    unsigned GTY(()) flags : 4;
    unsigned GTY(()) status : 2;
};

/* TYPE_USER_STRUCT: User-defined struct with special handling */
struct GTY((user)) my_user_struct {
    int GTY(()) user_id;
    void* GTY(()) user_data;
};

/* TYPE_UNION: Regular union type */
union GTY(()) my_union {
    int GTY(()) int_val;
    double GTY(()) double_val;
    char* GTY(()) string_val;  /* TYPE_STRING */
    struct my_struct* GTY(()) struct_ptr;  /* TYPE_POINTER */
};

/* TYPE_ARRAY: Array types */
typedef struct my_struct GTY(()) struct_array[10];
typedef int GTY(()) int_matrix[5][5];  /* Multi-dimensional array */
typedef union my_union GTY(()) union_array[8];

/* TYPE_POINTER: Various pointer types */
typedef void* GTY(()) void_ptr;
typedef int* GTY(()) int_ptr;
typedef struct my_struct** GTY(()) struct_ptr_ptr;  /* Pointer to pointer */
typedef int (*GTY(()) func_ptr)(int, int);  /* Function pointer for TYPE_CALLBACK */

/* TYPE_CALLBACK: Callback function pointer type */
typedef int GTY(()) (*comparator_fn)(const void*, const void*);

/* Struct containing callback */
struct GTY(()) callback_container {
    comparator_fn GTY(()) compare;
    void* GTY(()) data;
    int GTY(()) count;
};

/* TYPE_STRING: String type */
typedef char* GTY(()) gty_string;

/* String-specific struct */
struct GTY(()) string_container {
    char* GTY(()) name;  /* Treated as TYPE_STRING */
    const char* GTY(()) constant_string;
    gty_string GTY(()) dynamic_string;
};

/* Complex nested type demonstrating inter-dependencies */
struct GTY(()) complex_type {
    struct GTY(()) {
        int GTY(()) ref_count;
        struct complex_type* GTY(()) self_ptr;
    } header;
    
    union GTY(()) {
        struct my_struct GTY(()) data_struct;
        union my_union GTY(()) data_union;
        int GTY(()) data_array[20];
    } payload;
    
    enum GTY(()) {
        MODE_STRUCT,
        MODE_UNION,
        MODE_ARRAY
    } mode;
    
    /* Chain of structures */
    struct complex_type* GTY((chain_next)) chain_next;
    
    /* Array of pointers */
    void* GTY(()) ptr_array[5];
    
    /* Callback */
    comparator_fn GTY(()) sorter;
};

/* Container with length annotation */
struct GTY(()) variable_array {
    int GTY(()) count;
    int GTY((length("%0.count"))) data[1];  /* Variable length array */
};

/* Tagged union with desc annotation */
struct GTY(()) tagged_union_container {
    enum GTY(()) tag_type {
        TAG_INT,
        TAG_DOUBLE,
        TAG_STRING,
        TAG_STRUCT
    } tag;
    
    union GTY((desc("%0.tag"))) {
        int GTY(()) int_val;
        double GTY(()) double_val;
        char* GTY(()) string_val;
        struct my_struct GTY(()) struct_val;
    } value;
};

#endif /* TEST_TYPES_H */
