#include "disk_geometry.h"

#include <iostream>
#include <memory>

int main()
{
    std::shared_ptr<DiskGeometry> disk_geometry = nullptr;
#if defined(_WIN32) || defined(_WIN64)
    // to get the physical device use the following command in Powershell: wmic diskdrive list brief
    disk_geometry = std::make_shared<WindowsDiskGeometry>(R"(\\.\PhysicalDrive1)");
#elif defined(__linux__)
    // to get the physical device use the following command in Terminal: lsblk
    disk_geometry = std::make_shared<LinuxDiskGeometry>("/dev/sda");
#endif
    std::vector<Partition> partitions = disk_geometry->get_partitions();

    std::cout << "Number of partitions: " << partitions.size() << std::endl;
    std::cout << "get_disk_total_sectors: " << disk_geometry->get_disk_total_sectors() << std::endl;
    std::cout << "get_bytes_per_sector: " << disk_geometry->get_bytes_per_sector() << std::endl;
    std::cout << "get_physical_drive: " << disk_geometry->get_physical_drive() << std::endl;
    Partition partition;
    for (size_t i = 0; i != partitions.size(); ++i)
    {
        if (partitions[i].is_unallocated)
        {
            std::cout << "Unallocated space " << i + 1 << ":\n";
            partition = partitions[i];
        }
        else
        {
            std::cout << "Partition " << i + 1 << ":\n";
        }
        std::cout << "  Start Sector: " << partitions[i].start_sector << "\n";
        std::cout << "  End Sector: " << partitions[i].end_sector << "\n";
    }

    std::string string_data{"Lorem ipsum odor amet, consectetuer adipiscing elit. Feugiat amet nunc neque eros; nulla class. Parturient sociosqu eget donec praesent molestie ligula ligula. Nisl quisque hendrerit pharetra suspendisse quis gravida velit. Venenatis facilisi efficitur venenatis facilisi molestie est tempor magnis. Nisl malesuada bibendum finibus habitasse blandit nulla; maximus donec. Commodo litora enim nostra neque in. Fringilla dapibus interdum vitae arcu ligula. Gravida integer ullamcorper nibh dui egestas litora. Volutpat curabitur sociosqu molestie at gravida fringilla egestas vestibulum ante. Congue ridiculus turpis suscipit dapibus gravida taciti sapien vestibulum laoreet. Eleifend sed integer purus primis augue sed non. Tristique vestibulum suscipit augue ridiculus; tincidunt urna semper interdum. Vitae platea in amet senectus urna posuere maecenas. Aptent class vitae lacus nostra efficitur; tortor gravida dictumst. Varius lacus magna posuere gravida, turpis a mus sit dictum. Urna ullamcorper gravida consectetur morbi augue hendrerit consequat dictum. Ad integer rhoncus quam nostra feugiat? Vehicula mus morbi vestibulum montes vel scelerisque. Consectetur orci in montes in ipsum. Torquent litora habitant vivamus, congue praesent litora per elementum potenti. Porttitor suscipit quis convallis quam aenean aenean feugiat ex sagittis. Accumsan dis faucibus; proin finibus imperdiet diam. Accumsan per at nostra ligula nisi egestas lacinia mattis. Aptent id adipiscing senectus bibendum habitasse diam tincidunt interdum habitant. Gravida neque rhoncus pulvinar nostra pretium venenatis. Ipsum enim fusce metus consectetur commodo nulla dictum in congue. Eu suscipit dignissim cursus nostra iaculis enim sollicitudin rhoncus. Molestie hendrerit volutpat efficitur cubilia praesent fusce ultricies molestie odio. Ad purus imperdiet; lectus a ultricies dis? Aegestas aliquet egestas adipiscing dapibus interdum ad. Dis tristique elementum velit lacinia morbi porttitor condimentum velit laoreet. Quis sagittis tempor elit, hendrerit nec non. Suscipit etiam netus ridiculus sociosqu eros nunc risus. Consectetur enim sollicitudin netus, senectus ultricies luctus. At hendrerit integer euismod velit vulputate placerat tempus. Iaculis condimentum vehicula primis dictumst congue facilisi dictumst. Viverra dolor ridiculus arcu augue inceptos nunc. Dis penatibus convallis tortor montes facilisis molestie euismod sapien. Ridiculus nostra pharetra lobortis phasellus libero. Natoque vulputate neque quis laoreet platea aliquet. Dis ridiculus fusce inceptos cras quis convallis. Ut eleifend pulvinar vel sollicitudin sollicitudin. Cursus metus semper mi per adipiscing vulputate quam, tristique tempor. Felis lobortis id vulputate accumsan ullamcorper nam. Aptent dis porttitor massa turpis vehicula maximus maecenas amet. Efficitur rhoncus neque vestibulum dignissim per volutpat. Suscipit fringilla fames pellentesque ipsum suscipit ultricies phasellus condimentum. Efficitur tellus erat venenatis nullam faucibus ante imperdiet auctor nisi. Inceptos integer dictum dignissim porta; primis himenaeos praesent potenti. Varius congue natoque habitant potenti adipiscing vulputate. Massa proin gravida bibendum euismod purus hac. Vivamus etiam vivamus eros magna dignissim velit ligula varius dui. Tristique torquent euismod nunc bibendum eleifend placerat porttitor justo dolor. Urna magnis diam mattis vivamus aliquet tristique. Egestas malesuada non congue aliquam ligula diam venenatis hac. Eget netus eleifend magna elementum parturient. Interdum mus pellentesque integer et habitant posuere imperdiet fringilla? In viverra pretium penatibus laoreet ridiculus lacus nisi. Iaculis metus tempus justo inceptos facilisi ultrices dui. Donec commodo dolor nisl semper orci torquent. Aliquam ante sagittis convallis posuere hendrerit quam rutrum torquent. Ante molestie nec aliquet senectus dui magnis fusce class cubilia. Donec potenti dis at pharetra magna massa. Risus pharetra facilisi leo dis taciti ultrices auctor. Malesuada elementum et aptent tellus primis porttitor laoreet. Fames fringilla nisl consectetur at adipiscing. Commodo ad vel bibendum dis ad aptent. Vestibulum metus mattis augue mus fringilla ligula. Viverra vivamus libero morbi enim libero luctus quis efficitur. Pharetra vestibulum nostra nec molestie ipsum velit ad. Primis pretium porttitor eget imperdiet interdum; lobortis maecenas magnis. Cubilia nullam ipsum quisque, mattis lacinia etiam. Aptent malesuada mollis taciti cras pulvinar aliquet posuere rhoncus. Conubia dictumst maecenas nullam parturient sapien metus. Euismod varius aenean tempor netus; semper enim sollicitudin."};
    std::vector<int8_t> data(string_data.begin(), string_data.end());
    std::error_code write_ec;
    size_t bytes_written = disk_geometry->write_data(partition, partition.start_sector, data.data(), data.size(), write_ec);
    std::error_code read_ec;
    std::vector<int8_t> data_read = disk_geometry->read_data(partition, partition.start_sector, data.size(), read_ec);

    for (size_t i = 0; i != data_read.size(); ++i)
    {
        std::cout << data_read[i];
    }
    std::cout << std::endl;

    // Display the data in hexadecimal format
    for (size_t i = 0; i < data_read.size(); ++i)
    {
        printf("%02X ", static_cast<unsigned char>(data_read[i]));
        if ((i + 1) % 16 == 0)
        {
            printf("\n");
        }
    }
    std::cout << std::endl;
    return 0;
}