#ifndef GUI_SDL2_INSTANCE_HH
#define GUI_SDL2_INSTANCE_HH

#include <memory>

#include <SDL2/SDL_ttf.h>

namespace gui
{
    
    class Instance
    {
    private:
        std::unique_ptr<TTF_Font, decltype(&TTF_CloseFont)> font;
    public:
        Instance(/* args */);
        ~Instance();

        TTF_Font *loadFont(const std::string &url);
    };

} // namespace gui

#endif