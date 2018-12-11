// Code written by Daniel Ehrenberg, released into the public domain

//#include <pangolin/packet2/IostreamFile.h>
//#include <pangolin/packet2/LinuxAsioFile.h>
#include <pangolin/packet2/MemmappedFile.h>
#include <pangolin/utils/timer.h>


int main(int argc, char *argv[])
{
    using namespace pangolin;

    const std::string filename = "test.dat";
    constexpr size_t pagesize  = 4096000;
    auto test_data = std::shared_ptr<uint8_t>(new uint8_t[pagesize], [](uint8_t* p){delete[] p;});

    Timer t;
    if(1) {
        MemoryMappedFile file(filename);
        file.Clear();

        for(int i=0; i < 1000; ++i)
        {
            //            for(size_t j=0; j < pagesize; ++j) {
            //                test_data.get()[j] = j;
            //            }
            file.PutAsync( test_data, pagesize);
        }

        std::cout << "Finished submitting." << std::endl;
    }else{
        int fd = ::open(filename.c_str(), O_RDWR | O_CREAT, 0644);
        for(int i=0; i < 1000; ++i)
        {
            ::write(fd, test_data.get(), pagesize);
        }
        std::cout << "Finished submitting." << std::endl;
        ::sync();
    }
    std::cout << t.Elapsed_s() << std::endl;

    return 0;
}
