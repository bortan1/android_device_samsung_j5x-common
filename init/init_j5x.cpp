/*
   Copyright (c) 2017, The Linux Foundation. All rights reserved.
*/

#include <android-base/file.h>
#include <android-base/properties.h>
#include <android-base/strings.h>

#include <cstdio>
#include <string>

#include "vendor_init.h"

#define SIMSLOT_FILE "/proc/simslot_count"
#define SERIAL_NUMBER_FILE "/efs/FactoryApp/serial_no"

using android::base::GetProperty;
using android::base::ReadFileToString;
using android::base::SetProperty;
using android::base::StartsWith;
using android::base::Trim;


struct Variant {
    const char* bootloader;
    const char* device;
    const char* model;
    bool lte;
};


static const Variant variants[] = {
    { "J510FN", "j5xnlte",    "SM-J510FN", true },
    { "J510F",  "j5xlte",     "SM-J510F",  true },
    { "J510MN", "j5xnlte",    "SM-J510MN", true },
    { "J510GN", "j5xnlte",    "SM-J510GN", true },
    { "J5108",  "j5xltecmcc", "SM-J5108",  true },
    { "J510H",  "j5xn3g",     "SM-J510H",  false },
};


static void set_product_properties(
        const char* device,
        const char* model)
{
    SetProperty("ro.product.device", device);
    SetProperty("ro.product.model", model);

    SetProperty("ro.product.vendor.device", device);
    SetProperty("ro.product.vendor.model", model);
}


static int read_integer(const char* filename)
{
    FILE* file;
    int value = -1;

    file = fopen(filename, "r");

    if (!file)
        return -1;

    if (fscanf(file, "%d", &value) != 1)
        value = -1;

    fclose(file);

    return value;
}


static void set_fingerprint()
{
    std::string fingerprint =
        GetProperty("ro.build.fingerprint", "");

    if (!fingerprint.empty())
        return;


    std::string build_id =
        GetProperty("ro.build.id", "");

    std::string build_tags =
        GetProperty("ro.build.tags", "");

    std::string build_type =
        GetProperty("ro.build.type", "");

    std::string device =
        GetProperty("ro.product.device", "");

    std::string incremental =
        GetProperty(
            "ro.build.version.incremental",
            ""
        );

    std::string release =
        GetProperty(
            "ro.build.version.release",
            ""
        );


    std::string new_fingerprint =
        "samsung/" + device +
        "/" + device +
        ":" + release +
        "/" + build_id +
        "/" + incremental +
        ":" + build_type +
        "/" + build_tags;


    SetProperty(
        "ro.build.fingerprint",
        new_fingerprint
    );

    SetProperty(
        "ro.boot.fingerprint",
        new_fingerprint
    );
}


static void set_dsds_properties()
{
    SetProperty(
        "ro.multisim.simslotcount",
        "2"
    );

    SetProperty(
        "ro.telephony.ril.config",
        "simactivation"
    );

    SetProperty(
        "persist.radio.multisim.config",
        "dsds"
    );

    SetProperty(
        "rild.libpath2",
        "/system/lib/libsec-ril-dsds.so"
    );

    SetProperty(
        "ro.multisim.audio_follow_default_sim",
        "false"
    );
}


static void set_gsm_properties()
{
    SetProperty(
        "telephony.lteOnCdmaDevice",
        "0"
    );

    SetProperty(
        "ro.telephony.default_network",
        "9"
    );
}


static void set_lte_properties()
{
    SetProperty(
        "persist.radio.lte_vrte_ltd",
        "1"
    );

    SetProperty(
        "telephony.lteOnCdmaDevice",
        "0"
    );

    SetProperty(
        "telephony.lteOnGsmDevice",
        "1"
    );

    SetProperty(
        "ro.telephony.default_network",
        "10"
    );
}


static void set_target_properties(
        const Variant& variant)
{
    set_product_properties(
        variant.device,
        variant.model
    );


    SetProperty(
        "ro.ril.telephony.mqanelements",
        "6"
    );


    if (variant.lte)
        set_lte_properties();
    else
        set_gsm_properties();


    set_fingerprint();


    if (access(SIMSLOT_FILE, F_OK) == 0) {
        int sim_count =
            read_integer(SIMSLOT_FILE);

        if (sim_count == 2)
            set_dsds_properties();
    }


    std::string serial_number;

    if (ReadFileToString(
            SERIAL_NUMBER_FILE,
            &serial_number)) {

        serial_number = Trim(serial_number);

        SetProperty(
            "ro.serialno",
            serial_number
        );
    }
}


void vendor_load_properties()
{
    std::string bootloader =
        GetProperty(
            "ro.bootloader",
            ""
        );


    for (const auto& variant : variants) {

        if (StartsWith(
                bootloader,
                variant.bootloader)) {

            set_target_properties(
                variant
            );

            return;
        }
    }
}

