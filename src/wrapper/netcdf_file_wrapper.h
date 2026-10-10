#include <array>
#include <filesystem>
#include <netcdf.h>
#include <stdexcept>
#include <vector>

namespace astron::wrapper
{
    /**
     * @brief A RAII wrapper for managing a NetCDF file handle. \class NetcdfFile
     * @details This class opens a NetCDF file in read-only mode and ensures that the file is properly closed when the object goes out of scope.
     */
    class NetcdfFile
    {
    public:
        /**
         * @brief Constructs a NetcdfFile object and opens the specified NetCDF file.
         * @param path The path to the NetCDF file to open.
         * @throws std::runtime_error if the file cannot be opened.
         */
        explicit NetcdfFile(const std::filesystem::path& path);

        /**
         * @brief Destructor that closes the NetCDF file if it is open.
         */
        ~NetcdfFile();

        /**
         * @brief Deleted copy constructor to prevent copying of NetcdfFile objects.
         */
        NetcdfFile(const NetcdfFile&) = delete;

        /**
         * @brief Deleted copy assignment operator to prevent copying of NetcdfFile objects.
         */
        NetcdfFile& operator=(const NetcdfFile&) = delete;

        /**
         * @brief Gets the NetCDF file identifier.
         * @return The NetCDF file identifier (int) for use with NetCDF library functions.
         */
        [[nodiscard]] int id() const;

        /**
         * @brief Retrieves the variable ID for a given variable name in a NetCDF file.
         * @param file The NetCDF file identifier.
         * @param name The name of the variable to retrieve.
         * @param required Indicates whether the variable is required (true) or optional (false).
         * @return The variable ID if found, or -1 if not found and not required.
         * @throws std::runtime_error if the variable is required but not found.
         */
        static int variableId(int file, const char* name, bool required);

        /**
         * @brief Reads an integer variable from a NetCDF file.
         * @param file The NetCDF file identifier.
         * @param name The name of the variable to read.
         * @return The integer value of the variable.
         * @throws std::runtime_error if the variable cannot be read.
         */
        static int readInt(int file, const char* name);

        /**
         * @brief Reads an optional double variable from a NetCDF file.
         * @param file The NetCDF file identifier.
         * @param name The name of the variable to read.
         * @return The double value of the variable.
         * @throws std::runtime_error if the variable cannot be read.
         */
        static double readOptionalDouble(int file, const char* name);

        /**
         * @brief Reads a vector of doubles from a NetCDF file.
         * @param file The NetCDF file identifier.
         * @param name The name of the variable to read.
         * @param expectedSize The expected size of the vector.
         * @param required Indicates whether the variable is required (true) or optional (false).
         * @return The vector of double values.
         * @throws std::runtime_error if the variable cannot be read or has an unexpected size.
         */
        static std::vector<double> readVector(int file, const char* name, int expectedSize, bool required = true);

        /**
         * @brief Reads a two-dimensional array of doubles from a NetCDF file.
         * @param file The NetCDF file identifier.
         * @param name The name of the variable to read.
         * @param surfaces The number of radial surfaces.
         * @param modes The number of poloidal modes.
         * @param required Indicates whether the variable is required (true) or optional (false).
         * @return The vector of double values.
         * @throws std::runtime_error if the variable cannot be read or has an unexpected size.
         */
        static std::vector<double> readModes(int file, const char* name, int surfaces, int modes, bool required = true);

    private:
        int id_ = -1;
    };
} // namespace astron::wrapper