/* Test header for gengtype-state.cc coverage */
#ifndef GTY_TEST_H
#define GTY_TEST_H

#include "gtype-desc.h"

/* TYPE_SCALAR: Basic scalar types */
typedef int scalar_int_t;
typedef double scalar_double_t;

/* Enum type (also scalar) */
enum color { RED, GREEN, BLUE };

/* TYPE_STRING: String-like structure */
struct GTY(()) gcc_string {
    int length;
    char *GTY((length("%0.length"))) data;
};

/* TYPE_ARRAY: Array typedef */
typedef int vec4_t[4];
typedef struct gcc_string* string_array_t[10];

/* TYPE_CALLBACK: Function pointer type */
typedef int (*callback_t)(void* data, int param);
typedef void (*simple_callback_t)(void);

/* Non-annotated struct (may become TYPE_UNDEFINED) */
struct unannotated_struct {
    int x;
    double y;
    char* name;
};

/* TYPE_STRUCT: Basic annotated struct */
struct GTY(()) basic_struct {
    int id;
    double value;
    enum color color;
    char* GTY((tag("0"))) name;
};

/* TYPE_USER_STRUCT: User-defined struct with special handling */
struct GTY((user)) user_struct {
    void* GTY((skip)) opaque_data;
    int user_id;
    callback_t user_callback;
};

/* TYPE_UNION: Union type */
union GTY(()) data_union {
    int int_val;
    double double_val;
    char* string_val;
    struct basic_struct* struct_ptr;
};

/* TYPE_POINTER: Pointer-only struct */
struct GTY(()) pointer_container {
    struct basic_struct* GTY((tag("1"))) struct_ptr;
    struct user_struct* user_ptr;
    struct unannotated_struct* unannotated_ptr;
    int* int_ptr;
    callback_t* callback_ptr;
    union data_union* union_ptr;
};

/* Complex struct with nested arrays */
struct GTY(()) complex_struct {
    int matrix[3][3];
    struct basic_struct* items[5];
    union data_union variants[8];
    vec4_t vector;
    string_array_t strings;
};

/* Linked list structure (for traversal) */
struct GTY(()) list_node {
    int data;
    struct list_node* GTY((tag("0"))) next;
    struct list_node* GTY((tag("1"))) prev;
};

/* Container with function pointer */
struct GTY(()) callback_container {
    callback_t handler;
    simple_callback_t cleanup;
    void* GTY((tag("2"))) user_data;
};

/* Mixed array types */
struct GTY(()) array_container {
    int fixed_array[20];
    struct basic_struct* ptr_array[10];
    union data_union union_array[5];
    callback_t callback_array[3];
};

/* TYPE_LANG_STRUCT: Language-specific structure */
struct GTY(()) lang_struct {
    int lang_specific_tag;
    void* GTY((length("0"))) lang_data;
    struct GTY((desc("%1.lang_specific_tag"))) lang_struct* next;
};

/* Root structure containing pointers to everything */
struct GTY(()) root_container {
    struct basic_struct* basic;
    struct user_struct* user;
    struct pointer_container* pointers;
    struct complex_struct* complex;
    struct list_node* list_head;
    struct callback_container* callbacks;
    struct array_container* arrays;
    struct lang_struct* lang;
    union data_union current_union;
    struct gcc_string* current_string;
    struct unannotated_struct* unannotated;
    
    /* Direct scalar members */
    scalar_int_t counter;
    scalar_double_t total;
    enum color default_color;
    
    /* Arrays */
    vec4_t position;
    string_array_t messages;
    
    /* Callback */
    callback_t notify;
};

/* External declaration for gengtype to process */
extern struct root_container GTY((tag("0"))) global_root;

#endif /* GTY_TEST_H */
