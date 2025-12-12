#pragma once


namespace sxd::commands{

class ApiCall {
    public:
        int operator()(std::string_view arg);

};


}