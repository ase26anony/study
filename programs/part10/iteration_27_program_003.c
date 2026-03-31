acc_j = init[j]
for i = 0..4999:
    acc_j = acc_j * mul[j] + input[i]
result[j] = acc_j
