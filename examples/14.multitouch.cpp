#include <iostream>
#include <mtdev.h>
#include <linux/input.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>


int main()
{
    // Open the input device
    int fd = open("/dev/input/event8", O_RDONLY | O_NONBLOCK);
    if (fd < 0) {
        std::cerr << "Failed to open input device" << std::endl;
        return 1;
    }

    // Create an mtdev device object from the file descriptor
    mtdev dev;
    if (mtdev_open(&dev, fd) < 0) {
        std::cerr << "Failed to create mtdev device" << std::endl;
        return 1;
    }

    // Set up a loop to read events
    while (true) {
        struct input_event ev[64];
        int n = mtdev_get(&dev, fd, ev, sizeof(ev) / sizeof(ev[0]));
        if (n > 0) {
            // Process the events here
            for (int i = 0; i < n; i++) {
                std::cout << "Type: " << ev[i].type << " Code: " << ev[i].code << " Value: " << ev[i].value << std::endl;
            }
        }
    }

    // Clean up
    mtdev_close(&dev);
    close(fd);
    return 0;
}
