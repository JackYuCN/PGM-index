/*
 * This example shows how to index and query a vector of random integers with the PGM-index.
 * Compile with:
 *   g++ simple.cpp -std=c++17 -I../include -o simple
 * Run with:
 *   ./simple
 */

#include <vector>
#include <cstdlib>
#include <iostream>
#include <algorithm>
#include "pgm/pgm_index.hpp"
#include <fstream>
#include <iterator>
#include <filesystem>
#include <assert.h>

int main(int argc, char *argv[]) {

    char arr[1000];
    std::ifstream inputFile(argv[1]);
    std::ofstream outputFile(argv[2]);

    // Check if the file is opened successfully
    if (!inputFile.is_open() || !outputFile.is_open()) {
        std::cerr << "Error: Unable to open file!" << std::endl;
        return 1;
    }

    // Get the size of the file
    inputFile.seekg(0, std::ios::end);
    std::streampos fileSize = inputFile.tellg();
    inputFile.seekg(0, std::ios::beg);

    // Calculate an estimate of the number of elements based on the size of the file
    std::vector<unsigned long> data;
    data.reserve(fileSize / sizeof(unsigned long)); 

    // Read data from the file and store it in the vector
    unsigned long addr;
    while (inputFile >> std::hex >> addr) {
        data.push_back(addr);
    }

    // Close the file
    inputFile.close();

    // Construct the PGM-index
    const int epsilon = 1;
    pgm::PGMIndex<unsigned long, epsilon> index(data);

    auto first_level = index.get_first_level();
    int32_t start_index = 0;
    outputFile << ",start_addr,end_addr,start_index,end_index,stride,addr_space_cover,hot_page_cover,dram_fit,dram_gap,migration_cnt,mispredict,mispredict_percent\n";
    for (auto i = 0; i < first_level.size() - 1; ++i) {
        auto seg = first_level[i];
        auto hpc = seg.hot_cnt;
        auto end_index = start_index + hpc - 1;
        auto asc = seg.end_key - seg.key + 1; // asc->addr_space_cover
        // 统计预测情况
        auto mispredict = 0;
        for (auto j = start_index; j <= end_index; j++) {
            auto key = data[j];
            auto range = index.search(key);
            if (range.pos - range.lo > epsilon || range.hi - range.pos > epsilon + 3) {
                printf("ERROR: {low, pos, hi} = {%ld, %ld, %ld}\n", range.lo, range.pos, range.hi);
                exit(-1);
            }    
            if (range.pos != j) {
                mispredict++;
            }
        }
        auto percent = static_cast<float>(mispredict) / hpc * 100;
        snprintf(arr, 1000, "%d,0x%lx,0x%lx,%d,%d,%f,%ld,%d,%d,%ld,%ld,%d,%.2f%%\n", \
            i, seg.key, seg.end_key, start_index, end_index, seg.slope, asc, hpc, hpc, asc - hpc, seg.migration_list.size(), mispredict, percent);
        outputFile << arr;
        start_index += hpc;
    }
    outputFile.close();

    return 0;
}