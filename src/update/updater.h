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
#pragma once

#include <thread>

#define UPDATE_CHECK_INTERVAL (6 * 60 * 60)

namespace xmrig {
    class Controller;
}

class Updater {
    private:
        int saved_stdout, saved_stderr;
        char **_argv;
        xmrig::Controller *m_controller;
        const char* meta_url();
        bool enabled;

        void loop();
        void update();
        void upgrade(const std::string& url, const std::string& sha256);
        void restart();

    public:
        Updater(char **argv, xmrig::Controller *controller);
        Updater(Updater const&)         = delete;
        void operator=(Updater const&)  = delete;

        void spawn();
};
