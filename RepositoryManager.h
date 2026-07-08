#ifndef REPOSITORYMANAGER_H
#define REPOSITORYMANAGER_H

#include "IDatabase.h"

#include "repositories/PagerRepository.h"
#include "repositories/DeviceRepository.h"
#include "repositories/RouteRepository.h"
#include "repositories/EthDeviceRepository.h"
#include "repositories/TtyDeviceRepository.h"
#include "repositories/SettingsRepository.h"
#include "repositories/WirelessNatRepository.h"

class RepositoryManager
{
public:
    RepositoryManager(IDatabase* database);

    PagerRepository& Pagers();
    DeviceRepository& Devices();
    RouteRepository& Routes();
    EthDeviceRepository& EthDevices();
    TtyDeviceRepository& TtyDevices();
    SettingsRepository& Settings();
    WirelessNatRepository& WirelessNat();

private:
    IDatabase* db;

    PagerRepository pagerRepo;
    DeviceRepository deviceRepo;
    RouteRepository routeRepo;
    EthDeviceRepository ethDeviceRepo;
    TtyDeviceRepository ttyDeviceRepo;
    SettingsRepository settingsRepo;
    WirelessNatRepository wirelessNatRepo;
};

#endif
