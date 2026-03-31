#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* Include GCC's GTY marker macros */
#ifndef GTY
#define GTY(x) 
#endif

/* Forward declaration for TYPE_UNDEFINED */
struct opaque;

/* TYPE_SCALAR: Fundamental scalar types */
typedef enum GTY(()) color_enum {
    RED,
    GREEN,
    BLUE
} color_t;

/* TYPE_STRUCT: Basic struct with various members */
struct GTY(()) my_struct {
    int id;                     /* scalar */
    char name[32];              /* array */
    struct my_struct *next;     /* pointer with chain_next */
    union my_union *data;       /* pointer to union */
    float weight;               /* scalar */
    double score;               /* scalar */
    
    /* Bit-fields */
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    unsigned int : 4;           /* padding */
    
    /* Anonymous struct */
    struct GTY(()) {
        int x;
        int y;
    } point;
};

/* TYPE_USER_STRUCT: User-defined struct */
struct GTY((user)) my_user_struct {
    int custom_data;
    void (*user_callback)(void);  /* callback pointer */
};

/* TYPE_UNION: Tagged union */
union GTY(()) my_union {
    int int_value;
    float float_value;
    char *string_value;          /* string pointer */
    struct my_struct *struct_ptr;
    
    /* Anonymous union within union */
    union {
        long long_value;
        double double_value;
    } numeric;
};

/* TYPE_CALLBACK: Function pointer type */
typedef void (*callback_func_t)(int, const char*);

/* TYPE_STRUCT with callback */
struct GTY(()) struct_with_callback {
    int id;
    callback_func_t handler;     /* TYPE_CALLBACK */
    void (*alt_handler)(void);   /* Another callback */
};

/* TYPE_ARRAY: Various array types */
struct GTY(()) array_container {
    int scalar_array[10];                /* array of scalars */
    struct my_struct *ptr_array[5];      /* array of pointers */
    union my union_array[3][2];          /* 2D array of unions */
    callback_func_t callback_array[4];   /* array of callbacks */
};

/* TYPE_STRING: String types */
struct GTY(()) string_container {
    char * GTY((length("strlen(%h.string_field) + 1"))) string_field;
    const char * GTY((length("strlen(%h.const_string) + 1"))) const_string;
    char fixed_string[64];
};

/* TYPE_POINTER: Complex pointer relationships */
struct GTY(()) pointer_network {
    void *void_ptr;                      /* void pointer */
    struct my_struct **double_ptr;       /* pointer to pointer */
    struct opaque *opaque_ptr;           /* pointer to undefined */
    int (*func_ptr)(int, int);           /* function pointer */
    
    /* Chain of structures */
    struct pointer_network * GTY((chain_next("%h.next_chain"))) next_chain;
    struct pointer_network * GTY((chain_prev("%h.prev_chain"))) prev_chain;
};

/* TYPE_LANG_STRUCT will be defined in C++ file */

/* Global variables to force type instantiation */
extern struct my_struct global_struct;
extern union my_union global_union;
extern struct array_container global_array;
extern struct string_container global_strings;
extern struct pointer_network global_network;
extern struct struct_with_callback global_callback_struct;

#endif /* TEST_TYPES_H */
