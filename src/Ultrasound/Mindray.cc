#include "Mindray.hh"

#include <algorithm>
#include <cctype>
#include <charconv>
#include <istream>
#include <iostream>
#include <string_view>

#include <SDL2/SDL_rwops.h>

#include "../IO/SDL2/RWOpsStream.hh"

namespace ultrasound
{

    Mindray::Mindray(/* args */)
    {
    }

    Mindray::~Mindray()
    {
    }

    bool Mindray::load(const char *vmTxt, const char *vmBin, const char *cp)
    {

        std::unique_ptr<SDL_RWops, decltype(&SDL_RWclose)> cpOps(nullptr, SDL_RWclose);
        io::RWOpsStream cpRWStream;
        std::string s;

        {
            std::unique_ptr<SDL_RWops, decltype(&SDL_RWclose)> vmTxtOps(nullptr, SDL_RWclose);

            vmTxtOps.reset(SDL_RWFromFile(vmTxt, "r"));
            if (vmTxtOps == nullptr)
            {
                std::cout << "SDL2 failed to open Mindray virtual machine file '" << vmTxt << "', error: " << SDL_GetError();
                return false;
            }

            io::RWOpsStream vmTxtRWStream = io::RWOpsStream(vmTxtOps.get());
            std::istream vmTxtIs(&vmTxtRWStream);
            std::vector<std::reference_wrapper<vmTxtInfoStore>> isDepth;
            isDepth.push_back(vmTxtStore);

            while (vmTxtIs >> s)
            {
                std::string_view sv(s);

                if (sv.starts_with("DATA_TREE_BEGIN"))
                {
                    isDepth.emplace_back(isDepth.back().get().template load<vmTxtInfoStore>(std::string(sv.substr(16)), vmTxtInfoStore()));
                }
                else if (sv.starts_with("DATA_TREE_END"))
                {
                    isDepth.pop_back();
                }
                else
                {
                    if (auto p = sv.find_first_of('='))
                    {
                        if (std::any_of(std::next(sv.begin(), p + 1), sv.end(), [](char c) { return std::isalpha(static_cast<unsigned char>(c)); }))
                        {
                            isDepth.back().get().template load<std::string>(std::string(sv.substr(0, p)), std::move(std::string(sv.substr(p + 1))));
                        }
                        //// From_chars is not supported by g++ 10.3.0, however I'm yet to encounter floating points in this file.
                        // else if (std::any_of(std::next(sv.begin(), p + 1), sv.end(), [](char c) { return c == '.'; }))
                        // {
                        //     if (sv.at(p + 1) == '[')
                        //     {
                        //         decltype(p) prev = sv.substr(p).find_first_of('{') + 1;
                        //         decltype(p) next;

                        //         std::vector<double> vd;
                        //         double v = 0;
                        //         do
                        //         {
                        //             next = sv.substr(prev).find_first_of(',');
                        //             next = (next == std::string_view::npos ? sv.size() : prev + next);
                        //             std::from_chars(sv.data() + prev, sv.data() + next, v);
                        //             vd.push_back(v);
                        //             prev = next + 1;
                        //         } while (next < sv.size());
                        //         isDepth.back().get().template load<double>(std::string(sv.substr(0, p)), std::move(vd));
                        //     }
                        //     else
                        //     {
                        //         double v = 0;
                        //         std::from_chars(sv.data() + p, sv.data() + sv.size(), v);
                        //         isDepth.back().get().template load<double>(std::string(sv.substr(0, p)), {v});
                        //     }
                        // }
                        else
                        {
                            if (sv.at(p + 1) == '[')
                            {
                                decltype(p) prev = sv.substr(p).find_first_of('{') + 1;
                                decltype(p) next;

                                std::vector<uint32_t> vs;
                                uint32_t v = 0;
                                do
                                {
                                    next = sv.substr(prev).find_first_of(',');
                                    next = (next == std::string_view::npos ? sv.size() : prev + next);
                                    std::from_chars(sv.data() + prev, sv.data() + next, v);
                                    vs.push_back(v);
                                    prev = next + 1;
                                } while (next < sv.size());
                                isDepth.back().get().template load<uint32_t>(std::string(sv.substr(0, p)), std::move(vs));
                            }
                            else
                            {
                                uint32_t v = 0;
                                std::from_chars(sv.data() + p, sv.data() + sv.size(), v);
                                isDepth.back().get().template load<uint32_t>(std::string(sv.substr(0, p)), {v});
                            }
                        }
                    }
                    else
                    {
                        isDepth.back().get().template load<std::string>(std::string(sv.substr(0, p)), {std::string()});
                    }
                }
            }
        }

        {
            std::unique_ptr<SDL_RWops, decltype(&SDL_RWclose)> vmBinOps(nullptr, SDL_RWclose);

            vmBinOps.reset(SDL_RWFromFile(vmBin, "r"));
            if (vmBinOps == nullptr)
            {
                std::cout << "SDL2 failed to open Mindray virtual machine file '" << vmBin << "', error: " << SDL_GetError();
                return false;
            }

            io::RWOpsStream vmBinRWStream = io::RWOpsStream(vmBinOps.get());
            std::istream vmBinIs(&vmBinRWStream);
            std::vector<std::reference_wrapper<vmBinInfoStore>> isDepth;
            isDepth.push_back(vmBinStore);

            uint32_t offset = vmTxtStore.fetch<vmTxtInfoStore>("CinePartition", 0).fetch<vmTxtInfoStore>("CinePartition0", 0).fetch<vmTxtInfoStore>("FeParam", 0).fetch<vmTxtInfoStore>("Version0", 0).fetch<vmTxtInfoStore>("param_content", 0).fetch<uint32_t>("OFFSET", 0);
            vmBinIs.seekg(offset);

            while (std::getline(vmBinIs, s))
            {
                std::string_view sv(s);

                if (sv.starts_with('"'))
                {
                    std::string_view::size_type colon = sv.find_first_of(':');

                    if (sv.ends_with('{'))
                    {
                        if (sv.at(colon + 1) == ':')
                        {
                            // We don't want to make a new store here, but one gets closed later anyway due to the closing brace, so we make a dummy copy.
                            isDepth.emplace_back(isDepth.back());
                        }
                        else
                        {
                            std::string_view name = sv.substr(1, sv.find_first_of('[') - 1);
                            isDepth.emplace_back(isDepth.back().get().template load<vmBinInfoStore>(std::string(name), vmBinInfoStore()));
                        }
                    }
                    else
                    {
                        std::string_view type = sv.substr(1, colon - 1);
                        std::string_view name = sv.substr(colon + 2, sv.substr(colon + 2).find_first_of('['));

                        if (type.starts_with('i'))
                        {
                            if (type.ends_with('8'))
                            {
                                int8_t v = 0;
                                while (vmBinIs >> s && !s.starts_with(']'))
                                {
                                    std::from_chars(s.data(), s.data() + s.length(), v);
                                    isDepth.back().get().template load<int8_t>(std::string(name), std::move(v));
                                }
                            }
                            else if (type.ends_with('6'))
                            {
                                int16_t v = 0;
                                while (vmBinIs >> s && !s.starts_with(']'))
                                {
                                    std::from_chars(s.data(), s.data() + s.length(), v);
                                    isDepth.back().get().template load<int16_t>(std::string(name), std::move(v));
                                }
                            }
                            else
                            {
                                int32_t v = 0;
                                while (vmBinIs >> s && !s.starts_with(']'))
                                {
                                    std::from_chars(s.data(), s.data() + s.length(), v);
                                    isDepth.back().get().template load<int32_t>(std::string(name), std::move(v));
                                }
                            }
                        }
                        else if (type.starts_with('u'))
                        {
                            if (type.ends_with('8'))
                            {
                                uint8_t v = 0;
                                while (vmBinIs >> s && !s.starts_with(']'))
                                {
                                    std::from_chars(s.data(), s.data() + s.length(), v);
                                    isDepth.back().get().template load<uint8_t>(std::string(name), std::move(v));
                                }
                            }
                            else if (type.ends_with('6'))
                            {
                                uint16_t v = 0;
                                while (vmBinIs >> s && !s.starts_with(']'))
                                {
                                    std::from_chars(s.data(), s.data() + s.length(), v);
                                    isDepth.back().get().template load<uint16_t>(std::string(name), std::move(v));
                                }
                            }
                            else
                            {
                                uint32_t v = 0;
                                while (vmBinIs >> s && !s.starts_with(']'))
                                {
                                    std::from_chars(s.data(), s.data() + s.length(), v);
                                    isDepth.back().get().template load<uint32_t>(std::string(name), std::move(v));
                                }
                            }
                        }
                        else if (type.starts_with('f'))
                        {
                            float v = 0;
                            while (vmBinIs >> s && !s.starts_with(']'))
                            {
                                v = std::stof(s);
                                // std::from_chars(s.data(), s.data() + s.length(), v);
                                isDepth.back().get().template load<float>(std::string(name), std::move(v));
                            }
                        }
                        else if (type.starts_with('d'))
                        {
                            double v = 0;
                            while (vmBinIs >> s && !s.starts_with(']'))
                            {
                                v = std::stod(s);
                                // std::from_chars(s.data(), s.data() + s.length(), v);
                                isDepth.back().get().template load<double>(std::string(name), std::move(v));
                            }
                        }
                        else
                        {
                            bool v = false;
                            while (vmBinIs >> s && !s.starts_with(']'))
                            {
                                v = s.starts_with('t') ? true : false;
                                isDepth.back().get().template load<bool>(std::string(name), std::move(v));
                            }
                        }
                    }
                }
                else if (sv.starts_with('}'))
                {
                    isDepth.pop_back();
                }
            }
        }

        cpOps.reset(SDL_RWFromFile(cp, "r"));
        if (cpOps == nullptr)
        {
            std::cout << "SDL2 failed to open Mindray cine partition file '" << cp << "', error: " << SDL_GetError();
            return false;
        }
        cpRWStream = io::RWOpsStream(cpOps.get());

        return true;
    }

} // namespace ultrasound