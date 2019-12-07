'''
Written by Kuan Yu to illustrate how quickly can a dataset file be parsed.
'''

import os

res = []

for root, dir, files in os.walk("data"):
    for filename in files:
        if filename.endswith(".csv"):
            filepath = root + "/" + filename
            with open(filepath) as lines:
                for line in lines:
                    res.append(line)

print(len(res))
