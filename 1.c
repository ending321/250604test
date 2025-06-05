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

//线程执行函数
static void* thread_worker(void *arg);

//ThreadPool* thread_pool_create(int thread_count);
//int thread_pool_add_task(ThreadPool* pool, void(*function)(void*), void* arg);
//static void* thread_worker(void* arg);
//int thread_pool_destroy(ThreadPool* pool);

//创建线程池
ThreadPool* thread_pool_create(int thread_count){
	ThreadPool *pool = (ThreadPool*)malloc(sizeof(threadPool));
	if(!pool){
	    frpintf(stderr,"Error:无法分配线程池内存\n");
	    return NULL;
	}
	//初始化线程池参数
	pool->thread_count = thread_count;
	pool->queue_head = NULL;
	pool->queue_tail = NULL;
	pool->queue_size = 0;
	pool->shutdown = 0;
	pool->started = 0;

	//初始化锁和条件变量
	if(pthread_mutex_init(&pool->lock, NULL) != 0 || pthread_cond_init(&pool->notify != 0)){
	    fprintf(stderr,"Error:初始化锁或条件变量失败\n");
	    free(pool);
	    return NULL;
	}

	//分配线程数组
	pool->threads = (pthread_t*)malloc(sizeof(pthread_t)*thread_count);
	if(!pool->threads){
	    fprintf(stderr,"Error:无法分配线程数组内存\n");
	    free(pool);
	    return NULL;
	}

	//创建工作线程
	for(int i=0; i<thread_count; i++){
	    if(pthread_create(&pool->threads[i],NULL, thread_worker,(void*)pool)!=0){
	        fprintf(stderr,"Error:创建线程%d失败\n", i);
		return NULL;
	    }
	    pool->start++;
	}
	return pool;
}

//向线程池添加任务
int thread_pool_add_task(ThreadPool* pool, void(*function)(void*), void* arg){
	if(!pool || !function) return -1;

	//加锁
	if(pthread_mutex_lock(&pool->lock)!=0) return -1;

	//如果线程池已关闭，则解锁并返回错误
	if(pool->shutdown){
	    pthread_mutex_unlock(&pool->lock);
	    return -1;
	}

	//创建新任务
	*Task task = (Task*)malloc(sizeof(Task));
	if(!task){
	    pthread_mutex_unlock(&pool->lock);
	    return -1;
	}

	task->function = function;
	task->arg = arg;
	task->next = NULL;

	//将任务添加到队列
	if(!pool->queue_head){
	    pool->queue_head = pool->queue_tail = task;
	}else{
	    pool->queue_tail->next = task;
	    pool->queue_tail = task;
	}
	pool->queue_size++;

	//通知等待的线程
	if(pthread_cond_signal(&pool->notify)!=0){
	    pthread_mutex_unlock(&pool->lock);
	    free(task);
	    return -1;
	}

	//解锁
	if(pthread_mutex_unlock(&pool->lock)!=0) return -1;

	return 0;
}

//线程工作函数
static void* thread_worker(void* arg){
	ThreadPool *pool = (ThreadPool*)arg;
	Task *task = NULL;

	while(1){
	    //加锁
	    pthread_mutex_lock(&pool->lock);

	    //等待任务或关闭信号
	    while(!pool->queue_size && !pool->shutdown){
	        pthread_cond_wait(&pool->notify, &pool->lock);
	    }

	    //如果线程池已关闭且队列为空，则退出
	    if(pool->shutdown && !pool->queue_size) break;

	    //从队列中取出任务
	    task = pool->queue_head;
	    if(task){
	        pool->queue_head = task->next;
		if(!pool->queue_head) pool->queue_tail = NULL;
		pool->queue_size--;
	    }

	    //解锁
	    pthread_mutex_unlock(&pool->lock);

	    //如果取出了任务，则执行它
	    if(task){
	        task->function(task->arg);
		free(task);
	    }
	}

	//减少活跃线程计数
	pool->started--;
	pthread_mutex_unlock(&pool->lock);
	pthread_exit(NULL);
	return NULL;
}

//销毁线程池
int thread_pool_destroy(ThreadPool* pool){
	if(!pool) return -1;

	//加锁
	if(pthread_mutex_lock(&pool->lock) != 0) return -1;

	//如果线程池已关闭，则直接返回
	if(pool->shutdown){
	    pthread_mutex_unlock(&pool->lock);
	    return -1;
	}

	//设置关闭标志
	pool->shutdown = 1;

	//解锁并通知所有线程
	if(pthread_mutex_unlock(&pool->lock)!=0) return -1;
	if(pthread_cond_broadcast(&pool->notify) != 0) return -1;

	//等待所有线程结束
	for(int i = 0; i < pool->thread_count; i++){
	    if(pthread_join(pool->threads[i], NULL)!=0){
	        return -1;
	    }
	}

	//清理资源
	pthread_mutex_destroy(&pool->lock);
	pthread_cond_destroy(&pool->notify);
	free(pool->threads);

	//清理任务队列
	while(pool->queue_head){
	    Task *temp = pool->queue_head;
	    pool->queue_head = temp->next;
	    free(temp);
	}
	free(pool);
	return 0;
}

//实例任务函数
void example_task(void* arg){
	int task_id = *(int*)arg;
	printf("执行任务 #%d\n", task_id);
	//模拟任务执行时间
	sleep(1);
	free(arg);
}

int main(void){
	
	//创建线程池，包含4个工作线程
	Thread* pool = thread_pool_create(4);
	if(!pool){
	    fprintf(stderr, "Error:创建线程池失败\n");
	    return 1;
	}

	//添加10个任务到线程池
	for(int i=0; i<10; i++){
	    int* task_id = (int*)malloc(sizeof(int));
	    *task_id = i;
	    if(pthread_pool_add_task(pool, example_task, task_id)!=0){
	        fprintf(stderr, "Error:添加任务#%d失败\n");
		free(task_id);
	    }
	}

	//销毁线程池
	if(thread_pool_destroy(pool)!=0){
	    fprintf(stderr, "Error:销毁线程池失败\n");
	    return 1;
	}
	printf("所有任务执行完毕,线程池已销毁");
	return 0;
}
