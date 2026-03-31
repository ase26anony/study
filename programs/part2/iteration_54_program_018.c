int i = 0;
while (i < 10) {
    if (some_condition) {
        i = 20; // skip loop — okay but drastic
    } else {
        i++;    // normal increment
    }
}
