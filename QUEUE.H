#ifndef QUEUE_H_
#define QUEUE_H_
#define QUEUE_SIZE (MAPSIZE*MAPSIZE)
extern word queue[];
extern word queue_head, queue_tail;
#define QUEUE_PUSH(n) {queue[queue_head++] = (n); queue_head %= QUEUE_SIZE;}
word QueuePop();
#define QUEUE_RESET queue_head = queue_tail;
#define QUEUE_EMPTY (queue_head == queue_tail)
#endif
 