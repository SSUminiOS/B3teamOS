#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
#include <semaphore.h>

#include "helper.h"
#include "primitive.h"
// 세마포어 생성 함수

struct sem_info create_sem(const char *prefix) {
    struct sem_info semaphore;
    semaphore.name = get_random_str(prefix);
    semaphore.sem = sem_open(semaphore.name, O_CREAT | O_EXCL, 0666, 0);
    if (semaphore.sem == SEM_FAILED) {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }
    return semaphore;
}

// 세마포어 제거 함수
void destroy_sem(struct sem_info *semaphore) {
    if (sem_close(semaphore->sem) == -1) {
        perror("sem_close");
        exit(EXIT_FAILURE);
    }

    if (sem_unlink(semaphore->name) == -1) {
        perror("sem_unlink");
        exit(EXIT_FAILURE);
    }

    free(semaphore->name);
}


struct shm_info create_shm(const char *prefix, size_t size) {
    struct shm_info shmem;
    shmem.name = get_random_str(prefix);
    shmem.fd = shm_open(shmem.name, O_CREAT | O_RDWR, 0666);
    if (shmem.fd == -1) {
        perror("shm_open");
        exit(EXIT_FAILURE);
    }

    if (ftruncate(shmem.fd, size) == -1) {
        perror("ftruncate");
        exit(EXIT_FAILURE);
    }

    shmem.sz = size;
    shmem.data = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, shmem.fd, 0);
    if (shmem.data == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }

    shmem.lock = create_sem(SEM_NAME);
    memset(shmem.data, 0, size);
    return shmem;
}

void destroy_shm(struct shm_info *shmem) {
    if (munmap(shmem->data, shmem->sz) == -1) {
        perror("munmap");
        exit(EXIT_FAILURE);
    }

    if (close(shmem->fd) == -1) {
        perror("close");
        exit(EXIT_FAILURE);
    }

    if (shm_unlink(shmem->name) == -1) {
        perror("shm_unlink");
        exit(EXIT_FAILURE);
    }

    destroy_sem(&(shmem->lock));
    free(shmem->name);
}

void INIT_LIST_HEAD(struct list_head *list) {
	list->next = list;
	list->prev = list;
}

// 해당 목록에 새 entry 추가
static void __list_add(struct list_head *_new, struct list_head *prev,
                struct list_head *next) {
	next->prev = _new;
	_new->next = next;
	_new->prev = prev;
	prev->next = _new;
}

// 해당 목록의 entry 제거
static void __list_del(struct list_head *prev, struct list_head *next) {
    next->prev = prev;
    prev->next = next;
}

// 해당 목록의 entry 제거
static void __list_del_entry(struct list_head *entry) {
    __list_del(entry->prev, entry->next);
}


// 해당 목록에 새 entry 추가
void list_add(struct list_head *_new, struct list_head *head) {
    __list_add(_new, head, head->next);
}

// 해당 목록의 마지막에 새 entry 추가
void list_add_tail(struct list_head *_new, struct list_head *head) {
    __list_add(_new, head->prev, head);
}

// 해당 목록의 entry 제거
void list_del(struct list_head *entry) {
    __list_del_entry(entry);
    entry->next = entry;
    entry->prev = entry;
}

// 해당 목록에 재삽입
void list_reinsert(struct list_head *entry, struct list_head *head) {
    list_del(entry);
    list_add(entry, head);
}

// 해당 목록의 마지막에 재삽입
void list_reinsert_tail(struct list_head *entry, struct list_head *head) {
    list_del(entry);
    list_add_tail(entry, head);
}