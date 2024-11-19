# rawdisk

This project allows you to write data to the unallocated partition on your device, such as a USB drive or hard drive.

### Why do you want to write data to an unallocated partition?

You may want to prevent the system (such as Windows or Linux) from identifying, recognizing, and automatically mounting your partition, which could potentially transfer a virus from the host machine to your USB device. To avoid this, you need to format the USB drive to make it an unallocated partition. Afterwards, you can read and write data to this device using a special program that performs low-level operations and interacts directly with the disk sectors. By doing this, only you can understand the underlying data format that has been written to the USB stick.

To compile this program, modify the device ID in `main.cpp` to point to your device. Then, compile it using `scons` and run it with `sudo` (on Linux/macOS) or administrator privileges (on Windows).

```
scons
```

and then

```
sudo ./bin/disk.out
```
