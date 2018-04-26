# CNRig

[![GitHub release](https://img.shields.io/github/release/cnrig/cnrig/all.svg)](https://github.com/cnrig/cnrig/releases)
[![GitHub Release Date](https://img.shields.io/github/release-date-pre/cnrig/cnrig.svg)](https://github.com/cnrig/cnrig/releases)
[![GitHub license](https://img.shields.io/github/license/cnrig/cnrig.svg)](https://github.com/cnrig/cnrig/blob/master/LICENSE)

CNRig is a high performance CryptoNight CPU miner for Linux.
Based on the formidable [XMRig](https://github.com/xmrig/xmrig), its distinguishing features are automatic updates and compatibility with old distributions.

<img src="https://i.imgur.com/2avaSmL.png" width="586" >

#### Table of contents
* [Features](#features)
* [Download](#download)
* [Usage](#usage)
* [Algorithm variations](#algorithm-variations)
* [Compatibility](#compatibility)
* [Common Issues](#common-issues)
* [Other information](#other-information)
* [Release checksums](#release-checksums)
* [Contacts](#contacts)

## Features
* Automatic updates.
* Binary compatible with many Linux distributions.
* High performance.
* Support for backup (failover) mining server.
* CryptoNight-Lite support for AEON.
* CryptoNight-Heavy support for SUMO.
* Smart automatic [CPU configuration](https://github.com/xmrig/xmrig/wiki/Threads).
* Nicehash support
* It's open source software.

## Download
* Binary releases: https://github.com/cnrig/cnrig/releases
* Git tree: https://github.com/cnrig/cnrig.git
  * Clone with `git clone https://github.com/cnrig/cnrig.git`

```
wget -O cnrig https://github.com/cnrig/cnrig/releases/download/v0.1.5/cnrig-0.1.5-linux-x86_64
chmod +x cnrig
./cnrig
```

## Usage
Use [config.xmrig.com](https://config.xmrig.com/xmrig) to generate, edit or share configurations.

### Options
```
  -a, --algo=ALGO          cryptonight (default), cryptonight-lite or cryptonight-heavy
  -o, --url=URL            URL of mining server
  -O, --userpass=U:P       username:password pair for mining server
  -u, --user=USERNAME      username for mining server
  -p, --pass=PASSWORD      password for mining server
  -t, --threads=N          number of miner threads
  -v, --av=N               algorithm variation, 0 auto select
  -k, --keepalive          send keepalived for prevent timeout (need pool support)
  -r, --retries=N          number of times to retry before switch to backup server (default: 5)
  -R, --retry-pause=N      time to pause between retries (default: 5)
      --cpu-affinity       set process affinity to CPU core(s), mask 0x3 for cores 0 and 1
      --cpu-priority       set process priority (0 idle, 2 normal to 5 highest)
      --no-huge-pages      disable huge pages support
      --no-color           disable colored output
      --no-auto-update     disable automatic updates
      --variant            algorithm PoW variant
      --donate-level=N     donate level, default 5% (5 minutes in 100 minutes)
      --user-agent         set custom user-agent string for pool
  -B, --background         run the miner in the background
  -c, --config=FILE        load a JSON-format configuration file
  -l, --log-file=FILE      log all output to a file
  -S, --syslog             use system log for output messages
      --max-cpu-usage=N    maximum CPU usage for automatic threads mode (default 75)
      --safe               safe adjust threads and av settings for current CPU
      --nicehash           enable nicehash/xmrig-proxy support
      --print-time=N       print hashrate report every N seconds
      --api-port=N         port for the miner API
      --api-access-token=T access token for API
      --api-worker-id=ID   custom worker-id for API
  -h, --help               display this help and exit
  -V, --version            output version information and exit
```

Also you can use configuration via config file, default **config.json**. You can load multiple config files and combine it with command line options.

## Algorithm variations
* `--av=1` For CPUs with hardware AES.
* `--av=2` Lower power mode (double hash) of `1`.
* `--av=3` Software AES implementation.
* `--av=4` Lower power mode (double hash) of `3`.

## Compatibility
CNRig is compiled 100% statically and should work on a wide variety of Linux distributions, It has been tested with:

| Distro           | Kernel     | Compatible   |
| ---------------- | ----------:|:------------:|
| Ubuntu 18.04     | 4.15       | Yes          |
| Ubuntu 10.04     | 2.6.32     | Yes          |
| CentOS 6         | 2.6.32     | Yes          |
| CentOS 5         | 2.6.18     | Yes          |

## Common Issues
## Other information
* No HTTP support, only stratum protocol support.
* No TLS support.
* Default donation 5% (5 minutes in 100 minutes) can be reduced to 1% via command line option `--donate-level`.


### CPU mining performance
* **Intel i7-7700** - 307 H/s (4 threads)
* **AMD Ryzen 7 1700X** - 560 H/s (8 threads)

Please note performance is highly dependent on system load. The numbers above are obtained on an idle system. Tasks heavily using a processor cache, such as video playback, can greatly degrade hashrate. Optimal number of threads depends on the size of the L3 cache of a processor, 1 thread requires 2 MB of cache.

### Maximum performance checklist
* Idle operating system.
* Do not exceed optimal thread count.
* Use modern CPUs with AES-NI instruction set.
* Try setup optimal cpu affinity.
* Enable fast memory (Large/Huge pages).

## Release checksums
[see update.json](update.json)

## Contacts
* [https://github.com/cnrig/cnrig/issues](https://github.com/cnrig/cnrig/issues)
* mode0x13@firemail.cc
