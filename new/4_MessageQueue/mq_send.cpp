#include <iostream>
#include <mqueue.h>
#include <cstring>

int main() {
    const char* queue_name = "/my_queue";
    const char* msg = "Message from MQ!";

    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = 1024;
    attr.mq_curmsgs = 0;

    mqd_t mq = mq_open(queue_name, O_CREAT | O_WRONLY, 0644, &attr);
    
    if (mq == (mqd_t)-1) {
        perror("Sender: mq_open");
        return 1;
    }

    mq_send(mq, msg, strlen(msg) + 1, 0);
    std::cout << "Sender: Message sent." << std::endl;

    mq_close(mq);
    return 0;
}