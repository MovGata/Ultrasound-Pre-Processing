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

            std::vector<int16_t> lineRange = vmBinStore.fetch<int16_t>("BDscLineRange");
            std::vector<uint16_t> pointRange = vmBinStore.fetch<uint16_t>("BDscPointRange");
            std::vector<int32_t> frameCount = vmBinStore.fetch<int32_t>("FrameCountPerVolume");
            std::vector<float> angleRange = vmBinStore.fetch<float>("BDispLineRange");
            std::vector<float> bzoom = vmBinStore.fetch<float>("BUploadPointGap");

            uint32_t vLength = std::abs(lineRange.at(0) - lineRange.at(1)) + 1;
            uint32_t vDepth = std::abs(pointRange.at(0) - pointRange.at(1)) + 1;
            uint32_t vWidth = frameCount.at(0);
            float angleDelta = std::abs(angleRange.at(0) - angleRange.at(1));
            float zoom = 2 * bzoom.at(0);

            std::vector<uint8_t> data;
            std::vector<uint8_t> pData;

            // uint32_t vmOffset = vmTxtStore.fetch<vmTxtInfoStore>("CinePartition", 0).fetch<vmTxtInfoStore>("CinePartition0", 0).fetch<vmTxtInfoStore>("FeParam", 0).fetch<vmTxtInfoStore>("Version0", 0).fetch<vmTxtInfoStore>("param_content", 0).fetch<uint32_t>("OFFSET", 0);

            cpOps.reset(SDL_RWFromFile(cp.c_str(), "rb"));
            if (cpOps == nullptr)
            {
                std::cout << "SDL2 failed to open Mindray cine partition file '" << cp << "', error: " << SDL_GetError();
                return false;
            }
            cpRWStream = io::RWOpsStream(cpOps.get());
            std::istream cpIs(&cpRWStream);

            uint32_t dataOffset;
            uint32_t dataSize;
            uint32_t dopplerOffset;
            uint32_t dopplerSize;
            uint16_t pDepth;
            uint16_t pLength;
            uint32_t frameSize;

            cpIs.seekg(8).read(reinterpret_cast<char *>(&frameSize), sizeof(frameSize));
            cpIs.sync();
            cpIs.seekg(104).read(reinterpret_cast<char *>(&dopplerOffset), sizeof(dopplerOffset));
            cpIs.sync();
            cpIs.seekg(112).read(reinterpret_cast<char *>(&pLength), sizeof(pLength));
            cpIs.sync();
            cpIs.seekg(114).read(reinterpret_cast<char *>(&pDepth), sizeof(pDepth));
            cpIs.sync();
            cpIs.seekg(116).read(reinterpret_cast<char *>(&dataOffset), sizeof(dataOffset));
            cpIs.sync();
            cpIs.seekg(0);
            cpIs.sync();

            dataSize = vDepth * vLength;
            dopplerSize = frameSize - dopplerOffset;

            std::vector<char> buf;
            buf.reserve(frameSize);


            vWidth = 0;

            while (cpIs.read(buf.data(), frameSize))
            {
                data.reserve(data.size() + dataSize);
                pData.reserve(pData.size() + dopplerSize);

                std::copy(buf.data() + dataOffset, buf.data() + dataOffset + dataSize, std::back_inserter(data));
                std::copy(buf.data() + dopplerOffset, buf.data() + dopplerOffset + dopplerSize, std::back_inserter(pData));

                ++vWidth;
            }

            cpStore.load<uint8_t>("Data", std::move(data));
            cpStore.load<uint8_t>("Doppler", std::move(pData));

            cpStore.load<std::size_t>("dataOffset", std::move(dataOffset));
            cpStore.load<std::size_t>("DataSize", std::move(dataSize));
            cpStore.load<std::size_t>("DopplerSize", std::move(dopplerSize));

            cpStore.load<uint16_t>("dLength", std::move(pLength));
            cpStore.load<uint16_t>("dDepth", std::move(pDepth));

            cpStore.load<int32_t>("Length", std::move(vLength));
            cpStore.load<int32_t>("Depth", std::move(vDepth));
            cpStore.load<int32_t>("Width", std::move(vWidth));
            cpStore.load<float>("AngleDelta", std::move(angleDelta));
            cpStore.load<float>("Ratio", std::move(zoom));
        }

        return true;
    }

    void Mindray::load(const cl::Context &context)
    {
        depth = cpStore.fetch<int32_t>("Depth", 0);
        length = cpStore.fetch<int32_t>("Length", 0);
        width = cpStore.fetch<int32_t>("Width", 0);
        delta = cpStore.fetch<float>("AngleDelta", 0);
        ratio = cpStore.fetch<float>("Ratio", 0);

        auto pLength = cpStore.fetch<uint16_t>("dLength", 0);
        auto pDepth = cpStore.fetch<uint16_t>("dDepth", 0);

        std::vector<uint8_t> &data = cpStore.fetch<uint8_t>("Data");
        std::vector<uint8_t> &doppler = cpStore.fetch<uint8_t>("Doppler");

        std::vector<float> bGap = vmBinStore.fetch<float>("BDispPointRange");
        std::vector<float> pGap = vmBinStore.fetch<float>("CDispPointRange");
        std::vector<float> pAngle = vmBinStore.fetch<float>("CDispLineRange");

        uint32_t t, b, l, r;

        t = static_cast<uint32_t>((pGap.at(0) - bGap.at(0)) / (bGap.at(1) - bGap.at(0)) * static_cast<float>(depth));
        b = static_cast<uint32_t>((pGap.at(1) - bGap.at(0)) / (bGap.at(1) - bGap.at(0)) * static_cast<float>(depth));
        l = static_cast<uint32_t>((pAngle.at(0) / delta + 0.5f) * static_cast<float>(length));
        r = static_cast<uint32_t>((pAngle.at(1) / delta + 0.5f) * static_cast<float>(length));


        raw.reserve(width * depth * length);
        for (unsigned int z = 0; z < width; ++z)
        {
            auto zyx = z * length * depth;
            auto pz = z * pLength * pDepth * 3;
            for (unsigned int y = 0; y < length; ++y)
            {
                auto yx = y * depth;
                auto py = std::clamp(static_cast<unsigned int>(static_cast<float>((y - l) * pLength) / static_cast<float>(r-l)), static_cast<unsigned int>(0), static_cast<unsigned int>(pLength - 1)) * pDepth;
                for (unsigned int x = 0; x < depth; ++x)
                {
                    auto px = std::clamp(static_cast<unsigned int>(static_cast<float>((x - t) * pDepth) / static_cast<float>(b-t)), static_cast<unsigned int>(0), static_cast<unsigned int>(pDepth - 1));
                    cl_uchar bnw = data.at(x + yx + zyx);
                    cl_uchar4 arr;
                    if (doppler.size() > 0 && x >= t && x < b && y >= l && y < r)
                    {
                        int8_t dData = static_cast<int8_t>(doppler.at(px + py + pz));
                        // std ::cout << px << ' ' << py << ' ' << dData << std::endl;
                        if (dData < 0)
                        {
                            arr = {0x00, static_cast<cl_uchar>(static_cast<uint8_t>(static_cast<int8_t>(-1) * dData) * static_cast<uint8_t>(2)), 0xFF, 0xFF};
                        }
                        else if (dData > 0)
                        {
                            arr = {0xFF, static_cast<cl_uchar>(static_cast<uint8_t>(dData) * static_cast<uint8_t>(2)), 0x00, 0xFF};
                        }
                        else
                        {
                            arr = {0xFF, 0xFF, 0xFF, bnw};
                        }
                    }
                    else
                    {
                        arr = {0xFF, 0xFF, 0xFF, bnw};
                    }
                    raw.push_back(arr);
                }
            }
        }

        buffer = cl::Buffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(raw[0]) * raw.size(), raw.data());
        // mvBuffer = cl::Buffer(context, CL_MEM_READ_ONLY, 12 * sizeof(cl_float));
    }

    void Mindray::sendToCl(const cl::Context &context)
    {
        buffer = cl::Buffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(raw[0]) * raw.size(), raw.data());
    }

    void Mindray::execute()
    {
        modified = true;
    }

} // namespace ultrasound