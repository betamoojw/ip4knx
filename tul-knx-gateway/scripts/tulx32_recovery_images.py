"""TULX32: flash the recovery bootloader at 0x0 and the recovery app at 0x2B0000.

The recovery system (bootloader with the S1 check + recovery app in the factory
partition) is built separately; its binaries are expected in tulx32_recovery/.
pioarduino's own bootloader would boot the recovery only with an empty otadata
and never on the button, so it is replaced here.
"""
import os
Import("env")  # noqa: F821

proj = env.subst("$PROJECT_DIR")
rec_dir = os.path.join(proj, "tulx32_recovery")
bootloader = os.path.join(rec_dir, "bootloader.bin")
recovery = os.path.join(rec_dir, "tulx32_recovery.bin")
missing = [p for p in (bootloader, recovery) if not os.path.isfile(p)]
if missing:
    raise SystemExit("tulx32_recovery_images: missing " + ", ".join(missing))

images = []
for offset, path in env.get("FLASH_EXTRA_IMAGES", []):
    if int(str(offset), 16) == 0x0:
        path = bootloader
    images.append((offset, path))
images.append(("0x2B0000", recovery))
env.Replace(FLASH_EXTRA_IMAGES=images)
print("tulx32_recovery_images: " + ", ".join("%s %s" % (o, os.path.basename(p)) for o, p in images))
