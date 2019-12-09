#!/usr/bin/env python3


import matplotlib.pyplot as plt


ax = [1229.73, -3182.52, -6928.13, 403.978, 993.378, 109.576, -4674.26, -5680.47, -1996.08, -3875.75, -3247.87]
ay = [-1336.69, -740.99, 3798.77, -5015.29, -1541.38, -3571.93, 4113.23, 3572.93, -2898.26, 1056.86, -783.336]
bx = [814.901, 10178, 5179.07, 1342.15, 2587.43, 1446.85, 503.118, 3308.18, 1488.74, 1229.73]
by = [-523.475, 5648.14, 350.824, -3622.78, 4883.57, 4569.54, -5340.67, -1008.2, -1610.87, -1336.69]


def plot():
    plt.scatter( ax, ay, color='blue' )
    plt.scatter( bx, by, color='green' )
    plt.show()



if __name__ == '__main__':
    plot()