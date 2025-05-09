# broadcast
```cpp
#include<iostream>
#include<cstring>
#include<functional>
#include<numeric>
#include<cstdlib>
#include<fstream>
#include<algorithm>

using namespace std;
#define DIM 8

void mem(int *dst, int *src, int stride, int len1, int repeat) {
  for (int i = 0; i < repeat; i++) {
    memcpy(dst + i * stride, src, len1 * sizeof(int));
  }
}

int getSize(int *p, int len) {
  int ans = 1;
  for (int i = 0; i < len; i++) {
    ans *= p[i];
  }
  return ans;
}

void init(int *p, int len) {
  for (int i = 0; i < len; i++) {
    p[i] = i % 7;
  }
}

void broadcast(int *data, int *dst_dims, int *src_dims) {
  int bk = 1;
  int BK = 1;
  for (int i = 0; i < DIM; i++) {
    bk *= src_dims[i];
    BK *= dst_dims[i];
    if (bk < BK) {
      for (int j = getSize(src_dims + i + 1, DIM - (i + 1)) - 1; j >= 0; j--) {
        mem(data + j * BK, data + j * bk, bk, bk, dst_dims[i]);
      }
    }
    bk = BK;
  }
}

int main() {

  int s1[] = {3, 1, 4, 4};
  int s2[] = {3, 2, 4, 4};

  int src[DIM] = {1, 1, 1, 1, 1, 1, 1, 1};
  int dst[DIM] = {1, 1, 1, 1, 1, 1, 1, 1};

  int len1 = sizeof(s1) / sizeof(int);
  int len2 = sizeof(s2) / sizeof(int);

  int index = 0;
  for (int i = len1 - 1; i >= 0; i--) {
    src[index++] = s1[i];
  }
  index = 0;
  for (int i = len2 - 1; i >= 0; i--) {
    dst[index++] = s2[i];
  }
  for (int i = 0; i < DIM; i++) {
    printf("%d ", src[i]);
  }
  printf("\n");
  for (int i = 0; i < DIM; i++) {
    printf("%d ", dst[i]);
  }
  printf("\n");

  int size1 = std::accumulate(src, src + DIM, 1, std::multiplies<int>());
  int size2 = std::accumulate(dst, dst + DIM, 1, std::multiplies<int>());
  std::cout << "size1:" << size1 << " size2:" << size2 << "\n";
  int *data = new int[size2];
  int *base = new int[size2];
  init(data, size1);
  init(base, size1);

  broadcast(data, dst, src);

  ofstream outFile("src.bin", ios::out | ios::binary);
  outFile.write((char *)base, size1 * sizeof(int));
  outFile.close();
  system("python diff.py");

  int flag = 1;
  ifstream inFile("dst.bin", ios::in | ios::binary);
  inFile.read((char *)base, size2 * sizeof(int));
  for (int i = 0; i < size2; i++) {
    if (base[i] != data[i]) {
      flag = 0;
      printf("Error at %d\n", i);
      break;
    }
  }
  flag == 1 ? printf("All Passed\n") : 0;
  delete[] data;
  delete[] base;
}
```

diff.py文件
```python
import numpy as np
with open('src.bin') as f:
  src = np.fromfile(f, dtype = np.int32, count = 48).reshape(3,1,4,4)
  # print(src)
  dst = np.zeros((3,2,4,4)).astype(np.int32) + src
  # print(dst)
  dst.tofile('dst.bin')
```