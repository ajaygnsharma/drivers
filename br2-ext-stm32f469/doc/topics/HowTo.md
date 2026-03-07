
Changes need in the kernel

CONFIG_I2C=y
CONFIG_I2C_STM32F4=y
CONFIG_I2C_STM32F7=y

CONFIG_CHARLCD=y
CONFIG_HD44780=y

CONFIG_GPIO_PCF857X=y


Great 👍 — that means the kernel side is **100% correct now**.

Below is the **practical, no-nonsense way** to use `/dev/lcd`, plus a few gotchas specific to HD44780 + charlcd.

---

## 1. The simplest possible test

Just write text to it:

```sh
echo "Hello World" > /dev/lcd
```

You should see it appear on **line 1**, starting at column 1.

---

## 2. Newlines = next row

For a 16×2 display:

```sh
printf "Line 1\nLine 2" > /dev/lcd
```

Notes:

* `\n` moves to the **next LCD row**
* Extra characters are truncated
* No scrolling unless you implement it yourself

---

## 3. Clear the display

```sh
echo -e "\f" > /dev/lcd
```

`\f` = **form feed** = clear screen (HD44780 command)

---

## 4. Cursor positioning (important)

The charlcd driver understands **escape sequences**.

### Move cursor (row, column)

```sh
printf "\033[2;5HHello" > /dev/lcd
```

Meaning:

* `2` = row
* `5` = column
* Rows and columns are **1-based**

---

## 5. Full example (clean output)

```sh
printf "\f\033[1;1HLinux\n\033[2;1HSTM32 + PCF8574" > /dev/lcd
```

This:

* Clears the LCD
* Writes “Linux” on row 1
* Writes text on row 2

---

## 6. Backlight control (two ways)

### A) If backlight GPIO is wired and exported

```sh
echo 1 > /sys/class/leds/*backlight*/brightness
echo 0 > /sys/class/leds/*backlight*/brightness
```

(Exact path depends on DT naming.)

### B) If backlight is tied to LCD only

You may not have runtime control (depends on your DT).

---

## 7. Permissions (very common issue)

If non-root access fails:

```sh
ls -l /dev/lcd
```

Temporary fix:

```sh
chmod 666 /dev/lcd
```

Permanent fix: add a **udev rule**.

---

## 8. Using it from C (minimal)

```c
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

int main(void)
{
    int fd = open("/dev/lcd", O_WRONLY);
    if (fd < 0) return 1;

    write(fd, "\fLinux\nHello!", 13);
    close(fd);
    return 0;
}
```

Compile:

```sh
gcc lcd.c -o lcd
```

---

## 9. Things that are *not* supported (by design)

* ❌ Graphics
* ❌ Unicode / UTF-8
* ❌ Scrolling text automatically
* ❌ Framebuffer (`/dev/fb0`)

HD44780 is **pure character-mode**.

---

## 10. Pro tip (very useful for embedded products)

If you want:

* animations
* progress bars
* menus
* background updates

→ write a **small userspace daemon** that owns `/dev/lcd` and exposes a UNIX socket or FIFO.
This avoids multiple apps fighting for the display (you already do similar patterns in your config daemons 😉).

---

### Bottom line

* `/dev/lcd` = **simple, reliable, kernel-driven**
* Works great for **boot messages, status, alarms**
* Perfect fit for STM32 + PCF8574 designs

If you want, next we can:

* add a **boot-time systemd service** that prints status
* make a **progress bar**
* or integrate it with your existing embedded logging / alarm framework
In **Buildroot**, `$(BINARIES_DIR)` (or `$BINARIES_DIR` in shell hooks) is a **Buildroot-defined output directory** where all *final bootable images* are placed.

### What `BINARIES_DIR` actually is

It expands to:

```text
output/images
```

(relative to your Buildroot top directory)

So in your line:

```sh
install -m 0644 -D $BOARD_DIR/extlinux.conf \
    $BINARIES_DIR/extlinux/extlinux.conf
```

it installs the file to:

```text
output/images/extlinux/extlinux.conf
```

Buildroot then picks that up when creating the SD-card / image layout.

---

## Common Buildroot directories (very useful mental model)

| Variable          | Typical path              | Purpose                  |
| ----------------- | ------------------------- | ------------------------ |
| `$(TOPDIR)`       | `buildroot/`              | Buildroot source root    |
| `$(OUTPUT_DIR)`   | `output/`                 | All generated output     |
| `$(BINARIES_DIR)` | `output/images/`          | Final boot images        |
| `$(TARGET_DIR)`   | `output/target/`          | Root filesystem staging  |
| `$(BUILD_DIR)`    | `output/build/`           | Temporary package builds |
| `$(HOST_DIR)`     | `output/host/`            | Host tools               |
| `$(BOARD_DIR)`    | `board/<vendor>/<board>/` | Board support files      |

---

## Why extlinux.conf goes into `BINARIES_DIR`

`extlinux.conf` is **not part of the root filesystem**. It belongs to the **boot partition**, so Buildroot expects it under:

```text
output/images/extlinux/extlinux.conf
```

That directory is later copied into the boot filesystem (FAT/ext) when:

* generating an SD card image
* using `genimage`
* or installing manually

---

## How to verify at build time

After `make`:

```sh
ls output/images/
ls output/images/extlinux/
```

You should see:

```text
extlinux.conf
zImage
dtb/
```

---

## One important gotcha (people hit this often)

`BINARIES_DIR` exists **only in Buildroot hooks** and Makefiles.

If you try to use it in a normal shell:

```sh
echo $BINARIES_DIR
```

it will be empty.

It’s exported only when Buildroot runs:

* `post-build.sh`
* `post-image.sh`
* board hooks

---

### Bottom line

* `$BINARIES_DIR` = **`output/images`**
* Correct place for `extlinux.conf`
* Safe across rebuilds
* Exactly how Buildroot expects boot assets to be installed

If you want, I can also explain:

* when to use `TARGET_DIR` vs `BINARIES_DIR`
* how `genimage.cfg` consumes `output/images`
* or how to drop DT overlays cleanly into the boot partition
