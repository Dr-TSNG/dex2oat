#include <android/log.h>
#include <climits>
#include <cstdio>
#include <unistd.h>
#include <string>
#include <sys/socket.h>
#include <sys/un.h>

#include "log.h"
#include "utils.h"

constexpr char kTmpDir[PATH_MAX] = "placeholder";

void clean(int exit_code) {
    LOGE("replace dex2oat failed");
    exit(exit_code);
}

int main(int argc, char** argv) {
    LOGI("dex2oat wrapper");
    struct sockaddr_un sock {
            .sun_family = AF_UNIX
    };
    snprintf(sock.sun_path, sizeof(sock.sun_path), "%s/dex2oat" LP_SELECT("32", "64") ".sock", kTmpDir);
    int sock_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (-1 == connect(sock_fd, (struct sockaddr*) &sock, sizeof(sock))) {
        PLOGE("failed to connect to %s", sock.sun_path);
        clean(1);
    }
    int stock_fd = recv_fd(sock_fd);
    LOGD("sock: %s %d", sock.sun_path, stock_fd);

    const char* new_argv[argc + 2];
    for (int i = 0; i < argc; i++) new_argv[i] = argv[i];
    new_argv[argc] = "--inline-max-code-units=0";
    new_argv[argc + 1] = nullptr;
    fexecve(stock_fd, (char* const*) new_argv, environ);
    PLOGE("execvp failed");
    clean(2);
}
