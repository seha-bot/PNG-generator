#define ADLER_BASE 65521

unsigned int adler32(unsigned char *data, size_t len)
{
    int a = 1, b = 0;
    
    // Process each byte of the data in order
    for (int i = 0; i < len; ++i)
    {
        a = (a + data[i]) % ADLER_BASE;
        b = (b + a) % ADLER_BASE;
    }
    
    return (b << 16) | a;
}


void inverse_copy32(char *array, int value, int index)
{
    array[index + 0] = (value >> 24) & 0xFF;
    array[index + 1] = (value >> 16) & 0xFF;
    array[index + 2] = (value >> 8) & 0xFF;
    array[index + 3] = (value >> 0) & 0xFF;
}
void copy32(char *array, int value, int index)
{
    array[index + 0] = (value >> 0) & 0xFF;
    array[index + 1] = (value >> 8) & 0xFF;
    array[index + 2] = (value >> 16) & 0xFF;
    array[index + 3] = (value >> 24) & 0xFF;
}


/* Table of CRCs of all 8-bit messages. */
unsigned long crc_table[256];

/* Flag: has the table been computed? Initially false. */
int crc_table_computed = 0;

/* Make the table for a fast CRC. */
void make_crc_table(void)
{
    unsigned long c;
    int n, k;

    for (n = 0; n < 256; n++)
    {
        c = (unsigned long) n;
        for (k = 0; k < 8; k++)
        {
            if (c & 1) c = 0xedb88320L ^ (c >> 1);
            else c = c >> 1;
        }
        crc_table[n] = c;
    }
    crc_table_computed = 1;
}

void crc(unsigned char *chunk, int size)
{
    unsigned long c = 0xffffffffL;
    if (!crc_table_computed) make_crc_table();
    for (int n = 4; n < size - 4; n++) c = crc_table[(c ^ chunk[n]) & 0xff] ^ (c >> 8);
    c ^= 0xffffffffL;

    inverse_copy32(chunk, c, size - 4);
}


void rgb_to_png(const char* filename, unsigned char* rgb, int width, int height)
{
    char header[] = {
        137, 80, 78, 71, 13, 10, 26, 10
    };

    char IHDR[] = {
        0, 0, 0, 13, // 4 bytes for data size
        73, 72, 68, 82, //Spells IHDR
        (width >> 24) & 0xFF, (width >> 16) & 0xFF, (width >> 8) & 0xFF, (width >> 0) & 0xFF,
        (height >> 24) & 0xFF, (height >> 16) & 0xFF, (height >> 8) & 0xFF, (height >> 0) & 0xFF,
        8, 2, 0, 0, 0, //Format parameters
        0, 0, 0, 0 // CRC
    };
    crc(IHDR, sizeof(IHDR));
    
    int pixelWidth = width * 3;
    int pixelLength = pixelWidth * height;
    int dataLength = (6/*6 is for 0size0*/ + pixelWidth) * height;
    char IDAT[18/*InitialData*/ + dataLength];
    inverse_copy32(IDAT, dataLength + 2/*header*/ + 4/*adler*/, 0); //Data Length parameter
    IDAT[4] = 73; //I
    IDAT[5] = 68; //D
    IDAT[6] = 65; //A
    IDAT[7] = 84; //T
    IDAT[8] = 0x78; //Compression header
    IDAT[9] = 0xDA; //Compression header

    unsigned char headerOffset[4] = {
        ((pixelWidth + 1) >> 0) & 0xFF,
        ((pixelWidth + 1) >> 8) & 0xFF,
        (~(pixelWidth + 1) >> 0) & 0xFF,
        (~(pixelWidth + 1) >> 8) & 0xFF
    };

    unsigned char adlerRGB[pixelLength + height];
    int pos = 10, offset = 0;
    for(int y = 0; y < height; y++)
    {
        //Data row header
        if(y == height - 1) IDAT[pos++] = 0x01;
        else IDAT[pos++] = 0x00;
        IDAT[pos++] = headerOffset[0];
        IDAT[pos++] = headerOffset[1];
        IDAT[pos++] = headerOffset[2];
        IDAT[pos++] = headerOffset[3];
        IDAT[pos++] = 0x00;

        //Putting zeroes in the adler
        adlerRGB[y * width * 3 + offset] = 0x00;
        offset++;

        for(int x = 0; x < width; x++)
        {
            int index = (x + y * width) * 3;

            //Copying rgb into data array
            IDAT[pos++] = rgb[index + 0];
            IDAT[pos++] = rgb[index + 1];
            IDAT[pos++] = rgb[index + 2];

            //Copying rgb int adler
            adlerRGB[index + offset] = rgb[index];
            adlerRGB[index + offset + 1] = rgb[index + 1];
            adlerRGB[index + offset + 2] = rgb[index + 2];
        }
    }

    unsigned long adl = adler32(adlerRGB, pixelLength + height);
    inverse_copy32(IDAT, adl, sizeof(IDAT) - 8);
    crc(IDAT, sizeof(IDAT));


    char IEND[] = {
        0, 0, 0, 0, // Data Length
        73, 69, 78, 68, // Spells IEND
        174, 66, 96, 130 // CRC
    };

    FILE *file = fopen(filename, "wb");
    fwrite(header, sizeof(header), 1, file);
    fwrite(IHDR, sizeof(IHDR), 1, file);
    fwrite(IDAT, sizeof(IDAT), 1, file);
    fwrite(IEND, sizeof(IEND), 1, file);
    fclose(file);
}