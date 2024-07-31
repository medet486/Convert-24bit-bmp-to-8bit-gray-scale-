#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// #include <conio.h> does not exist in Xcode in macOS

#pragma pack(1)
typedef unsigned char BYTE; // 1 byte of memory
typedef unsigned short WORD; // 2 bytes of memory
typedef unsigned int DWORD; // 4 bytes of memory
typedef int LONG; // 4 bytes of memory

typedef struct _BMPFH { // takes 14 bytes of memory
    BYTE bftype1; // 1 byte and must be 'B'
    BYTE bftype2; // 1 byte and must be 'M'
    DWORD bfsize; // 4 bytes gives us the total size of bmp file (including headers, palette (if it has), data)
    WORD bfreserved1; // 2 bytes of memory could be set as 0
    WORD bfreserved2; // 2 bytes of memory could be set as 0
    DWORD bfOffbits; // 4 bytes of memory gives the position of data starts in the bmp file
} BMPFH;

typedef struct _BMPIH { // 40 bytes for Windows bitmap file
    DWORD bisize; // 4 bytes and it gives the size of info header
    LONG biw; // 4 bytes and it is the width of image
    LONG bih; // 4 bytes and it is the height of image
    WORD biplane; // 2 bytes which is not important for us
    WORD bibitcount; // 2 bytes it is about the type of bitmap file if it is 1 2 color, 4 (16 colors) ..
    DWORD biComp; // 4 bytes not important
    DWORD bisizeimage; // 4 bytes and it gives the size of data in the image
    LONG bix; // 4 bytes not important
    LONG biy; // 4 bytes not important
    DWORD biclused; // 4 bytes not important
    DWORD biclimp; // 4 bytes not important for us
} BMPIH;

typedef struct _PALET { // in palette it describes colors (what is the color number)
    BYTE rgbblue; // blue component
    BYTE rgbgreen; // green component
    BYTE rgbred; // red component
    BYTE rgbreserved; // reserved byte the user can use this for their aims
} PALET;

typedef struct _IMAGE {
    BMPFH bmpfh;
    BMPIH bmpih;
    PALET *palet;
    BYTE *data;
} IMAGE;

IMAGE *ImageRead(IMAGE *image, char *filename) {
    BMPFH bmpfh;
    BMPIH bmpih;
    FILE *fp;
    DWORD r, rowsize, size;

    fp = fopen(filename, "rb"); // tries to open the filename
    if (fp == NULL) {
        printf("File is not found..");
        exit(1);
    }
    fread(&bmpfh, sizeof(BMPFH), 1, fp); // reads bitmap file header from the file and sets to bmpfh
    if (bmpfh.bftype1 != 'B' || bmpfh.bftype2 != 'M') {
        printf("This is not a bitmap file.");
        exit(1);
    }
    fread(&bmpih, sizeof(BMPIH), 1, fp); // reads bitmap info header from the file and sets to bmpih
    image = (IMAGE *)malloc(bmpfh.bfsize); // allocates memory for image
    if (image == NULL) {
        printf("There is no enough memory for this operation");
        exit(1);
    }
    image->bmpfh = bmpfh; // sets bmpfh to image
    image->bmpih = bmpih; // sets bmpih to image

    r = 0; // r is set to 0 in case 24 bits per pixel or more (this kind of images does not have color palette)
    if (bmpih.bibitcount == 1)
        r = 2; // if the image 1 bit per pixel (the number of colors is 2)
    if (bmpih.bibitcount == 4)
        r = 16; // if the image 4 bits per pixel (the number of colors is 16)
    if (bmpih.bibitcount == 8)
        r = 256; // if the image 8 bits per pixel (the number of colors is 256)
    if (r != 0) {
        image->palet = (PALET *)malloc(4 * r); // allocate memory for color palette
        fread(image->palet, sizeof(BYTE), 4 * r, fp); // read color palette from image to the memory
    }
    rowsize = (image->bmpih.biw * image->bmpih.bibitcount + 31) / 32 * 4; // calculates 1 row of image size
    size = rowsize * image->bmpih.bih; // total size of image is calculated and set to size
    image->data = (BYTE *)malloc(size); // allocates enough memory for image data
    fread(image->data, size, 1, fp); // reads image data from file and sets to image->data
    fclose(fp); // closes the file
    return image; // returns the address of image on the memory
}

void ImageWrite(IMAGE *image, char *filename) {
    FILE *fp;
    int r, rowsize, size;

    fp = fopen(filename, "wb"); // opens the file
    if (fp == NULL) {
        printf("File opening error!");
        exit(1);
    }
    fwrite(&image->bmpfh, sizeof(BMPFH), 1, fp); // writes the bitmap file header to the file
    fwrite(&image->bmpih, sizeof(BMPIH), 1, fp); // writes the bitmap info header to the file
    r = 0;
    if (image->bmpih.bibitcount == 1)
        r = 2; // if the image has 2 colors
    if (image->bmpih.bibitcount == 4)
        r = 16; // if the image has 16 colors
    if (image->bmpih.bibitcount == 8)
        r = 256; // if the image has 256 colors
    if (r != 0)
        fwrite(image->palet, 4 * r, 1, fp); // if the image has color palette, the palette is written to the file
    rowsize = (image->bmpih.biw * image->bmpih.bibitcount + 31) / 32 * 4; // a row size of image is calculated
    size = rowsize * image->bmpih.bih; // total size of image is calculated
    fwrite(image->data, size, 1, fp); // writes image data to the file
    fclose(fp); // closes the file
}

void writeInfo(IMAGE *image, char *fname) {
    printf("--------Info about %s image file\n", fname);
    printf("bfType value       :%c%c\n", image->bmpfh.bftype1, image->bmpfh.bftype2);
    printf("bfsize             :%d\n", image->bmpfh.bfsize);
    printf("bfreserved1        :%d\n", image->bmpfh.bfreserved1);
    printf("bfreserved2        :%d\n", image->bmpfh.bfreserved2);
    printf("bfOffbits          :%d\n", image->bmpfh.bfOffbits);
    printf("bisize             :%d\n", image->bmpih.bisize);
    printf("Width              :%d\n", image->bmpih.biw);
    printf("Height             :%d\n", image->bmpih.bih);
    printf("biplane            :%d\n", image->bmpih.biplane);
    printf("bibitcount         :%d\n", image->bmpih.bibitcount);
    printf("Compression        :%d\n", image->bmpih.biComp);
    printf("bisizeimage        :%d\n", image->bmpih.bisizeimage);
    printf("bix                :%d\n", image->bmpih.bix);
    printf("biy                :%d\n", image->bmpih.biy);
    printf("bi color used      :%d\n", image->bmpih.biclused);
    printf("bi color imp.      :%d\n", image->bmpih.biclimp);
}

IMAGE *convert2GrayScale(IMAGE *image) {
    
    IMAGE *newImage = (IMAGE *)malloc(sizeof(IMAGE));
    if (newImage == NULL) {
        printf("Memory allocation failed for new image.");
        exit(1);
    }

    newImage->bmpfh = image->bmpfh;
    newImage->bmpih = image->bmpih;

    newImage->bmpfh.bfsize = sizeof(BMPFH) + sizeof(BMPIH) + 256 * sizeof(PALET) + image->bmpih.bisizeimage;
    newImage->bmpfh.bfOffbits = sizeof(BMPFH) + sizeof(BMPIH) + 256 * sizeof(PALET);
    newImage->bmpih.bibitcount = 8;
    newImage->bmpih.bisizeimage = ((image->bmpih.biw * newImage->bmpih.bibitcount + 31) / 32) * 4 * image->bmpih.bih;

    newImage->palet = (PALET *)malloc(256 * sizeof(PALET));
    if (newImage->palet == NULL) {
        printf("Memory allocation failed for color palette.");
        exit(1);
    }
    for (int i = 0; i < 256; i++) {
        newImage->palet[i].rgbblue = i;
        newImage->palet[i].rgbgreen = i;
        newImage->palet[i].rgbred = i;
        newImage->palet[i].rgbreserved = 0;
    }

    newImage->data = (BYTE *)malloc(newImage->bmpih.bisizeimage);
    if (newImage->data == NULL) {
        printf("Memory allocation failed for image data.");
        exit(1);
    }

    int rowsize = (image->bmpih.biw * 3 + 3) & ~3; 
    int newRowsize = (newImage->bmpih.biw + 3) & ~3; 

    for (int y = 0; y < image->bmpih.bih; y++) {
        for (int x = 0; x < image->bmpih.biw; x++) {
            int srcIndex = y * rowsize + x * 3;
            int destIndex = y * newRowsize + x;
            BYTE blue = image->data[srcIndex];
            BYTE green = image->data[srcIndex + 1];
            BYTE red = image->data[srcIndex + 2];
            BYTE gray = (BYTE)(0.299 * red + 0.587 * green + 0.114 * blue);
            newImage->data[destIndex] = gray;
        }
    }

    return newImage;
}

int main() {
    IMAGE *image = (IMAGE *)malloc(sizeof(IMAGE));
    if (image == NULL) {
        printf("Memory allocation failed for image.");
        exit(1);
    }
    
    IMAGE *newimage;
    image = ImageRead(image, "parrots.bmp");
    newimage = convert2GrayScale(image);
    ImageWrite(newimage, "parrots8bit.bmp");

    // Free the memory allocated for images
    free(image->data);
    free(image->palet);
    free(image);

    free(newimage->data);
    free(newimage->palet);
    free(newimage);

    return 0;
}
