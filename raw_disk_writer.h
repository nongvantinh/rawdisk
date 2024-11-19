#ifndef RAW_DISK_WRITER_H
#define RAW_DISK_WRITER_H

#include <cstdint>

class RawDiskWriter
{

public:
    void write(uint64_t p_start_sector, const int8_t *p_data, const uint64_t p_data_size);
};

#endif