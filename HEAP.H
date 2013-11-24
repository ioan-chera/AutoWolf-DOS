#ifndef HEAP_H_
#define HEAP_H_
extern word heap_size;
extern word heap[];
#define HEAP_BRUTE_ADD(n) heap[++heap_size] = (n);
void HeapInit();
void HeapPush(word n);
void Heapify(word i);
word HeapPop();
void HeapMake();
#endif
