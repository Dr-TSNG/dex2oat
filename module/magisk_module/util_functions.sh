check_magisk_version() {
  ui_print "- Magisk version: $MAGISK_VER_CODE"
  if [ "$MAGISK_VER_CODE" -lt 24000 ]; then
    ui_print "*********************************************************"
    ui_print "! Please install Magisk v24+"
    abort    "*********************************************************"
  fi
}

require_new_android() {
  ui_print "*********************************************************"
  ui_print "! Unsupported Android version ${1} (below Oreo MR1)"
  abort    "*********************************************************"
}

check_android_version() {
  if [ "$API" -ge 27 ]; then
    ui_print "- Android SDK version: $API"
  else
    require_new_android "$API"
  fi
}
