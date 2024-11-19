#ifndef DISK_GEOMETRY_H
#define DISK_GEOMETRY_H

#include <cstdint>
#include <string>
#include <system_error>
#include <vector>


struct Partition {
  Partition() : start_sector(0), end_sector(0), is_unallocated(false) {}
  Partition(const uint64_t p_start_sector, const uint64_t p_end_sector,
            bool p_is_unallocated)
      : start_sector(p_start_sector), end_sector(p_end_sector),
        is_unallocated(p_is_unallocated) {}
  uint64_t start_sector;
  uint64_t end_sector;
  bool is_unallocated;
};

class DiskGeometry {

public:
  DiskGeometry(const std::string &p_physical_drive);
  DiskGeometry(const uint64_t &p_disk_size, const uint32_t &p_bytes_per_sector);
  uint64_t get_disk_total_sectors() const;
  std::string get_physical_drive() const;
  uint32_t get_bytes_per_sector() const;
  std::vector<Partition> get_partitions() const;
  virtual size_t write_data(const Partition &p_partition,
                            size_t p_starting_sector, const int8_t *p_data,
                            size_t p_data_size, std::error_code &p_ec) = 0;
  virtual std::vector<int8_t> read_data(const Partition &p_partition,
                                        size_t p_starting_sector,
                                        size_t p_read_size,
                                        std::error_code &p_ec) = 0;

protected:
  uint64_t disk_size;
  uint32_t bytes_per_sector;
  std::string physical_drive;
  std::vector<Partition> partitions;
};

#if defined(_WIN32) || defined(_WIN64)

class WindowsDiskGeometry : public DiskGeometry {
public:
  WindowsDiskGeometry(const std::string &p_physical_drive);
  wchar_t *string_to_wchar_ptr(const std::string &str);

  size_t write_data(const Partition &p_partition, size_t p_starting_sector,
                    const int8_t *p_data, size_t p_data_size,
                    std::error_code &p_ec) override;
  std::vector<int8_t> read_data(const Partition &p_partition,
                                size_t p_starting_sector, size_t p_read_size,
                                std::error_code &p_ec) override;
};

#elif defined(__linux__)

class LinuxDiskGeometry : public DiskGeometry {
public:
  LinuxDiskGeometry(const std::string &p_physical_drive);

  size_t write_data(const Partition &p_partition, size_t p_starting_sector,
                    const int8_t *p_data, size_t p_data_size,
                    std::error_code &p_ec) override;
  std::vector<int8_t> read_data(const Partition &p_partition,
                                size_t p_starting_sector, size_t p_read_size,
                                std::error_code &p_ec) override;
};

#endif

#endif // !DISK_GEOMETRY_H
