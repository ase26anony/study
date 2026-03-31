union {
    struct type *pointer_to;           // TYPE_POINTER
    struct type *element_type;         // TYPE_ARRAY
    struct struct_type_info *struct_info;  // TYPE_STRUCT/TYPE_USER_STRUCT
    struct union_type_info *union_info;    // TYPE_UNION
    struct callback_info *callback;    // TYPE_CALLBACK
    // For TYPE_SCALAR: might have scalar_kind field
    // For TYPE_STRING: might have length info
} u;
