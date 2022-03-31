#include <sys/mount.h>

#include "zygisk.hpp"

class Dex2oat : public zygisk::ModuleBase {
public:
    void onLoad(zygisk::Api* api, JNIEnv* env) override {
        if (api->getFlags() & zygisk::StateFlag::PROCESS_ON_DENYLIST) {
            umount("/apex/com.android.art/bin/dex2oat32");
            umount("/apex/com.android.art/bin/dex2oat64");
            umount("/system/apex/com.android.art.release/bin/dex2oat32");
            umount("/system/apex/com.android.art.release/bin/dex2oat64");
        }
    }
};

REGISTER_ZYGISK_MODULE(Dex2oat)
