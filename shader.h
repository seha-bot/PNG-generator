typedef struct
{
    int x;
    int y;
    int z;
} v3;

v3 vec(int x, int y, int z)
{
    v3 vec;
    vec.x = x;
    vec.y = y;
    vec.z = z;

    return vec;
}

char clamp(int value)
{
    if(value < 0) return 0;
    else if(value > 255) return 255;
    return value;
}

char* genrgbdata(void (*func)(v3 *value, int x, int y), int width, int height)
{
    int size = width * height * 3;
    char* rgb = (char*)calloc(size, 1);

    for(int y = 0; y < height; y++)
    {
        for(int x = 0; x < width; x++)
        {
            int index = (x + y * width) * 3;
            v3 v;
            func(&v, x, -(y - height));
            rgb[index + 0] = clamp(v.x);
            rgb[index + 1] = clamp(v.y);
            rgb[index + 2] = clamp(v.z);
        }
    }
    return rgb;
}