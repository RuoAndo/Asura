import numpy as np
import matplotlib.pyplot as plt

data_set = np.loadtxt(
    fname="tmp",
    dtype="float",
    delimiter=",",
)

for data in data_set:
    plt.scatter(data[0], data[1])

plt.title("packets")
plt.xlabel("counts")
plt.ylabel("bytes")
plt.grid()

plt.show()
