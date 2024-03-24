# Pipelines

``` c++
int repeat;
int *buffer;
int pang_offset;

if (repeat > 0) {
  load(buffer);
}
sync();

if (repeat > 1) {
  load(buffer + pang_offset);
  compute(buffer);
}
sync();

for (int i = 0; i < repeat - 2; i++) {
  store(buffer + i % 2 * pang_offset);
  load(buffer + i % 2 * pang_offset);
  compute(buffer + (i + 1) % 2 * pang_offset);
  sync();
}

if (repeat > 1) {
  store(buffer + repeat % 2 * pang_offset);
}
if (repeat > 0) {
  compute(buffer + (repeat - 1) % 2 * pang_offset);
}
sync();

if (repeat > 0) {
  store(buffer + (repeat - 1) % 2 * pang_offset);
}
sync();

```