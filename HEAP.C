#include "wl_def.h"

// Min heap: it uses minimum size root

word heap_size;
word heap[MAPSIZE*MAPSIZE];

void HeapInit()
{
	heap_size = 1;
	heap[0] = 0;	// dummy element
}

void HeapPush(word n)
{
	word aux;
	word half_path;
	heap[++heap_size] = n;
	while(heap[heap_size] < heap[half_path = heap_size / 2])
		SWAP(heap[heap_size], heap[half_path], aux)
}

void Heapify(word i)
{
	word left;
	word right;
	word smallest;
	word aux;
	boolean found;
	do
	{
		found = false;
		left = 2 * i;
		right = left + 1;
		smallest = i;
		if(left <= heap_size && heap[left] < heap[smallest])
			smallest = left;
		if(right <= heap_size && heap[right] < heap[smallest])
			smallest = right;
		if(smallest != i)
		{
			SWAP(heap[i], heap[smallest], aux)
			found = true;
			i = smallest;
		}
	} while(found);
}

word HeapPop()
{
	word root = heap[1];
	heap[1] = heap[heap_size--];
	Heapify(1);
	return root;
}

void HeapMake()
{
	word i;
	for(i = heap_size / 2; i >= 1; --i)
		Heapify(i);
}