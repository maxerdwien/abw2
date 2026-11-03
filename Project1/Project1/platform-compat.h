#pragma once

// Portability shim so the game builds on both Windows (winsock) and Linux (BSD sockets).

#ifdef _WIN32

#include <winsock2.h>
#include <ws2tcpip.h>
#include <process.h>

#else

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>

typedef int SOCKET;
typedef const char* PCSTR;
typedef struct addrinfo ADDRINFO;
typedef struct sockaddr_storage SOCKADDR_STORAGE;
typedef struct sockaddr* LPSOCKADDR;

#define INVALID_SOCKET (-1)
#define SOCKET_ERROR (-1)

typedef struct { int unused; } WSADATA;
#define MAKEWORD(a, b) ((unsigned short)(((unsigned char)(a)) | ((unsigned short)((unsigned char)(b))) << 8))
static inline int WSAStartup(unsigned short, WSADATA*) { return 0; }
static inline int WSAGetLastError() { return errno; }

// Minimal stand-in for MSVC's _beginthread: runs func(arg) on a detached thread.
struct compat_thread_start {
	void (*func)(void*);
	void* arg;
};

static void* compat_thread_trampoline(void* p) {
	compat_thread_start start = *(compat_thread_start*)p;
	delete (compat_thread_start*)p;
	start.func(start.arg);
	return nullptr;
}

static inline uintptr_t _beginthread(void (*func)(void*), unsigned, void* arg) {
	pthread_t t;
	compat_thread_start* start = new compat_thread_start{ func, arg };
	if (pthread_create(&t, nullptr, compat_thread_trampoline, start) != 0) {
		delete start;
		return (uintptr_t)-1;
	}
	pthread_detach(t);
	return (uintptr_t)t;
}

#endif
