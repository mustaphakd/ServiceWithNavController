//
// Created by musta on 12/26/2021.
//

#ifndef SERVICEWITHNAVCONTROLLER_BASESERVICE_H
#define SERVICEWITHNAVCONTROLLER_BASESERVICE_H
namespace wrsft {

    typedef std::function<void(const std::string, const std::string)> LoggerType;
class BaseService {
    public:
    BaseService(const LoggerType &logger) : logfunc{logger} {
            logfunc("baseService::ctr", "start ***  *** - end");
        }

    protected:

        const LoggerType logfunc;
    };
}

#endif //SERVICEWITHNAVCONTROLLER_BASESERVICE_H
