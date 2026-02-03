//
//  kern_start.cpp
//  EmeraldGraphics
//
//  Created by RoyalGraphX on 1/20/26.
//

#include "kern_start.hpp"
#include "NVDAResmen.hpp"

static EMRLD emrldInstance;
EMRLD *EMRLD::callbackEMRLD;

int EMRLD::darwinMajor = 0;
int EMRLD::darwinMinor = 0;

// To only be modified by CarnationsInternal, to display various Internal logs and headers
// Software is currently in the experimental phase and shouldn't be used publically.
const bool EMRLD::IS_INTERNAL = true;

// Main EMRLD Routine function
void EMRLD::init() {
	
    callbackEMRLD = this;
	EMRLD::darwinMajor = getKernelVersion();
	EMRLD::darwinMinor = getKernelMinorVersion();
    const char* emrldVersionNumber = EMRLD_VERSION;
    DBGLOG(MODULE_INIT, "Hello World from EmeraldGraphics!");
    DBGLOG(MODULE_INFO, "Current Build Version running: %s", emrldVersionNumber);
    DBGLOG(MODULE_INFO, "Copyright © 2026 Carnations Botanica. All rights reserved.");
    if (EMRLD::darwinMajor > 0) {
        DBGLOG(MODULE_INFO, "Current Darwin Kernel version: %d.%d", EMRLD::darwinMajor, EMRLD::darwinMinor);
    } else {
        DBGLOG(MODULE_ERROR, "WARNING: Failed to retrieve Darwin Kernel version.");
    }

    // Internal Header BEGIN
    if (EMRLD::IS_INTERNAL) {
        DBGLOG(MODULE_WARN, "");
        DBGLOG(MODULE_WARN, "==================================================================");
		DBGLOG(MODULE_WARN, "This build of %s is for CarnationsInternal usage only!", MODULE_LONG);
        DBGLOG(MODULE_WARN, "If you received a copy of this binary as a tester, DO NOT SHARE.");
        DBGLOG(MODULE_WARN, "==================================================================");
        DBGLOG(MODULE_WARN, "");
    }
    // Internal Header END
	
    // Begin based on kernel version detected.
	// This is messy because internally, we're debugging 
    // and creating work for each version and gpu combo
    // independently, and one at a time...
	if (EMRLD::darwinMajor >= KernelVersion::Tahoe) {
        DBGLOG(MODULE_INIT, "Detected macOS Tahoe (16.x) or newer.");

    } else if (EMRLD::darwinMajor >= KernelVersion::Sequoia) {
        DBGLOG(MODULE_INIT, "Detected macOS Sequoia (15.x).");

    } else if (EMRLD::darwinMajor >= KernelVersion::Sonoma) {
        DBGLOG(MODULE_INIT, "Detected macOS Sonoma (14.x).");

    } else if (EMRLD::darwinMajor >= KernelVersion::Ventura) {
        DBGLOG(MODULE_INIT, "Detected macOS Ventura (13.x).");

    } else if (EMRLD::darwinMajor >= KernelVersion::Monterey) {
        DBGLOG(MODULE_INIT, "Detected macOS Monterey (12.x).");

    } else if (EMRLD::darwinMajor >= KernelVersion::BigSur) {
        DBGLOG(MODULE_INIT, "Detected macOS Big Sur (11.x).");

    } else if (EMRLD::darwinMajor >= KernelVersion::Catalina) {
        DBGLOG(MODULE_INIT, "Detected macOS Catalina (10.15.x).");

    } else if (EMRLD::darwinMajor >= KernelVersion::Mojave) {
        DBGLOG(MODULE_INIT, "Detected macOS Mojave (10.14.x).");

    } else if (EMRLD::darwinMajor >= KernelVersion::HighSierra) {
        DBGLOG(MODULE_INIT, "Detected macOS High Sierra (10.13.x).");

    } else if (EMRLD::darwinMajor >= KernelVersion::Sierra) {
        DBGLOG(MODULE_INIT, "Detected macOS Sierra (10.12.x).");

    } else if (EMRLD::darwinMajor >= KernelVersion::ElCapitan) {
        DBGLOG(MODULE_INIT, "Detected OS X El Capitan (10.11.x).");

    } else if (EMRLD::darwinMajor >= KernelVersion::Yosemite) {
        DBGLOG(MODULE_INIT, "Detected OS X Yosemite (10.10.x).");

    } else if (EMRLD::darwinMajor >= KernelVersion::Mavericks) {
        DBGLOG(MODULE_INIT, "Detected OS X Mavericks (10.9.x).");
		
    } else if (EMRLD::darwinMajor >= KernelVersion::MountainLion) {
        DBGLOG(MODULE_INIT, "Detected OS X Mountain Lion (10.8.x).");
		
    } else if (EMRLD::darwinMajor >= KernelVersion::Lion) {
        DBGLOG(MODULE_INIT, "Detected OS X Lion (10.7.x).");
        
    } else if (EMRLD::darwinMajor >= KernelVersion::SnowLeopard) {
        DBGLOG(MODULE_INIT, "Detected OS X Snow Leopard (10.6.x).");
		
    } else if (EMRLD::darwinMajor >= KernelVersion::Leopard) {
        DBGLOG(MODULE_INIT, "Detected OS X Leopard (10.5.x).");
		
    } else if (EMRLD::darwinMajor >= KernelVersion::Tiger) {
        DBGLOG(MODULE_INIT, "Detected OS X Tiger (10.4.x).");
		
    } else {
        // How the helly? you're on PPC or some shit?
        DBGLOG(MODULE_ERROR, "Detected an unsupported version of OS X / macOS.");
        panic(MODULE_LONG, "Detected an unsupported version of OS X / macOS.");
    }

	// todo
}

// We use emrldState to determine EMRLD behaviour
void EMRLD::deinit() {
    DBGLOG(MODULE_ERROR, "This kernel extension cannot be disabled this way!");
    SYSLOG(MODULE_ERROR, "This kernel extension cannot be disabled this way!");
}
void EMRLD::initingNVDA(){
    SYSLOG(MODULE_INIT,"Initialising NVDAResmen");
    NVDAResman::init();

}
const char *bootargOff[] {
    "-emrldoff"
};

const char *bootargDebug[] {
    "-emrlddbg"
};

const char *bootargBeta[] {
    "-emrldbeta"
};

PluginConfiguration ADDPR(config) {
    xStringify(PRODUCT_NAME),
    parseModuleVersion(xStringify(MODULE_VERSION)),
    LiluAPI::AllowNormal |
    LiluAPI::AllowSafeMode |
    LiluAPI::AllowInstallerRecovery,
    bootargOff,
    arrsize(bootargOff),
    bootargDebug,
    arrsize(bootargDebug),
    bootargBeta,
    arrsize(bootargBeta),
	KernelVersion::Tiger,
	KernelVersion::Tahoe,
    []() {
        
        // Start the main EMRLD routine
        ADDPR(debugEnabled) = true;
        emrldInstance.init();
        
    }
};
