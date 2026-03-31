// Create an integer type
struct type int_type = {TYPE_SCALAR, NULL};

// Create a pointer to integer
struct type int_ptr_type = {TYPE_POINTER, NULL};
int_ptr_type.u.pointer_to = &int_type;

// Create an array of 10 integers
struct type int_array_type = {TYPE_ARRAY, NULL};
int_array_type.u.element_type = &int_type;
// Note: Would likely need array size field too
