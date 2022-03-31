# shellcheck disable=SC2034
SKIPUNZIP=1

FLAVOR=@FLAVOR@

enforce_install_from_magisk_app() {
  if $BOOTMODE; then
    ui_print "- Installing from Magisk app"
  else
    ui_print "*********************************************************"
    ui_print "! Install from recovery is NOT supported"
    ui_print "! Please install from Magisk app"
    abort    "*********************************************************"
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
  abort    "*********************************************************"
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

# Extract files
ui_print "- Extracting module files"

extract "$ZIPFILE" 'module.prop' "$MODPATH"
extract "$ZIPFILE" 'post-fs-data.sh' "$MODPATH"
extract "$ZIPFILE" 'sepolicy.rule' "$MODPATH"

ui_print "- Extracting binaries"

mkdir -p "$MODPATH/bin"

if [ "$ARCH" = "arm" ] ; then
  extract "$ZIPFILE" 'bin/armeabi-v7a/dex2oatd' "$MODPATH/bin" true
elif [ "$ARCH" = "arm64" ]; then
  extract "$ZIPFILE" 'bin/arm64-v8a/dex2oatd' "$MODPATH/bin" true
elif [ "$ARCH" = "x86" ]; then
  extract "$ZIPFILE" 'bin/x86/dex2oatd' "$MODPATH/bin" true
elif [ "$ARCH" = "x64" ]; then
  extract "$ZIPFILE" 'bin/x86_64/dex2oatd' "$MODPATH/bin" true
fi

if [ "$ARCH" = "arm" ] || [ "$ARCH" = "arm64" ]; then
  extract "$ZIPFILE" "bin/armeabi-v7a/dex2oat" "$MODPATH/bin" true
  mv "$MODPATH/bin/dex2oat" "$MODPATH/bin/dex2oat32"

  if [ "$IS64BIT" = true ]; then
    extract "$ZIPFILE" "bin/arm64-v8a/dex2oat" "$MODPATH/bin" true
    mv "$MODPATH/bin/dex2oat" "$MODPATH/bin/dex2oat64"
  fi
elif [ "$ARCH" == "x86" ] || [ "$ARCH" == "x64" ]; then
  extract "$ZIPFILE" "bin/x86/dex2oat" "$MODPATH/bin" true
  mv "$MODPATH/bin/dex2oat" "$MODPATH/bin/dex2oat32"

  if [ "$IS64BIT" = true ]; then
    extract "$ZIPFILE" "bin/x86_64/dex2oat" "$MODPATH/bin" true
    mv "$MODPATH/bin/dex2oat" "$MODPATH/bin/dex2oat64"
  fi
fi

set_perm_recursive "$MODPATH" 0 0 0755 0644
set_perm_recursive "$MODPATH/bin" 0 0 0755 0755

ui_print "- Install complete"
