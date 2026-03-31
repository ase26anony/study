#pragma omp parallel for
for (int j = 0; j < 4; j++) {
    int acc = init[j];
    int mul_val = mul[j];
    for (int i = 0; i < 5000; i++) {
        acc = (acc * mul_val) + input[i];
    }
    result[j] = acc;
}
