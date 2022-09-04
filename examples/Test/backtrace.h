#pragma once

#include <execinfo.h>
#include <malloc/_malloc.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <cxxabi.h>

//template <class T>
//inline std::string to_string(T t, std::ios_base & (*f)(std::ios_base&))
//{
//    std::ostringstream oss;
//    oss << f << t;
//    return oss.str();
//}

inline void PrintBacktrace()
{
    using namespace std;

    struct Row
    {
        int frame;
        string module;
        void* load_address;
        void* location;
        string symbol;
        size_t offset;
    };

    constexpr size_t MAX_STACK_SIZE = 1024;
    void* buffer[MAX_STACK_SIZE];

    const size_t stack_size = backtrace(buffer, MAX_STACK_SIZE);

    char** out = backtrace_symbols(buffer, stack_size);

    std::vector<Row> rows;

    for(size_t i=0; i < stack_size; ++i) {
        char* str = out[i];

        if(str) {
            vector<string> column;

            istringstream iss(str);
            copy(istream_iterator<string>(iss),
                 istream_iterator<string>(),
                 std::back_insert_iterator<vector<string>>(column)
                 );

            size_t offset = stoi(column[5]);
            char* addr = (char*)nullptr + strtoul(column[2].data(), nullptr, 0);
            char* load_address = addr - offset;

            if(column.size() == 6) {
                Row row {
                    stoi(column[0]), // frame
                    column[1],       // module
                    load_address,    // not sure about this...
                    addr,            //location
                    column[3],       // symbol
                    offset           // offset
                };


                int status;
                std::unique_ptr<char, decltype(free)*> demangle_name(abi::__cxa_demangle(row.symbol.c_str(), 0, 0, &status), free);
                if(demangle_name) {
                    row.symbol = demangle_name.get();
                }

                rows.push_back(row);
            }
        }else{
            break;
        }
    }

    for(auto row : rows) {
        std::cout << row.frame << ": " << row.module << ", " /*<< row.load_address << ", "*/ << row.location << ", " << row.symbol << " + " << row.offset << std::endl;
    }

    free(out);
}
