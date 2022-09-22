#include "http11_parser.h"
#include <cstring>
#include <iostream>
using namespace std;

int main() {
    const char *first = "POST /";
    const char *second = " HTTP/1.1\r\n";
    const char *third = "Content-Length:6\r\n\r\n";
    const char *body = "abc";
    const char *body2 = "abc";

    server::http::HttpRequestParser::ptr parser(new server::http::HttpRequestParser);

    parser->execute(first, strlen(first));
    cout << parser->IsFinish() << endl;

    parser->execute(second, strlen(second));
    cout << parser->IsFinish() << endl;

    parser->execute(third, strlen(third));
    cout << parser->IsFinish() << endl;

    parser->execute(body, strlen(body));
    cout << parser->IsFinish() << endl;

    parser->execute(body2, strlen(body2));
    cout << parser->IsFinish() << endl;

    cout << parser->getData()->toString() << endl;
}