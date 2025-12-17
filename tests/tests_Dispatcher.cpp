#include <array>
#include <string_view>
#include <iostream>
#include "wsparsing.hpp"

int tests_failed = 0;
#define CHECK(cond) \
    do { \
        if (!(cond)) { \
            printf("FAILED: %s (%s:%d)\n", #cond, __FILE__, __LINE__); \
            tests_failed++; \
        } \
    } while(0)

// ---------- Test helpers ----------
std::array<sxd::CommandMapping, sxd::Dispatcher::DISPATCH_TABLE_SIZE> make_table() {
    return {{
        {"test:", [](std::string_view arg){ return 1; }},
        {"json:", [](std::string_view arg){ return 2; }},
        {"names:", [](std::string_view arg){ return 3; }},
        {"alert:", [](std::string_view arg){ return 4; }}
    }};
}

// ---------- Tests ----------
void test_dispatcher_test_command() {
    DisplayHandler dh;
    sxd::Dispatcher disp(dh);
    sxd::defaultDispatcher = &disp;
    disp.set_dispatch_table(make_table());

    CHECK(disp.dispatch_ws_message("test:hello") == 1);
    CHECK(disp.dispatch_ws_message("test:world") == 1);
}

void test_dispatcher_json_command() {
    DisplayHandler dh;
    sxd::Dispatcher disp(dh);
    sxd::defaultDispatcher = &disp;
    disp.set_dispatch_table(make_table());

    CHECK(disp.dispatch_ws_message("json:abc") == 2);
    CHECK(disp.dispatch_ws_message("json:def") == 2);
}

void test_dispatcher_names_command() {
    DisplayHandler dh;
    sxd::Dispatcher disp(dh);
    sxd::defaultDispatcher = &disp;
    disp.set_dispatch_table(make_table());

    CHECK(disp.dispatch_ws_message("names:foo") == 3);
}

void test_dispatcher_alert_command() {
    DisplayHandler dh;
    sxd::Dispatcher disp(dh);
    sxd::defaultDispatcher = &disp;
    disp.set_dispatch_table(make_table());

    CHECK(disp.dispatch_ws_message("alert:bar") == 4);
}

void test_dispatcher_unknown_command() {
    DisplayHandler dh;
    sxd::Dispatcher disp(dh);
    sxd::defaultDispatcher = &disp;
    disp.set_dispatch_table(make_table());

    CHECK(disp.dispatch_ws_message("unknown") == -1);
}

void test_dispatcher_empty_message() {
    DisplayHandler dh;
    sxd::Dispatcher disp(dh);
    sxd::defaultDispatcher = &disp;
    disp.set_dispatch_table(make_table());

    CHECK(disp.dispatch_ws_message("") == -1);
}

void test_dispatcher_partial_match() {
    DisplayHandler dh;
    sxd::Dispatcher disp(dh);
    sxd::defaultDispatcher = &disp;
    disp.set_dispatch_table(make_table());

    CHECK(disp.dispatch_ws_message("tes") == -1);
    CHECK(disp.dispatch_ws_message("js") == -1);
}

void test_dispatcher_long_message() {
    DisplayHandler dh;
    sxd::Dispatcher disp(dh);
    sxd::defaultDispatcher = &disp;
    disp.set_dispatch_table(make_table());

    std::string long_msg(1000, 'x');
    CHECK(disp.dispatch_ws_message("test:" + long_msg) == 1);
}

void test_dispatcher_special_chars() {
    DisplayHandler dh;
    sxd::Dispatcher disp(dh);
    sxd::defaultDispatcher = &disp;
    disp.set_dispatch_table(make_table());

    CHECK(disp.dispatch_ws_message("test:!@#$%^&*()") == 1);
    CHECK(disp.dispatch_ws_message("json:<>?/") == 2);
}

void test_dispatcher_c_bridge() {
    DisplayHandler dh;
    sxd::Dispatcher disp(dh);
    sxd::defaultDispatcher = &disp;
    disp.set_dispatch_table(make_table());

    CHECK(handle_ws_message("test:hello") == 1);
    CHECK(handle_ws_message("json:foo") == 2);
}

// ---------- Main ----------
int main() {
    static constexpr size_t TOTAL_TESTS = 10;
    test_dispatcher_test_command();
    test_dispatcher_json_command();
    test_dispatcher_names_command();
    test_dispatcher_alert_command();
    test_dispatcher_unknown_command();
    test_dispatcher_empty_message();
    test_dispatcher_partial_match();
    test_dispatcher_long_message();
    test_dispatcher_special_chars();
    test_dispatcher_c_bridge();

    if (tests_failed == 0) {
        std::cout << "All " << TOTAL_TESTS <<" Dispatcher tests passed!\n";
    } else {
        std::cout << TOTAL_TESTS-tests_failed << "Dispatcher tests PASSED\n";
        std::cout << tests_failed << " Dispatcher tests FAILED!\n";
    }
    return tests_failed;
}
