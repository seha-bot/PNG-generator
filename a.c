#include<pngha.h>
#include<shader.h>
#include<math.h>
#include<time.h>

int width = 600;
int height = 600;

int length(int x, int y)
{
    return sqrt(x * x + y * y);
}

int circle(int x, int y, int radius)
{
    return (length(x, y) < radius) ? 255 : 0;
}

int ellipse(int x, int y, float radx, float rady)
{
    return (x/radx) * (x/radx) + (y/rady) * (y/rady) > 1 ? 0 : 255;
}

void banana(v3 *value, int x, int y)
{
    float halfH = height >> 1;
    char gradient = y * 255 / height;
    // *value = vec(circle(x - halfH + 10, y - halfH - 60, 200), circle(x - halfH - 10, y - halfH + 30, 200), 0);
    *value = vec(ellipse(x - halfH + 10, y - halfH - 60, 190, 200), circle(x - halfH - 10, y - halfH + 30, 200), 0);

    if(value->x == 0 && value->y == 255) *value = vec(255, 255, 0);
    else *value = vec(0, 0, 0);

    //Left ear
    *value = vec(circle(x-110, y-270, 40) == 255 ? 0 : value->x, value->y, value->z);
    *value = vec(value->x, value->y, circle(x-110, y-270, 57) == 255 ? 255 : value->z);
    if(value->x == 0 && value->y == 0 && value->z == 255) *value = vec(0, 0, 0);
    if(value->x == 0 && value->y == 255 && value->z == 255) *value = vec(0, 0, 0);
    if(value->x == 255 && value->y == 255 && value->z == 255) *value = vec(102, 51, 0);

    // //Right ear
    *value = vec(value->x, value->y, circle(x-480, y-250, 40) == 255 ? 255 : value->z);
    if(value->x == 0 && value->y == 0 && value->z == 255) *value = vec(0, 0, 0);
    if(y + 30 > halfH && value->x == 255 && value->y == 255 && value->z == 0) *value = vec(0, 0, 0);
    if(value->x == 255 && value->y == 255 && value->z == 255) *value = vec(255, 255, 0);

    //Noise
    // if(rand() % 2000 == 0 && value->x == 255 && value->y == 255 && value->z == 0) *value = vec(102, 51, 0);
    // if(value->x == 255 && value->y == 255 && value->z == 0) *value = vec(rand() % 220 + 210, 200, 0);

    //Shading
    if(value->x == 255 && value->y == 255 && value->z == 0) *value = vec(180, 180, 0);
    if(value->x == 0 && value->y == 0 && value->z == 0) value->z = 10;
    if(value->x == 180 && value->y == 180 && value->z == 0) *value = vec(y * 450 / height, y * 450 / height, 0);
    int circle1 = circle(x - halfH + 10, y - halfH - 40, 200);
    // int circle2 = circle(x - halfH, y - halfH - 5, 200);
    int circle2 = ellipse(x - halfH, y - halfH - 5, 210, 200);
    if(value->y != 51 && value->z != 10) *value = vec(circle1 == 255 ? value->x + 20 : value->x, circle1 == 255 ? value->x + 20 : value->y, value->z); //70
    if(value->y != 51 && value->z != 10) *value = vec(circle2 == 255 ? value->x + 10 : value->x, circle2 == 255 ? value->x + 10 : value->y, value->z); //40
    if(value->z == 10) value->z = 0;


    if((value->x != 0 || value->y != 0 || value->z != 0) && value->y != 51)
    {
        //OLD
        // if(rand() % 2000 == 0) *value = vec(102, 51, 0);
        // *value = vec(rand() % 220 + 210, 200, 0);

        //NEW
        int big = rand() % 120 + 90;
        int smol = rand() % 70 + 40;
        if(rand() % 2000 == 0) *value = vec(value->x - smol, value->y - big, 0);
    }

    //Background
    if(value->x == 0 && value->y == 0 && value->z == 0) *value = vec(100, 100, 100);

    //Shadows
    int shadow = ellipse(x - halfH - 10, y - halfH + 270, 350, 30);
    int s = ((x - halfH - 10)/14.0f) * ((x - halfH - 10)/14.0f) + ((y - halfH + 270)/1.5f) * ((y - halfH + 270)/1.5f);
    if(s > 100) s = 100;
    *value = vec(shadow == 255 ? s : value->x, shadow == 255 ? s : value->y, shadow == 255 ? s : value->z);
}

int main(int argc, char** args)
{
    if(argc <= 1) return 0;
    srand(time(0));
    char* rgb = genrgbdata(banana, width, height);

    rgb_to_png("banana.png", rgb, width, height);
    printf("Generation Complete!\n");
    free(rgb);

    return 0;
}