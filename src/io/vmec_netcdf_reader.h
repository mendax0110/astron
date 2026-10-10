#pragma once

#include "core/equilibrium.h"

#include <filesystem>

/**
 * @brief Astron namespace for input/output operations \namespace astron::io
 */
namespace astron::io
{
    /**
     * @brief Loads a VMEC equilibrium from a NetCDF file. \fn loadVmecEquilibrium
     * @param path The path to the NetCDF file containing the VMEC equilibrium data.
     * @return An Equilibrium object representing the loaded VMEC equilibrium.
     * @throws std::runtime_error if the file cannot be opened or if required variables are missing.
     */
    [[nodiscard]] core::Equilibrium loadVmecEquilibrium(const std::filesystem::path& path);
} // namespace astron::io
