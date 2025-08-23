#ifndef APPCONFIG_H
#define APPCONFIG_H

/********************************************************
 * Configuration section
********************************************************/
#define STD_ON                          1
#define STD_OFF                         0

#define APP_LINUX_CODE_ENABLE           STD_OFF

#define CPU_USAGE_PATH                      "/proc/stat"
#define CPU_LOAD_PATH                       "/proc/loadavg"
#define CPU_TEMPER_PATH                     "/sys/class/thermal/thermal_zone0/temp"

#define RAM_USAGE_PATH                      "/proc/meminfo"
#define RAM_TOTAL_PREFIX                    "MemTotal:"
#define RAM_AVAILABLE_PREFIX                "MemAvailable:"
#define SWAP_TOTAL_PREFIX                   "SwapTotal:"
#define SWAP_FREE_PREFIX                    "SwapFree:"

#endif // APPCONFIG_H
