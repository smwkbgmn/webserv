#include "HTTP.hpp"

http_t HTTP::http;
keys_t HTTP::key;

/* METHOD - init: load keys */
void HTTP::init(void) {
    http.signature = "HTTP";
    http.type = "text/plain";

    _assignVec(http.version, strVersion, CNT_VERSION);
    _assignVec(http.method, strMethod, CNT_METHOD);

    _assignHeader();
    _assignStatus();
    _assignMime();
}

void HTTP::_assignHeader(void) {
    str_t header;

    File fileIn(nameHeaderIn, R);
    while (std::getline(fileIn.fs, header))
        key.header_in.push_back(header);

    File fileOut(nameHeaderOut, R);
    while (std::getline(fileOut.fs, header))
        key.header_out.push_back(header);
}

void HTTP::_assignStatus(void) {
    File file(nameStatus, R);

    while (!file.fs.eof()) {
        uint_t code;
        str_t reason;

        file.fs >> code;
        file.fs.get();
        std::getline(file.fs, reason);

        key.status.insert(std::make_pair(code, reason));
    }
}

void HTTP::_assignMime(void) {
    File file(nameMime, R);
    str_t type, exts, ext;

    while (!file.fs.eof()) {
        file.fs >> type;

        std::getline(file.fs, exts, ';');
        isstream_t iss(exts);
        while (iss >> ext)
            key.mime.insert(std::make_pair(ext, type));
    }
}

void HTTP::_assignVec(vec_str_t &target, const str_t source[], size_t cnt) {
    for (size_t idx = 0; idx < cnt; ++idx)
        target.push_back(source[idx]);
}

/* METHOD - transaction: send response message */
void HTTP::transaction(const Request &rqst) {
    osstream_t oss;
    _message(Response(rqst), oss);

    logfile.fs << oss.str() << std::endl;
    ssize_t bytesSent =
        send(rqst.client().socket(), oss.str().c_str(), oss.str().size(), 0);

    if (bytesSent == ERROR)
        throw err_t("http: send: " + errMsg[FAIL_SEND]);
}

void HTTP::_message(const Response &rspn, osstream_t &oss) {
    _msgLine(rspn, oss);
    _msgHeader(rspn, oss);
    if (rspn.body())
        _msgBody(rspn, oss);
}

void HTTP::_msgLine(const Response &rspn, osstream_t &oss) {
    map_uint_str_t::iterator iter = key.status.find(rspn.line().status);

    oss << http.signature << '/'
        << http.version.at(static_cast<size_t>(rspn.line().version)) << ' '
        << iter->first << " " << iter->second << CRLF;
}

void HTTP::_msgHeader(const Response &rspn, osstream_t &oss) {
    for (vec_uint_t::const_iterator iter = rspn.header().list.begin();
         iter != rspn.header().list.end(); ++iter) {
        _msgHeaderName(*iter, oss);
        _msgHeaderValue(rspn.header(), *iter, oss);
    }
    oss << CRLF;
}

void HTTP::_msgHeaderName(uint_t id, osstream_t &oss) {
    oss << key.header_out.at(id) << ": ";
}

void HTTP::_msgHeaderValue(const response_header_t &header, uint_t id,
                           osstream_t &oss) {
    switch (id) {
    case OUT_SERVER:
        oss << header.server;
        break;
    case OUT_DATE:
        break;
    case OUT_CONNECTION:
        break;
    case OUT_CHUNK:
        break;
    case OUT_CONTENT_LEN:
        oss << header.content_length;
        break;
    case OUT_CONTENT_TYPE:
        oss << header.content_type;
        break;
    }
    oss << CRLF;
}

void HTTP::_msgBody(const Response &rspn, osstream_t &oss) {
    for (size_t idx = 0; idx < rspn.header().content_length; ++idx)
        oss << rspn.body()[idx];
}

/* STRUCT INIT */
config_s::config_s(void) {
    location = "/";
    root = "./html";
    file40x = "/40x.html";
    file40x = "/50x.html";

    allow.insert(std::make_pair(GET, TRUE));
    allow.insert(std::make_pair(POST, TRUE));
    allow.insert(std::make_pair(DELETE, TRUE));
}

request_header_s::request_header_s(void) {
    connection = KEEP_ALIVE;
    chunked = FALSE;
    content_length = 0;
}

response_line_s::response_line_s(void) {
    version = VERSION_11;
    status = 200;
}

response_header_s::response_header_s(void) {
    connection = KEEP_ALIVE;
    chunked = FALSE;
    content_length = 0;
}
