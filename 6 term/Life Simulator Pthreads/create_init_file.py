

N = 200

with open('init_file_int.txt', 'w') as f:
    for i in range(0, N):
        for j in range(0, N):
            f.write('0')
        f.write('\n')
