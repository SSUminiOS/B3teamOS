#ifndef __PRIMITIVE_H__
#define __PRIMITIVE_H__

#include <semaphore.h>

struct os_args {
    void *arg0;
    void *arg1;
    void *arg2;
};

struct sem_info {
    char *name;
    sem_t *sem;
};

struct shm_info {
    struct sem_info lock;
    char *name;
    void *data;
    size_t sz;
    int fd;
};

// 세마포어 생성 함수
struct sem_info create_sem(const char *prefix);
void destroy_sem(struct sem_info *semaphore);
struct shm_info create_shm(const char *prefix, size_t size);
void destroy_shm(struct shm_info *shmem);

// list_head 구조체는 prev와 next를 가지고 있어 해당 list_head 구조체를 연결하고자 하는 대상에 집어넣음으로써 연결 리스트를 구현
// 이와 같은 접근법을 사용한 이유는, 링크드 리스트를 구현함에 있어 새로운 메모리를 할당하지 않고, 기존의 메모리만을 가지고 연결 리스트를 구현 가능
struct list_head {
	struct list_head *prev, *next;
};

#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)

#define container_of(ptr, type, member) ({			\
	const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
	(type *)( (char *)__mptr - offsetof(type, member) );})

// LIST_HEAD_INIT, INIT_LIST_HEAD는 링크드 리스트를 초기화하는 구조
#define LIST_HEAD_INIT(name) { &(name), &(name) }

#define LIST_HEAD(name) \
	struct list_head name = LIST_HEAD_INIT(name);

void INIT_LIST_HEAD(struct list_head *list);
void list_add(struct list_head *_new, struct list_head *head);
void list_add_tail(struct list_head *_new, struct list_head *head);
void list_reinsert(struct list_head *entry, struct list_head *head);
void list_reinsert_tail(struct list_head *entry, struct list_head *head);
void list_del(struct list_head *entry);

// 순회 관련 매크로
#define list_entry(ptr, type, member) \
	container_of(ptr, type, member)

#define list_first_entry(ptr, type, member) \
	list_entry((ptr)->next, type, member)

#define list_last_entry(ptr, type, member) \
	list_entry((ptr)->prev, type, member)

#define list_next_entry(pos, member) \
	list_entry((pos)->member.next, typeof(*(pos)), member)

#define list_prev_entry(pos, member) \
	list_entry((pos)->member.prev, typeof(*(pos)), member)

#define list_entry_is_head(pos, head, member)			 \
	(&pos->member == (head))

#define list_empty(head) ((head)->next == (head))

// 각 리스트를 순회하는 매크로
#define list_for_each_entry(pos, head, member)                   \
	for (pos = list_first_entry(head, typeof(*pos), member); \
	     !list_entry_is_head(pos, head, member);             \
	     pos = list_next_entry(pos, member))

// 각 리스트를 역으로 순회하는 매크로
#define list_for_each_entry_reverse(pos, head, member)          \
	for (pos = list_last_entry(head, typeof(*pos), member); \
	     !list_entry_is_head(pos, head, member);            \
	     pos = list_prev_entry(pos, member))

// 각 리스트를 순회하되, list_del을 사용할 수 있도록 개선
#define list_for_each_entry_safe(pos, n, head, member)			\
	for (pos = list_first_entry(head, typeof(*pos), member),	\
		n = list_next_entry(pos, member);			\
	     !list_entry_is_head(pos, head, member); 			\
	     pos = n, n = list_next_entry(n, member))

// 각 리스트를 순회하되, list_del을 사용할 수 있도록 개선
#define list_for_each_entry_safe_reverse(pos, n, head, member)		\
	for (pos = list_last_entry(head, typeof(*pos), member),		\
		n = list_prev_entry(pos, member);			\
	     !list_entry_is_head(pos, head, member); 			\
	     pos = n, n = list_prev_entry(n, member))

// 고정 크기 배열(링크드 리스트 x) 순회 매크로
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#define for_each(item, arr) \
    for (int i = 0; i < ARRAY_SIZE(arr) && (item = &arr[i]); i++)

#define for_each_until(item, arr, until) \
    for (int i = 0; i < until && (item = &arr[i]); i++)

#define for_each_offset_until(item, arr, offset, until) \
    for (int i = offset; i < until && (item = &arr[i]); i++)

#endif

