
#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/inotify.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

#define DEV_DIR "/dev"
#define BUF_LEN (10 * (sizeof(struct inotify_event) + NAME_MAX + 1))

void *capture_uart(void *arg) {
    char *device = (char *)arg;
    char filepath[256];
    snprintf(filepath, sizeof(filepath), "/tmp/%s", device);
    char devpath[256];
    snprintf(devpath, sizeof(devpath), "%s/%s", DEV_DIR, device);

    int fd = open(devpath, O_RDONLY | O_NOCTTY);
    if (fd < 0) {
        perror("open UART");
        free(device);
        return NULL;
    }

    FILE *outfile = fopen(filepath, "w");
    if (!outfile) {
        perror("open output file");
        close(fd);
        free(device);
        return NULL;
    }

    char buffer[256];
    ssize_t bytes_read;
    while ((bytes_read = read(fd, buffer, sizeof(buffer))) > 0) {
        fwrite(buffer, 1, bytes_read, outfile);
        fflush(outfile);
    }

    close(fd);
    fclose(outfile);
    free(device);
    return NULL;
}

void monitor_usb_devices() {
    int inotify_fd = inotify_init();
    if (inotify_fd < 0) {
        perror("inotify_init");
        return;
    }

    int wd = inotify_add_watch(inotify_fd, DEV_DIR, IN_CREATE | IN_DELETE);
    if (wd < 0) {
        perror("inotify_add_watch");
        close(inotify_fd);
        return;
    }

    char buf[BUF_LEN];
    while (1) {
        int length = read(inotify_fd, buf, BUF_LEN);
        if (length < 0) {
            perror("read");
            break;
        }

        int i = 0;
        while (i < length) {
            struct inotify_event *event = (struct inotify_event *)&buf[i];
            if (event->len > 0 && strncmp(event->name, "ttyUSB", 6) == 0) {
                if (event->mask & IN_CREATE) {
                    char *device = strdup(event->name);
                    pthread_t thread;
                    pthread_create(&thread, NULL, capture_uart, device);
                    pthread_detach(thread);
                }
            }
            i += sizeof(struct inotify_event) + event->len;
        }
    }
    close(inotify_fd);
}

int main() {
    monitor_usb_devices();
    return 0;
}
