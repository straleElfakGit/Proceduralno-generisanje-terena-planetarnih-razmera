#include "Noise.h"
#include "Logging/Logger.h"

namespace
{
    const int Source[256] = {
    151, 160, 137, 91, 90, 15, 131, 13, 201, 95, 96, 53, 194, 233, 7, 225, 140, 36, 103, 30, 69, 142,
    8, 99, 37, 240, 21, 10, 23, 190, 6, 148, 247, 120, 234, 75, 0, 26, 197, 62, 94, 252, 219, 203,
    117, 35, 11, 32, 57, 177, 33, 88, 237, 149, 56, 87, 174, 20, 125, 136, 171, 168, 68, 175, 74, 165,
    71, 134, 139, 48, 27, 166, 77, 146, 158, 231, 83, 111, 229, 122, 60, 211, 133, 230, 220, 105, 92, 41,
    55, 46, 245, 40, 244, 102, 143, 54, 65, 25, 63, 161, 1, 216, 80, 73, 209, 76, 132, 187, 208, 89,
    18, 169, 200, 196, 135, 130, 116, 188, 159, 86, 164, 100, 109, 198, 173, 186, 3, 64, 52, 217, 226, 250,
    124, 123, 5, 202, 38, 147, 118, 126, 255, 82, 85, 212, 207, 206, 59, 227, 47, 16, 58, 17, 182, 189,
    28, 42, 223, 183, 170, 213, 119, 248, 152, 2, 44, 154, 163, 70, 221, 153, 101, 155, 167, 43, 172, 9,
    129, 22, 39, 253, 19, 98, 108, 110, 79, 113, 224, 232, 178, 185, 112, 104, 218, 246, 97, 228, 251, 34,
    242, 193, 238, 210, 144, 12, 191, 179, 162, 241, 81, 51, 145, 235, 249, 14, 239, 107, 49, 192, 214, 31,
    181, 199, 106, 157, 184, 84, 204, 176, 115, 121, 50, 45, 127, 4, 150, 254, 138, 236, 205, 93, 222, 114,
    67, 29, 24, 72, 243, 141, 128, 195, 78, 66, 215, 61, 156, 180
    };

    const int Grad3[12][3] = {
        { 1,  1,  0}, {-1,  1,  0}, { 1, -1,  0}, {-1, -1,  0},
        { 1,  0,  1}, {-1,  0,  1}, { 1,  0, -1}, {-1,  0, -1},
        { 0,  1,  1}, { 0, -1,  1}, { 0,  1, -1}, { 0, -1, -1}
    };

    const double F3 = 1.0 / 3.0;
    const double G3 = 1.0 / 6.0;


    inline int FastFloor(double x)
    {
        return static_cast<int>(std::floor(x));
    }

    inline double Dot(const int g[3], double x, double y, double z)
    {
        return g[0] * x + g[1] * y + g[2] * z;
    }
}


Noise::Noise()
{
    LOG_FUNC();
    Randomize(0);
}

Noise::Noise(int seed)
{
    Randomize(seed);
}

Noise::~Noise() {}

void Noise::Randomize(int seed)
{
    if (seed != 0)
    {
        const std::uint32_t s = static_cast<std::uint32_t>(seed);
        const int mask = static_cast<int>(
            (s & 0xff) ^ ((s >> 8) & 0xff) ^ ((s >> 16) & 0xff) ^ ((s >> 24) & 0xff));

        for (int i = 0; i < RandomSize; i++)
        {
            random[i] = Source[i] ^ mask;
            random[i + RandomSize] = random[i];
        }
    }
    else
    {
        for (int i = 0; i < RandomSize; i++)
            random[i + RandomSize] = random[i] = Source[i];
    }
}

float Noise::Evaluate(float px, float py, float pz) const
{
    const double x = px;
    const double y = py;
    const double z = pz;
    double n0 = 0, n1 = 0, n2 = 0, n3 = 0;

    const double s = (x + y + z) * F3;

    const int i = FastFloor(x + s);
    const int j = FastFloor(y + s);
    const int k = FastFloor(z + s);

    const double t = (i + j + k) * G3;

    const double x0 = x - (i - t);
    const double y0 = y - (j - t);
    const double z0 = z - (k - t);

    int i1, j1, k1;
    int i2, j2, k2;

    if (x0 >= y0)
    {
        if (y0 >= z0) { i1 = 1; j1 = 0; k1 = 0; i2 = 1; j2 = 1; k2 = 0; } // X Y Z order
        else if (x0 >= z0) { i1 = 1; j1 = 0; k1 = 0; i2 = 1; j2 = 0; k2 = 1; } // X Z Y order
        else { i1 = 0; j1 = 0; k1 = 1; i2 = 1; j2 = 0; k2 = 1; } // Z X Y order
    }
    else
    {
        if (y0 < z0) { i1 = 0; j1 = 0; k1 = 1; i2 = 0; j2 = 1; k2 = 1; } // Z Y X order
        else if (x0 < z0) { i1 = 0; j1 = 1; k1 = 0; i2 = 0; j2 = 1; k2 = 1; } // Y Z X order
        else { i1 = 0; j1 = 1; k1 = 0; i2 = 1; j2 = 1; k2 = 0; } // Y X Z order
    }

    const double x1 = x0 - i1 + G3;
    const double y1 = y0 - j1 + G3;
    const double z1 = z0 - k1 + G3;

    const double x2 = x0 - i2 + F3;
    const double y2 = y0 - j2 + F3;
    const double z2 = z0 - k2 + F3;

    const double x3 = x0 - 0.5;
    const double y3 = y0 - 0.5;
    const double z3 = z0 - 0.5;

    const int ii = i & 0xff;
    const int jj = j & 0xff;
    const int kk = k & 0xff;

    double t0 = 0.6 - x0 * x0 - y0 * y0 - z0 * z0;
    if (t0 > 0)
    {
        t0 *= t0;
        const int gi0 = random[ii + random[jj + random[kk]]] % 12;
        n0 = t0 * t0 * Dot(Grad3[gi0], x0, y0, z0);
    }

    double t1 = 0.6 - x1 * x1 - y1 * y1 - z1 * z1;
    if (t1 > 0)
    {
        t1 *= t1;
        const int gi1 = random[ii + i1 + random[jj + j1 + random[kk + k1]]] % 12;
        n1 = t1 * t1 * Dot(Grad3[gi1], x1, y1, z1);
    }

    double t2 = 0.6 - x2 * x2 - y2 * y2 - z2 * z2;
    if (t2 > 0)
    {
        t2 *= t2;
        const int gi2 = random[ii + i2 + random[jj + j2 + random[kk + k2]]] % 12;
        n2 = t2 * t2 * Dot(Grad3[gi2], x2, y2, z2);
    }

    double t3 = 0.6 - x3 * x3 - y3 * y3 - z3 * z3;
    if (t3 > 0)
    {
        t3 *= t3;
        const int gi3 = random[ii + 1 + random[jj + 1 + random[kk + 1]]] % 12;
        n3 = t3 * t3 * Dot(Grad3[gi3], x3, y3, z3);
    }

    return static_cast<float>((n0 + n1 + n2 + n3) * 32.0);
}
