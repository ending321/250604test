#include<stdio.h>

//任务节点结构
typedef struct Task{
	void(*function) (void*); //任务函数指针
	void* arg;		//任务参数
	struct Task* next;	//指向下一任务的指针
}Task;

//线程池结构
typedef struct{
	pthread_t* threads;	//线程数组
	Task* queue_head;	//任务队列头指针
	Task* queue_tail;	//任务队列尾指针
	int thread_count;	//当前线程数量
	int queue_size;		//任务队列大小
	int shutdown; 		//关闭标志
	int started;		//已启动线程数
	pthread_mutex_t lock;	//互斥锁
	pthread_cond_t notify;	//条件变量
}ThreadPool;

int main(void){
	
	//创建线程池，包含4个工作线程
	Thread* pool = thread_pool_reate(4);
	return 0;
}
