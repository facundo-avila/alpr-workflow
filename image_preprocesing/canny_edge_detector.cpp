#include <iostream>
#include <vector>
#include <cmath>
#include <fstream>
#include <cstdint>

#define PI 3.14159265

#pragma pack(push, 1)
struct BMPHeader {
    uint16_t type;
    uint32_t size;
    uint16_t reserved1;
    uint16_t reserved2;
    uint32_t offset;
};

struct DIBHeader {
    uint32_t size;
    int32_t width;
    int32_t height;
    uint16_t planes;
    uint16_t bitCount;
    uint32_t compression;
    uint32_t imageSize;
    int32_t xPixelsPerMeter;
    int32_t yPixelsPerMeter;
    uint32_t colorsUsed;
    uint32_t importantColors;
};
#pragma pack(pop)

// Function to read a BMP image
bool read_bmp(const char* filename, std::vector<uint8_t>& image, int& width, int& height) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) return false;

    BMPHeader bmpHeader;
    DIBHeader dibHeader;

    file.read(reinterpret_cast<char*>(&bmpHeader), sizeof(BMPHeader));
    file.read(reinterpret_cast<char*>(&dibHeader), sizeof(DIBHeader));

    width = dibHeader.width;
    height = dibHeader.height;

    image.resize(width * height * 3);
    file.seekg(bmpHeader.offset, std::ios::beg);
    file.read(reinterpret_cast<char*>(image.data()), image.size());
    file.close();
    return true;
}

// Function to write a BMP image
bool write_bmp(const char* filename, const std::vector<uint8_t>& image, int width, int height) {
    std::ofstream file(filename, std::ios::binary);
    if (!file) return false;

    BMPHeader bmpHeader = { 0x4D42, static_cast<uint32_t>(54 + width * height * 3), 0, 0, 54 };
    DIBHeader dibHeader = { 40, width, height, 1, 24, 0, static_cast<uint32_t>(width * height * 3), 0, 0, 0, 0 };

    file.write(reinterpret_cast<const char*>(&bmpHeader), sizeof(BMPHeader));
    file.write(reinterpret_cast<const char*>(&dibHeader), sizeof(DIBHeader));
    file.write(reinterpret_cast<const char*>(image.data()), image.size());
    file.close();
    return true;
}

// Function to convert an RGB image to grayscale
std::vector<uint8_t> convert_to_grayscale(const std::vector<uint8_t>& image, int width, int height) {
    std::vector<uint8_t> gray_image(width * height);
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int idx = (y * width + x) * 3;
            uint8_t r = image[idx + 2];
            uint8_t g = image[idx + 1];
            uint8_t b = image[idx];
            gray_image[y * width + x] = static_cast<uint8_t>(0.299 * r + 0.587 * g + 0.114 * b);
        }
    }
    return gray_image;
}

// Function to apply Gaussian blur (simplified, fixed kernel)
void gaussian_blur(const std::vector<uint8_t>& src, std::vector<uint8_t>& dst, int width, int height) {
    const int kSize = 5;
    const float kernel[kSize][kSize] = {
        { 2 / 273.0,  4 / 273.0,  5 / 273.0,  4 / 273.0,  2 / 273.0},
        { 4 / 273.0,  9 / 273.0, 12 / 273.0,  9 / 273.0,  4 / 273.0},
        { 5 / 273.0, 12 / 273.0, 15 / 273.0, 12 / 273.0,  5 / 273.0},
        { 4 / 273.0,  9 / 273.0, 12 / 273.0,  9 / 273.0,  4 / 273.0},
        { 2 / 273.0,  4 / 273.0,  5 / 273.0,  4 / 273.0,  2 / 273.0}
    };

    dst.resize(width * height, 0);
    for (int y = 2; y < height - 2; ++y) {
        for (int x = 2; x < width - 2; ++x) {
            float sum = 0.0;
            for (int ky = -2; ky <= 2; ++ky) {
                for (int kx = -2; kx <= 2; ++kx) {
                    sum += src[(y + ky) * width + (x + kx)] * kernel[ky + 2][kx + 2];
                }
            }
            dst[y * width + x] = static_cast<uint8_t>(sum);
        }
    }
}

// Function to compute the gradient using Sobel operator
void sobel_operator(const std::vector<uint8_t>& src, std::vector<int>& grad_x, std::vector<int>& grad_y, int width, int height) {
    const int sobel_x[3][3] = {
        {-1, 0, 1},
        {-2, 0, 2},
        {-1, 0, 1}
    };

    const int sobel_y[3][3] = {
        {-1, -2, -1},
        { 0,  0,  0},
        { 1,  2,  1}
    };

    grad_x.resize(width * height, 0);
    grad_y.resize(width * height, 0);

    for (int y = 1; y < height - 1; ++y) {
        for (int x = 1; x < width - 1; ++x) {
            int gx = 0, gy = 0;
            for (int ky = -1; ky <= 1; ++ky) {
                for (int kx = -1; kx <= 1; ++kx) {
                    gx += src[(y + ky) * width + (x + kx)] * sobel_x[ky + 1][kx + 1];
                    gy += src[(y + ky) * width + (x + kx)] * sobel_y[ky + 1][kx + 1];
                }
            }
            grad_x[y * width + x] = gx;
            grad_y[y * width + x] = gy;
        }
    }
}

// Function to compute the gradient magnitude and direction
void gradient_magnitude_direction(const std::vector<int>& grad_x, const std::vector<int>& grad_y, std::vector<float>& magnitude, std::vector<float>& direction, int width, int height) {
    magnitude.resize(width * height, 0.0f);
    direction.resize(width * height, 0.0f);

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int idx = y * width + x;
            float gx = grad_x[idx];
            float gy = grad_y[idx];
            magnitude[idx] = std::sqrt(gx * gx + gy * gy);
            direction[idx] = std::atan2(gy, gx) * 180 / PI;
            if (direction[idx] < 0) {
                direction[idx] += 180;
            }
        }
    }
}

// Function to apply non-maximum suppression
void non_maximum_suppression(const std::vector<float>& magnitude, const std::vector<float>& direction, std::vector<uint8_t>& edges, int width, int height) {
    edges.resize(width * height, 0);

    for (int y = 1; y < height - 1; ++y) {
        for (int x = 1; x < width - 1; ++x) {
            int idx = y * width + x;
            float angle = direction[idx];
            float mag = magnitude[idx];

            float q = 255, r = 255;

            if ((0 <= angle && angle < 22.5) || (157.5 <= angle && angle <= 180)) {
                q = magnitude[idx + 1];
                r = magnitude[idx - 1];
            } else if (22.5 <= angle && angle < 67.5) {
                q = magnitude[idx + width + 1];
                r = magnitude[idx - width - 1];
            } else if (67.5 <= angle && angle < 112.5) {
                q = magnitude[idx + width];
                r = magnitude[idx - width];
            } else if (112.5 <= angle && angle < 157.5) {
                q = magnitude[idx - width + 1];
                r = magnitude[idx + width - 1];
            }

            if (mag >= q && mag >= r) {
                edges[idx] = static_cast<uint8_t>(mag);
            } else {
                edges[idx] = 0;
            }
        }
    }
}

// Function to apply double threshold and edge tracking by hysteresis
void double_threshold_and_hysteresis(const std::vector<uint8_t>& src, std::vector<uint8_t>& dst, int width, int height, uint8_t low_threshold, uint8_t high_threshold) {
    dst.resize(width * height, 0);

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int idx = y * width + x;
            if (src[idx] >= high_threshold) {
                dst[idx] = 255;
            } else if (src[idx] >= low_threshold) {
                bool strong_edge = false;
                for (int ky = -1; ky <= 1; ++ky) {
                    for (int kx = -1; kx <= 1; ++kx) {
                        int nidx = (y + ky) * width + (x + kx);
                        if (nidx >= 0 && nidx < width * height && src[nidx] >= high_threshold) {
                            strong_edge = true;
                        }
                    }
                }
                if (strong_edge) {
                    dst[idx] = 255;
                }
            }
        }
    }
}

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <image_path>" << std::endl;
        return -1;
    }

    const char* input_filename = argv[1];
    const char* output_filename = "canny_edges.bmp";

    std::vector<uint8_t> image;
    int width, height;

    if (!read_bmp(input_filename, image, width, height)) {
        std::cerr << "Error: Could not read the BMP image." << std::endl;
        return -1;
    }

    std::vector<uint8_t> gray_image = convert_to_grayscale(image, width, height);
    std::vector<uint8_t> blurred_image;
    gaussian_blur(gray_image, blurred_image, width, height);

    std::vector<int> grad_x, grad_y;
    sobel_operator(blurred_image, grad_x, grad_y, width, height);

    std::vector<float> magnitude, direction;
    gradient_magnitude_direction(grad_x, grad_y, magnitude, direction, width, height);

    std::vector<uint8_t> suppressed_edges;
    non_maximum_suppression(magnitude, direction, suppressed_edges, width, height);

    std::vector<uint8_t> final_edges;
    double_threshold_and_hysteresis(suppressed_edges, final_edges, width, height, 10, 75);

    std::vector<uint8_t> output_image(width * height * 3);
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int idx = y * width + x;
            uint8_t value = final_edges[idx];
            output_image[idx * 3] = value;
            output_image[idx * 3 + 1] = value;
            output_image[idx * 3 + 2] = value;
        }
    }

    if (!write_bmp(output_filename, output_image, width, height)) {
        std::cerr << "Error: Could not write the BMP image." << std::endl;
        return -1;
    }

    std::cout << "Canny edge detection complete. Output saved as " << output_filename << std::endl;
    return 0;
}

