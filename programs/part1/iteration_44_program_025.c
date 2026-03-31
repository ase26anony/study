struct my_struct s;

// Using the union as a pointer
s.u.ptr = malloc(100);
strcpy(s.u.ptr, "Hello");

// OR using the union as an array
for (int i = 0; i < 10; i++) {
    s.u.arr[i] = i * 10;
}

// Setting the callback function
void my_callback(int x, char c) {
    printf("Callback: %d, %c\n", x, c);
}
s.callback = my_callback;

// Calling the callback
s.callback(42, 'A');
