#pragma once

#include "config.h"
#ifdef USE_IP

#include <stdint.h>
#include <atomic>
#include "data_link_layer.h"
#include "ip_parameter_object.h"
#include "knx_ip_tunnel_connection.h"
#include "service_families.h"

class IpDataLinkLayer : public DataLinkLayer
{
    using DataLinkLayer::_deviceObject;

  public:
    IpDataLinkLayer(DeviceObject& devObj, IpParameterObject& ipParam, NetworkLayerEntity& netLayerEntity,
                    Platform& platform, BusAccessUnit& busAccessUnit, DataLinkLayerCallbacks* dllcb = nullptr);

    void loop();
    void enabled(bool value);
    bool enabled() const;
    // Re-announce the joined group without leaving it (see Platform::refreshMultiCast).
    void refreshMultiCast();
    void knxBusConnected(bool connected);  // Set KNX bus connection status
    bool routingActive();                  // KNXnet/IP routing only once an individual address is set
    DptMedium mediumType() const override;
#ifdef KNX_TUNNELING
    void dataRequestToTunnel(CemiFrame& frame) override;
    void dataRequestToChannelId(CemiFrame& frame, uint8_t channelId) override;
    void dataConfirmationToTunnel(CemiFrame& frame) override;
    void dataIndicationToTunnel(CemiFrame& frame) override;
    bool isTunnelAddress(uint16_t addr) override;
    bool isConfigChannel(uint8_t channelId) override;
    bool isSentToTunnel(uint16_t address, bool isGrpAddr);
    // True for a tunnel address the device acknowledges on TP whether a tunnel
    // holds it or not (see refreshDefendedTunnelAddresses).
    bool isDefendedTunnelAddress(uint16_t address) const;
    uint8_t getActiveTunnelCount() const;
#endif

  private:
    bool _enabled = false;
    bool _knxBusConnected = false;  // Track KNX bus connection status
    uint8_t _frameCount[10] = {0,0,0,0,0,0,0,0,0,0};
    uint8_t _frameCountBase = 0;
    uint32_t _frameCountTimeBase = 0;
    bool sendFrame(CemiFrame& frame);
#ifdef KNX_TUNNELING
    void sendFrameToTunnel(KnxIpTunnelConnection *tunnel, CemiFrame& frame);
    void loopHandleConnectRequest(uint8_t* buffer, uint16_t length, uint32_t& src_addr, uint16_t& src_port);
    bool fromTunnelPeer(const KnxIpTunnelConnection* tun, uint32_t src_addr);
    bool tunnelAddressInUse(uint16_t pa);
    void loopHandleConnectionStateRequest(uint8_t* buffer, uint16_t length);
    void loopHandleDisconnectRequest(uint8_t* buffer, uint16_t length, uint32_t src_addr);
    void loopHandleDescriptionRequest(uint8_t* buffer, uint16_t length);
    void loopHandleDeviceConfigurationRequest(uint8_t* buffer, uint16_t length, uint32_t src_addr);
    void loopHandleTunnelingRequest(uint8_t* buffer, uint16_t length, uint32_t src_addr);
#endif
#if KNX_SERVICE_FAMILY_CORE >= 2
    void loopHandleSearchRequestExtended(uint8_t* buffer, uint16_t length);
#endif
    bool sendBytes(uint8_t* buffer, uint16_t length);
    bool isSendLimitReached();

    IpParameterObject& _ipParameters;
    DataLinkLayerCallbacks* _dllcb;
#ifdef KNX_TUNNELING
    KnxIpTunnelConnection tunnels[KNX_TUNNELING];
    uint8_t _lastChannelId = 0;

    void refreshDefendedTunnelAddresses();
    // Two copies, because the TP acknowledge path reads them in the UART task: the
    // refresh fills the one not in use and then switches over.
    uint16_t _defendedAddresses[2][KNX_TUNNELING] = {};
    uint8_t _defendedCount[2] = {0, 0};
    std::atomic<uint8_t> _defendedSet{0};
    uint32_t _defendedRefreshMs = 0;
    bool _defendedRefreshed = false;
#endif
};
#endif