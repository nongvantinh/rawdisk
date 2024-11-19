
#include "disk_geometry.h"

#include <algorithm>
#include <cerrno>
#include <cmath>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#elif defined(__linux__)
#include <fcntl.h>
#include <fstream>
#include <linux/fs.h>
#include <linux/hdreg.h>
#include <sstream>
#include <sys/ioctl.h>
#include <unistd.h>

#elif defined(__APPLE__)
std::cout << "This is a macOS platform!" << std::endl;
#else
std::cout << "Unknown platform!" << std::endl;
#endif

DiskGeometry::DiskGeometry(const std::string &p_physical_drive)
    : physical_drive(p_physical_drive) {}

DiskGeometry::DiskGeometry(const uint64_t &p_disk_size,
                           const uint32_t &p_bytes_per_sector)
    : disk_size(p_disk_size), bytes_per_sector(p_bytes_per_sector) {}

uint64_t DiskGeometry::get_disk_total_sectors() const {
  return disk_size / bytes_per_sector;
}

std::string DiskGeometry::get_physical_drive() const { return physical_drive; }

uint32_t DiskGeometry::get_bytes_per_sector() const { return bytes_per_sector; }

std::vector<Partition> DiskGeometry::get_partitions() const {
  return partitions;
}

#if defined(_WIN32) || defined(_WIN64)

size_t WindowsDiskGeometry::write_data(const Partition &p_partition,
                                       size_t p_starting_sector,
                                       const int8_t *p_data, size_t p_data_size,
                                       std::error_code &p_ec) {

  if (p_starting_sector < p_partition.start_sector ||
      p_partition.end_sector < p_starting_sector) {
    throw std::out_of_range("Writing outside the partition");
  }

  if (p_partition.end_sector < p_starting_sector + p_data_size) {
    throw std::out_of_range("Writing outside the partition");
  }

  wchar_t *physical_drive = string_to_wchar_ptr(get_physical_drive());
  HANDLE h_device = CreateFile(physical_drive, GENERIC_WRITE,
                               FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
                               OPEN_EXISTING, 0, NULL);

  free(physical_drive);

  if (h_device == INVALID_HANDLE_VALUE) {
    DWORD dwError = GetLastError();
    p_ec = std::error_code(dwError, std::system_category());
    std::cerr << "Error: Could not open the USB device. Error code: "
              << p_ec.message().c_str() << std::endl;
    return 0;
  }

  LARGE_INTEGER offset;
  offset.QuadPart = p_starting_sector * get_bytes_per_sector();
  size_t total_written = 0;
  DWORD bytes_written;

  while (total_written < p_data_size) {
    DWORD chunk_size = get_bytes_per_sector();

    if (!SetFilePointerEx(h_device, offset, NULL, FILE_BEGIN)) {
      DWORD dwError = GetLastError();
      p_ec = std::error_code(dwError, std::system_category());
      std::cerr << "Error: Could not set the file pointer. Error code: "
                << p_ec.message().c_str() << std::endl;
      CloseHandle(h_device);
      return total_written;
    }

    if (!WriteFile(h_device, &p_data[total_written], chunk_size, &bytes_written,
                   NULL)) {
      DWORD dwError = GetLastError();
      p_ec = std::error_code(dwError, std::system_category());
      std::cerr << "Error: Write operation failed. Error code: "
                << p_ec.message().c_str() << std::endl;
      CloseHandle(h_device);
      return total_written;
    }

    std::cout << "Chunk written successfully. Bytes written: " << bytes_written
              << std::endl;

    offset.QuadPart += get_bytes_per_sector();
    total_written += chunk_size;
  }

  std::cout << "All data written successfully." << std::endl;
  CloseHandle(h_device);
  return total_written;
}

std::vector<int8_t> WindowsDiskGeometry::read_data(const Partition &p_partition,
                                                   size_t p_starting_sector,
                                                   size_t p_read_size,
                                                   std::error_code &p_ec) {
  if (p_starting_sector < p_partition.start_sector ||
      p_partition.end_sector < p_starting_sector) {
    throw std::out_of_range("Reading outside the partition");
  }

  if (p_partition.end_sector < p_starting_sector + p_read_size) {
    throw std::out_of_range("Reading outside the partition");
  }

  wchar_t *physical_drive = string_to_wchar_ptr(get_physical_drive());
  HANDLE h_device = CreateFile(physical_drive, GENERIC_READ,
                               FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
                               OPEN_EXISTING, 0, NULL);

  free(physical_drive);

  if (h_device == INVALID_HANDLE_VALUE) {
    DWORD dwError = GetLastError();
    p_ec = std::error_code(dwError, std::system_category());
    std::cerr
        << "Error: Could not open the USB device for reading. Error code: "
        << p_ec.message().c_str() << std::endl;
    return std::vector<int8_t>();
  }

  LARGE_INTEGER offset;
  offset.QuadPart = p_starting_sector * get_bytes_per_sector();

  if (!SetFilePointerEx(h_device, offset, NULL, FILE_BEGIN)) {
    DWORD dwError = GetLastError();
    p_ec = std::error_code(dwError, std::system_category());
    std::cerr << "Error: Could not set the file pointer. Error code: "
              << p_ec.message().c_str() << std::endl;
    CloseHandle(h_device);
    return std::vector<int8_t>();
  }

  size_t total_size = static_cast<size_t>(std::ceil(
                          p_read_size / float(get_bytes_per_sector()))) *
                      get_bytes_per_sector();
  std::vector<int8_t> buffer(total_size);
  DWORD bytes_read;
  size_t total_bytes_read = 0;
  while (total_bytes_read < p_read_size) {
    DWORD bytes_to_read = get_bytes_per_sector();
    if (!ReadFile(h_device, buffer.data() + total_bytes_read, bytes_to_read,
                  &bytes_read, NULL)) {
      DWORD dwError = GetLastError();
      p_ec = std::error_code(dwError, std::system_category());
      std::cerr << "Error: Read operation failed. Error code: "
                << p_ec.message().c_str() << std::endl;
      CloseHandle(h_device);
      return buffer;
    }

    total_bytes_read += bytes_read;
    if (bytes_read == 0) {
      break;
    }
    std::cout << "Data read successfully. Bytes read: " << bytes_read
              << std::endl;
  }

  std::cout << "All data read successfully." << std::endl;
  CloseHandle(h_device);
  buffer.resize(p_read_size);
  return buffer;
}

wchar_t *WindowsDiskGeometry::string_to_wchar_ptr(const std::string &str) {
  int size_needed = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);
  if (size_needed == 0) {
    return nullptr;
  }

  wchar_t *result = new wchar_t[size_needed];
  MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, result, size_needed);
  return result;
}

WindowsDiskGeometry::WindowsDiskGeometry(const std::string &p_physical_drive)
    : DiskGeometry(p_physical_drive) {
  wchar_t *physical_drive = string_to_wchar_ptr(p_physical_drive);
  HANDLE h_device = CreateFile(physical_drive, GENERIC_READ,
                               FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
                               OPEN_EXISTING, 0, NULL);

  free(physical_drive);

  if (h_device == INVALID_HANDLE_VALUE) {
    DWORD dwError = GetLastError();
    std::cerr << "Failed to open physical drive. Error code: " << dwError
              << std::endl;

    throw std::runtime_error("Error: Could not open the device for reading.\n");
  }

  // Get total disk size
  DISK_GEOMETRY_EX disk_geometry;
  DWORD bytes_returned = 0;
  if (!DeviceIoControl(h_device, IOCTL_DISK_GET_DRIVE_GEOMETRY_EX, NULL, 0,
                       &disk_geometry, sizeof(disk_geometry), &bytes_returned,
                       NULL)) {
    CloseHandle(h_device);
    throw std::runtime_error("Error: Failed to retrieve disk geometry.\n");
  }

  bytes_per_sector = disk_geometry.Geometry.BytesPerSector;

  // Get partition information
  DRIVE_LAYOUT_INFORMATION_EX *p_drive_layout =
      (DRIVE_LAYOUT_INFORMATION_EX *)malloc(
          sizeof(DRIVE_LAYOUT_INFORMATION_EX) +
          (sizeof(PARTITION_INFORMATION_EX) * 128));
  if (!DeviceIoControl(h_device, IOCTL_DISK_GET_DRIVE_LAYOUT_EX, NULL, 0,
                       p_drive_layout,
                       sizeof(DRIVE_LAYOUT_INFORMATION_EX) +
                           (sizeof(PARTITION_INFORMATION_EX) * 128),
                       &bytes_returned, NULL)) {
    CloseHandle(h_device);
    free(p_drive_layout);
    throw std::runtime_error("Error: Failed to retrieve the drive layout.\n");
  }

  for (DWORD i = 0; i < p_drive_layout->PartitionCount; ++i) {
    PARTITION_INFORMATION_EX partition = p_drive_layout->PartitionEntry[i];
    if (partition.PartitionStyle != PARTITION_STYLE_MBR &&
        partition.PartitionStyle != PARTITION_STYLE_GPT) {
      continue;
    }

    ULONGLONG start_sector =
        partition.StartingOffset.QuadPart / bytes_per_sector;
    ULONGLONG end_sector =
        start_sector + (partition.PartitionLength.QuadPart / bytes_per_sector) -
        1;

    partitions.push_back(Partition(start_sector, end_sector, false));
  }

  // unallocated space
  {
    std::sort(partitions.begin(), partitions.end(),
              [](const Partition &a, const Partition &b) {
                return a.start_sector < b.start_sector;
              });

    std::vector<Partition> primary_partitions(partitions);
    ULONGLONG previous_end_sector = 0;

    for (const auto &partition : primary_partitions) {
      ULONGLONG start = partition.start_sector;
      if (previous_end_sector < start - 1) {
        partitions.push_back(
            Partition(previous_end_sector + 1, start - 1, true));
      }
      previous_end_sector = partition.end_sector;
    }

    // Check space after the last partition
    ULONGLONG diskTotalSectors =
        disk_geometry.DiskSize.QuadPart / disk_geometry.Geometry.BytesPerSector;
    if (previous_end_sector < diskTotalSectors - 1) {
      partitions.push_back(Partition(
          previous_end_sector + 1, disk_geometry.DiskSize.QuadPart - 1, true));
    }
  }

  std::sort(partitions.begin(), partitions.end(),
            [](const Partition &a, const Partition &b) {
              return a.start_sector < b.start_sector;
            });

  CloseHandle(h_device);
  free(p_drive_layout);
}
#elif defined(__linux__)

LinuxDiskGeometry::LinuxDiskGeometry(const std::string &p_physical_drive)
    : DiskGeometry(p_physical_drive) {
  int fd = open(p_physical_drive.c_str(), O_RDONLY);
  if (fd == -1) {
    std::error_code ec = std::error_code(errno, std::generic_category());
    std::cerr << "Error: Could not open the device. " << ec.message()
              << std::endl;
    throw std::runtime_error("Error: Could not open the device. " +
                             ec.message());
  }

  if (ioctl(fd, BLKSSZGET, &bytes_per_sector) == -1) {
    std::error_code ec = std::error_code(errno, std::generic_category());
    std::cerr << "Error: Could not get sector size. " << ec.message()
              << std::endl;
    close(fd);
    throw std::runtime_error("Error: Could not get sector size. " +
                             ec.message());
  }

  if (ioctl(fd, BLKGETSIZE64, &disk_size) == -1) {
    std::error_code ec = std::error_code(errno, std::generic_category());
    std::cerr << "Error: Could not get total disk size. " << ec.message()
              << std::endl;
    close(fd);
    throw std::runtime_error("Error: Could not get total disk size. " +
                             ec.message());
  }

  close(fd);

  // List partition paths (e.g., /sys/block/sda/sda1)
  for (int i = 1;; ++i) {
    std::ostringstream partition_path_start;
    partition_path_start << "/sys/block/" << physical_drive.substr(5) << "/"
                         << physical_drive.substr(5) << i << "/start";

    std::ifstream start_file(partition_path_start.str());
    if (!start_file.is_open()) {
      break;
    }

    size_t start_sector;
    start_file >> start_sector;
    start_file.close();

    std::ostringstream partition_path_size;
    partition_path_size << "/sys/block/" << physical_drive.substr(5) << "/"
                        << physical_drive.substr(5) << i << "/size";

    std::ifstream size_file(partition_path_size.str());
    if (!size_file.is_open()) {
      break;
    }

    size_t num_sectors;
    size_file >> num_sectors;
    size_file.close();

    Partition partition(start_sector, start_sector + num_sectors - 1, false);
    partitions.push_back(partition);
  }

  std::sort(partitions.begin(), partitions.end(),
            [](const Partition &a, const Partition &b) {
              return a.start_sector < b.start_sector;
            });

  size_t previous_end_sector = 0;

  std::vector<Partition> primary_partitions(partitions);
  for (const Partition &partition : primary_partitions) {
    if (previous_end_sector < partition.start_sector - 1) {
      partitions.push_back(
          {previous_end_sector + 1, partition.start_sector - 1, true});
    }
    previous_end_sector = partition.end_sector;
  }

  if (previous_end_sector < disk_size - 1) {
    partitions.push_back({previous_end_sector + 1, disk_size - 1, true});
  }

  std::sort(partitions.begin(), partitions.end(),
            [](const Partition &a, const Partition &b) {
              return a.start_sector < b.start_sector;
            });
}

std::vector<int8_t> LinuxDiskGeometry::read_data(const Partition &p_partition,
                                                 size_t p_starting_sector,
                                                 size_t p_read_size,
                                                 std::error_code &p_ec) {

  if (p_starting_sector < p_partition.start_sector ||
      p_partition.end_sector < p_starting_sector) {
    throw std::out_of_range("Reading outside the partition");
  }

  if (p_partition.end_sector < p_starting_sector + p_read_size) {
    throw std::out_of_range("Reading outside the partition");
  }

  int fd = open(physical_drive.c_str(), O_RDONLY);
  if (fd == -1) {
    p_ec = std::error_code(errno, std::generic_category());
    std::cerr << "Error: Could not open the device. " << p_ec.message()
              << std::endl;
    return {};
  }

  off_t offset = p_starting_sector * bytes_per_sector;
  if (lseek(fd, offset, SEEK_SET) == (off_t)-1) {
    p_ec = std::error_code(errno, std::generic_category());
    std::cerr << "Error: Could not seek to the specified sector. "
              << p_ec.message() << std::endl;
    close(fd);
    return {};
  }

  size_t total_size = static_cast<size_t>(std::ceil(
                          p_read_size / float(get_bytes_per_sector()))) *
                      get_bytes_per_sector();
  std::vector<int8_t> buffer(total_size);

  ssize_t bytes_read = read(fd, buffer.data(), total_size);
  if (bytes_read == -1) {
    p_ec = std::error_code(errno, std::generic_category());
    std::cerr << "Error: Read operation failed. " << p_ec.message()
              << std::endl;
    close(fd);
    return {};
  }

  buffer.resize(p_read_size);
  close(fd);
  return buffer;
}

size_t LinuxDiskGeometry::write_data(const Partition &p_partition,
                                     size_t p_starting_sector,
                                     const int8_t *p_data, size_t p_data_size,
                                     std::error_code &p_ec) {

  if (p_starting_sector < p_partition.start_sector ||
      p_partition.end_sector < p_starting_sector) {
    throw std::out_of_range("Writing outside the partition");
  }

  if (p_partition.end_sector < p_starting_sector + p_data_size) {
    throw std::out_of_range("Writing outside the partition");
  }

  int fd = open(physical_drive.c_str(), O_WRONLY);
  if (fd == -1) {
    p_ec = std::error_code(errno, std::generic_category());
    std::cerr << "Error: Could not open the device. " << p_ec.message()
              << std::endl;
    return 0;
  }

  off_t offset = p_starting_sector * bytes_per_sector;
  if (lseek(fd, offset, SEEK_SET) == (off_t)-1) {
    p_ec = std::error_code(errno, std::generic_category());
    std::cerr << "Error: Could not seek to the specified sector. "
              << p_ec.message() << std::endl;
    close(fd);
    return 0;
  }

  size_t total_written = 0;
  while (total_written < p_data_size) {
    size_t chunk_size = bytes_per_sector;
    ssize_t bytes_written = write(fd, p_data + total_written, chunk_size);
    if (bytes_written == -1) {
      p_ec = std::error_code(errno, std::generic_category());
      std::cerr << "Error: Write operation failed. " << p_ec.message()
                << std::endl;
      close(fd);
      return total_written;
    }

    total_written += bytes_written;
  }

  std::cout << "All data written successfully." << std::endl;
  close(fd);
  return total_written;
}

#endif