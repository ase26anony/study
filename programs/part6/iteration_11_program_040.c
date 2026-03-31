/* Test header for gengtype-state.cc coverage */
#include "gtype-desc.h"

/* TYPE_SCALAR: Basic scalar types and enums */
typedef int scalar_int_t;
typedef double scalar_double_t;
enum color { RED, GREEN, BLUE };

/* TYPE_UNDEFINED: Type without GTY marker but referenced from annotated types */
struct undefined_struct {
    int x;
    char y;
};

/* TYPE_STRUCT: Basic struct with GTY marker */
struct GTY(()) basic_struct {
    int id;
    char name[32];
    double value;
};

/* TYPE_USER_STRUCT: Struct with user marker */
struct GTY((user)) user_struct {
    long user_id;
    char* username;
    void (*cleanup)(void*);
};

/* TYPE_UNION: Union type */
union GTY(()) data_union {
    int int_val;
    float float_val;
    char* string_val;
    double double_val;
};

/* TYPE_POINTER: Various pointer types */
typedef basic_struct* struct_ptr_t;
typedef int* int_ptr_t;
typedef void (*void_func_ptr)(void);

/* TYPE_ARRAY: Array types */
typedef int int_array_10[10];
typedef struct_ptr_t struct_ptr_array[5];

/* TYPE_STRING: String-like structure */
struct GTY(()) gcc_string {
    int length;
    int capacity;
    char* GTY((length("%0.length"))) data;
};

/* TYPE_CALLBACK: Function pointer types */
typedef int (*comparator_t)(const void*, const void*);
typedef void (*iterator_t)(void* data, void* user_data);

/* Complex nested structures to ensure deep traversal */

/* Linked list node - TYPE_STRUCT with TYPE_POINTER member */
struct GTY(()) list_node {
    int data;
    struct list_node* GTY((skip)) next;
    struct list_node* GTY((skip)) prev;
};

/* Tree node - TYPE_STRUCT with recursive pointers */
struct GTY(()) tree_node {
    int key;
    struct tree_node* GTY((skip)) left;
    struct tree_node* GTY((skip)) right;
    void* GTY((skip)) data;
};

/* Container with multiple type references */
struct GTY(()) type_container {
    /* TYPE_SCALAR members */
    int id;
    enum color color;
    
    /* TYPE_POINTER members */
    basic_struct* basic_ptr;
    struct undefined_struct* undefined_ptr;  /* TYPE_UNDEFINED reference */
    
    /* TYPE_ARRAY members */
    int numbers[20];
    tree_node* node_array[8];
    
    /* TYPE_UNION member */
    union data_union storage;
    
    /* TYPE_STRING member */
    struct gcc_string description;
    
    /* TYPE_CALLBACK member */
    comparator_t compare_func;
    
    /* Nested TYPE_STRUCT */
    struct GTY(()) nested {
        int x;
        int y;
        list_node* node_list;
    } inner;
    
    /* Pointer to union */
    union data_union* union_ptr;
    
    /* Array of function pointers */
    iterator_t GTY((skip)) iterators[4];
};

/* TYPE_LANG_STRUCT: Language-specific structure */
struct GTY(()) lang_struct {
    int lang_specific_tag;
    void* lang_data;
    
    /* Mark this as language-specific with chain_next */
    struct lang_struct* GTY((skip)) chain_next;
};

/* Root structure containing pointers to everything */
struct GTY(()) root_container {
    /* Direct struct instances */
    struct basic_struct basic;
    struct user_struct user;
    struct type_container container;
    
    /* Pointers to various types */
    struct list_node* head;
    struct tree_node* root;
    struct lang_struct* lang_chain;
    
    /* Arrays of different types */
    struct basic_struct struct_array[3];
    union data_union union_array[2];
    struct gcc_string strings[5];
    
    /* Function pointer */
    void (*traversal_func)(struct root_container*);
    
    /* Self-referential pointer */
    struct root_container* GTY((skip)) parent;
};

/* External declaration to force inclusion in type graph */
extern struct root_container GTY((tag("ROOT"))) global_root;

/* Callback function type used in traversal */
typedef void (*node_visitor_t)(struct tree_node* node, void* context);
