#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <cstring>
#include <string>
#include <ostream>
#include <time.h>
#include <queue>
using namespace std;

queue<int> M;
struct PCB {
	char name[8];
	int size;
	int *page_table;
	struct PCB *next;
};

struct PCB *ready, *blocked, *running;

#define BLOCK_SIZE 1024
#define MEM_SIZE 64
#define initialPageNum 3

int bitmap[MEM_SIZE / 8][8];     //内存位示图
int helpBitMap[MEM_SIZE / 4][8]; //外存位示图

int block_count, page_offset;

char menu(char c);
void start(char c);
void init();
void bitmapInit();
void creatProcess();
void add(struct PCB *head, struct PCB *process);
void showProcess();
void callReadyProcess();
void timeOver();
void blockedProcess();
void awakeProcess();
void endProcess();

struct PCB *copyPCB(struct PCB *p, struct PCB *t);
struct PCB *allotPageTable(struct PCB *p);
void showBitMap(int flag);
void showQueue();
void showRunningProcess();
struct PCB *runProcess(struct PCB *tmp);
void recycleMemory(struct PCB *t);
struct PCB *replaceFIFO(struct PCB *temp, int cnt); //置换

// 链表初始化 (带头节点)
void init() {
	ready = (struct PCB *)malloc(sizeof(struct PCB));
	blocked = (struct PCB *)malloc(sizeof(struct PCB));
	running = (struct PCB *)malloc(sizeof(struct PCB));
	ready->next = NULL;
	blocked->next = NULL;
	running->next = NULL;
	bitmapInit();
}

// 初始化位示图（随机出内存占用情况）
void bitmapInit() {
	for (int i = 0; i < MEM_SIZE / 8; i++) {
		for (int j = 0; j < 8; j++) {
			bitmap[i][j] = rand() % 2;
			helpBitMap[i][j] = rand() % 2;
		}
		for (int k = 8; k < MEM_SIZE / 4; k++) {
			helpBitMap[i][k] = rand() % 2;
		}
	}
}

//创建新进程 （新建结点并初始化 加入到就绪队列尾部）
void creatProcess() {
	struct PCB *p = (struct PCB *)malloc(sizeof(struct PCB));
	struct PCB *r = ready;
	struct PCB *run = running;
	struct PCB *b = blocked;
	char tmp[8];
	int length;
	printf("输入进程名：");
	scanf("%s", &tmp);
	while (r->next != NULL) {
		if (!strcmp(tmp, r->next->name)) {
			printf("该进程已创建...\n");
			showProcess();
			return;
		}
		r = r->next;
	}
	while (run->next != NULL) {
		if (!strcmp(tmp, run->next->name)) {
			printf("该进程已被创建，正在执行中...\n");
			showProcess();
			return;
		}
		run = run->next;
	}

	while (b->next != NULL) {
		if (!strcmp(tmp, b->next->name)) {
			printf("该进程已创建，阻塞中...\n");
			// showProcess();
			return;
		}
		b = b->next;
	}
	strcpy(p->name, tmp);
	printf("输入进程的内存大小：");
	scanf("%d", &length);
	showBitMap(1);
	p->size = length;
	// 分配页表空间
	p = allotPageTable(p);

	add(ready, p);
	showBitMap(1);
	showProcess();

	if (running->next == NULL) {
		callReadyProcess();
	}
}

// 页表空间的分配
struct PCB *allotPageTable(struct PCB *p) {
	//计算出块个数
	int count = 0;
	struct PCB *tmp = (struct PCB *)malloc(sizeof(struct PCB));
	tmp = copyPCB(tmp, p);
	block_count = (int)ceil(p->size / (int)BLOCK_SIZE);
	page_offset = (int)(p->size % (int)BLOCK_SIZE);
	printf("逻辑地址%d对应的页号：%d,页内偏移地址为：%d\n", p->size, block_count, page_offset);

	tmp->page_table = (int *)malloc(sizeof(int) * block_count*2);  //空间为2倍
	for (int i = 0; i < MEM_SIZE / 8; i++) {
		for (int j = 0; j < 8; j++) {
			if (count == block_count) { // 内外存分配完毕
				printf("逻辑地址%d对应的物理地址：%d\n", p->size, tmp->page_table[block_count - 1] * (int)BLOCK_SIZE + page_offset);
				return tmp;
			}

			if (count == initialPageNum) {  //分配外存
				// 初始化队列
				int num = 0;
				while (num != count) {
					M.push(num);
					num++;
				}

				for (int p = 0; p < MEM_SIZE / 8; p++) {
					for (int q = 0; q < MEM_SIZE / 4; q++) {
						if (count == block_count) { //内外存分配完毕
							return tmp;
						}
						if (!helpBitMap[p][q]) {
							tmp->page_table[count] = MEM_SIZE / 4 * p + q;
							tmp->page_table[count + block_count] = 0;
							helpBitMap[p][q] = 1; //外存占用
							count++;
						}
					}
				}
			}

			if (!bitmap[i][j]) {
				tmp->page_table[count] = 8 * i + j;
				tmp->page_table[count + block_count] = 1;
				bitmap[i][j] = 1;
				count++;
			}

		}
	}
	if (count < block_count) {
		printf("内存不足,无法进入！！！\n");
		exit(0);
	}

}

// 调用就绪队列进程
void callReadyProcess() {

	struct PCB *tmp = (struct PCB *)malloc(sizeof(struct PCB));
	printf("cpu空闲,调用进程%s\n\n", &ready->next->name);
	tmp = copyPCB(tmp, ready->next);
	tmp->next = NULL;
	ready->next = ready->next->next;
	running->next = tmp;
	showProcess();
	running->next = runProcess(tmp);  //运行进程

}

//按页面执行进程
struct PCB *runProcess(struct PCB *tmp) {
	char keyIndex; //需要调入内存的页面下标
	while (1) {
		showBitMap(0); //显示外存
		showRunningProcess();//显示进程内外存使用情况
		printf("\n队列情况：");
		showQueue();
		printf("请输入需要执行的页面虚号（要调入内存的页面）(退出请输入q)：");
		scanf(" %c", &keyIndex);
		if(keyIndex == 'q') {
			return tmp;
		}
		keyIndex = (int)(keyIndex*1-48);
		if (tmp->page_table[keyIndex + block_count] == 1) { //在内存
			printf("该页面已在内存！\n");
		} else {
			printf("\n%d号页不在内存，外存号为%d,需置换...\n", keyIndex, tmp->page_table[keyIndex]);
			tmp = replaceFIFO(tmp, keyIndex); //置换
			printf("逻辑地址%d对应的物理地址：%d\n", tmp->size, tmp->page_table[keyIndex] * (int)BLOCK_SIZE + page_offset);
			M.pop();
		}
	}
}

// 置换
struct PCB *replaceFIFO(struct PCB *temp, int cnt) {
	int index = M.front();
	for (int p = 0; p < MEM_SIZE / 8; p++) {
		for (int q = 0; q < MEM_SIZE / 4; q++) {
			if (!helpBitMap[p][q]) {
				int helpIndex = temp->page_table[index]; //存要移到外存的内存号
				int bitIndex = temp->page_table[index];
				temp->page_table[index + block_count] = 0;        //将该页面移到外存
				temp->page_table[index] = p * (MEM_SIZE / 4) + q; //外存号
				helpBitMap[p][q] = 1;                             //此外存空间设为被占用
				printf("将内存%d号块内容写入外存%d号块,成功！\n", helpIndex, temp->page_table[index]);

				int helpIndex2 = temp->page_table[cnt];  //存要移进内存的页面的外存号
				temp->page_table[cnt + block_count] = 1; //将该页面移到内存
				temp->page_table[cnt] = helpIndex;       //内存号
				printf("将外存%d号块内容调入内存%d号块中,置换完毕！\n", helpIndex2, temp->page_table[cnt]);
				helpBitMap[helpIndex2 / (MEM_SIZE / 4)][helpIndex2 % 8] = 0; //释放外存空间

				M.push(cnt);

				return temp;
			}
		}
	}
	return temp;
}

// 显示内外存分配情况
void showBitMap(int flag) {
	for (int i = 0; i < MEM_SIZE / 8; i++) {
		if (flag == 1) {
			for (int j = 0; j < 8; j++) {
				printf("%d", bitmap[i][j]);
				j != 7 ? printf(" -> ") : printf("");
			}
		} else if (flag == 0) {
			for (int k = 0; k < MEM_SIZE / 4; k++) {
				printf("%d", helpBitMap[i][k]);
				k != MEM_SIZE / 4 - 1 ? printf(" -> ") : printf("");
			}
		}
		printf("\n");
	}
	printf("\n");
}

//显示进程内外存使用情况
void showRunningProcess() {
	struct PCB *tmp = running->next;
	for(int i = 0; i<block_count; i++) {  //显示进程内外存使用情况
		printf("%d->%d(%d)  ", i, tmp->page_table[i], tmp->page_table[i + block_count]);
	}
}

//显示队列
void showQueue() {
	queue<int> m = M;
	while(!m.empty()) {
		printf("%d",m.front());
		m.pop();
		!m.empty() ? printf("->") : printf("");
	}
	printf("\n");
}

// 终止正在执行的进程 （正常结束）
void endProcess() {
	if (running->next == NULL) {
		printf("没有正在执行的进程！\n");
		showProcess();
		return;
	}
	struct PCB *t = (struct PCB *)malloc(sizeof(struct PCB));
	copyPCB(t, running->next);
	printf("%s进程以结束...\n", running->next->name);
	running->next = NULL;
	recycleMemory(t); // 内外存回收
	showBitMap(1);
	showBitMap(0);
	queue<int> m;
	M = m;
	block_count = page_offset = 0;
	if (ready->next != NULL)
		callReadyProcess();
	else if (ready->next == NULL && blocked->next == NULL) {
		printf("所有进程已全部执行完毕！\n");
	} else
		showProcess();
}

// 回收内外存
void recycleMemory(struct PCB *t) {
	int block_count = (int)ceil(t->size / (int)BLOCK_SIZE);
	for (int i = 0; i < block_count; i++) {
		if(t->page_table[i + block_count] == 1) { //释放在内存的空间
			bitmap[t->page_table[i] / 8][t->page_table[i] % 8] = 0;
		} else { //释放在外存的空间
			helpBitMap[t->page_table[i] / (MEM_SIZE/4)][t->page_table[i] % 8] = 0;
		}
	}
	printf("\n该进程的内外存已释放...\n");
}

// 进程尾插
void add(struct PCB *head, struct PCB *process) {
	struct PCB *tmp = head;
	while (tmp->next != NULL)
		tmp = tmp->next;
	tmp->next = process;
	process->next = NULL;
}

// 显示进程状态
void showProcess() {
	struct PCB *r = ready;
	struct PCB *run = running;
	struct PCB *b = blocked;
	printf("*************************\n");
	printf("ready : ");
	while (r->next != NULL) {
		printf("%s", r->next->name);
		r = r->next;
		r->next != NULL ? printf("-->") : printf("");
	}
	printf("\n");

	printf("running : ");
	while (run->next != NULL) {
		printf("%s", run->next->name);
		run = run->next;
		run->next != NULL ? printf("-->") : printf("");
	}
	printf("\n");

	printf("blocked : ");
	while (b->next != NULL) {
		printf("%s", b->next->name);
		b = b->next;
		b->next != NULL ? printf("-->") : printf("");
	}
	printf("\n");
	printf("*************************\n");
}

// 节点复制
struct PCB *copyPCB(struct PCB *p, struct PCB *t) {
	p->size = t->size;
	p->page_table = t->page_table;
	strcpy(p->name, t->name);
	return p;
}

// 时间片到 重新排队
void timeOver() {
	if (running->next == NULL) {
		printf("没有执行中的进程!\n");
		showProcess();
		return;
	}
	struct PCB *temp = (struct PCB *)malloc(sizeof(struct PCB));
	temp = copyPCB(temp, running->next);
	temp->next = NULL;
	running->next = NULL;
	add(ready, temp);
	showProcess();
	callReadyProcess();
}

// 阻塞执行过程
void blockedProcess() {
	if (running->next == NULL) {
		printf("没有执行中的进程，无法阻塞!\n");
		showProcess();
		return;
	}

	struct PCB *temp = (struct PCB *)malloc(sizeof(struct PCB));
	temp = copyPCB(temp, running->next);
	temp->next = NULL;
	running->next = NULL;
	add(blocked, temp);
	if (ready->next != NULL) {
		callReadyProcess();
	} else {
		printf("cpu空闲，没有需要执行的进程...\n");
		showProcess();
	}
}

// 唤醒第一个阻塞进程
void awakeProcess() {
	if (blocked->next == NULL) {
		printf("没有被阻塞的进程！\n");
		showProcess();
		return;
	}

	struct PCB *temp = (struct PCB *)malloc(sizeof(struct PCB));
	temp = copyPCB(temp, blocked->next);
	temp->next = NULL;
	blocked->next = blocked->next->next;
	add(ready, temp);
	// 唤醒的进程先进入ready后直接被运行
	if (ready->next->next == NULL && running->next == NULL)
		showProcess();
	if (running->next == NULL && ready->next != NULL)
		callReadyProcess();
	else
		showProcess();
}

char menu(char c) {
	printf("\n\n--------------------------\n");
	printf("      *** 菜单 ***\n");
	printf("c: 创建新进程\n");
	printf("d: 执行进程时间片到\n");
	printf("b: 阻塞执行进程\n");
	printf("a: 唤醒第一个阻塞进程\n");
	printf("e: 终止执行进程\n");
	printf("s: 显示内存位示图\n");
	printf("h: 显示外存位示图\n");
	printf("f: 显示当前进程的内外存占用情况\n");
	printf("p: 显示队列情况\n");
	printf("q: 退出\n");
	printf("--------------------------\n");
	printf("请输入操作对应的符号： ");
	scanf(" %c", &c);
	return c;
}

void start() {
	char c = ' ';
	while (c != 'q') {
		c = menu(c);
		switch (c) {
			case 'c':
				creatProcess();
				break;
			case 'd':
				timeOver();
				break;
			case 'b':
				blockedProcess();
				break;
			case 'a':
				awakeProcess();
				break;
			case 'e':
				endProcess();
				break;
			case 'f':
				showRunningProcess();
				break;
			case 's':
				showBitMap(1);
				break;
			case 'h':
				showBitMap(0);
				break;
			case 'p':
				showQueue();
				break;
			case 'q':
				break;
			default:
				break;
		}
	}
}

int main() {
	init();
	start();
	return 0;
}