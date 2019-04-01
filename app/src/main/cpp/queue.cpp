#include <malloc.h>
#include "queue.h"

//
// Created by kylodw on 2019/4/1.
//队列
//

/*
 * 这里主要存放ACPacket的指针，至少需要两个队列示例，分别用来视频的av和音频的av
 * 队列
 *  Created by kylodw on 2019/4/1.
 */


/*
 * 初始化队列
 */
Queue *queue_init(int size) {
    Queue *queue = (Queue*)malloc(sizeof(Queue));
    queue->size = size;
    queue->next_to_write = 0;
    queue->next_to_read = 0;
    queue->tab = (void**)malloc(sizeof(*queue->tab) * size);
    int i = 0;
    for (; i < size; ++i) {
        queue->tab[i] = malloc(sizeof(*queue->tab));
    }

    return queue;
}

/*
 * 销毁队列
 */
void queue_free(Queue *queue,queue_free_func free_func) {
    int i = 0;
    for (; i < queue->size; ++i) {
        free_func((void*)queue->tab[i]);
    }
    free(queue->tab);
    free(queue);
}

int queue_get_next(Queue *queue,int index){
    return (index+1)%queue->size;
}
/**
 * 入队
 * @param queue
 * @return
 */
void* queue_push(Queue* queue){
   int index= queue->next_to_write;
   queue->next_to_write=queue_get_next(queue,index);
    return queue->tab[index];
}

/*
 * 出队
 */
void* queue_pop(Queue* queue){
    int current=queue->next_to_read;
    queue->next_to_read=queue_get_next(queue,current);
    return queue->tab[current];
}