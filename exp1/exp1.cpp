#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <cstring>
#include <string>
#include <ostream>
#include <time.h>
using namespace std;
// #include <algorithm>
// #include <stack>
// #include <queue>

struct PCB
{
    char name[8];
    int baseAddress = 0;  //基址
    int memoryLength = 0; //内存长度
    struct PCB *next;
};
struct PCB *ready, *blocked, *running;

struct Memory
{
    char flag;        // H/P 内存段空闲/被占用
    int baseAddress;  //基址
    int memoryLength; //内存长度
    struct Memory *prior;
    struct Memory *next;
};
struct Memory *memory;

// 内存条容量
#define M 100

char menu(char c);
void start(char c);
void init();
void creatProcess();
void add(struct PCB *head, struct PCB *process);
void showProcess();
void callReadyProcess();
void timeOver();
void blockedProcess();
void awakeProcess();
void endProcess();

struct PCB *copyPCB(struct PCB *p, struct PCB *t);

struct Memory *allotMemory(struct PCB *p);
void showMemory();
void recycleMemory(struct PCB *t);

// 链表初始化 (带头节点)
void init()
{
    ready = (struct PCB *)malloc(sizeof(struct PCB));
    blocked = (struct PCB *)malloc(sizeof(struct PCB));
    running = (struct PCB *)malloc(sizeof(struct PCB));
    ready->next = NULL;
    blocked->next = NULL;
    running->next = NULL;

    memory = (struct Memory *)malloc(sizeof(struct Memory));
    struct Memory *temp = (struct Memory *)malloc(sizeof(struct Memory));
    temp->flag = 'H';
    temp->baseAddress = 0;
    temp->memoryLength = M;
    // 尾插
    memory->next = temp;
    temp->prior = memory;
    temp->next = NULL;
}

//创建新进程 （新建结点并初始化 加入到就绪队列尾部）
void creatProcess()
{
    struct PCB *p = (struct PCB *)malloc(sizeof(struct PCB));
    struct PCB *r = ready;
    struct PCB *run = running;
    struct PCB *b = blocked;
    char tmp[8];
    int length;
    printf("输入进程名：");
    scanf("%s", &tmp);
    while (r->next != NULL)
    {
        if (!strcmp(tmp, r->next->name))
        {
            printf("该进程已创建...\n");
            showProcess();
            return;
        }
        r = r->next;
    }
    while (run->next != NULL)
    {
        if (!strcmp(tmp, run->next->name))
        {
            printf("该进程已被创建，正在执行中...\n");
            showProcess();
            return;
        }
        run = run->next;
    }

    while (b->next != NULL)
    {
        if (!strcmp(tmp, b->next->name))
        {
            printf("该进程已创建，阻塞中...\n");
            showProcess();
            return;
        }
        b = b->next;
    }
    printf("输入进程的内存大小：");
    scanf("%d", &length);
    // 分配内存
    struct Memory *temp = (struct Memory *)malloc(sizeof(struct Memory));
    p->memoryLength = length;
    temp = allotMemory(p);
    if (!temp)
    {
        return;
    }
    p->baseAddress = temp->baseAddress;
    strcpy(p->name, tmp);
    p->next = NULL;
    add(ready, p);
    showProcess();

    // running空闲时 CPU空闲时调用进程
    if (running->next == NULL)
    {
        callReadyProcess();
    }

    // printf("创建成功，-->%s", ready->next->name);
}

// 内存分配  首次适应
struct Memory *allotMemory(struct PCB *p)
{
    struct Memory *temp = (struct Memory *)malloc(sizeof(struct Memory));
    struct Memory *m = memory->next;
    while (m->flag == 'P' || m->memoryLength < p->memoryLength)
    {
        m = m->next;
        if (m == NULL)
            break;
    }
    showMemory();
    if (m == NULL)
    {
        printf("没有空闲内存！\n");
        return NULL;
    }
    temp->flag = 'P';
    temp->baseAddress = m->baseAddress;
    temp->memoryLength = p->memoryLength;
    struct Memory *dividedNode = (struct Memory *)malloc(sizeof(struct Memory));
    dividedNode->flag = 'H';
    dividedNode->baseAddress = temp->baseAddress + temp->memoryLength;
    dividedNode->memoryLength = m->memoryLength - temp->memoryLength;

    // 连接划分后剩余的空闲片段
    if (dividedNode->memoryLength > 2) //不是碎片
    {

        if (m->next == NULL)
        {
            m->next = dividedNode;
            dividedNode->prior = m;
            dividedNode->next = NULL;
        }
        else
        {
            m->next->prior = dividedNode;
            dividedNode->next = m->next;
            m->next = dividedNode;
            dividedNode->prior = m;
        }
    }
    // 连接刚刚分配到内存的片段
    m->next->prior = temp;
    temp->next = m->next;
    m->next = temp;
    temp->prior = m;

    // 删除m节点
    struct Memory *priorNode = m->prior;
    m->next->prior = priorNode;
    priorNode->next = m->next;
    showMemory();
    return temp;
}

// 显示内存分配情况
void showMemory()
{
    struct Memory *m = memory->next;
    while (m != NULL)
    {
        printf("%c %d(%d)", m->flag, m->baseAddress, m->memoryLength);
        m = m->next;
        m != NULL ? printf("-->") : printf("");
    }
    printf("\n");
}

// 进程尾插
void add(struct PCB *head, struct PCB *process)
{
    struct PCB *tmp = head;
    while (tmp->next != NULL)
        tmp = tmp->next;
    tmp->next = process;
    process->next = NULL;
    // return tmp;
}

// 显示进程状态
void showProcess()
{
    struct PCB *r = ready;
    struct PCB *run = running;
    struct PCB *b = blocked;
    printf("*************************\n");
    printf("ready : ");
    while (r->next != NULL)
    {
        printf("%s(%d)", r->next->name, r->next->baseAddress);
        r = r->next;
        r->next != NULL ? printf("-->") : printf("");
    }
    printf("\n");

    printf("running : ");
    while (run->next != NULL)
    {
        printf("%s(%d)", run->next->name, run->next->baseAddress);
        run = run->next;
        run->next != NULL ? printf("-->") : printf("");
    }
    printf("\n");

    printf("blocked : ");
    while (b->next != NULL)
    {
        printf("%s(%d)", b->next->name, b->next->baseAddress);
        b = b->next;
        b->next != NULL ? printf("-->") : printf("");
    }
    printf("\n");
    printf("*************************\n");
}

// 节点复制
struct PCB *copyPCB(struct PCB *p, struct PCB *t)
{
    p->baseAddress = t->baseAddress;
    p->memoryLength = t->memoryLength;
    strcpy(p->name, t->name);
    return p;
}

// 调用就绪队列进程
void callReadyProcess()
{

    printf("cpu空闲,调用进程%s\n\n", &ready->next->name);
    struct PCB *temp = (struct PCB *)malloc(sizeof(struct PCB));
    temp = copyPCB(temp, ready->next);
    // strcpy(temp->name, ready->next->name);
    temp->next = NULL;
    ready->next = ready->next->next;
    running->next = temp;
    showProcess();
}

// 时间片到 重新排队
void timeOver()
{
    if (running->next == NULL)
    {
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
void blockedProcess()
{
    if (running->next == NULL)
    {
        printf("没有执行中的进程，无法阻塞!\n");
        showProcess();
        return;
    }
    
    struct PCB *temp = (struct PCB *)malloc(sizeof(struct PCB));
    temp = copyPCB(temp, running->next);
    temp->next = NULL;
    running->next = NULL;
    add(blocked, temp);
    if (ready->next != NULL)
    {
        callReadyProcess();
    }
    else
    {
        printf("cpu空闲，没有需要执行的进程...\n");
        showProcess();
    }
    showMemory();
}

// 唤醒第一个阻塞进程
void awakeProcess()
{
    if (blocked->next == NULL)
    {
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
    showMemory();
}

// 终止正在执行的进程 （正常结束）
void endProcess()
{
    if (running->next == NULL)
    {
        printf("没有正在执行的进程！\n");
        showProcess();
        return;
    }
    struct PCB *t = (struct PCB *)malloc(sizeof(struct PCB));
    t->baseAddress = running->next->baseAddress;
    printf("%s(%d)进程以结束...\n", running->next->name, running->next->baseAddress);
    running->next = NULL;
    // 内存回收
    recycleMemory(t);

    if (ready->next != NULL)
        callReadyProcess();
    else if (ready->next == NULL && blocked->next == NULL)
    {
        printf("所有进程已全部执行完毕！\n");
    }
    else
        showProcess();
}

// 回收内存
void recycleMemory(struct PCB *t)
{
    struct Memory *m = memory->next;
    while (m != NULL)
    {
        if (m->baseAddress == t->baseAddress)
        {
            m->flag = 'H';
            showMemory();

            // 空白内存段合并
            if (m->prior->flag == 'H' )
            {
                if (m->next->flag == 'H' )
                {
                    struct Memory *priorNode = m->prior;
                    priorNode->memoryLength = priorNode->memoryLength + m->memoryLength + m->next->memoryLength;
                    if (priorNode->memoryLength > 2)
                    {
                        if (m->next->next != NULL)
                            m->next->next->prior = priorNode;
                        priorNode->next = m->next->next;
                        showMemory();
                    }
                }
                else
                {
                    m->prior->memoryLength = m->prior->memoryLength + m->memoryLength;
                    if (m->prior->memoryLength > 2)
                    {
                        m->next->prior = m->prior;
                        m->prior->next = m->next;
                        showMemory();
                    }
                }
            }
            else
            {
                if (m->next->flag == 'H')
                {

                    m->memoryLength = m->memoryLength + m->next->memoryLength;
                    if (m->memoryLength > 2)
                    {
                        if (m->next->next != NULL)
                            m->next->next->prior = m;
                        m->next = m->next->next;
                        showMemory();
                    }
                }
                else
                {
                }
            }
            return;
        }
        m = m->next;
    }
}

char menu(char c)
{
    printf("\n\n--------------------------\n");
    printf("      *** 菜单 ***\n");
    printf("c: 创建新进程\n");
    printf("d: 执行进程时间片到\n");
    printf("b: 阻塞执行进程\n");
    printf("a: 唤醒第一个阻塞进程\n");
    printf("e: 终止执行进程\n");
    printf("f: 显示进程状态\n");
    printf("s: 显示内存空间状态\n");
    printf("q: 退出\n");
    printf("--------------------------\n");
    printf("请输入操作对应的符号： ");
    scanf(" %c", &c);
    return c;
}

void start()
{
    char c = ' ';
    while (c != 'q')
    {
        c = menu(c);
        switch (c)
        {
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
            showProcess();
            break;
        case 's':
            showMemory();
            break;
        case 'q':
            break;
        default:
            break;
        }
    }
}

// g++ knn.cpp -o knn.exe
// ./knn.exe
// chcp 65001
int main()
{
    init();
    start();
    return 0;
}