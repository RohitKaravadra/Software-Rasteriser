#pragma once
#include <mutex>

template <typename T>
class SentinelQueue {
	struct node {
		T data;
		node* next;
		node(T val) : data(val), next(nullptr) {}
	};

	node* head;
	node* tail;
	std::mutex headLock, tailLock;

public:
	SentinelQueue() {
		head = tail = new node(T{});
	}

	~SentinelQueue() {
		clear();
		delete head;
	}

	void enqueue(const T& val) {

		node* newNode = new node(val);

		std::lock_guard<std::mutex> lock(tailLock);
		tail->next = newNode;
		tail = newNode;
	}

	bool dequeue(T& data) {

		std::lock_guard<std::mutex> lock(headLock);
		if (!head->next) return false;

		node* oldNode = head;
		head = head->next;
		data = head->data;

		delete oldNode;
		return true;
	}

	void clear() {
		T data;
		while (dequeue(data));
	}

	bool isEmpty() {
		std::lock_guard<std::mutex> lock(headLock);
		return head->next;
	}
};
