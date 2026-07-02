#include "RepositoryManager.h"

RepositoryManager::RepositoryManager(IDatabase* database)
    : db(database),
      pagerRepo(database),
      deviceRepo(database),
      routeRepo(database),
      ethDeviceRepo(database),
      ttyDeviceRepo(database),
      settingsRepo(database),
      wirelessNatRepo(database)
{
}

PagerRepository& RepositoryManager::Pagers()
{
    return pagerRepo;
}

DeviceRepository& RepositoryManager::Devices()
{
    return deviceRepo;
}

RouteRepository& RepositoryManager::Routes()
{
    return routeRepo;
}

EthDeviceRepository& RepositoryManager::EthDevices()
{
    return ethDeviceRepo;
}

TtyDeviceRepository& RepositoryManager::TtyDevices()
{
    return ttyDeviceRepo;
}

SettingsRepository& RepositoryManager::Settings()
{
    return settingsRepo;
}

WirelessNatRepository& RepositoryManager::WirelessNat()
{
    return wirelessNatRepo;
}
