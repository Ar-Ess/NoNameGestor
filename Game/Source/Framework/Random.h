#include <cstdint>

class Random
{
public:

    static float RealFloat(float min = 0, float max = 1);
    
    static float PsdoFloat(float min = 0, float max = 1);

    static int RealInt(int min = 0, int max = 1);

    static int PsdoInt(int min = 0, int max = 1);

    static bool RealBool();

    static bool PsdoBool();
    
    static void Seed(int seed);

private:

    Random() = default;
    ~Random() = default;
    Random(const Random& ) = delete;
    Random(Random&&) = delete;
    Random& operator=(const Random&) = delete;
    Random& operator=(Random&&) = delete;

    static Random& Instance();
    
    float Float(bool real, float min, float max);
    
    float Int(bool real, int min, int max);
    
private:

    uint32_t psdoSeed = 1;
    uint32_t realSeed = 1;

};
