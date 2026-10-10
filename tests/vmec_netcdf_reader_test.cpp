#include "io/vmec_netcdf_reader.h"
#include "wrapper/netcdf_file_wrapper.h"

#include <QTemporaryDir>

#include <gtest/gtest.h>

#include <netcdf.h>

#include <array>
#include <filesystem>
#include <numbers>
#include <stdexcept>
#include <string>

using namespace astron::io;

namespace
{
    void checkNetcdf(const int status)
    {
        if(status != NC_NOERR)
        {
            throw std::runtime_error(nc_strerror(status));
        }
    }

    class WritableNetcdf
    {
    public:
        explicit WritableNetcdf(const std::filesystem::path& path)
        {
            checkNetcdf(nc_create(path.string().c_str(), NC_CLOBBER, &id_));
        }

        ~WritableNetcdf()
        {
            if(id_ >= 0)
            {
                nc_close(id_);
            }
        }

        WritableNetcdf(const WritableNetcdf&) = delete;
        WritableNetcdf& operator=(const WritableNetcdf&) = delete;

        [[nodiscard]] int id() const
        {
            return id_;
        }

        void close()
        {
            checkNetcdf(nc_close(id_));
            id_ = -1;
        }

    private:
        int id_ = -1;
    };

    void writeWout(const std::filesystem::path& path, bool transposed, bool includeRadius)
    {
        WritableNetcdf file(path);
        int surfaceDimension = -1;
        int modeDimension = -1;
        checkNetcdf(nc_def_dim(file.id(), "surface", 3, &surfaceDimension));
        checkNetcdf(nc_def_dim(file.id(), "mode", 2, &modeDimension));

        int fieldPeriods = -1;
        int surfaces = -1;
        int modes = -1;
        int poloidalModes = -1;
        int toroidalModes = -1;
        int radiusCos = -1;
        int heightSin = -1;
        int transform = -1;
        int volume = -1;
        int beta = -1;
        checkNetcdf(nc_def_var(file.id(), "nfp", NC_INT, 0, nullptr, &fieldPeriods));
        checkNetcdf(nc_def_var(file.id(), "ns", NC_INT, 0, nullptr, &surfaces));
        checkNetcdf(nc_def_var(file.id(), "mnmax", NC_INT, 0, nullptr, &modes));
        checkNetcdf(nc_def_var(file.id(), "xm", NC_DOUBLE, 1, &modeDimension, &poloidalModes));
        checkNetcdf(nc_def_var(file.id(), "xn", NC_DOUBLE, 1, &modeDimension, &toroidalModes));
        const std::array<int, 2> dimensions = transposed ? std::array{modeDimension, surfaceDimension} : std::array{surfaceDimension, modeDimension};
        if(includeRadius)
        {
            checkNetcdf(nc_def_var(file.id(), "rmnc", NC_DOUBLE, 2, dimensions.data(), &radiusCos));
        }
        checkNetcdf(nc_def_var(file.id(), "zmns", NC_DOUBLE, 2, dimensions.data(), &heightSin));
        checkNetcdf(nc_def_var(file.id(), "iotaf", NC_DOUBLE, 1, &surfaceDimension, &transform));
        checkNetcdf(nc_def_var(file.id(), "volume_p", NC_DOUBLE, 0, nullptr, &volume));
        checkNetcdf(nc_def_var(file.id(), "betatotal", NC_DOUBLE, 0, nullptr, &beta));
        checkNetcdf(nc_enddef(file.id()));

        constexpr int nfpValue = 5;
        constexpr int surfaceCount = 3;
        constexpr int modeCount = 2;
        constexpr std::array<double, 2> xm = {0.0, 1.0};
        constexpr std::array<double, 2> xn = {0.0, 0.0};
        const std::array<double, 6> radius = transposed ? std::array{5.0, 5.1, 5.2, 0.5, 0.6, 0.7} : std::array{5.0, 0.5, 5.1, 0.6, 5.2, 0.7};
        const std::array<double, 6> height = transposed ? std::array{0.0, 0.0, 0.0, 0.5, 0.6, 0.7} : std::array{0.0, 0.5, 0.0, 0.6, 0.0, 0.7};
        constexpr std::array<double, 3> iota = {0.1, 0.2, 0.3};
        constexpr double volumeValue = 30.0;
        constexpr double betaValue = 0.02;
        checkNetcdf(nc_put_var_int(file.id(), fieldPeriods, &nfpValue));
        checkNetcdf(nc_put_var_int(file.id(), surfaces, &surfaceCount));
        checkNetcdf(nc_put_var_int(file.id(), modes, &modeCount));
        checkNetcdf(nc_put_var_double(file.id(), poloidalModes, xm.data()));
        checkNetcdf(nc_put_var_double(file.id(), toroidalModes, xn.data()));
        if(includeRadius)
        {
            checkNetcdf(nc_put_var_double(file.id(), radiusCos, radius.data()));
        }
        checkNetcdf(nc_put_var_double(file.id(), heightSin, height.data()));
        checkNetcdf(nc_put_var_double(file.id(), transform, iota.data()));
        checkNetcdf(nc_put_var_double(file.id(), volume, &volumeValue));
        checkNetcdf(nc_put_var_double(file.id(), beta, &betaValue));
        file.close();
    }
} // namespace

TEST(VmecNetcdfReaderTest, ReadsSurfaceMajorWout)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());
    const auto path = std::filesystem::path(directory.filePath("wout_test.nc").toStdString());
    ASSERT_NO_THROW(writeWout(path, false, true));

    using namespace astron::wrapper;
    NetcdfFile checkFile(path);
    EXPECT_EQ(NetcdfFile::readInt(checkFile.id(), "nfp"), 5);
    EXPECT_EQ(NetcdfFile::readInt(checkFile.id(), "ns"), 3);

    const auto equilibrium = loadVmecEquilibrium(path);

    EXPECT_EQ(equilibrium.fieldPeriods, 5);
    EXPECT_EQ(equilibrium.radialSurfaces, 3);
    ASSERT_EQ(equilibrium.modes.size(), 2U);
    EXPECT_NEAR(equilibrium.radius(2, 0.0, 0.0), 5.9, 1e-12);
    EXPECT_NEAR(equilibrium.height(2, std::numbers::pi / 2.0, 0.0), 0.7, 1e-12);
    ASSERT_EQ(equilibrium.rotationalTransform.size(), 3U);
    EXPECT_DOUBLE_EQ(equilibrium.rotationalTransform[2], 0.3);
    EXPECT_DOUBLE_EQ(equilibrium.plasmaVolume, 30.0);
    EXPECT_DOUBLE_EQ(equilibrium.betaTotal, 0.02);
}

TEST(VmecNetcdfReaderTest, ReordersModeMajorWout)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());
    const auto path = std::filesystem::path(directory.filePath("wout_transposed.nc").toStdString());
    ASSERT_NO_THROW(writeWout(path, true, true));

    using namespace astron::wrapper;
    NetcdfFile checkFile2(path);
    EXPECT_EQ(NetcdfFile::readInt(checkFile2.id(), "mnmax"), 2);

    const auto equilibrium = loadVmecEquilibrium(path);

    EXPECT_NEAR(equilibrium.radius(2, 0.0, 0.0), 5.9, 1e-12);
    EXPECT_NEAR(equilibrium.height(2, std::numbers::pi / 2.0, 0.0), 0.7, 1e-12);
}

TEST(VmecNetcdfReaderTest, RejectsMissingRequiredModeArray)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());
    const auto path = std::filesystem::path(directory.filePath("wout_missing.nc").toStdString());
    ASSERT_NO_THROW(writeWout(path, false, false));

    using namespace astron::wrapper;
    NetcdfFile missingFile(path);
    EXPECT_THROW(NetcdfFile::readModes(missingFile.id(), "rmnc", 3, 2), std::runtime_error);
    EXPECT_THROW((void)loadVmecEquilibrium(path), std::runtime_error);
}
