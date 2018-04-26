/* CNRig
 * Copyright 2018 mode0x13 <mode0x13@firemail.cc>
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include <chrono>
#include <experimental/filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <iomanip>

#include <curl/curl.h>
#include <sys/auxv.h>
#include <mbedtls/sha256.h>
#include <unistd.h>

#include "core/Config.h"
#include "core/Controller.h"
#include "log/Log.h"
#include "update/cacert.h"
#include "update/updater.h"
#include "version.h"

#include "rapidjson/document.h"
#include "rapidjson/error/en.h"
#include "rapidjson/filereadstream.h"

namespace fs = std::experimental::filesystem;


static const char* default_meta_url = "https://raw.githubusercontent.com/cnrig/cnrig/master/update.json";
static const char* cacert_file = ".cnrig.cacert.pem";
static const char* backup_file = ".cnrig.previous";
static fs::path cacert_path;
static fs::path exe_path_;
static fs::path exe_dir_;

bool startswith(const std::string& s, const std::string& prefix) {
    return s.substr(0, prefix.length()) == prefix;
}

std::string sha256sum(const std::string& data) {
    uint8_t digest[32];
    std::ostringstream hexdigest;
    mbedtls_sha256_ret((const unsigned char*) data.c_str(), data.length(), digest, 0);
    for (int i=0; i<32; i++) {
        hexdigest << std::hex << std::setfill('0') << std::setw(2);
        hexdigest << static_cast<int>(digest[i]);
    }
    return hexdigest.str();
}

void quit(int status) {
    Log::i()->release();
    uv_tty_reset_mode();
    exit(status);
}

fs::path exe_path() {
    char* p = (char*) ::getauxval(AT_EXECFN);
    if (p == NULL) {
        //std::cout << "[UP] getauxval(AT_EXECFN) == NULL" << std::endl;
        char buffer[PATH_MAX];
        p = realpath("/proc/self/exe", buffer);
        if (p == NULL) {
            std::cerr << "[UP] unexpected error: can't determine executable path" << std::endl;
            quit(1);
        }
    }
    fs::path result(p);
    return fs::canonical(result);
}

fs::path exe_dir() {
    return exe_path().parent_path();
}

static size_t curlWriteString(void *contents, size_t size, size_t nmemb, void *userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}


Updater::Updater(char **argv, xmrig::Controller *controller) :
    _argv(argv),
    m_controller(controller)
{
    saved_stdout = dup(1);
    saved_stderr = dup(2);

    exe_path_ = exe_path();
    exe_dir_ = exe_dir();
    cacert_path = exe_dir_.append(cacert_file);
    writeCAcerts(cacert_path.c_str());
    if (!fs::exists(cacert_path)) {
        std::cerr << "[UP] failed to extract CA certificates" << std::endl;
        quit(1);
    }
}


void Updater::spawn() {
    std::thread( [=] { loop(); } ).detach();
}


void Updater::loop() {
    update();
    while (1) {
        std::this_thread::sleep_for(std::chrono::seconds(UPDATE_CHECK_INTERVAL));
        update();
    }
}

const char* Updater::meta_url() {
#   ifdef APP_DEBUG
    char* e = getenv("CNRIG_META_URL");
    if (e)
        return e;
#endif
    return default_meta_url;
}

void Updater::update() {
    LOG_INFO("[UP] Checking for updates");
    std::string metajson;

    CURL *curl = curl_easy_init();
    CURLcode ret;
    if (!curl) {
        LOG_ERR("[UP] curl_easy_init() failed");
        return;
    }
    curl_easy_setopt(curl, CURLOPT_URL, meta_url());
    curl_easy_setopt(curl, CURLOPT_CAINFO, cacert_path.c_str());
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlWriteString);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &metajson);
    ret = curl_easy_perform(curl);
    if (ret != CURLE_OK) {
        LOG_ERR("[UP] curl_easy_perform() failed: %s", curl_easy_strerror(ret));
        curl_easy_cleanup(curl);
        return;
    }
    if (startswith(meta_url(), "http")) {
        long response_code;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
        if (response_code != 200) {
            LOG_ERR("[UP] HTTPS response code %d", response_code);
            curl_easy_cleanup(curl);
            return;
        }
    }
    curl_easy_cleanup(curl);

    rapidjson::Document doc;
    doc.Parse(metajson.c_str());
    if (doc.HasParseError()) {
        LOG_ERR("[UP] metajson parse error");
        return;
    }
    if (!doc.HasMember("Version") ||
        !doc.HasMember("URL") ||
        !doc.HasMember("SHA2-256")) {
        LOG_ERR("[UP] metadata invalid");
        return;
    }
    if (strverscmp(APP_VERSION, doc["Version"].GetString()) < 0) {
        if (m_controller->config()->isColors()) {
            LOG_INFO("[UP] \x1B[01;32mNew version available: %s", doc["Version"].GetString());
        } else {
            LOG_INFO("[UP] New version available: %s", doc["Version"].GetString());
        }
        upgrade(doc["URL"].GetString(), doc["SHA2-256"].GetString());
    } else {
        LOG_INFO("[UP] This is the latest version.");
    }
}


void Updater::upgrade(const std::string& url, const std::string& sha256) {
    LOG_INFO("[UP] Installing %s", url.c_str());
    std::string new_exe;
    CURL *curl;
    CURLcode ret;

    curl = curl_easy_init();
    if (!curl) {
        LOG_ERR("[UP] curl_easy_init() failed");
        return;
    }
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_CAINFO, cacert_path.c_str());
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlWriteString);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &new_exe);
    ret = curl_easy_perform(curl);
    if (ret != CURLE_OK) {
        LOG_ERR("[UP] curl_easy_perform() failed: %s", curl_easy_strerror(ret));
        curl_easy_cleanup(curl);
        return;
    }
    if (startswith(url, "http")) {
        long response_code;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
        if (response_code != 200) {
            LOG_ERR("[UP] HTTPS response code %d", response_code);
            curl_easy_cleanup(curl);
            return;
        }
    }
    curl_easy_cleanup(curl);

    std::string newsum = sha256sum(new_exe);
    if (newsum != sha256) {
        LOG_ERR("[UP] checksum mismatch :( should be: %s actual: %s", sha256.c_str(), newsum.c_str());
    }
    LOG_INFO("[UP] %d bytes, SHA2-256: %s", new_exe.length(), newsum.c_str());

    LOG_INFO("[UP] Backup %s => %s", exe_path_.filename().c_str(), backup_file);
    fs::path backup_path = exe_dir_.append(backup_file);
    fs::rename(exe_path_, backup_path);

    std::ofstream of(exe_path_, std::ios::out | std::ios::binary);
    if (of.fail()) {
        LOG_ERR("[UP] failed to open executable");
        return;
    }
    of << new_exe;
    if (of.bad()) {
        LOG_ERR("[UP] failed to write executable");
        of.close();
        return;
    }
    of.close();
    fs::permissions(exe_path_, fs::perms::add_perms
        | fs::perms::owner_read | fs::perms::owner_exec
        | fs::perms::group_read | fs::perms::group_exec
        | fs::perms::others_read | fs::perms::others_exec);

    restart();
}

void Updater::restart() {
    LOG_INFO("[UP] Restarting...");
    Log::i()->release();
    uv_tty_reset_mode();
    dup2(saved_stdout, 1);
    dup2(saved_stderr, 2);
    execvp(_argv[0], _argv);
    std::cerr << "[UP] execve failed" << std::endl;
    quit(1);
}
