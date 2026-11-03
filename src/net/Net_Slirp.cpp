/*
    Copyright 2016-2025 melonDS team

    This file is part of melonDS.

    melonDS is free software: you can redistribute it and/or modify it under
    the terms of the GNU General Public License as published by the Free
    Software Foundation, either version 3 of the License, or (at your option)
    any later version.

    melonDS is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with melonDS. If not, see http://www.gnu.org/licenses/.
*/

#include <stdio.h>
#include <string.h>
#include "Net.h"
#include "Net_Slirp.h"
#include "FIFO.h"
#include "Platform.h"

#include <libslirp.h>

#include <algorithm>
#include <cctype>
#include <mutex>
#include <memory>

#ifdef __WIN32__
	#include <ws2tcpip.h>
#else
	#include <sys/socket.h>
	#include <netdb.h>
	#include <poll.h>
	#include <time.h>
#endif

namespace melonDS
{

using Platform::Log;
using Platform::LogLevel;

const u32 kSubnet   = 0x0A400000;
const u32 kServerIP = kSubnet | 0x01;
const u32 kDNSIP    = kSubnet | 0x02;
const u32 kClientIP = kSubnet | 0x10;

const u8 kServerMAC[6] = {0x00, 0xAB, 0x33, 0x28, 0x99, 0x44};

#ifdef __WIN32__

#define poll WSAPoll
#ifndef CLOCK_MONOTONIC
#define CLOCK_MONOTONIC 1
#endif

// https://stackoverflow.com/questions/5404277/porting-clock-gettime-to-windows

struct timespec { long tv_sec; long tv_nsec; };

int clock_gettime(int, struct timespec *spec)
{
    __int64 wintime;
    GetSystemTimeAsFileTime((FILETIME*)&wintime);
    wintime -=116444736000000000LL;                 //1jan1601 to 1jan1970
    spec->tv_sec  = wintime / 10000000LL;           //seconds
    spec->tv_nsec = wintime % 10000000LL * 100;     //nano-seconds
    return 0;
}

#endif // __WIN32__


ssize_t Net_Slirp::SlirpCbSendPacket(const void* buf, size_t len, void* opaque) noexcept
{
    if (len > 2048)
    {
        Log(LogLevel::Warn, "slirp: packet too big (%zu)\n", len);
        return 0;
    }

    Log(LogLevel::Debug, "slirp: response packet of %zu bytes, type %04X\n", len, ntohs(((u16*)buf)[6]));

    Net_Slirp& self = *static_cast<Net_Slirp*>(opaque);
    if (self.Callback)
    {
        self.Callback((const u8*)buf, len);
    }

    return len;
}

void SlirpCbGuestError(const char* msg, void* opaque)
{
    Log(LogLevel::Error, "SLIRP: error: %s\n", msg);
}

int64_t SlirpCbClockGetNS(void* opaque)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000000000LL + ts.tv_nsec;
}

void* SlirpCbTimerNew(SlirpTimerCb cb, void* cb_opaque, void* opaque)
{
    return nullptr;
}

void SlirpCbTimerFree(void* timer, void* opaque)
{
}

void SlirpCbTimerMod(void* timer, int64_t expire_time, void* opaque)
{
}

void SlirpCbRegisterPollFD(int fd, void* opaque)
{
    Log(LogLevel::Debug, "Slirp: register poll FD %d\n", fd);
}

void SlirpCbUnregisterPollFD(int fd, void* opaque)
{
    Log(LogLevel::Debug, "Slirp: unregister poll FD %d\n", fd);
}

void SlirpCbNotify(void* opaque)
{
    Log(LogLevel::Debug, "Slirp: notify???\n");
}

const SlirpCb Net_Slirp::cb =
{
    .send_packet = SlirpCbSendPacket,
    .guest_error = SlirpCbGuestError,
    .clock_get_ns = SlirpCbClockGetNS,
    .timer_new = SlirpCbTimerNew,
    .timer_free = SlirpCbTimerFree,
    .timer_mod = SlirpCbTimerMod,
    .register_poll_fd = SlirpCbRegisterPollFD,
    .unregister_poll_fd = SlirpCbUnregisterPollFD,
    .notify = SlirpCbNotify
};

Net_Slirp::Net_Slirp(const Platform::SendPacketCallback& callback) noexcept : Callback(callback)
{
    SlirpConfig cfg {};
    memset(&cfg, 0, sizeof(cfg));
    cfg.version = 1;

    cfg.in_enabled = true;
    cfg.vnetwork.s_addr = htonl(kSubnet);
    cfg.vnetmask.s_addr = htonl(0xFFFFFF00);
    cfg.vhost.s_addr = htonl(kServerIP);
    cfg.vhostname = "melonServer";
    cfg.vdhcp_start.s_addr = htonl(kClientIP);
    cfg.vnameserver.s_addr = htonl(kDNSIP);
    cfg.disable_host_loopback = true;

    Ctx = slirp_new(&cfg, &cb, this);
}

Net_Slirp::~Net_Slirp() noexcept
{
    if (Ctx)
    {
        slirp_cleanup(Ctx);
        Ctx = nullptr;
    }
}

std::mutex Net_Slirp::HostOverridesMutex {};
std::vector<std::pair<std::string, u32>> Net_Slirp::HostOverrides {};

// Normalizes a domain name for comparison: lowercased, with a single trailing
// dot (if any) stripped, so "NAS.Kaeru.SGP." and "nas.kaeru.sgp" match.
static std::string NormalizeHostName(const std::string& name)
{
    std::string out = name;
    if (!out.empty() && out.back() == '.')
        out.pop_back();
    std::transform(out.begin(), out.end(), out.begin(),
        [](unsigned char c) { return std::tolower(c); });
    return out;
}

void Net_Slirp::SetHostOverrides(std::vector<std::pair<std::string, u32>> overrides)
{
    for (auto& entry : overrides)
        entry.first = NormalizeHostName(entry.first);

    std::lock_guard<std::mutex> lock(HostOverridesMutex);
    HostOverrides = std::move(overrides);
}

bool Net_Slirp::LookupHostOverride(const std::string& name, u32& addr)
{
    std::string key = NormalizeHostName(name);

    std::lock_guard<std::mutex> lock(HostOverridesMutex);
    for (const auto& entry : HostOverrides)
    {
        if (entry.first == key)
        {
            addr = entry.second;
            return true;
        }
    }

    return false;
}

static u16 ReadBE16(const u8* source)
{
    u16 value;
    memcpy(&value, source, sizeof(value));
    return ntohs(value);
}

static u32 ReadBE32(const u8* source)
{
    u32 value;
    memcpy(&value, source, sizeof(value));
    return ntohl(value);
}

static void StoreBE16(u8* destination, u16 value)
{
    value = htons(value);
    memcpy(destination, &value, sizeof(value));
}

static void StoreBE32(u8* destination, u32 value)
{
    value = htonl(value);
    memcpy(destination, &value, sizeof(value));
}

static void StoreNetwork32(u8* destination, u32 value)
{
    memcpy(destination, &value, sizeof(value));
}

static void AppendBE16(u8*& destination, u16 value)
{
    StoreBE16(destination, value);
    destination += sizeof(value);
}

static void AppendBE32(u8*& destination, u32 value)
{
    StoreBE32(destination, value);
    destination += sizeof(value);
}

static void AppendNetwork32(u8*& destination, u32 value)
{
    StoreNetwork32(destination, value);
    destination += sizeof(value);
}

void FinishUDPFrame(u8* data, int len)
{
    u8* ipheader = &data[0xE];
    u8* udpheader = &data[0x22];

    // lengths
    StoreBE16(&ipheader[2], len - 0xE);
    StoreBE16(&udpheader[4], len - (0xE + 0x14));

    // IP checksum
    u32 tmp = 0;

    for (int i = 0; i < 20; i += 2)
        tmp += ReadBE16(&ipheader[i]);
    while (tmp >> 16)
        tmp = (tmp & 0xFFFF) + (tmp >> 16);
    tmp ^= 0xFFFF;
    StoreBE16(&ipheader[10], tmp);

    // UDP checksum
    // (note: normally not mandatory, but some older sgIP versions require it)
    tmp = 0;
    tmp += ReadBE16(&ipheader[12]);
    tmp += ReadBE16(&ipheader[14]);
    tmp += ReadBE16(&ipheader[16]);
    tmp += ReadBE16(&ipheader[18]);
    tmp += 0x11;
    tmp += (len-0x22);
    for (u8* i = udpheader; i < &udpheader[len-0x23]; i += 2)
        tmp += ReadBE16(i);
    if (len & 1)
        tmp += static_cast<u16>(udpheader[len-0x23]) << 8;
    while (tmp >> 16)
        tmp = (tmp & 0xFFFF) + (tmp >> 16);
    tmp ^= 0xFFFF;
    if (tmp == 0) tmp = 0xFFFF;
    StoreBE16(&udpheader[6], tmp);
}

void Net_Slirp::HandleDNSFrame(u8* data, int len) noexcept
{
    if (!data || len < 14 + 20 + 8 + 12) return;

    u8* ipheader = &data[14];
    const u32 ipHeaderLength = (ipheader[0] & 0x0F) * 4;
    if ((ipheader[0] >> 4) != 4 || ipHeaderLength < 20 ||
        ipHeaderLength > static_cast<u32>(len - 14)) return;

    u16 ipLength;
    memcpy(&ipLength, &ipheader[2], sizeof(ipLength));
    ipLength = ntohs(ipLength);
    if (ipLength < ipHeaderLength + 8 || ipLength > static_cast<u32>(len - 14)) return;

    u8* udpheader = ipheader + ipHeaderLength;
    u16 udpLength;
    memcpy(&udpLength, &udpheader[4], sizeof(udpLength));
    udpLength = ntohs(udpLength);
    if (udpLength < 8 + 12 || udpLength > ipLength - ipHeaderLength) return;

    u8* dnsbody = udpheader + 8;
    const u32 dnsLength = udpLength - 8;

    u32 srcip = ReadBE32(&ipheader[12]);
    u16 srcport = ReadBE16(&udpheader[0]);

    u16 id = ReadBE16(&dnsbody[0]);
    u16 flags = ReadBE16(&dnsbody[2]);
    u16 numquestions = ReadBE16(&dnsbody[4]);
    u16 numanswers = ReadBE16(&dnsbody[6]);
    u16 numauth = ReadBE16(&dnsbody[8]);
    u16 numadd = ReadBE16(&dnsbody[10]);

    Log(LogLevel::Debug, "DNS: ID=%04X, flags=%04X, Q=%d, A=%d, auth=%d, add=%d\n",
           id, flags, numquestions, numanswers, numauth, numadd);

    // for now we only take 'simple' DNS requests
    if (flags & 0x8000) return;
    if (numquestions != 1 || numanswers != 0) return;

    u8 resp[1024];
    u8* out = &resp[0];

    // ethernet
    memcpy(out, &data[6], 6); out += 6;
    memcpy(out, kServerMAC, 6); out += 6;
    AppendBE16(out, 0x0800);

    // IP
    u8* resp_ipheader = out;
    *out++ = 0x45;
    *out++ = 0x00;
    AppendBE16(out, 0); // total length
    AppendBE16(out, IPv4ID); IPv4ID++;
    *out++ = 0x00;
    *out++ = 0x00;
    *out++ = 0x80; // TTL
    *out++ = 0x11; // protocol (UDP)
    AppendBE16(out, 0); // checksum
    AppendBE32(out, kDNSIP); // source IP
    AppendBE32(out, srcip); // destination IP

    // UDP
    u8* resp_udpheader = out;
    AppendBE16(out, 53); // source port
    AppendBE16(out, srcport); // destination port
    AppendBE16(out, 0); // length
    AppendBE16(out, 0); // checksum

    // DNS
    u8* resp_body = out;
    AppendBE16(out, id); // ID
    AppendBE16(out, 0x8000); // flags
    AppendBE16(out, numquestions); // num questions
    AppendBE16(out, numquestions); // num answers
    AppendBE16(out, 0); // num authority
    AppendBE16(out, 0); // num additional

    u32 curoffset = 12;
    char domainname[256] = {};
    u32 domainOffset = 0;
    while (true)
    {
        if (curoffset >= dnsLength) return;
        const u8 labelLength = dnsbody[curoffset++];
        if (labelLength == 0) break;
        const u32 separatorLength = domainOffset ? 1U : 0U;
        if ((labelLength & 0xC0) != 0 || labelLength > 63 ||
            labelLength > dnsLength - curoffset || domainOffset > 255 ||
            separatorLength + labelLength > 255 - domainOffset) return;

        if (domainOffset) domainname[domainOffset++] = '.';
        memcpy(&domainname[domainOffset], &dnsbody[curoffset], labelLength);
        domainOffset += labelLength;
        curoffset += labelLength;
    }
    domainname[domainOffset] = '\0';
    if (dnsLength - curoffset < 4) return;

    u16 type;
    u16 cls;
    memcpy(&type, &dnsbody[curoffset], sizeof(type));
    memcpy(&cls, &dnsbody[curoffset + 2], sizeof(cls));
    type = ntohs(type);
    cls = ntohs(cls);
    curoffset += 4;

    u32 qlen = curoffset - 12;
    if (qlen > 512) return;
    memcpy(out, &dnsbody[12], qlen); out += qlen;

    u16 actualAnswers = numquestions;
	printf("- q0: %04X %04X %s", type, cls, domainname);

		// get answer
		struct addrinfo dns_hint;
		struct addrinfo* dns_res = nullptr;
		u32 addr_res = 0;
		bool haveAnswer = (type == 1 && cls == 1);

		u32 overrideAddr;
		if (haveAnswer && LookupHostOverride(domainname, overrideAddr))
		{
		    if (type == 1) // A
		    {
		        addr_res = htonl(overrideAddr);
		        Log(LogLevel::Debug, "DNS override %s -> %d.%d.%d.%d\n", domainname,
		            (overrideAddr >> 24) & 0xFF, (overrideAddr >> 16) & 0xFF,
		            (overrideAddr >> 8) & 0xFF, overrideAddr & 0xFF);
		    }
		    else
		    {
		        // Overridden name but not an A query (eg. AAAA): answer with no
		        // records for this question rather than a bogus 4-byte address.
		        haveAnswer = false;
		    }
		}
		else if (haveAnswer)
		{
		    memset(&dns_hint, 0, sizeof(dns_hint));
		    dns_hint.ai_family = AF_INET; // TODO: other address types (INET6, etc)
		    if (getaddrinfo(domainname, "0", &dns_hint, &dns_res) == 0)
	        {
	            std::unique_ptr<addrinfo, decltype(&freeaddrinfo)> dnsOwner(dns_res, freeaddrinfo);
	            struct addrinfo* p = dns_res;
	            if (p && p->ai_addr && p->ai_addrlen >= sizeof(sockaddr_in))
	            {
                struct sockaddr_in* addr = (struct sockaddr_in*)p->ai_addr;
                memcpy(&addr_res, &addr->sin_addr, sizeof(addr_res));

	                printf(" -> %d.%d.%d.%d",
	                       addr_res & 0xFF, (addr_res >> 8) & 0xFF,
	                       (addr_res >> 16) & 0xFF, addr_res >> 24);
	            }
	            else haveAnswer = false;
	        }
	        else
	        {
	            printf(" shat itself :(");
	            haveAnswer = false;
	        }
		}

		printf("\n");

		if (!haveAnswer)
		{
		    actualAnswers--;
		}
		else
		{

		    // TODO: betterer support
		    // (under which conditions does the C00C marker work?)
            AppendBE16(out, 0xC00C);
            AppendBE16(out, type);
            AppendBE16(out, cls);
            AppendBE32(out, 3600); // TTL (hardcoded for now)
            AppendBE16(out, 4); // address length
            AppendNetwork32(out, addr_res); // address
		}

    if (actualAnswers != numquestions)
        StoreBE16(&resp_body[6], actualAnswers); // patch ANCOUNT for skipped (overridden non-A) answers

    u32 framelen = (u32)(out - &resp[0]);
    if (framelen & 1) { *out++ = 0; framelen++; }
    FinishUDPFrame(resp, framelen);

    if (Callback)
        Callback(resp, framelen);
}

int Net_Slirp::SendPacket(u8* data, int len) noexcept
{
    if (!Ctx || !data || len < 14) return 0;

    if (len > 2048)
    {
        Log(LogLevel::Error, "Net_SendPacket: error: packet too long (%d)\n", len);
        return 0;
    }

    u16 ethertype;
    memcpy(&ethertype, &data[0xC], sizeof(ethertype));
    ethertype = ntohs(ethertype);

    if (ethertype == 0x800)
    {
        if (len < 14 + 20) return 0;
        const u8* ipheader = &data[14];
        const u32 ipHeaderLength = (ipheader[0] & 0x0F) * 4;
        if ((ipheader[0] >> 4) != 4 || ipHeaderLength < 20 ||
            ipHeaderLength > static_cast<u32>(len - 14)) return 0;

        u16 ipLength;
        memcpy(&ipLength, &ipheader[2], sizeof(ipLength));
        ipLength = ntohs(ipLength);
        if (ipLength < ipHeaderLength || ipLength > static_cast<u32>(len - 14)) return 0;

        u16 fragmentField;
        memcpy(&fragmentField, &ipheader[6], sizeof(fragmentField));
        fragmentField = ntohs(fragmentField);
        const bool fragmented = (fragmentField & 0x3FFF) != 0;

        u8 protocol = ipheader[9];
        if (protocol == 0x11 && !fragmented) // complete UDP datagram
        {
            if (ipLength < ipHeaderLength + 8) return 0;
            const u8* udpheader = ipheader + ipHeaderLength;
            u16 udpLength;
            u16 dstport;
            memcpy(&udpLength, &udpheader[4], sizeof(udpLength));
            memcpy(&dstport, &udpheader[2], sizeof(dstport));
            udpLength = ntohs(udpLength);
            dstport = ntohs(dstport);
            if (udpLength < 8 || udpLength > ipLength - ipHeaderLength) return 0;

            u32 destination;
            memcpy(&destination, &ipheader[16], sizeof(destination));
            destination = ntohl(destination);
            if (dstport == 53 && destination == kDNSIP) // DNS
            {
                HandleDNSFrame(data, len);
                return len;
            }
        }

        u32 destination;
        memcpy(&destination, &ipheader[16], sizeof(destination));
        destination = ntohl(destination);
        const bool privateDestination =
            (destination >> 24) == 0 ||
            (destination >> 24) == 10 ||
            (destination >= 0x64400000 && destination <= 0x647FFFFF) ||
            (destination >> 24) == 127 ||
            (destination >= 0xA9FE0000 && destination <= 0xA9FEFFFF) ||
            (destination >= 0xAC100000 && destination <= 0xAC1FFFFF) ||
            (destination >= 0xC0A80000 && destination <= 0xC0A8FFFF) ||
            (destination >= 0xC6120000 && destination <= 0xC613FFFF) ||
            destination >= 0xE0000000;
        const bool dhcpBroadcast = protocol == 0x11 && destination == 0xFFFFFFFF;
        if (privateDestination && !dhcpBroadcast) return len;
    }
    else if (ethertype == 0x86DD)
    {
        if (len < 14 + 40) return 0;
        const u8* destination = &data[14 + 24];
        const bool unspecified = std::all_of(destination, destination + 16, [](u8 b) { return b == 0; });
        const bool loopback = std::all_of(destination, destination + 15, [](u8 b) { return b == 0; }) && destination[15] == 1;
        const bool uniqueLocal = (destination[0] & 0xFE) == 0xFC;
        const bool linkLocal = destination[0] == 0xFE && (destination[1] & 0xC0) == 0x80;
        if (unspecified || loopback || uniqueLocal || linkLocal) return len;
    }

    slirp_input(Ctx, data, len);
    return len;
}

int Net_Slirp::SlirpCbAddPoll(int fd, int events, void* opaque) noexcept
{
    Net_Slirp& self = *static_cast<Net_Slirp*>(opaque);

    if (self.PollListSize >= PollListMax)
    {
        Log(LogLevel::Error, "slirp: POLL LIST FULL\n");
        return -1;
    }

    int idx = self.PollListSize++;

    u16 evt = 0;

    if (events & SLIRP_POLL_IN) evt |= POLLIN;
    if (events & SLIRP_POLL_OUT) evt |= POLLWRNORM;

#ifndef __WIN32__
    // CHECKME
    if (events & SLIRP_POLL_PRI) evt |= POLLPRI;
    if (events & SLIRP_POLL_ERR) evt |= POLLERR;
    if (events & SLIRP_POLL_HUP) evt |= POLLHUP;
#endif // !__WIN32__

    self.PollList[idx].fd = fd;
    self.PollList[idx].events = evt;

    return idx;
}

int Net_Slirp::SlirpCbGetREvents(int idx, void* opaque) noexcept
{
    Net_Slirp& self = *static_cast<Net_Slirp*>(opaque);

    if (idx < 0 || idx >= self.PollListSize)
        return 0;

    u16 evt = self.PollList[idx].revents;
    int ret = 0;

    if (evt & POLLIN) ret |= SLIRP_POLL_IN;
    if (evt & POLLWRNORM) ret |= SLIRP_POLL_OUT;
    if (evt & POLLPRI) ret |= SLIRP_POLL_PRI;
    if (evt & POLLERR) ret |= SLIRP_POLL_ERR;
    if (evt & POLLHUP) ret |= SLIRP_POLL_HUP;

    return ret;
}

void Net_Slirp::RecvCheck() noexcept
{
    if (!Ctx) return;

    //if (PollListSize > 0)
    {
        u32 timeout = 0;
        PollListSize = 0;
        slirp_pollfds_fill(Ctx, &timeout, SlirpCbAddPoll, this);
        int res = poll(PollList, PollListSize, timeout);
        slirp_pollfds_poll(Ctx, res<0, SlirpCbGetREvents, this);
    }
}

}
