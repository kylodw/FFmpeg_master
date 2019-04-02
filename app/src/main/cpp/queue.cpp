#include <malloc.h>
#include <android/log.h>

#include "queue.h"

//
// Created by kylodw on 2019/4/1.
//队列
//
#define LOGE(format, ...)  __android_log_print(ANDROID_LOG_ERROR, "这是loge", format, ##__VA_ARGS__)
#define LOGI(format, ...)  __android_log_print(ANDROID_LOG_INFO,  "这是logi", format, ##__VA_ARGS__)
/*
 * 这里主要存放ACPacket的指针，至少需要两个队列示例，分别用来视频的av和音频的av
 * 队列
 *  Created by kylodw on 2019/4/1.
 */


/*
 * 初始化队列
 */
Queue *queue_init(int size, queue_fill_func queueFillFunc) {
    Queue *queue = (Queue *) malloc(sizeof(Queue));
    queue->size = size;
    queue->next_to_write = 0;
    queue->next_to_read = 0;
    queue->tab = (void **) malloc(sizeof(*queue->tab) * size);
    int i = 0;
    for (; i < size; ++i) {
        queue->tab[i] = queueFillFunc();
    }

    return queue;
}

/*
 * 销毁队列
 */
void queue_free(Queue *queue, queue_free_func free_func) {
    int i = 0;
    for (; i < queue->size; ++i) {
        free_func((void *) queue->tab[i]);
    }
    free(queue->tab);
    free(queue);
}

int queue_get_next(Queue *queue, int index) {
    return (index + 1) % queue->size;
}

/**
 * 入队
 * @param queue
 * @return
 */
void *queue_push(Queue *queue, pthread_mutex_t *mutex, pthread_cond_t *cond) {
    //这里报错
    int index = queue->next_to_write;
    int next_to_write;
    for (;;) {
        next_to_write = queue_get_next(queue, index);
        //下一个读的位置等于下一个写的位置，那就阻塞（写完在读）
        if (next_to_write != queue->next_to_read) {
            break;
        }
        //阻塞等待
        pthread_cond_wait(cond, mutex);
    }
    queue->next_to_write = next_to_write;
    //通知消费者线程
    pthread_cond_broadcast(cond);
    return queue->tab[index];
}

/*
 * 出队
 */
void *queue_pop(Queue *queue,pthread_mutex_t *mutex, pthread_cond_t *cond) {
    int current = queue->next_to_read;
    for (;;) {
        if (queue->next_to_read != queue->next_to_write) {
            break;
        }
        pthread_cond_wait(cond, mutex);
    }

    queue->next_to_read = queue_get_next(queue, current);
    //读完发送通知
    pthread_cond_broadcast(cond);
    return queue->tab[current];
}