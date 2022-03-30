# shellcheck disable=SC2034
SKIPUNZIP=1

FLAVOR=@FLAVOR@

enforce_install_from_magisk_app() {
  if $BOOTMODE; then
    ui_print "- Installing from Magisk app"
  else
    ui_print "*********************************************************"
    ui_print "! Install from recovery is NOT supported"
    ui_print "! Some recovery has broken implementations, install with such recovery will finally cause Riru or Riru modules not working"
    ui_print "! Please install from Magisk app"
    abort "*********************************************************"
  fi
}

VERSION=$(grep_prop version "${TMPDIR}/module.prop")
ui_print "- Dex2oat version ${VERSION}"

# Extract verify.sh
ui_print "- Extracting verify.sh"
unzip -o "$ZIPFILE" 'verify.sh' -d "$TMPDIR" >&2
if [ ! -f "$TMPDIR/verify.sh" ]; then
  ui_print "*********************************************************"
  ui_print "! Unable to extract verify.sh!"
  ui_print "! This zip may be corrupted, please try downloading again"
  abort "*********************************************************"
fi
. "$TMPDIR/verify.sh"

# Base check
extract "$ZIPFILE" 'customize.sh' "$TMPDIR"
extract "$ZIPFILE" 'util_functions.sh' "$TMPDIR"
extract "$ZIPFILE" 'verify.sh' "$TMPDIR"
. "$TMPDIR/util_functions.sh"
check_android_version
check_magisk_version
enforce_install_from_magisk_app

# Check architecture
if [ "$ARCH" != "arm" ] && [ "$ARCH" != "arm64" ] && [ "$ARCH" != "x86" ] && [ "$ARCH" != "x64" ]; then
  abort "! Unsupported platform: $ARCH"
else
  ui_print "- Device platform: $ARCH"
fi

# Extract libs
ui_print "- Extracting module files"

extract "$ZIPFILE" 'module.prop' "$MODPATH"
extract "$ZIPFILE" 'post-fs-data.sh' "$MODPATH"
extract "$ZIPFILE" 'sepolicy.rule' "$MODPATH"

ui_print "- Extracting libraries"

mkdir -p "$MODPATH/bin"
mkdir -p "$MODPATH/zygisk"

if [ "$ARCH" = "arm" ] ; then
  extract "$ZIPFILE" 'lib/armeabi-v7a/install' "$MODPATH/bin" true
elif [ "$ARCH" = "arm64" ]; then
  extract "$ZIPFILE" 'lib/arm64-v8a/install' "$MODPATH/bin" true
elif [ "$ARCH" = "x86" ]; then
  extract "$ZIPFILE" 'lib/x86/install' "$MODPATH/bin" true
elif [ "$ARCH" = "x64" ]; then
  extract "$ZIPFILE" 'lib/x86_64/install' "$MODPATH/bin" true
fi

if [ "$ARCH" = "arm" ] || [ "$ARCH" = "arm64" ]; then
  extract "$ZIPFILE" "lib/armeabi-v7a/dex2oat" "$MODPATH/bin" true
  mv "$MODPATH/bin/dex2oat" "$MODPATH/bin/dex2oat32"

  extract "$ZIPFILE" "lib/armeabi-v7a/libinject.so" "$MODPATH/zygisk" true
  mv "$MODPATH/zygisk/libinject.so" "$MODPATH/zygisk/armeabi-v7a.so"

  if [ "$IS64BIT" = true ]; then
    extract "$ZIPFILE" "lib/arm64-v8a/dex2oat" "$MODPATH/bin" true
    mv "$MODPATH/bin/dex2oat" "$MODPATH/bin/dex2oat64"

    extract "$ZIPFILE" "lib/arm64-v8a/libinject.so" "$MODPATH/zygisk" true
    mv "$MODPATH/zygisk/libinject.so" "$MODPATH/zygisk/arm64-v8a.so"
  fi
elif [ "$ARCH" == "x86" ] || [ "$ARCH" == "x64" ]; then
  extract "$ZIPFILE" "lib/x86/dex2oat" "$MODPATH/bin" true
  mv "$MODPATH/bin/dex2oat" "$MODPATH/bin/dex2oat32"

  extract "$ZIPFILE" "lib/x86/libinject.so" "$MODPATH/zygisk" true
  mv "$MODPATH/zygisk/libinject.so" "$MODPATH/zygisk/x86.so"

  if [ "$IS64BIT" = true ]; then
    extract "$ZIPFILE" "lib/x86_64/dex2oat" "$MODPATH/bin" true
    mv "$MODPATH/bin/dex2oat" "$MODPATH/bin/dex2oat64"

    extract "$ZIPFILE" "lib/x86_64/libinject.so" "$MODPATH/zygisk" true
    mv "$MODPATH/zygisk/libinject.so" "$MODPATH/zygisk/x86_64.so"
  fi
fi

set_perm_recursive "$MODPATH" 0 0 0755 0644
set_perm_recursive "$MODPATH/bin" 0 0 0755 0755

ui_print "- Install complete"
