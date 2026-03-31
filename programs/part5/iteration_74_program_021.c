#ifndef TEST_GTY_H
#define TEST_GTY_H

/* For TYPE_UNDEFINED - incomplete/forward declaration */
struct opaque;

/* Basic scalar types */
typedef int my_scalar_t;
typedef double my_double_t;

/* For TYPE_CALLBACK - function pointer type */
typedef void (*callback_t)(int param);
typedef void (*complex_callback_t)(struct opaque*, int);

/* For TYPE_USER_STRUCT */
struct GTY((user)) user_struct {
    int user_data;
    void* user_pointer;
};

/* For TYPE_STRUCT with various members */
struct GTY(()) base_struct {
    int scalar_field;              /* TYPE_SCALAR */
    double double_field;           /* TYPE_SCALAR */
    char* string_field;            /* TYPE_STRING */
    callback_t callback_field;     /* TYPE_CALLBACK */
};

/* For TYPE_UNION */
union GTY(()) data_union {
    int as_int;
    double as_double;
    char* as_string;
    struct base_struct* as_struct;
    callback_t as_callback;
};

/* For TYPE_ARRAY */
struct GTY(()) array_container {
    int fixed_array[10];           /* Fixed-size array */
    struct base_struct* ptr_array[5]; /* Array of pointers */
    int* dynamic_array GTY((length("dynamic_len")));
    int dynamic_len;
};

/* Nested structure with pointers */
struct GTY(()) nested_struct {
    struct base_struct* base_ptr;  /* TYPE_POINTER to TYPE_STRUCT */
    union data_union data;         /* TYPE_UNION */
    struct opaque* opaque_ptr;     /* TYPE_POINTER to TYPE_UNDEFINED */
    struct nested_struct* next;    /* TYPE_POINTER (linked list) */
    int matrix[3][4];              /* Multi-dimensional array */
};

/* For TYPE_LANG_STRUCT - mimicking GCC internal structure */
struct GTY(()) lang_tree_node {
    int code;
    union {
        int int_value;
        double real_value;
        struct lang_tree_node* node_ptr;
    } GTY((desc("code"))) u;
    struct lang_tree_node* children[2];
};

/* Complex top-level structure containing all types */
struct GTY(()) top_level {
    /* Basic types */
    int id;                        /* TYPE_SCALAR */
    char* name;                    /* TYPE_STRING */
    
    /* Structures and unions */
    struct base_struct base;       /* TYPE_STRUCT */
    union data_union current_data; /* TYPE_UNION */
    
    /* Pointers */
    struct nested_struct* nested;  /* TYPE_POINTER to TYPE_STRUCT */
    struct user_struct* user;      /* TYPE_POINTER to TYPE_USER_STRUCT */
    struct lang_tree_node* lang;   /* TYPE_POINTER to TYPE_LANG_STRUCT */
    
    /* Arrays */
    struct array_container arrays; /* TYPE_STRUCT containing TYPE_ARRAY */
    int scores[20];                /* TYPE_ARRAY of TYPE_SCALAR */
    
    /* Callbacks */
    callback_t handler;            /* TYPE_CALLBACK */
    complex_callback_t complex_handler; /* TYPE_CALLBACK with params */
    
    /* Self-referential pointer */
    struct top_level* next;        /* TYPE_POINTER */
    
    /* Pointer to undefined type (forward declared) */
    struct opaque* future;         /* TYPE_POINTER to TYPE_UNDEFINED */
};

/* Now define the previously opaque structure */
struct GTY(()) opaque {
    int revealed;
    struct top_level* connection;
    struct opaque* chain;
};

/* Root variable for gengtype to trace */
extern GTY(()) struct top_level* global_root;

/* Additional global variables to ensure processing */
extern GTY(()) struct nested_struct* global_list;
extern GTY(()) union data_union global_union;
extern GTY(()) struct lang_tree_node* global_lang_node;
extern GTY(()) callback_t global_callback;

#endif /* TEST_GTY_H */
