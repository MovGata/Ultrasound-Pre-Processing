#include "Mindray.hh"

#include <algorithm>
#include <cctype>
#include <charconv>
#include <filesystem>
#include <istream>
#include <iostream>
#include <string_view>

#include <SDL2/SDL_rwops.h>

#include "../IO/SDL2/RWOpsStream.hh"

namespace ultrasound
{

    Mindray::Mindray() : Volume()
    {
    }

    Mindray::~Mindray()
    {
    }

    bool Mindray::load(const char *dir)
    {
        std::string vmTxt, vmBin, cp;
        for (const auto &entry : std::filesystem::directory_iterator(dir))
        {
            if (entry.path().filename() == "VirtualMachine.txt")
            {
                vmTxt = entry.path().generic_string();
            }
            else if (entry.path().filename() == "VirtualMachine.bin")
            {
                vmBin = entry.path().generic_string();
            }
            else if (entry.path().filename() == "BC_CinePartition0.bin")
            {
                cp = entry.path().generic_string();
            }
        }

        if (vmTxt.empty() || vmBin.empty() || cp.empty())
        {
            std::cout << "Could not find Mindray files" << std::endl;
            return false;
        }

        std::string s;
        {
            std::unique_ptr<SDL_RWops, decltype(&SDL_RWclose)> vmTxtOps(nullptr, SDL_RWclose);

            vmTxtOps.reset(SDL_RWFromFile(vmTxt.c_str(), "r"));
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
                        if (std::any_of(std::next(sv.begin(), p + 1), sv.end(), [](char c)
                                        { return std::isalpha(static_cast<unsigned char>(c)); }))
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
                                decltype(p) prev = sv.find_first_of('{') + 1;
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
                                std::from_chars(sv.data() + p + 1, sv.data() + sv.size(), v);
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

            vmBinOps.reset(SDL_RWFromFile(vmBin.c_str(), "r"));
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
                std::string_view sv(s.data() + s.find_first_not_of('\t'));

                if (sv.starts_with('"'))
                {
                    std::string_view::size_type colon = sv.find_first_of(':');

                    if (sv.ends_with('{'))
                    {
                        if (sv.at(colon + 1) == ':')
                        {
                            // We don't want to make a new store here, but one gets closed later anyway due to the closing brace, so we make a dummy copy.
                            std::string_view name = sv.substr(colon + 2, sv.substr(colon + 2).find_first_of('['));
                            isDepth.emplace_back(isDepth.back().get().template load<vmBinInfoStore>(std::string(name), vmBinInfoStore()));
                        }
                        else
                        {
                            std::string_view name = sv.substr(1, sv.find_first_of(']'));
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
                                std::string n;
                                std::vector<int8_t> vs;
                                int8_t v = 0;
                                while (vmBinIs >> n >> std::ws && !n.starts_with(']'))
                                {
                                    std::from_chars(n.data(), n.data() + n.length(), v);
                                    vs.push_back(v);
                                }
                                isDepth.back().get().template load<int8_t>(std::string(name), std::move(vs));
                            }
                            else if (type.ends_with('6'))
                            {
                                std::string n;
                                std::vector<int16_t> vs;
                                int16_t v = 0;
                                while (vmBinIs >> n >> std::ws && !n.starts_with(']'))
                                {
                                    std::from_chars(n.data(), n.data() + n.length(), v);
                                    vs.push_back(v);
                                }
                                isDepth.back().get().template load<int16_t>(std::string(name), std::move(vs));
                            }
                            else
                            {
                                std::string n;
                                std::vector<int32_t> vs;
                                int32_t v = 0;
                                while (vmBinIs >> n >> std::ws && !n.starts_with(']'))
                                {
                                    std::from_chars(n.data(), n.data() + n.length(), v);
                                    vs.push_back(v);
                                }
                                isDepth.back().get().template load<int32_t>(std::string(name), std::move(vs));
                            }
                        }
                        else if (type.starts_with('u'))
                        {
                            if (type.ends_with('8'))
                            {
                                std::string n;
                                std::vector<uint8_t> vs;
                                uint8_t v = 0;
                                while (vmBinIs >> n >> std::ws && !n.starts_with(']'))
                                {
                                    std::from_chars(n.data(), n.data() + n.length(), v);
                                    vs.push_back(v);
                                }
                                isDepth.back().get().template load<uint8_t>(std::string(name), std::move(vs));
                            }
                            else if (type.ends_with('6'))
                            {
                                std::string n;
                                std::vector<uint16_t> vs;
                                uint16_t v = 0;
                                while (vmBinIs >> n >> std::ws && !n.starts_with(']'))
                                {
                                    std::from_chars(n.data(), n.data() + n.length(), v);
                                    vs.push_back(v);
                                }
                                isDepth.back().get().template load<uint16_t>(std::string(name), std::move(vs));
                            }
                            else
                            {
                                std::string n;
                                std::vector<uint32_t> vs;
                                uint32_t v = 0;
                                while (vmBinIs >> n >> std::ws && !n.starts_with(']'))
                                {
                                    std::from_chars(n.data(), n.data() + n.length(), v);
                                    vs.push_back(v);
                                }
                                isDepth.back().get().template load<uint32_t>(std::string(name), std::move(vs));
                            }
                        }
                        else if (type.starts_with('f'))
                        {
                            std::string n;
                            std::vector<float> vs;
                            float v = 0;
                            while (vmBinIs >> n >> std::ws && !n.starts_with(']'))
                            {
                                // std::from_chars(n.data(), n.data() + n.length(), v);
                                v = std::stof(n);
                                vs.push_back(v);
                            }
                            isDepth.back().get().template load<float>(std::string(name), std::move(vs));
                        }
                        else if (type.starts_with('d'))
                        {
                            std::string n;
                            std::vector<double> vs;
                            double v = 0;
                            while (vmBinIs >> n >> std::ws && !n.starts_with(']'))
                            {
                                // std::from_chars(n.data(), n.data() + n.length(), v);
                                v = std::stod(n);
                                vs.push_back(v);
                            }
                            isDepth.back().get().template load<double>(std::string(name), std::move(vs));
                        }
                        else
                        {
                            std::string n;
                            std::vector<io::Bool> vs;
                            bool v = false;
                            while (vmBinIs >> n >> std::ws >> std::ws && !n.starts_with(']'))
                            {
                                v = n.starts_with('t') ? true : false;
                                vs.push_back({v});
                            }
                            isDepth.back().get().template load<bool>(std::string(name), std::move(vs));
                        }
                    }
                }
                else if (sv.starts_with('}'))
                {
                    isDepth.pop_back();
                }
            }
        }

        {
            std::unique_ptr<SDL_RWops, decltype(&SDL_RWclose)> cpOps(nullptr, SDL_RWclose);
            io::RWOpsStream cpRWStream = io::RWOpsStream(cpOps.get());
            std::istream cpIs(&cpRWStream);

            std::vector<int16_t> lineRange = vmBinStore.fetch<int16_t>("BDscLineRange");
            std::vector<uint16_t> pointRange = vmBinStore.fetch<uint16_t>("BDscPointRange");
            std::vector<int32_t> frameCount = vmBinStore.fetch<int32_t>("FrameCountPerVolume");
            std::vector<float> angleRange = vmBinStore.fetch<float>("BDispLineRange");

            int32_t vLength = std::abs(lineRange.at(0) - lineRange.at(1)) + 1;
            int32_t vDepth = std::abs(pointRange.at(0) - pointRange.at(1)) + 1;
            int32_t vWidth = frameCount.at(0);
            float angleDelta = std::abs(angleRange.at(0) - angleRange.at(1)) / static_cast<float>(vLength);

            std::vector<uint8_t> headers;
            std::vector<uint8_t> data;
            std::vector<uint8_t> other;

            uint32_t vmOffset = vmTxtStore.fetch<vmTxtInfoStore>("CinePartition", 0).fetch<vmTxtInfoStore>("CinePartition0", 0).fetch<vmTxtInfoStore>("FeParam", 0).fetch<vmTxtInfoStore>("Version0", 0).fetch<vmTxtInfoStore>("param_content", 0).fetch<uint32_t>("OFFSET", 0);

            std::size_t headerSize = 128;
            std::size_t otherSize = vmOffset > 16 ? 304 : 0;
            std::size_t dataSize = vLength * vDepth;
            std::size_t frameSize = dataSize + headerSize + otherSize;

            cpOps.reset(SDL_RWFromFile(cp.c_str(), "r"));
            if (cpOps == nullptr)
            {
                std::cout << "SDL2 failed to open Mindray cine partition file '" << cp << "', error: " << SDL_GetError();
                return false;
            }
            cpRWStream = io::RWOpsStream(cpOps.get());

            std::vector<char> buf;
            buf.reserve(frameSize);

            while (cpIs.read(buf.data(), frameSize))
            {
                headers.reserve(headers.size() + headerSize);
                other.reserve(other.size() + otherSize);
                data.reserve(data.size() + dataSize);

                std::copy(buf.data(), buf.data() + headerSize, std::back_inserter(headers));
                std::copy(buf.data() + headerSize, buf.data() + headerSize + otherSize, std::back_inserter(other));
                std::copy(buf.data() + headerSize + otherSize, buf.data() + headerSize + otherSize + dataSize, std::back_inserter(data));
            }

            cpStore.load<uint8_t>("Headers", std::move(headers));
            cpStore.load<uint8_t>("Other", std::move(other));
            cpStore.load<uint8_t>("Data", std::move(data));

            cpStore.load<std::size_t>("HeaderSize", std::move(headerSize));
            cpStore.load<std::size_t>("OtherSize", std::move(otherSize));
            cpStore.load<std::size_t>("DataSize", std::move(dataSize));

            cpStore.load<int32_t>("Length", std::move(vLength));
            cpStore.load<int32_t>("Depth", std::move(vDepth));
            cpStore.load<int32_t>("Width", std::move(vWidth));
            cpStore.load<float>("AngleDelta", std::move(angleDelta));
            
        }


        return true;
    }

    void Mindray::load(const cl::Context &context, unsigned int d, unsigned int l, unsigned int w, const std::vector<uint8_t> &data)
    {
        depth = d;
        length = l;
        width = w;

        raw.reserve(width * depth * length);
        for (unsigned int z = 0; z < width; ++z)
        {
            auto zyx = z * length * depth;
            for (unsigned int y = 0; y < length; ++y)
            {
                auto yx = y * depth;
                for (unsigned int x = 0; x < depth; ++x)
                {
                    cl_uchar bnw = data.at(x + yx + zyx);
                    cl_uchar4 arr = {bnw, bnw, bnw, 0xFF};
                    raw.push_back(arr);
                }
            }
        }

        buffer = cl::Buffer(context, CL_MEM_READ_ONLY, d * l * w * sizeof(cl_uchar4));
        // mvBuffer = cl::Buffer(context, CL_MEM_READ_ONLY, 12 * sizeof(cl_float));
    }

    void Mindray::sendToCl(const cl::Context &context)
    {
        buffer = cl::Buffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(raw[0]) * raw.size(), raw.data());
    }

} // namespace ultrasound