#include "io/vmec_netcdf_reader.h"
#include "wrapper/netcdf_file_wrapper.h"

#include <array>
#include <cmath>
#include <netcdf.h>
#include <stdexcept>
#include <string>
#include <utility>

using namespace astron::io;
using namespace astron::wrapper;

astron::core::Equilibrium astron::io::loadVmecEquilibrium(const std::filesystem::path& path)
{
    const NetcdfFile file(path);
    core::Equilibrium result;
    result.source = path;
    result.fieldPeriods = NetcdfFile::readInt(file.id(), "nfp");
    result.radialSurfaces = NetcdfFile::readInt(file.id(), "ns");
    const int modeCount = NetcdfFile::readInt(file.id(), "mnmax");

    if (result.fieldPeriods < 1 || result.radialSurfaces < 2 || modeCount < 1)
    {
        throw std::runtime_error("Invalid VMEC dimensions");
    }

    const auto poloidalModes = NetcdfFile::readVector(file.id(), "xm", modeCount);
    const auto toroidalModes = NetcdfFile::readVector(file.id(), "xn", modeCount);
    result.modes.reserve(modeCount);
    for (int index = 0; index < modeCount; ++index)
    {
        result.modes.push_back({poloidalModes[index], toroidalModes[index]});
    }

    result.radiusCos = NetcdfFile::readModes(file.id(), "rmnc", result.radialSurfaces, modeCount);
    result.heightSin = NetcdfFile::readModes(file.id(), "zmns", result.radialSurfaces, modeCount);
    result.radiusSin = NetcdfFile::readModes(file.id(), "rmns", result.radialSurfaces, modeCount, false);
    result.heightCos = NetcdfFile::readModes(file.id(), "zmnc", result.radialSurfaces, modeCount, false);
    result.rotationalTransform = NetcdfFile::readVector(file.id(), "iotaf", result.radialSurfaces, false);
    result.plasmaVolume = NetcdfFile::readOptionalDouble(file.id(), "volume_p");
    result.betaTotal = NetcdfFile::readOptionalDouble(file.id(), "betatotal");

    // Magnetic field lives on the Nyquist mode set (mnmax_nyq) and the half radial mesh.
    // It is optional so geometry-only files still load.
    if (NetcdfFile::variableId(file.id(), "mnmax_nyq", false) >= 0)
    {
        const int fieldModeCount = NetcdfFile::readInt(file.id(), "mnmax_nyq");
        if (fieldModeCount > 0)
        {
            const auto fieldPoloidal = NetcdfFile::readVector(file.id(), "xm_nyq", fieldModeCount, false);
            const auto fieldToroidal = NetcdfFile::readVector(file.id(), "xn_nyq", fieldModeCount, false);
            auto bTheta = NetcdfFile::readModes(file.id(), "bsupumnc", result.radialSurfaces, fieldModeCount, false);
            auto bPhi = NetcdfFile::readModes(file.id(), "bsupvmnc", result.radialSurfaces, fieldModeCount, false);
            auto bStrength = NetcdfFile::readModes(file.id(), "bmnc", result.radialSurfaces, fieldModeCount, false);
            if (!fieldPoloidal.empty() && !fieldToroidal.empty() && !bTheta.empty() && !bPhi.empty())
            {
                result.fieldModes.reserve(fieldModeCount);
                for (int index = 0; index < fieldModeCount; ++index)
                {
                    result.fieldModes.push_back({fieldPoloidal[index], fieldToroidal[index]});
                }
                result.bThetaCos = std::move(bTheta);
                result.bPhiCos = std::move(bPhi);
                result.bStrengthCos = std::move(bStrength);
            }
        }
    }
    return result;
}