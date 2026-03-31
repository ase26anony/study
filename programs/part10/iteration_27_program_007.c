for (int j = 0; j < 4; j++) {
    int mul_val = mul[j];
    int acc = init[j];
    for (int i = 0; i < 5000; i++) {
        acc = (acc * mul_val) + input[i];
    }
    result[j] = acc;
}
