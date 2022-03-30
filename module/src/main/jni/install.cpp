#include <android/log.h>
#include <cctype>
#include <fcntl.h>
#include <string>
#include <string_view>
#include <sys/mount.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <thread>
#include <unistd.h>

#include "log.h"
#include "utils.h"

namespace {
    std::string kMagiskPath, kModuleDir;
    std::string kTmpDir("/dev/", 12);

    void Prepare() {
        LOGD("Preparing paths");
        FILE* fp = popen("magisk --path", "r");
        kMagiskPath.resize(PATH_MAX);
        fscanf(fp, "%s", kMagiskPath.data());
        kMagiskPath.resize(strlen(kMagiskPath.c_str()));
        pclose(fp);
        kModuleDir = kMagiskPath + "/.magisk/modules/icu.nullptr.dex2oat";
        LOGD("Magisk path: %s", kMagiskPath.c_str());
        LOGD("Module path: %s", kModuleDir.c_str());

        fp = fopen("/dev/urandom", "rb");
        do {
            for (int i = 5; i < 12; i++) {
                while (!isdigit(kTmpDir[i]) && !isalpha(kTmpDir[i])) {
                    fread(&kTmpDir[i], 1, 1, fp);
                }
            }
        } while (access(kTmpDir.c_str(), F_OK) == 0);
        fclose(fp);
        mkdir(kTmpDir.c_str(), 0755);
    }

    int HexPatch(const std::string& lp) {
        std::string src = kModuleDir + "/bin/dex2oat" + lp;
        std::string dst = kTmpDir + "/dex2oat" + lp;
        std::string mnt = "/apex/com.android.art/bin/dex2oat" + lp;

        LOGD("Patching %s -> %s", src.c_str(), dst.c_str());
        static char buf[51200];
        FILE* fp = fopen(src.c_str(), "rb");
        size_t len = fread(buf, 1, sizeof(buf), fp);
        fclose(fp);
        size_t off = std::string_view(buf, len).find("placeholder");
        LOGD("Len: 0x%zx Offset: 0x%zx", len, off);
        strcpy(buf + off, kTmpDir.c_str());
        fp = fopen(dst.c_str(), "wb");
        fwrite(buf, 1, len, fp);
        fclose(fp);

        LOGD("Setting permissions");
        chmod(dst.c_str(), 0755);
        chown(dst.c_str(), 0, 2000);
        std::string chcon_cmd = "chcon u:object_r:magisk_file:s0 " + dst;
        system(chcon_cmd.c_str());
        int stock = open(mnt.c_str(), O_RDONLY);
        mount(dst.c_str(), mnt.c_str(), nullptr, MS_BIND, nullptr);
        return stock;
    }

    [[noreturn]]
    void Daemon(const std::string& lp, int stock_fd) {
        LOGD("Daemon %s", lp.c_str());
        std::string sock_path = kTmpDir + "/dex2oat" + lp + ".sock";
        struct sockaddr_un sock {
                .sun_family = AF_UNIX
        };
        snprintf(sock.sun_path, sizeof(sock.sun_path), "%s", sock_path.c_str());

        int sock_fd = socket(AF_UNIX, SOCK_STREAM, 0);
        if (-1 == bind(sock_fd, (struct sockaddr*) &sock, sizeof(sock))) {
            PLOGE("Failed to bind socket");
            exit(1);
        }
        listen(sock_fd, 10);
        while (true) {
            int client_fd = accept(sock_fd, nullptr, nullptr);
            LOGD("%s: send fd %d", lp.c_str(), client_fd);
            send_fd(client_fd, stock_fd);
            close(client_fd);
        }
    }
}

int main() {
    Prepare();
    int stock32 = HexPatch("32");
#ifdef __LP64__
    int stock64 = HexPatch("64");
#endif

    if (fork()) {
        LOGD("Bootstrap complete");
        return 0;
    } else {
        std::thread daemon32(Daemon, "32", stock32);
#ifdef __LP64__
        std::thread daemon64(Daemon, "64", stock64);
#endif
        daemon32.join();
#ifdef __LP64__
        daemon64.join();
#endif
        LOGE("Daemon exited");
        return -1;
    }
}
