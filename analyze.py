lst = ['1/30', '1/60', '1/90', '1/120',
       '2/30', '2/60', '2/90', '2/120']

total = 0
count = 0

for filename in lst:
    filename = filename + '.recv.csv'
    for line in open(filename).read().strip().split('\n'):
        line = line.split(',')
        t = float(line[1]) - float(line[0])
        total += t
        count += 1
    print filename, count, total/count