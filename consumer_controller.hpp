#include <pthread.h>
#include <unistd.h>
#include <vector>
#include <iostream>
#include "consumer.hpp"
#include "ts_queue.hpp"
#include "item.hpp"
#include "transformer.hpp"
#include <chrono>
#include <iostream>
#include <fstream> // For file streams
#include <string>

#ifndef CONSUMER_CONTROLLER
#define CONSUMER_CONTROLLER


//std::vector<int> log;

class ConsumerController : public Thread {
public:
	// constructor
	ConsumerController(
		TSQueue<Item*>* worker_queue,
		TSQueue<Item*>* writer_queue,
		Transformer* transformer,
		int check_period,
		int low_threshold,
		int high_threshold
	);

	// destructor
	~ConsumerController();

	virtual void start();
	bool finished = false;

private:
	std::vector<Consumer*> consumers;

	TSQueue<Item*>* worker_queue;
	TSQueue<Item*>* writer_queue;

	Transformer* transformer;

	// Check to scale down or scale up every check period in microseconds.
	int check_period;
	// When the number of items in the worker queue is lower than low_threshold,
	// the number of consumers scaled down by 1.
	int low_threshold;
	// When the number of items in the worker queue is higher than high_threshold,
	// the number of consumers scaled up by 1.
	int high_threshold;

	static void* process(void* arg);

	std::chrono::time_point<std::chrono::high_resolution_clock> startTime;
};

// Implementation start

ConsumerController::ConsumerController(
	TSQueue<Item*>* worker_queue,
	TSQueue<Item*>* writer_queue,
	Transformer* transformer,
	int check_period,
	int low_threshold,
	int high_threshold
) : worker_queue(worker_queue),
	writer_queue(writer_queue),
	transformer(transformer),
	check_period(check_period),
	low_threshold(low_threshold),
	high_threshold(high_threshold) {
}

ConsumerController::~ConsumerController() {
	//std::cout << "~ConsumerController()" << std::endl;
}

void ConsumerController::start() {
	// TODO: starts a ConsumerController thread
	pthread_create(&t, 0, ConsumerController::process, (void*)this);
	startTime = std::chrono::high_resolution_clock::now();
}

void* ConsumerController::process(void* arg) {
	// TODO: implements the ConsumerController's work
	// Check Worker Queue status periodically. (The period is defined in main.cpp as CONSUMER_CONTROLLER_CHECK_PERIOD in microsecond)
	ConsumerController* consumerController = (ConsumerController*)arg;
	while ( !consumerController->finished ) {
		//std::cout << "controller\n";
		auto elapsed = std::chrono::high_resolution_clock::now() - consumerController->startTime;
		long long microseconds = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
		//std::cout << "microsecond: " << microseconds << "\n";
		
		if (microseconds > consumerController->check_period) {
			
			float ratio = (float)consumerController->worker_queue->get_size() / (float)consumerController->worker_queue->get_bufferSize();
			//std::cout << "ratio: " << ratio << "\n";
			int num = static_cast<int>(ratio*100); 
			//std::cout << "num: " << num << "\n";
			if (num >= consumerController->high_threshold) {
				// increase
				Consumer* consumerPtr = new Consumer(consumerController->worker_queue, consumerController->writer_queue, consumerController->transformer);
				consumerPtr->start();
				consumerController->consumers.push_back(consumerPtr);
				std::cout << "Scaling up consumers from " << consumerController->consumers.size()-1 << " to " << consumerController->consumers.size() << std::endl;
				//log.push_back(consumerController->consumers.size());
			}
			if (num <= consumerController->low_threshold) {
				// decrease
				if (consumerController->consumers.size() > 1) {
					Consumer* consumerPtr = consumerController->consumers.back();
					consumerController->consumers.pop_back();
					consumerPtr->cancel();
					std::cout << "Scaling down consumers from " << consumerController->consumers.size()+1 << " to " << consumerController->consumers.size() << std::endl;
					//log.push_back(consumerController->consumers.size());
				}
			}
			consumerController->startTime = std::chrono::high_resolution_clock::now();
		}
		
	}
	// std::string st = "";
	// for (int i=0; i<log.size(); i++) {
	// 	st +=  (std::to_string(log[i]) + ", ");
	// }
	// std::cout << st << std::endl;
	
	for (auto consumer : consumerController->consumers) {
		delete consumer;
	}
	
	return nullptr;
}

#endif // CONSUMER_CONTROLLER_HPP
