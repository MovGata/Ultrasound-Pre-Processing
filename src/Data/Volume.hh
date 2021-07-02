#ifndef DATA_VOLUME_HH
#define DATA_VOLUME_HH

#include <array>
#include <utility>
#include <vector>

#include <CL/cl2.hpp>
#include <SDL2/SDL.h>

#include "../events/EventManager.hh"

class Volume : public events::EventManager<Volume>
{
private:
    std::vector<cl_uchar4> raw;
    std::pair<int, int> rotation = {0, 0};
    float scale = 1.0f;
    std::pair<float, float> offset = {0.0f, 0.0f};
    unsigned int depth;
    unsigned int length;
    unsigned int width;


public:
    cl::Buffer buffer;
    std::array<float, 12> invMVTransposed;

    void zoomEvent(const SDL_Event &e);
    void rotateEvent(const SDL_Event &e);
    void dragEvent(const SDL_Event &e);
    
    Volume(const Volume &)=default;
    Volume(Volume &&)=default;
    Volume(unsigned int depth, unsigned int length, int unsigned width, const std::vector<uint8_t> &data);
    ~Volume();

    void sendToCl(const cl::Context &context);
    void update();

    void process(const SDL_Event &e);
};

#endif