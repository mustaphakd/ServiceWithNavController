//
// Created by musta on 12/23/2021.
//

#ifndef SERVICEWITHNAVCONTROLLER_STRINGHELPER_H
#define SERVICEWITHNAVCONTROLLER_STRINGHELPER_H

#include <string>
#include <vector>

namespace wrsft {

    enum class NetworkDevice
    {
        lo, eth, tr, sl, ppp, plip, ax, seth_w, sit, ip6tnl, wlan, p2p
    };

    std::vector<std::string> split(std::string str, std::string token);
    std::string NetworkDeviceToString(NetworkDevice device);
}

#endif //SERVICEWITHNAVCONTROLLER_STRINGHELPER_H
