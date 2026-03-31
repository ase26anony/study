// Function matching the callback signature
void my_callback(int x, char c) {
    printf("Called with %d and %c\n", x, c);
}

int main() {
    struct my_struct s;
    
    s.field1 = 42;
    s.u.ptr = "Hello";  // Using the pointer member
    // OR
    // s.u.arr[0] = 100;  // Using the array member
    
    s.callback = my_callback;
    s.callback(10, 'A');  // Calls my_callback
    
    return 0;
}
