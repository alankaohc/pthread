#include <assert.h>
#include <stdlib.h>
#include "ts_queue.hpp"
#include "item.hpp"
#include "reader.hpp"
#include "writer.hpp"
#include "producer.hpp"
#include "consumer_controller.hpp"
#include <iostream>
#include <chrono>

#define READER_QUEUE_SIZE 200
#define WORKER_QUEUE_SIZE 200
#define WRITER_QUEUE_SIZE 4000
#define CONSUMER_CONTROLLER_LOW_THRESHOLD_PERCENTAGE 20
#define CONSUMER_CONTROLLER_HIGH_THRESHOLD_PERCENTAGE 80
#define CONSUMER_CONTROLLER_CHECK_PERIOD 1000000


int main(int argc, char** argv) {
	
	//auto startTime = std::chrono::high_resolution_clock::now();
	assert(argc == 4);
	
	int n = atoi(argv[1]);                 // 總共多少行的資訊要處理
	std::string input_file_name(argv[2]);  // 輸入的檔案名稱
	std::string output_file_name(argv[3]); // 輸出的檔案名稱
	// TODO: implements main function	
	// 準備好3個queue
	TSQueue<Item*>* input_queue = new TSQueue<Item*>(READER_QUEUE_SIZE);
	TSQueue<Item*>* worker_queue = new TSQueue<Item*>(WORKER_QUEUE_SIZE);
	TSQueue<Item*>* output_queue = new TSQueue<Item*>(WRITER_QUEUE_SIZE);
	// 準備好轉換的方式
	Transformer transformer;
	// 準備好不同的threads
	Reader reader(n, input_file_name, input_queue);
	Producer producer1(input_queue, worker_queue, &transformer);
	Producer producer2(input_queue, worker_queue, &transformer);
	Producer producer3(input_queue, worker_queue, &transformer);
	Producer producer4(input_queue, worker_queue, &transformer);
	
	ConsumerController consumerController(worker_queue, output_queue, &transformer,
	                                      CONSUMER_CONTROLLER_CHECK_PERIOD,
										  CONSUMER_CONTROLLER_LOW_THRESHOLD_PERCENTAGE,
	                                      CONSUMER_CONTROLLER_HIGH_THRESHOLD_PERCENTAGE); 
	Writer writer(n, output_file_name, output_queue);
	
	// 將threads啟動
	reader.start(); 
	writer.start();
	producer1.start();
	producer2.start();
	producer3.start();
	producer4.start();
	consumerController.start();
	
	// 最後要等writer和reader都執行完，才可以結束main，
	reader.join();
	writer.join();
	
	consumerController.finished = true;
	producer1.finished = true;
	producer2.finished = true;
	producer3.finished = true;
	producer4.finished = true;

	//auto elapsed = std::chrono::high_resolution_clock::now() - startTime;
	//long long microseconds = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
	//std::cout << "time: " << microseconds << std::endl;
	return 0;
}

