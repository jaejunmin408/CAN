#ifndef DEBUG_H
#define DEBUG_H


#define DEBUG(...)                                                            \
        do {                                                                  \
            char buf_tag[512];                                                \
            char buf_cmt[512];                                                \
            sprintf(buf_tag,                                                  \
                "%s/%s(%d) ",                                                 \
                (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__), \
                __FUNCTION__,                                                 \
                __LINE__);                                                    \
            sprintf(buf_cmt, ##__VA_ARGS__);                                  \
            printf("%s%s", buf_tag,buf_cmt);                                  \
        } while (0)


#define QUEUE_SIZE 5                    //[user custom] : queue size
#define ITEM_SIZE (sizeof(char) * 512) //[user custom] : in the queue
        

void startDebug(void *argument);
int _write(int file, char *ptr, int len);
void checkHeapMemory(void *argument);
void LWIP_init(void *argument);
void MX_IWDG_Init(void);

#endif // DEBUG_H

