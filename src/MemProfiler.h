#pragma once

#include <iostream>
#include <execinfo.h>
#include "MemStack.h"

namespace MemProfiler {
	// If enabled all allocations & de-allocations are logged to file via stderr
	// If disabled only memory leaks are logged to file
	#define LOG_ALL true
	// If enabled errors will be logged to stdout as well as stderr, otherwise errors will be saved only to the log
	#define OUTPUT_STDOUT true

	class MemProfiler {
	private:
		MemStack* allocated = new MemStack;
		MemStack* freed = new MemStack;
		MemStack* dangling = new MemStack;

		// Default constructor
		MemProfiler() {
			std::cout << "Memory Profiler initialised" << std::endl;
			MemProfiler::enableLogging();
		}

		~MemProfiler() {
			// Find dangling pointers (if any exist)
			MemNode* n = allocated->getTop();

			// Check for mismatching pointers between allocations / frees
			while (n != nullptr) {
				if (!freed->find(n->getData().p)) {
					MemNode *nodeCopy = new MemNode(n->getData());
					nodeCopy->clearMeta();
					dangling->push(*nodeCopy);
				}
				n = n->getNext();
			}

			if (dangling->getTop() == nullptr) {
				std::cout << "No dangling pointers logged" << std::endl;
			} else {
				n = dangling->getTop();

				while (n != nullptr) {
					#ifdef OUTPUT_STDOUT
						std::cout << "===! ERROR !===" << std::endl;
						std::cout << "Dangling pointer at memory address: " << n->getData().p << std::endl;
						std::cout << "Memory leak size: " << n->getData().size << " bytes" << std::endl;
						std::cout << "Stack Trace: " << std::endl;
					#endif

					std::cerr << "===! ERROR !===" << std::endl;
					std::cerr << "[ERROR] - Dangling pointer at memory address: " << n->getData().p << std::endl;
					std::cerr << "[ERROR] - Memory leak size: " << n->getData().size << " bytes" << std::endl;
					std::cerr << "[ERROR] - Stack Trace: " << std::endl;

					for (int i = 0; i < n->getData().frames; i++) {
						std::cout << n->getData().stackTrace[i] << std::endl;
						std::cerr << n->getData().stackTrace[i] << std::endl;
					}
					std::cout << std::endl;
					std::cerr << std::endl;

					n = n->getNext();
				}
			}

			free(allocated);
			free(freed);
			free(dangling);
		}

		// Prohibit external replication constructs & assignments
		MemProfiler(MemProfiler const&) = delete;
		void operator=(MemProfiler const&) = delete;

		// Internal class overloads to prevent self logging
		void* operator new(size_t size) {
			void * p = malloc(size);
			return p;
		}

		void operator delete(void * p) {
			free(p);
		}

	public:
		// Singleton instance
		static MemProfiler &GetInstance() {
			static MemProfiler mem;
			return mem;
		}

		// Enable logging, pipe stderr to output file
		static void enableLogging() {
			freopen("log.txt", "w", stderr);
		}

		// Memstack allocation tracking
		void allocate(void* p, size_t size) {
			void *stackFrames[BACKTRACE_DEPTH];
			int frames = backtrace(stackFrames, BACKTRACE_DEPTH);

			#ifdef LOG_ALL
				std::cerr << "==ALLOCATION==" << std::endl;
				std::cerr << "Pointer: " << p << std::endl;
				std::cerr << "Allocation Size: " << size << " bytes" << std::endl;
				std::cerr << "Stack Trace: " << std::endl;

				char** stackTrace = backtrace_symbols(stackFrames, frames);
				for (int i = 0; i < frames; i++) {
					std::cerr << stackTrace[i] << std::endl;
				}
				std::cerr << std::endl;
			#endif

			auto m = new MemNode(MemNodeData({p, size, frames, backtrace_symbols(stackFrames, frames)}));
			allocated->push(*m);
		}

		// Memstack deallocation tracking
		void deallocate(void* p) {
			void *stackFrames[BACKTRACE_DEPTH];
			int frames = backtrace(stackFrames, BACKTRACE_DEPTH);

			#ifdef LOG_ALL
				std::cerr << "==DE-ALLOCATION==" << std::endl;
				std::cerr << "Pointer: " << p << std::endl;
				std::cerr << "Stack Trace: " << std::endl;

				char** stackTrace = backtrace_symbols(stackFrames, frames);
				for (int i = 0; i < frames; i++) {
					std::cerr << stackTrace[i] << std::endl;
				}
				std::cerr << std::endl;
			#endif

			auto m = new MemNode(MemNodeData({p, 0, frames, backtrace_symbols(stackFrames, frames)}));
			MemProfiler::freed->push(*m);
		}

	};
}

/* New & Delete operand overloads, depending on include level this will override
   all calls to new/allocate/delete/free */
void * operator new(size_t size)
{
	void * p = malloc(size);
	MemProfiler::MemProfiler::GetInstance().allocate(p, size);
	return p;
}


// Delete operate overload, store allocation data
void operator delete(void * p)
{
	MemProfiler::MemProfiler::GetInstance().deallocate(p);
	free(p);
}