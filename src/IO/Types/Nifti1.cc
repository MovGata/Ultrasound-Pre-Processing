#include "Nifti1.hh"

#include <SDL2/SDL_rwops.h>
#include <iostream>
#include <filesystem>

#include "nifti1.h"

#include "../SDL2/RWOpsStream.hh"

namespace io
{

    Nifti1::Nifti1(const cl::CommandQueue &cq) : cQueue(cq)
    {
        Filter::input = std::bind(input, this, std::placeholders::_1);
        Filter::execute = std::bind(execute, this);
        Filter::getOptions = std::bind(getOptions, this);
    }

    void Nifti1::input(const std::weak_ptr<data::Volume> &wv)
    {
        inVolume = wv;
        std::shared_ptr<data::Volume> sptr = wv.lock();
        volume->raw.resize(1);

        if (Filter::toggle && sptr)
        {
            volume->raw[0] = sptr->loadFromCl(cQueue);
        }
    }

    void Nifti1::execute()
    {
        if (!Filter::toggle)
            return;

        std::shared_ptr<data::Volume> sptr = inVolume.lock();
        if (sptr->rFrame == 0)
        {            
            SDL_RWops *outFile = SDL_RWFromFile("./out.nii", "wb");

            short dimCount = 0;
            dimCount += static_cast<short>((sptr->depth > 1) + (sptr->length > 1) + (sptr->width > 1) + (sptr->frames > 1));

            nifti_1_header header{
                .sizeof_hdr = 348,
                .data_type = {'D', 'T', '_', 'R', 'G', 'B', 'A', '3', '2'},
                .db_name = {'N', 'U', 'L', 'L'},
                .extents = 16384,
                .session_error = 0,
                .regular = 'r',
                .dim_info = 0,

                .dim = {dimCount, static_cast<short>(sptr->depth > 1 ? sptr->depth : 0), static_cast<short>(sptr->length > 1 ? sptr->length : 0), static_cast<short>(sptr->width > 1 ? sptr->width : 0), static_cast<short>(sptr->frames > 1 ? sptr->frames : 0), 0, 0, 0},
                .intent_p1 = 0,
                .intent_p2 = 0,
                .intent_p3 = 0,
                .intent_code = NIFTI_INTENT_NONE,
                .datatype = DT_RGBA32,
                .bitpix = sizeof(cl_uchar4),
                .slice_start = 0,
                .pixdim = {1.0f, 1.0f, 1.0f, 1.0f, sptr->fRate, 0.0f, 0.0f, 0.0f},
                .vox_offset = 352.0,
                .scl_slope = 0,
                .scl_inter = 0,
                .slice_end = 0,
                .slice_code = 0,
                .xyzt_units = SPACE_TIME_TO_XYZT(NIFTI_UNITS_UNKNOWN, NIFTI_UNITS_MSEC),
                .cal_max = 0.0f,
                .cal_min = 0.0f,
                .slice_duration = 0,
                .toffset = 0,
                .glmax = sptr->max,
                .glmin = sptr->min,

                .descrip = {'N', 'i', 'f', 't', 'i', '1', ' ', 'U', 'l', 't', 'r', 'a', 's', 'o', 'u', 'n', 'd', ' ', 'F', 'i', 'l', 'e'},
                .aux_file = {'o', 'u', 't', '.', 'n', 'i', 'i'},

                .qform_code = NIFTI_XFORM_SCANNER_ANAT,
                .sform_code = NIFTI_XFORM_SCANNER_ANAT,

                .quatern_b = 0.0f,
                .quatern_c = 1.0f/std::sqrt(2.0f),
                .quatern_d = 0.0f,
                .qoffset_x = 0.0f,
                .qoffset_y = 0.0f,
                .qoffset_z = 0.0f,

                .srow_x = {0.0f, 0.0f, 1.0f, 0.0f},
                .srow_y = {0.0f, 1.0f, 0.0f, 0.0f},
                .srow_z = {-1.0f, 0.0f, 0.0f, 0.0f},

                .intent_name = {0},
                .magic = {'n', '+', '1', '\0'}};

            SDL_RWwrite(outFile, &header, sizeof(header), 1);

            nifti1_extender extender{
                .extension = {0, 0, 0, 0}};

            SDL_RWwrite(outFile, &extender, sizeof(extender), 1);
            SDL_RWwrite(outFile, volume->raw[0].data(), volume->raw[0].size(), 1);
            SDL_RWclose(outFile);
        }
        else
        {
            SDL_RWops *outFile = SDL_RWFromFile("./out.nii", "ab");
            SDL_RWwrite(outFile, volume->raw[0].data(), volume->raw[0].size(), 1);
            SDL_RWclose(outFile);
        }

        if (sptr->rFrame == sptr->frames - 1)
        {
            std::string astr = "File saved to: \n\n";
            astr += std::filesystem::absolute(std::filesystem::current_path()).string() + "\\out.nii";
            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "NIFTI-1 Save Complete", astr.c_str(), nullptr);
        }
    }

    std::shared_ptr<gui::Tree> Nifti1::getOptions()
    {
        std::shared_ptr<gui::Tree> options = gui::Tree::build("OPTIONS");
        return options;
    }
} // namespace io
