#!/bin/sh
# BOARD_DIR will contain the name of this folder containing the script
# Ex: /home/ajay/ext_ssd/stm32f429/buildroot/board/stmicroelectronics/stm32f746-disco/
BOARD_DIR="$(dirname "$0")"

# Kernel is built without devpts support
# So Remove all lines in /etc/fstab that start with
# devpts
sed -i '/^devpts/d' "${TARGET_DIR}"/etc/fstab

install -m 0644 -D "${BOARD_DIR}"/extlinux.conf "${BINARIES_DIR}"/extlinux/extlinux.conf
