#include <iostream>
#include <mqueue.h>
#include <vector>

int main() {
    const char* queue_name = "/my_queue";

    mqd_t mq = mq_open(queue_name, O_RDONLY);
    
    if (mq == (mqd_t)-1) {
        perror("Receiver: mq_open (run sender first?)");
        return 1;
    }

    struct mq_attr attr;
    mq_getattr(mq, &attr);
    std::vector<char> buffer(attr.mq_msgsize);

    ssize_t bytes_read = mq_receive(mq, buffer.data(), attr.mq_msgsize, NULL);
    
    if (bytes_read >= 0) {
        std::cout << "Receiver: Got message -> " << buffer.data() << std::endl;
    } else {
        perror("Receiver: mq_receive");
    }

    mq_close(mq);
    mq_unlink(queue_name);
    return 0;
}