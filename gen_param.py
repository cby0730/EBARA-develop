import csv
import random
from itertools import product

def generate_combinations():
    ps_range = range(40, 101, 1)  # 40到100，步長為5
    ds_range = range(0, 101, 1)  # 0到100，步長為10
    dms_range = range(5, 11)      # 5到10
    cycle_range = range(1, 7)     # 1到6

    combinations = list(product(ps_range, ds_range, dms_range, cycle_range))
    random.shuffle(combinations)
    return combinations

def write_to_files(combinations):
    total = len(combinations)
    train_size = int(0.7 * total)
    test_size = int(0.15 * total)

    with open('train.csv', 'w', newline='') as train_file, \
         open('test.csv', 'w', newline='') as test_file, \
         open('valid.csv', 'w', newline='') as valid_file:

        train_writer = csv.writer(train_file)
        test_writer = csv.writer(test_file)
        valid_writer = csv.writer(valid_file)

        # 寫入標題行
        headers = ['filename', 'ps', 'ds', 'dms', 'cycle']
        train_writer.writerow(headers)
        test_writer.writerow(headers)
        valid_writer.writerow(headers)

        for i, (ps, ds, dms, cycle) in enumerate(combinations):
            filename = f"{i+1:06d}.csv"
            row = [filename, ps, ds, dms, cycle]

            if i < train_size:
                train_writer.writerow(row)
            elif i < train_size + test_size:
                test_writer.writerow(row)
            else:
                valid_writer.writerow(row)

if __name__ == "__main__":
    combinations = generate_combinations()
    write_to_files(combinations)
    print(f"總共生成了 {len(combinations)} 個組合")
    print("資料已分別寫入 train.csv, test.csv 和 valid.csv 檔案")