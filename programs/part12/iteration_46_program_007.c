union {
    struct type *pointer_to;        // For TYPE_POINTER
    struct type *element_type;      // For TYPE_ARRAY
    struct struct_type *struct_def; // For TYPE_STRUCT/TYPE_USER_STRUCT
    struct union_type *union_def;   // For TYPE_UNION
    struct callback_info *callback; // For TYPE_CALLBACK
    // For TYPE_SCALAR: might need scalar type info (int, float, etc.)
    // For TYPE_STRING: might need length info or encoding
} u;
