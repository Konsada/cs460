struct semaphore{
  int value;
  PROC *queue;
};
// wait
int P(struct semaphore *s)
{
  // write YOUR C code for P()
	// disable CPU interrupts
	int statusRegister = int_off();
	// decrement number of units available of the resource
	s->value--;
	if(s->value < 0){
		// set running proc to BLOCK
		running->status = BLOCK;
		// enqueue running proc into semaphore queue
		enqueue(&s->queue, running);
		// switch proc
		tswitch();
	}
	// turn on interrupts
	int_on(statusRegister);
}

// signal
int V(struct semaphore *s)
{
  // write YOUR C code for V()
	PROC *p;
	// turn off interrupts
	int statusRegister = int_off();

	s->value++;
	if(s->value <= 0){
		// get proc from semaphore queue
		p = dequeue(&s->queue);
		// set proc status to ready
		p->status = READY;
		// add proc to readyqueue
		enqueue(&readyQueue, p);
	}
	// turn interrupts on
	int_on(statusRegister);
}
