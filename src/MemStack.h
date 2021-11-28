#pragma once
#include <cstddef>
#include <cstdlib>

// How many function calls to step back when logging calls to new / delete
const int BACKTRACE_DEPTH = 20;

// Node payload
struct MemNodeData {
	void* p;			// Pointer to log
	size_t size;		// Size of memory allocation
	int frames; 		// Total stack frames captured
	char** stackTrace;	// Iterable stack trace

	MemNodeData(void* p, size_t s, int f, char** st) : p(p), size(s), frames(f), stackTrace(st) {}
};

// Node data structure class
class MemNode {
private:
	MemNode* next = nullptr;
	MemNodeData data;

	friend class MemStack;

public:
	// Initialisation constructor
	explicit MemNode(MemNodeData d) : data(d) {}

	// Return data reference
	MemNodeData getData() {
		return data;
	}

	// Get next node
	MemNode* getNext() {
		return next;
	}

	// Clear metadata when making a copy
	void clearMeta() {
		next = nullptr;
	}

	// Internal class overloads to prevent self logging
	void* operator new(size_t size) {
		return malloc(size);
	}

	void operator delete(void* p) {
		free(p);
	}
};

// Mem Stack class
class MemStack {
private:
	MemNode* top = nullptr;

	~MemStack() {
		while (top != nullptr) {
			pop();
		}
	}

	// Remove top level node, return nullptr if no nodes exist
	// As this class isn't concerned about data manipulation (only storage) 
	// ref is deleted and not returned
	void pop() {
		if (top != nullptr) {
			MemNode* oldRef = top;
			top = top->next;
			delete oldRef;
		}
	}

public:
	// Return current top
	MemNode* getTop() {
		return top;
	}

	// Push new node to top of stack
	void push(MemNode &n) {
		if (top == nullptr) {
			top = &n;
		} else {
			n.next = top;
			top = &n;
		}
	}

	// Check if a memory address has been added as a node
	bool find(void* p) {
		MemNode* n = top;

		while (n != nullptr) {
			if (n->getData().p == p) {
				return true;
			}
			n = n->next;
		}

		return false;
	}

	// Internal class overloads to prevent self logging
	void* operator new(size_t size) {
		return malloc(size);
	}

	void operator delete(void* p) {
		free(p);
	}
};