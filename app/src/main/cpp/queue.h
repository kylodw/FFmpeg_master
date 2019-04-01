//
// Created by kylodw on 2019/4/1.
//

#ifndef FFMPEG_MASTER_QUEUE_H
#define FFMPEG_MASTER_QUEUE_H

#endif //FFMPEG_MASTER_QUEUE_H
struct _Queue {
    //长度
    int size;
    //任意类型的指针数组，二维数组
    void **tab;
    //push或者pop元素时需要按照先后顺序依次进行
    int next_to_write;
    int next_to_read;

    int *ready;
};
typedef struct  _Queue Queue;
//释放队列中所占用内存函数
typedef void*(*queue_free_func)(void* elem);

Queue *queue_init(int size);
void queue_free(Queue *queue,queue_free_func free_func);
int queue_get_next(Queue *queue,int index);
void* queue_push(Queue* queue);
void* queue_pop(Queue* queue);
