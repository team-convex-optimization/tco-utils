# Mendel Patches

In this directory, we present some patches to the mendel kernel/drivers to help things go a little smoother with this project. In case you get lazy, you can just apply the kernel we used (steps 4-7 of ov5645 section). The kernels update is left in the same directory of this readme.

## ov5645 mipi 720@60fps

For some reason, the ov5645 driver did not support the 720p@60fps. This is the exact sensor in the coral camera. This patch allows 720p@60fps without loss of other features (AE, AF, WB, etc).

To install this, you will need to:
1) Clone and build the Google Mendel Repository: https://coral.googlesource.com/docs/+/refs/heads/master/GettingStarted.md
2) Apply the patch/change the file provided in `ov5645/`. The patch must be applied in the following directory `linux-imx/drivers/media/platform/mxc/capture`. 
3) Build the kernel again with `m docker-linux-imx`
4) Copy over the deb update to the coral board `scp linux-image-4.14.98-imx_12-4_arm64.deb mendel@IP.OF.MDL.Brd:`
5) SSH into the mendel and apply the patch with `sudo dpkg -i linux-image-4.14.98-imx_12-4_arm64.deb`
6) Reboot
7) You will now see the 720p@60fps in the v4l2 format list `sudo v4l2-ctl --list-formats-ext`
