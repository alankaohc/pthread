#include <pthread.h>
#include "thread.hpp"
#include "ts_queue.hpp"
#include "item.hpp"
#include "transformer.hpp"

#ifndef PRODUCER_HPP
#define PRODUCER_HPP

class Producer : public Thread {
public:
	// constructor
	Producer(TSQueue<Item*>* input_queue, TSQueue<Item*>* worker_queue, Transformer* transfomrer);

	// destructor
	~Producer();

	virtual void start();
	bool finished = false;
private:
	TSQueue<Item*>* input_queue;
	TSQueue<Item*>* worker_queue;

	Transformer* transformer;

	// the method for pthread to create a producer thread
	static void* process(void* arg);
};

Producer::Producer(TSQueue<Item*>* input_queue, TSQueue<Item*>* worker_queue, Transformer* transformer)
	: input_queue(input_queue), worker_queue(worker_queue), transformer(transformer) {
}

Producer::~Producer() {
	//std::cout << "~Producer()" << std::endl;
}

void Producer::start() {
	// TODO: starts a Producer thread
	pthread_create(&t, 0, Producer::process, (void*)this);
}

void* Producer::process(void* arg) {
	// TODO: implements the Producer's work
	Producer* producer = (Producer*)arg;
	while ( !producer->finished ) {
		// Take an Item from Input Queue
		Item* item = producer->input_queue->dequeue(); 
		// Use Transformer::producer_transform to perform transform on the Item's value
		item->val = producer->transformer->producer_transform(item->opcode, item->val);
		// Put the Item with new value into the Worker Queue
		producer->worker_queue->enqueue(item);
	} 
	return nullptr;
}

#endif // PRODUCER_HPP
