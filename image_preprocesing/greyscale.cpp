#include <iostream>
#include <fstream>
#include <vector>

using namespace std;

// Struct to represent RGB pixel
struct RGB {
    unsigned char r, g, b;
};

// Function to convert RGB image to grayscale
void convertToGrayscale(vector<RGB>& image, int width, int height) {
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            // Access pixel
            RGB& pixel = image[i * width + j];

            // Calculate grayscale value
            unsigned char grayscale = (pixel.r + pixel.g + pixel.b) / 3;

            // Set grayscale value to all channels
            pixel.r = grayscale;
            pixel.g = grayscale;
            pixel.b = grayscale;
        }
    }
}

// Function to load image from file
vector<RGB> loadImage(const char* filename, int& width, int& height) {
    ifstream file(filename, ios::binary);
    if (!file) {
        cerr << "Error: Could not open file" << endl;
        exit(1);
    }

    // Read header (assuming a simple BMP format)
    file.seekg(18);
    file.read(reinterpret_cast<char*>(&width), sizeof(width));
    file.read(reinterpret_cast<char*>(&height), sizeof(height));
    file.seekg(54); // Skip to pixel data

    // Read pixel data
    vector<RGB> image(width * height);
    file.read(reinterpret_cast<char*>(&image[0]), width * height * sizeof(RGB));
    file.close();

    return image;
}

// Function to save image to file
void saveImage(const char* filename, const vector<RGB>& image, int width, int height) {
    ofstream file(filename, ios::binary);
    if (!file) {
        cerr << "Error: Could not create file" << endl;
        exit(1);
    }

    // Calculate padding
    int rowPadding = (4 - (width * 3) % 4) % 4;

    // File header
    unsigned char bmpFileHeader[14] = {
        'B', 'M',           // Signature
        0, 0, 0, 0,         // Image file size in bytes
        0, 0, 0, 0,         // Reserved
        54, 0, 0, 0         // Start of pixel array
    };

    // DIB header (BITMAPINFOHEADER)
    unsigned char bmpInfoHeader[40] = {
        40, 0, 0, 0,        // Header size
        0, 0, 0, 0,         // Image width
        0, 0, 0, 0,         // Image height
        1, 0,               // Planes
        24, 0,              // Bits per pixel
        0, 0, 0, 0,         // Compression (no compression)
        0, 0, 0, 0,         // Image size (can be 0 for uncompressed images)
        0, 0, 0, 0,         // X pixels per meter (unspecified)
        0, 0, 0, 0,         // Y pixels per meter (unspecified)
        0, 0, 0, 0,         // Total colors (0 = use default)
        0, 0, 0, 0          // Important colors (0 = all colors are important)
    };

    int fileSize = 54 + (width * 3 + rowPadding) * height;
    bmpFileHeader[2] = fileSize;
    bmpFileHeader[3] = fileSize >> 8;
    bmpFileHeader[4] = fileSize >> 16;
    bmpFileHeader[5] = fileSize >> 24;

    bmpInfoHeader[4] = width;
    bmpInfoHeader[5] = width >> 8;
    bmpInfoHeader[6] = width >> 16;
    bmpInfoHeader[7] = width >> 24;

    bmpInfoHeader[8] = height;
    bmpInfoHeader[9] = height >> 8;
    bmpInfoHeader[10] = height >> 16;
    bmpInfoHeader[11] = height >> 24;

    // Write headers
    file.write(reinterpret_cast<const char*>(bmpFileHeader), sizeof(bmpFileHeader));
    file.write(reinterpret_cast<const char*>(bmpInfoHeader), sizeof(bmpInfoHeader));

    // Write pixel data with padding
    for (int i = 0; i < height; ++i) {
        file.write(reinterpret_cast<const char*>(&image[i * width]), width * sizeof(RGB));
        file.write("\0\0\0", rowPadding); // Padding
    }

    file.close();
}


int main() {
    const char* inputFilename = "image.bmp";
    const char* outputFilename = "grayscale_image.bmp";
    int width, height;

    // Load image
    vector<RGB> image = loadImage(inputFilename, width, height);

    // Convert to grayscale
    convertToGrayscale(image, width, height);

    // Save grayscale image
    saveImage(outputFilename, image, width, height);

    cout << "Image converted to grayscale successfully." << endl;

    return 0;
}
