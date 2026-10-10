#include "netcdf_file_wrapper.h"

using namespace astron::wrapper;

NetcdfFile::NetcdfFile(const std::filesystem::path& path)
{
    const int status = nc_open(path.string().c_str(), NC_NOWRITE, &id_);
    if(status != NC_NOERR)
    {
        throw std::runtime_error(std::string("Cannot open VMEC result: ") + nc_strerror(status));
    }
}

NetcdfFile::~NetcdfFile()
{
    if(id_ >= 0)
    {
        nc_close(id_);
    }
}

int NetcdfFile::id() const
{
    return id_;
}

int NetcdfFile::variableId(int file, const char* name, const bool required)
{
    int id = -1;
    const int status = nc_inq_varid(file, name, &id);
    if(status != NC_NOERR && required)
    {
        throw std::runtime_error(std::string("VMEC result is missing '") + name + "'");
    }
    return id;
}

int NetcdfFile::readInt(int file, const char* name)
{
    const int id = variableId(file, name, true);
    int value = 0;
    if(nc_get_var_int(file, id, &value) != NC_NOERR)
    {
        throw std::runtime_error(std::string("Cannot read '") + name + "'");
    }
    return value;
}

double NetcdfFile::readOptionalDouble(int file, const char* name)
{
    const int id = variableId(file, name, false);
    double value = 0.0;
    if(id >= 0)
    {
        if(nc_get_var_double(file, id, &value) != NC_NOERR)
        {
            throw std::runtime_error(std::string("Cannot read '") + name + "'");
        }
    }
    return value;
}

std::vector<double> NetcdfFile::readVector(int file, const char* name, const int expectedSize, const bool required)
{
    const int id = variableId(file, name, required);
    if(id < 0)
    {
        return {};
    }

    int dimensions = 0;
    std::array<int, NC_MAX_VAR_DIMS> dimensionIds = {};
    if(nc_inq_var(file, id, nullptr, nullptr, &dimensions, dimensionIds.data(), nullptr) != NC_NOERR || dimensions != 1)
    {
        throw std::runtime_error(std::string("'") + name + "' must be a one-dimensional array");
    }

    size_t length = 0;
    nc_inq_dimlen(file, dimensionIds[0], &length);
    if(length != static_cast<size_t>(expectedSize))
    {
        throw std::runtime_error(std::string("Unexpected length of '") + name + "'");
    }

    std::vector<double> values(length);
    if(nc_get_var_double(file, id, values.data()) != NC_NOERR)
    {
        throw std::runtime_error(std::string("Cannot read '") + name + "'");
    }
    return values;
}

std::vector<double> NetcdfFile::readModes(const int file, const char* name, const int surfaces, const int modes, const bool required)
{
    const int id = variableId(file, name, required);
    if(id < 0)
    {
        return {};
    }

    int dimensions = 0;
    std::array<int, NC_MAX_VAR_DIMS> dimensionIds = {};
    if(nc_inq_var(file, id, nullptr, nullptr, &dimensions, dimensionIds.data(), nullptr) != NC_NOERR || dimensions != 2)
    {
        throw std::runtime_error(std::string("'") + name + "' must be a two-dimensional array");
    }

    size_t first = 0;
    size_t second = 0;
    nc_inq_dimlen(file, dimensionIds[0], &first);
    nc_inq_dimlen(file, dimensionIds[1], &second);
    if(first * second != static_cast<size_t>(surfaces) * static_cast<size_t>(modes))
    {
        throw std::runtime_error(std::string("Unexpected dimensions of '") + name + "'");
    }

    std::vector<double> raw(first * second);
    if(nc_get_var_double(file, id, raw.data()) != NC_NOERR)
    {
        throw std::runtime_error(std::string("Cannot read '") + name + "'");
    }

    if(first == static_cast<size_t>(surfaces) && second == static_cast<size_t>(modes))
    {
        return raw;
    }
    if(first == static_cast<size_t>(modes) && second == static_cast<size_t>(surfaces))
    {
        std::vector<double> reordered(raw.size());
        for(int surface = 0; surface < surfaces; ++surface)
        {
            for(int mode = 0; mode < modes; ++mode)
            {
                reordered[static_cast<size_t>(surface) * modes + mode] = raw[static_cast<size_t>(mode) * surfaces + surface];
            }
        }
        return reordered;
    }

    throw std::runtime_error(std::string("Unsupported dimensions of '") + name + "'");
}