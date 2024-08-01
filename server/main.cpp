#include "network.h"

int main(int argc, char** argv)
{
    network::init();
    network::update();
    network::shutdown();

    return 0;
}