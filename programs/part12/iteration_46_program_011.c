// Creating a pointer to int
struct type *int_type = create_scalar_type();
struct type *int_ptr = create_pointer_type(int_type);

// Creating an array of strings
struct type *string_type = create_string_type();
struct type *string_array = create_array_type(string_type);

// Linked list of types (parameter list for a function)
struct type *param1 = create_scalar_type();
struct type *param2 = create_string_type();
param1->next = param2;
