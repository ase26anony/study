if (x > 0) {
    x = -1;
    // Now x is negative, but the block executed because x was positive.
    // If code here assumes x > 0 still true, that’s wrong.
}
