#include "gt_context.h"

ucontext_t scheduler;
ucontext_t main_context;
char schedule_stack[STACKSIZE];
static bool allthreadsfinished = false;
void threadCtrl( void *func(), void *arg ) {

	#ifdef DEBUG
		printf("THREADCTRL\n\tactiveThreadCount %d\n", activeThreadCount);
	#endif

	threadList[currentThread].active = true;
	threadList[currentThread].returnval = func(arg);

	if( !threadList[currentThread].exited ){
		threadList[currentThread].finished = true;
		threadList[currentThread].active = false;
	}

#ifdef DEBUG
	printf("\tThread Ctrl swapping to schedule\n");
#endif

	setcontext( &scheduler );
	return;
}

void init_thread_structs(){
	int i;

	for( i=0; i<max; i++ ){
		threadList[i].active = false;
		threadList[i].finished = false;
		threadList[i].deleted = false;
		threadList[i].exited = false;
		threadList[i].threadid = -1;
	}
	return;
}

void make_sched_context(){

	if( !getcontext(&scheduler) ){
		scheduler.uc_stack.ss_sp = malloc( STACKSIZE );
		scheduler.uc_stack.ss_size = STACKSIZE;
		scheduler.uc_stack.ss_flags = 0;
		scheduler.uc_link = NULL;

#ifdef DEBUG
		printf("\n\tSched Context is %ld\n", scheduler.uc_stack.ss_sp );
#endif

		makecontext( &scheduler, (void(*)(void)) schedule, 0 );
	}

	return;
}

void make_main_context(){

	if( !getcontext( &main_context ) ){
		main_context.uc_link = NULL;
		main_context.uc_stack.ss_sp = malloc(STACKSIZE);
		main_context.uc_stack.ss_size = STACKSIZE;
		main_context.uc_stack.ss_flags = 0;
	}

	if( main_context.uc_stack.ss_sp == 0 ){
		printf( "Error: Could not allocate stack" );
		exit(-1);
	}

#ifdef DEBUG
	printf("\n\tMain Context is %ld\n", main_context.uc_stack.ss_sp );
#endif

	return;
}

int make_thread_context( gtthread_t *thread, void *(*start_routine)(void *), void *arg ){

	generate_thread_id( thread );

	getcontext( &threadList[index].context );
	threadList[index].threadid = *thread;
	threadList[index].context.uc_link = &scheduler;
	threadList[index].context.uc_stack.ss_sp = malloc(STACKSIZE);
	threadList[index].context.uc_stack.ss_size = STACKSIZE;
	threadList[index].context.uc_stack.ss_flags = 0;

	if( threadList[index].context.uc_stack.ss_sp == 0 ) {
		printf( "Error: Could not allocate stack.");
		return -1;
	}

#ifdef DEBUG
	printf("\nMAKE THREAD CONTEXT:\n\t Allocating stack for thread %ld with context %ld\n", index, threadList[index].context.uc_stack.ss_sp );
#endif

	makecontext( &threadList[index].context, (void(*)(void)) threadCtrl, 2, start_routine, arg );


	return 0;
}

void generate_thread_id( gtthread_t *thread ){
	int i;
	static bool unique = false;
	do{
		*thread = rand(); //Generate new thread ID and assign.
		for( i=0; i<max; i++ ){
			if( gtthread_equal(threadList[i].threadid, *thread ) ){
				break;
			}
			else unique = true;
		}
	}while(!unique);
	return;
}

void schedule() {
	// allthreadsfinished is a global variable, which is set to 1, if the thread function has finished
	while( !allthreadsfinished ){

#ifdef DEBUG
			printf("schedule\n\tCurrent thread %d\n", currentThread);
#endif


		if( qtyThreads != 0 ){
					atomic_increment( &currentThread );
					atomic_modulus( &currentThread, &qtyThreads );
		}

#ifdef DEBUG
			printf("\tCurrent thread after mod %d\n", currentThread);
			printf("\tallthreadsfinished %d\n", allthreadsfinished);
			printf("\tactiveThreads %d\n", activeThreadCount);
			printf("\tnumthreadscreated %d\n", qtyThreads);
			printf("\tcurrentThread %d\n", currentThread);
#endif

		if( activeThreadCount==0 )
			allthreadsfinished = true;


		if( !threadList[currentThread].finished && !threadList[currentThread].exited ){

			swapcontext( &scheduler, &threadList[currentThread].context );
		}
		else{
			gtthread_cancel(threadList[currentThread].threadid);
		}

	}

#ifdef DEBUG
	printf("All threads finished\n");
#endif
}
