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

/* Update a running CRC with the bytes buf[0..len-1]--the CRC
should be initialized to all 1's, and the transmitted value
is the 1's complement of the final running CRC (see the
crc() routine below). */

unsigned long update_crc(unsigned long crc, unsigned char *buf, int len)
{
    unsigned long c = crc;
    int n;

    if (!crc_table_computed) make_crc_table();

    for (n = 0; n < len; n++)
    {
        c = crc_table[(c ^ buf[n]) & 0xff] ^ (c >> 8);
    }
    return c;
}

/* Return the CRC of the bytes buf[0..len-1]. */
unsigned long crc(unsigned char *buf, int len)
{
    return update_crc(0xffffffffL, buf, len) ^ 0xffffffffL;
}


void convertToByte(char *buf, int value, int index)
{
    buf[index + 0] = (value >> 24) & 0xFF;
    buf[index + 1] = (value >> 16) & 0xFF;
    buf[index + 2] = (value >> 8) & 0xFF;
    buf[index + 3] = (value >> 0) & 0xFF;
}


void png(const char* filename, char* rgb, int width, int height)
{
    unsigned long crcData;
    char header[] = {
        137, 80, 78, 71, 13, 10, 26, 10
    };

    char IHDR[] = {
        0, 0, 0, 13, // 4 bytes for data size
        73, 72, 68, 82, //Spells IHDR
        0, 0, 0, width,
        0, 0, 0, height,
        8, 2, 0, 0, 0,
        0, 0, 0, 0 // CRC
    };

    char IHDRCRCBUF[13 + 4];
    for(int i = 4; i < sizeof(IHDR) - 4; i++) IHDRCRCBUF[i - 4] = IHDR[i];
    crcData = crc(IHDRCRCBUF, sizeof(IHDRCRCBUF));
    IHDR[sizeof(IHDR) - 4] = (crcData >> 24) & 0xFF; //Refactor this into a function
    IHDR[sizeof(IHDR) - 3] = (crcData >> 16) & 0xFF;
    IHDR[sizeof(IHDR) - 2] = (crcData >> 8) & 0xFF;
    IHDR[sizeof(IHDR) - 1] = (crcData >> 0) & 0xFF;


    // char PLTE[] = {
    //     0, 0, 0, 3, // Data Length
    //     80, 76, 84, 69, // Spells PLTE
    //     0xFF, 0, 0, // RGB data
    //     0, 0, 0, 0 // CRC
    // };
    // char PLTECRCBUF[3 + 4];
    // for(int i = 4; i < sizeof(PLTE) - 4; i++) PLTECRCBUF[i - 4] = PLTE[i];
    // crcData = crc(PLTECRCBUF, sizeof(PLTECRCBUF));
    // PLTE[sizeof(PLTE) - 4] = (crcData >> 24) & 0xFF;
    // PLTE[sizeof(PLTE) - 3] = (crcData >> 16) & 0xFF;
    // PLTE[sizeof(PLTE) - 2] = (crcData >> 8) & 0xFF;
    // PLTE[sizeof(PLTE) - 1] = (crcData >> 0) & 0xFF;

    // unsigned long adler = 1L;

    // while (read_buffer(buffer, length) != EOF) {
    //     adler = update_adler32(adler, buffer, length);
    // }

    // printf("adler?: %ld\n", adler32(rgb, 3));
    
    char IDAT[] = {
        0, 0, 0, 30, // Data Length
        73, 68, 65, 84, // Spells IDAT

        0x78, 0xDA, //These 2 bytes never change so I can just copy them
        0x00, /*if this is 0, there are more rows */ 0x07, 0x00, 0xF8, 0xFF, 0x00, //Undefined - These change only when I change image width
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, //Seems like these are pixels of a row 6 / 3 = 2(width)
        0x01, /*if this is 1, this is the last row*/ 0x07, 0x00, 0xF8, 0xFF, 0x00, //Undefined
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, //Seems like these are pixels of a row 6 / 3 = 2(width)
        0, 0, 0, 0, //Adler32

        // 0x78, 0xDA, //These 2 bytes never change so I can just copy them
        // 0x00, /*if this is 0, there are more rows */ 0x07, 0x00, 0xF8, 0xFF, 0x00, //Undefined - These change only when I change image width
        // 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //RGB
        // 0x01, /*if this is 1, this is the last row*/ 0x07, 0x00, 0xF8, 0xFF, 0x00, //Undefined
        // 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //RGB
        // 0, 0, 0, 0, //Adler32

        0, 0, 0, 0 // CRC
    };

    unsigned char adlerTest[(3 * width * height) + 2];
    for(int i = 0; i < ((3 * width * height) + 2); i++) adlerTest[i] = 0xFF;
    adlerTest[0] = 0x00;
    adlerTest[7] = 0x00;
    // for(int i = 0; i < ((3 * width * height) + 2); i++) printf("%d ", adlerTest[i]); printf("\n");
    unsigned long adl = adler32(adlerTest, (3 * width * height) + 2);
    // printf("Adler(Dec) = %lu\n", adl);

    // printf(BYTE_TO_BINARY_PATTERN" ", BYTE_TO_BINARY((adl >> 24) & 0xFF));
    // printf(BYTE_TO_BINARY_PATTERN" ", BYTE_TO_BINARY((adl >> 16) & 0xFF));
    // printf(BYTE_TO_BINARY_PATTERN" ", BYTE_TO_BINARY((adl >> 8) & 0xFF));
    // printf(BYTE_TO_BINARY_PATTERN"\n", BYTE_TO_BINARY((adl >> 0) & 0xFF));

    IDAT[sizeof(IDAT) - 8] = (adl >> 24) & 0xFF;
    IDAT[sizeof(IDAT) - 7] = (adl >> 16) & 0xFF;
    IDAT[sizeof(IDAT) - 6] = (adl >> 8) & 0xFF;
    IDAT[sizeof(IDAT) - 5] = (adl >> 0) & 0xFF;

    char IDATCRCBUF[30 + 4];
    for(int i = 4; i < sizeof(IDAT) - 4; i++) IDATCRCBUF[i - 4] = IDAT[i];
    crcData = crc(IDATCRCBUF, sizeof(IDATCRCBUF));
    IDAT[sizeof(IDAT) - 4] = (crcData >> 24) & 0xFF;
    IDAT[sizeof(IDAT) - 3] = (crcData >> 16) & 0xFF;
    IDAT[sizeof(IDAT) - 2] = (crcData >> 8) & 0xFF;
    IDAT[sizeof(IDAT) - 1] = (crcData >> 0) & 0xFF;


    char IEND[] = {
        0, 0, 0, 0, // Data Length
        73, 69, 78, 68, // Spells IEND
        0, 0, 0, 0 // CRC
    };
    char IENDCRCBUF[0 + 4];
    for(int i = 4; i < sizeof(IEND) - 4; i++) IENDCRCBUF[i - 4] = IEND[i];
    crcData = crc(IENDCRCBUF, sizeof(IENDCRCBUF));
    IEND[sizeof(IEND) - 4] = (crcData >> 24) & 0xFF;
    IEND[sizeof(IEND) - 3] = (crcData >> 16) & 0xFF;
    IEND[sizeof(IEND) - 2] = (crcData >> 8) & 0xFF;
    IEND[sizeof(IEND) - 1] = (crcData >> 0) & 0xFF;

    FILE *file = fopen(filename, "wb");
    fwrite(header, sizeof(header), 1, file);
    fwrite(IHDR, sizeof(IHDR), 1, file);
    // fwrite(PLTE, sizeof(PLTE), 1, file);
    fwrite(IDAT, sizeof(IDAT), 1, file);
    fwrite(IEND, sizeof(IEND), 1, file);
    fclose(file);
}

void rgb_to_png(const char* filename, unsigned char* rgb, int width, int height)
{
    unsigned long crcData;
    char header[] = {
        137, 80, 78, 71, 13, 10, 26, 10
    };

    char IHDR[] = {
        0, 0, 0, 13, // 4 bytes for data size
        73, 72, 68, 82, //Spells IHDR
        (width >> 24) & 0xFF, (width >> 16) & 0xFF, (width >> 8) & 0xFF, (width >> 0) & 0xFF,
        (height >> 24) & 0xFF, (height >> 16) & 0xFF, (height >> 8) & 0xFF, (height >> 0) & 0xFF,
        8, 2, 0, 0, 0,
        0, 0, 0, 0 // CRC
    };
    char IHDRCRCBUF[13 + 4];
    for(int i = 4; i < sizeof(IHDR) - 4; i++) IHDRCRCBUF[i - 4] = IHDR[i];
    crcData = crc(IHDRCRCBUF, sizeof(IHDRCRCBUF));
    IHDR[sizeof(IHDR) - 4] = (crcData >> 24) & 0xFF; //Refactor this into a function
    IHDR[sizeof(IHDR) - 3] = (crcData >> 16) & 0xFF;
    IHDR[sizeof(IHDR) - 2] = (crcData >> 8) & 0xFF;
    IHDR[sizeof(IHDR) - 1] = (crcData >> 0) & 0xFF;

    

    // char IDAT[] = {
    //     0, 0, 0, 30, // Data Length
    //     73, 68, 65, 84, // Spells IDAT
    //     0x78, 0xDA, //These 2 bytes never change so I can just copy them

    //     //Data...

    //     0, 0, 0, 0, //Adler32
    //     0, 0, 0, 0 // CRC
    // };
    int pixelWidth = width * 3;
    int pixelLength = pixelWidth * height;
    int dataLength = (6/*6 is for 0size0*/ + pixelWidth) * height;
    char IDAT[18/*InitialData*/ + dataLength];
    convertToByte(IDAT, dataLength + 2/*header*/ + 4/*adler*/, 0);
    IDAT[4] = 73;
    IDAT[5] = 68;
    IDAT[6] = 65;
    IDAT[7] = 84;
    IDAT[8] = 0x78;
    IDAT[9] = 0xDA;

    unsigned char headerOffset[4] = {
        ((pixelWidth + 1) >> 0) & 0xFF,
        ((pixelWidth + 1) >> 8) & 0xFF,
        (~(pixelWidth + 1) >> 0) & 0xFF,
        (~(pixelWidth + 1) >> 8) & 0xFF
    };

    int pos = 10;
    for(int y = 0; y < height; y++)
    {
        if(y == height - 1) IDAT[pos++] = 0x01;
        else IDAT[pos++] = 0x00;
        
        IDAT[pos++] = headerOffset[0];
        IDAT[pos++] = headerOffset[1];
        IDAT[pos++] = headerOffset[2];
        IDAT[pos++] = headerOffset[3];
        
        IDAT[pos++] = 0x00;

        for(int x = 0; x < width; x++)
        {
            int index = (x + y * width) * 3;
            // printf("%d\n", index);
            IDAT[pos++] = rgb[index + 0];
            IDAT[pos++] = rgb[index + 1];
            IDAT[pos++] = rgb[index + 2];
        }
    }

    unsigned char adlerRGB[pixelLength + height];
    int offset = 0;
    for(int y = 0; y < height; y++)
    {
        adlerRGB[y * width * 3 + offset] = 0x00;
        offset++;
        for(int x = 0; x < width; x++)
        {
            int index = (x + y * width) * 3;
            adlerRGB[index + offset] = rgb[index];
            adlerRGB[index + offset + 1] = rgb[index + 1];
            adlerRGB[index + offset + 2] = rgb[index + 2];
        }
    }
    // printf("pl: %d\n", pixelLength);
    // for(int i = 0; i < pixelLength + height; i++) printf("%d ", adlerRGB[i]); printf("\n");
    unsigned long adl = adler32(adlerRGB, pixelLength + height);
    // printf("Adler(Dec) = %lu\n", adl);

    // printf(BYTE_TO_BINARY_PATTERN" ", BYTE_TO_BINARY((adl >> 24) & 0xFF));
    // printf(BYTE_TO_BINARY_PATTERN" ", BYTE_TO_BINARY((adl >> 16) & 0xFF));
    // printf(BYTE_TO_BINARY_PATTERN" ", BYTE_TO_BINARY((adl >> 8) & 0xFF));
    // printf(BYTE_TO_BINARY_PATTERN"\n", BYTE_TO_BINARY((adl >> 0) & 0xFF));

    IDAT[sizeof(IDAT) - 8] = (adl >> 24) & 0xFF;
    IDAT[sizeof(IDAT) - 7] = (adl >> 16) & 0xFF;
    IDAT[sizeof(IDAT) - 6] = (adl >> 8) & 0xFF;
    IDAT[sizeof(IDAT) - 5] = (adl >> 0) & 0xFF;

    unsigned char IDATCRCBUF[dataLength + 2/*header*/ + 4/*adler*/ + 4/*This 4 is here because it calculates crc for everything except the 4bit length parameter*/];
    for(int i = 4; i < sizeof(IDAT) - 4; i++) IDATCRCBUF[i - 4] = IDAT[i];
    crcData = crc(IDATCRCBUF, sizeof(IDATCRCBUF));
    IDAT[sizeof(IDAT) - 4] = (crcData >> 24) & 0xFF;
    IDAT[sizeof(IDAT) - 3] = (crcData >> 16) & 0xFF;
    IDAT[sizeof(IDAT) - 2] = (crcData >> 8) & 0xFF;
    IDAT[sizeof(IDAT) - 1] = (crcData >> 0) & 0xFF;


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