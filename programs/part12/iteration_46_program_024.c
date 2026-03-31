union {
    struct type *pointer_to;           // TYPE_POINTER
    struct type *element_type;         // TYPE_ARRAY
    struct struct_type_info *struct_info; // TYPE_STRUCT/TYPE_USER_STRUCT
    struct union_type_info *union_info;   // TYPE_UNION
    struct callback_info *callback;    // TYPE_CALLBACK
    // ... other type-specific data
} u;
