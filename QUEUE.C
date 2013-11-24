#include "wl_def.h"
#pragma hdrstop
#include "queue.h"

word far queue[QUEUE_SIZE];
word queue_head, queue_tail;

void QueuePush(word n)
{
	queue[queue_head] = n;
	queue_head = (queue_head + 1) % QUEUE_SIZE;
}

word QueuePop()
{
	word ret = queue[queue_tail];
	queue_tail = (queue_tail + 1) % QUEUE_SIZE;
	return ret;
}
