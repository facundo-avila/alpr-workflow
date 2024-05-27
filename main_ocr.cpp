#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>

#pragma pack(push, 1)
struct BMPHeader {
    uint16_t bfType;
    uint32_t bfSize;
    uint16_t bfReserved1;
    uint16_t bfReserved2;
    uint32_t bfOffBits;
};

struct BMPInfoHeader {
    uint32_t biSize;
    int32_t  biWidth;
    int32_t  biHeight;
    uint16_t biPlanes;
    uint16_t biBitCount;
    uint32_t biCompression;
    uint32_t biSizeImage;
    int32_t  biXPelsPerMeter;
    int32_t  biYPelsPerMeter;
    uint32_t biClrUsed;
    uint32_t biClrImportant;
};
#pragma pack(pop)

std::vector<uint8_t> readBMP(const std::string& filename, BMPInfoHeader& infoHeader) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Cannot open file: " + filename);
    }

    BMPHeader header;
    file.read(reinterpret_cast<char*>(&header), sizeof(header));
    if (header.bfType != 0x4D42) { // 'BM' in hexadecimal
        throw std::runtime_error("Not a BMP file: " + filename);
    }

    file.read(reinterpret_cast<char*>(&infoHeader), sizeof(infoHeader));

    file.seekg(header.bfOffBits, std::ios::beg);

    std::vector<uint8_t> pixels(infoHeader.biSizeImage);
    file.read(reinterpret_cast<char*>(pixels.data()), pixels.size());
    file.close();

    return pixels;
}

double compareBMP(const std::string& file1, const std::string& file2) {
    BMPInfoHeader infoHeader1, infoHeader2;
    std::vector<uint8_t> pixels1 = readBMP(file1, infoHeader1);
    std::vector<uint8_t> pixels2 = readBMP(file2, infoHeader2);

    if (infoHeader1.biWidth != infoHeader2.biWidth || infoHeader1.biHeight != infoHeader2.biHeight || infoHeader1.biBitCount != infoHeader2.biBitCount) {
        throw std::runtime_error("BMP files do not match in dimensions or bit depth");
    }

    size_t totalPixels = infoHeader1.biWidth * infoHeader1.biHeight;
    size_t matchingPixels = 0;

    for (size_t i = 0; i < pixels1.size(); ++i) {
        if (pixels1[i] == pixels2[i]) {
            ++matchingPixels;
        }
    }

    return (static_cast<double>(matchingPixels) / pixels1.size()) * 100.0;
}

int main() {
    try {
        std::string file1 = "image1.bmp";
        std::string file2 = "image2.bmp";
        double similarity = compareBMP(file1, file2);
        std::cout << "The images are " << similarity << "% similar." << std::endl;
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        return 1;
    }
    return 0;
}
