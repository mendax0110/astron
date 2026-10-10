#include <QCoreApplication>

#include <gtest/gtest.h>

#include <cstdio>
#include <string_view>

int main(int argc, char* argv[])
{
    QCoreApplication application(argc, argv);
    if(argc == 2 && std::string_view(argv[1]) == "--solver-child")
    {
        std::puts("READY");
        std::fflush(stdout);
        return application.exec();
    }

    application.setApplicationName("AstronTests");
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
