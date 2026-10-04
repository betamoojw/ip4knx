#include "knx_ip_supported_service_dib.h"
#include "service_families.h"

#ifdef USE_IP
KnxIpSupportedServiceDIB::KnxIpSupportedServiceDIB(uint8_t* data) : KnxIpDIB(data)
{}


uint8_t KnxIpSupportedServiceDIB::serviceVersion(ServiceFamily family)
{
    uint8_t* start = _data + 2;
    uint8_t* end = _data + length();

    for (uint8_t* it = start; it < end; it += 2)
    {
        if (*it == family)
            return it[1];
    }
    return 0;
}


// Routing is announced only while it runs (IpDataLinkLayer::routingActive());
// only the 091A has the family at all.
uint8_t KnxIpSupportedServiceDIB::lengthFor(bool routing)
{
#if MASK_VERSION == 0x091A
    if (!routing)
        return LEN_SERVICE_DIB - LEN_SERVICE_FAMILIES;
#else
    (void)routing;
#endif
    return LEN_SERVICE_DIB;
}

void KnxIpSupportedServiceDIB::setServiceFamilies(bool routing)
{
    length(lengthFor(routing));
    code(SUPP_SVC_FAMILIES);
    serviceVersion(Core, KNX_SERVICE_FAMILY_CORE);
    serviceVersion(DeviceManagement, KNX_SERVICE_FAMILY_DEVICE_MANAGEMENT);
#ifdef KNX_TUNNELING
    serviceVersion(Tunnelling, KNX_SERVICE_FAMILY_TUNNELING);
#endif
#if MASK_VERSION == 0x091A
    if (routing)
        serviceVersion(Routing, KNX_SERVICE_FAMILY_ROUTING);
#endif
}

// 03_08_03 2.5.19 Table 2 p.13: bit 0 device management, bit 1 tunnelling, bit 2 routing.
// Bits 3 to 6 (remote logging, remote configuration, object server, security) stay clear:
// none of them is served. Mirrors setServiceFamilies() switch for switch. (upstream 8a8f2d1,
// which keys routing on KNX_IS_ROUTER; this build announces routing only while it runs)
uint16_t KnxIpSupportedServiceDIB::deviceCapabilities(bool routing)
{
    uint16_t caps = 1 << 0;     // Device Management
#ifdef KNX_TUNNELING
    caps |= 1 << 1;             // Tunnelling
#endif
#if MASK_VERSION == 0x091A
    if (routing)
        caps |= 1 << 2;         // Routing
#else
    (void)routing;
#endif
    return caps;
}

void KnxIpSupportedServiceDIB::serviceVersion(ServiceFamily family,  uint8_t version)
{
    uint8_t* start = _data + 2;
    uint8_t* end = _data + length();

    for (uint8_t* it = start; it < end; it += 2)
    {
        if (*it == family)
        {
            it[1] = version;
            break;
        }

        if (*it == 0)
        {
            *it = family;
            it[1] = version;
            break;
        }
    }
}
#endif