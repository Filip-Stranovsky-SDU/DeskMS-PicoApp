#pragma once

#include <string_view>
#include <array>
#include "lwip/altcp_tcp.h"
#include "lwip/altcp_tls.h"


extern "C" {
    #include "http_common.h"
    #include "http_verify.h"
    #include "tls_common.h"
}

namespace sxd::commands{

class ApiCall {
    public:
        static constexpr size_t RESPONSE_SIZE = 4096;
        ApiCall();
        int operator()(std::string_view arg);
        int store_json(std::string_view arg);
        void poll(void* arg);

    private:
        http_CLIENT_T* stateHTTP;
        std::array<char, RESPONSE_SIZE> json_body;
        size_t json_len = 0;


};


}