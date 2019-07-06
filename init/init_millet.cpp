#include "minui/minui.h"
#include <cutils/klog.h>
#include <fcntl.h>
#include "healthd.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define _REALLY_INCLUDE_SYS__SYSTEM_PROPERTIES_H_
#include <sys/_system_properties.h>

#include "log.h"
#include "property_service.h"
#include "util.h"
#include "vendor_init.h"

#ifndef BACKLIGHT_PATH
#define BACKLIGHT_PATH          "/sys/class/leds/lcd-backlight/brightness"
#endif
#define BACKLIGHT_ON_LEVEL      100

#define LOGE(x...) do { KLOG_ERROR("charger", x); } while (0)
#define LOGW(x...) do { KLOG_WARNING("charger", x); } while (0)
#define LOGV(x...) do { KLOG_DEBUG("charger", x); } while (0)

std::string bootloader;
std::string device;
char* devicename;

static const GRFont* gr_font = NULL;

static const GRFont* get_font()
{
    return gr_font;
}

void property_override(char const prop[], char const value[])
{
    prop_info *pi;

    pi = (prop_info*) __system_property_find(prop);
    if (pi)
        __system_property_update(pi, value, strlen(value));
    else
        __system_property_add(prop, strlen(prop), value, strlen(value));
}

void init_target_properties(void)
{

    std::string bootloader = property_get("ro.bootloader");

    if (bootloader.find("T330NU") == 0) {
		property_override("ro.build.fingerprint", "samsung/milletwifixx/milletwifi:5.0.2/LRX22G/T331XXU1BOD8:user/release-keys");
		property_override("ro.build.description", "milletwifixx-user 5.0.2 LRX22G T330XXU1BOD8 release-keys");
        property_override("ro.product.product", "milletwifi");
        property_override("ro.product.device", "milletwifi");
        property_override("ro.product.model", "SM-T330NU");
        property_override("ro.carrier", "wifi-only");
        property_override("ro.radio.noril", "1");
    }
    else if (bootloader.find("T330XX") == 0) {
        /* milletwifixx */
        property_override("ro.build.fingerprint", "samsung/milletwifixx/milletwifi:5.0.2/LRX22G/T330XXU1BOJ4:user/release-keys");
        property_override("ro.build.description", "milletwifixx-user 5.0.2 LRX22G T330XXU1BOJ4 release-keys");
        property_override("ro.product.model", "SM-T330");
        property_override("ro.product.name", "milletwifi");
        property_override("ro.product.device", "milletwifi");
        property_override("ro.carrier", "wifi-only");
        property_override("ro.radio.noril", "1");
    }
    else if (bootloader.find("T331XX") == 0) {
        /* millet3gxx */
		property_override("ro.build.fingerprint", "samsung/millet3gxx/millet3g:5.0.2/LRX22G/T331XXU1BOD8:user/release-keys");
		property_override("ro.build.description", "millet3gxx-user 5.0.2 LRX22G T331XXU1BOD8 release-keys");
        property_override("ro.product.model", "SM-T331");
        property_override("ro.product.name", "millet3g");
        property_override("ro.product.device", "millet3g");
        property_override("telephony.lteOnCdmaDevice", "0");
        property_override("telephony.lteOnGsmDevice", "0");
        property_override("ro.telephony.ril_class", "SamsungMSM8226RIL");
    }
    else if (bootloader.find("T335XX") == 0) {
        /* milletltexx */
		property_override("ro.build.fingerprint", "samsung/milletltexx/milletlte:5.0.2/LRX22G/T335XXU1BOD8:user/release-keys");
		property_override("ro.build.description", "milletltexx-user 5.0.2 LRX22G T335XXU1BOD8 release-keys");
        property_override("ro.product.model", "SM-T335");
        property_override("ro.product.name", "milletlte");
        property_override("ro.product.device", "milletlte");
        property_override("telephony.lteOnGsmDevice", "1");
        property_override("ro.telephony.default_network", "10");
        property_override("ro.telephony.ril_class", "SamsungMSM8226RIL");
    } else if (bootloader.find("T337T") == 0) {
        /* milletltexx */
		property_override("ro.build.fingerprint", "samsung/milletltetmo/milletltetmo:5.1.1/LMY47X/T337TUVS1CPL1:user/release-keys");
		property_override("ro.build.description", "milletltetmo-user 5.1.1 LMY47X T337TUVS1CPL1 release-keys");
        property_override("ro.product.model", "SM-T337T");
        property_override("ro.product.name", "milletltetmo");
        property_override("ro.product.device", "milletltetmo");
        property_override("telephony.lteOnGsmDevice", "1");
        property_override("ro.telephony.default_network", "9");
        property_override("ro.telephony.ril_class", "SamsungMSM8226RIL");
		property_override("rild.libpath", "/system/lib/libsec-ril.so");
		property_override("ril.subscription.types", "NV,RUIM");
		property_override("ro.telephony.ril_class", "-d /dev/smd0");
		property_override("DEVICE_PROVISIONED", "1");
    } else {
        /* milletwifi */
        property_override("ro.product.model", "SM-T3XX");
        property_override("ro.product.name", "milletxx");
        property_override("ro.product.device", "millet");
        property_override("ro.carrier", "wifi-only");
        property_override("ro.radio.noril", "1");
    }

    std::string device = property_get("ro.product.device");
    INFO("Found bootloader id %s setting build properties for %s device\n", bootloader.c_str(), device.c_str());
}

void healthd_board_init(struct healthd_config *config)
{
    config->batteryCapacityPath    = "/sys/class/power_supply/battery/capacity";
    config->batteryStatusPath      = "/sys/class/power_supply/battery/status";
    config->batteryVoltagePath     = "/sys/class/power_supply/battery/voltage_now";
    config->batteryPresentPath     = "/sys/class/power_supply/battery/present";
    config->batteryHealthPath      = "/sys/class/power_supply/battery/health";
    config->batteryTemperaturePath = "/sys/class/power_supply/battery/temp";
    config->batteryTechnologyPath  = "/sys/class/power_supply/battery/technology";
}

#define STR_LEN 8
void healthd_board_mode_charger_draw_battery(
    struct android::BatteryProperties *batt_prop)
{
    char cap_str[STR_LEN];
    int x, y;
    int str_len_px;
    static int char_height = -1, char_width = -1;

    if (char_height == -1 && char_width == -1)
        gr_font_size(get_font(), &char_width, &char_height);
    snprintf(cap_str, (STR_LEN - 1), "%d%%", batt_prop->batteryLevel);
    str_len_px = gr_measure(get_font(), cap_str);
    x = (gr_fb_width() - str_len_px) / 2;
    y = (gr_fb_height() + char_height) / 2;
    gr_color(0xa4, 0xc6, 0x39, 255);
    gr_text(get_font(), x, y, cap_str, 0);
}

int healthd_board_battery_update(__attribute__((unused)) struct android::BatteryProperties *props)
{
    // return 0 to log periodic polled battery status to kernel log
    return 0;
}

void healthd_board_mode_charger_battery_update(struct android::BatteryProperties*)
{

}

void healthd_board_mode_charger_set_backlight(bool on)
{
    int fd;
    char buffer[10];

    if (access(BACKLIGHT_PATH, R_OK | W_OK) != 0)
    {
        LOGW("Backlight control not support\n");
        return;
    }

    memset(buffer, '\0', sizeof(buffer));
    fd = open(BACKLIGHT_PATH, O_RDWR);
    if (fd < 0) {
        LOGE("Could not open backlight node : %s\n", strerror(errno));
        return;
    }
    LOGV("Enabling backlight\n");
    snprintf(buffer, sizeof(buffer), "%d\n", on ? BACKLIGHT_ON_LEVEL : 0);
    if (write(fd, buffer,strlen(buffer)) < 0) {
        LOGE("Could not write to backlight node : %s\n", strerror(errno));
    }
    close(fd);
}

void healthd_board_mode_charger_init()
{
    GRFont* tmp_font;
    int res = gr_init_font("font_log", &tmp_font);
    if (res == 0) {
        gr_font = tmp_font;
    } else {
        LOGW("Couldn't open font, falling back to default!\n");
        gr_font = gr_sys_font();
    }
}

void vendor_load_properties(void)
{
    /* set the device properties */
    init_target_properties();
}