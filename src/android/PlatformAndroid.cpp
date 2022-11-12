/*
    Copyright 2016-2019 StapleButter

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
#include <stdlib.h>
#include <semaphore.h>
#include <pthread.h>
#include "pcap/pcap.h"
#include "Platform.h"
#include "MelonDS.h"
#include "android_fopen.h"
#include "LAN_Socket.h"
#include "LAN_PCap.h"
#include "FileUtils.h"
#include "Config.h"
#include "ROMManager.h"

#include <unistd.h>
#include <arpa/inet.h>
//#include <egl/egl.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <string>

#define socket_t    int
#define sockaddr_t  struct sockaddr
#define closesocket close

#ifndef INVALID_SOCKET
#define INVALID_SOCKET  (socket_t)-1
#endif

extern char* EmuDirectory;

namespace Platform
{
    typedef struct
    {
        int val;
        pthread_mutex_t mutex;
        pthread_cond_t cond;
    } AndroidSemaphore;

    typedef struct
    {
        pthread_t ID;
        std::function<void()> Func;

    } AndroidThread;

    typedef pthread_mutex_t AndroidMutex;

    void* ThreadEntry(void* data)
    {
        AndroidThread* thread = (AndroidThread*)data;
        thread->Func();
        return NULL;
    }


    socket_t MPSocket;
    sockaddr_t MPSendAddr;
    u8 PacketBuffer[2048];

#define NIFI_VER 1


    const char* PCapLibNames[] =
            {
                    // TODO: Linux lib names
                    NULL
            };

    void* PCapLib = NULL;
    pcap_t* PCapAdapter = NULL;

    u8 PCapPacketBuffer[2048];
    int PCapPacketLen;
    int PCapRXNum;

    int InstanceID()
    {
        return 0;
    }

    std::string InstanceFileSuffix()
    {
        return "";
    }

    int GetConfigInt(ConfigEntry entry)
    {
        const int imgsizes[] = {0, 256, 512, 1024, 2048, 4096};

        switch (entry)
        {
#ifdef JIT_ENABLED
            case JIT_MaxBlockSize: return Config::JIT_MaxBlockSize;
#endif

            case DLDI_ImageSize: return imgsizes[Config::DLDISize];

            case DSiSD_ImageSize: return imgsizes[Config::DSiSDSize];

            case Firm_Language: return Config::FirmwareLanguage;
            case Firm_BirthdayMonth: return Config::FirmwareBirthdayMonth;
            case Firm_BirthdayDay: return Config::FirmwareBirthdayDay;
            case Firm_Color: return Config::FirmwareFavouriteColour;

            case AudioBitrate: return Config::AudioBitrate;
        }

        return 0;
    }

    bool GetConfigBool(ConfigEntry entry)
    {
        switch (entry)
        {
#ifdef JIT_ENABLED
            case JIT_Enable: return Config::JIT_Enable != 0;
            case JIT_LiteralOptimizations: return Config::JIT_LiteralOptimisations != 0;
            case JIT_BranchOptimizations: return Config::JIT_BranchOptimisations != 0;
            case JIT_FastMemory: return Config::JIT_FastMemory != 0;
#endif

            case ExternalBIOSEnable: return Config::ExternalBIOSEnable != 0;

            case DLDI_Enable: return Config::DLDIEnable != 0;
            case DLDI_ReadOnly: return Config::DLDIReadOnly != 0;
            case DLDI_FolderSync: return Config::DLDIFolderSync != 0;

            case DSiSD_Enable: return Config::DSiSDEnable != 0;
            case DSiSD_ReadOnly: return Config::DSiSDReadOnly != 0;
            case DSiSD_FolderSync: return Config::DSiSDFolderSync != 0;

            //case Firm_RandomizeMAC: return Config::RandomizeMAC != 0;
            case Firm_OverrideSettings: return Config::FirmwareOverrideSettings != 0;
        }

        return false;
    }

    std::string GetConfigString(ConfigEntry entry)
    {
        switch (entry)
        {
            case BIOS9Path: return Config::BIOS9Path;
            case BIOS7Path: return Config::BIOS7Path;
            case FirmwarePath: return Config::FirmwarePath;

            case DSi_BIOS9Path: return Config::DSiBIOS9Path;
            case DSi_BIOS7Path: return Config::DSiBIOS7Path;
            case DSi_FirmwarePath: return Config::DSiFirmwarePath;
            case DSi_NANDPath: return Config::DSiNANDPath;

            case DLDI_ImagePath: return Config::DLDISDPath;
            case DLDI_FolderPath: return Config::DLDIFolderPath;

            case DSiSD_ImagePath: return Config::DSiSDPath;
            case DSiSD_FolderPath: return Config::DSiSDFolderPath;

            case Firm_Username: return Config::FirmwareUsername;
            case Firm_Message: return Config::FirmwareMessage;
        }

        return "";
    }

    bool GetConfigArray(ConfigEntry entry, void* data)
    {
        switch (entry)
        {
            case Firm_MAC:
            {
                std::string& mac_in = Config::FirmwareMAC;
                u8* mac_out = (u8*)data;

                int o = 0;
                u8 tmp = 0;
                for (int i = 0; i < 18; i++)
                {
                    char c = mac_in[i];
                    if (c == '\0') break;

                    int n;
                    if      (c >= '0' && c <= '9') n = c - '0';
                    else if (c >= 'a' && c <= 'f') n = c - 'a' + 10;
                    else if (c >= 'A' && c <= 'F') n = c - 'A' + 10;
                    else continue;

                    if (!(o & 1))
                        tmp = n;
                    else
                        mac_out[o >> 1] = n | (tmp << 4);

                    o++;
                    if (o >= 12) return true;
                }
            }
                return false;
        }

        return false;
    }

    FILE* OpenFile(std::string path, std::string mode, bool mustexist)
    {
        if (path.empty())
        {
            return nullptr;
        }

        // If it's a standard absolute file path, open it a simple file. If not, delegate to the file handler
        if (path[0] == '/')
        {
            if (mustexist)
            {
                FILE* file = fopen(path.c_str(), mode.c_str());
                if (file)
                    return freopen(path.c_str(), mode.c_str(), file);
                else
                    return nullptr;
            }
            else
                return fopen(path.c_str(), mode.c_str());
        }
        else
        {
            return MelonDSAndroid::fileHandler->open(path.c_str(), mode.c_str());
        }
    }

    FILE* OpenLocalFile(std::string path, std::string mode)
    {
        if (path.empty())
            return nullptr;

        // Always open file as absolute
        return OpenFile(path, mode, mode[0] == 'r');
    }

    FILE* OpenDataFile(const char* path)
    {
        return android_fopen(MelonDSAndroid::assetManager, path, "rb");
    }

    Thread* Thread_Create(std::function<void()> func)
    {
        AndroidThread* data = new AndroidThread;
        data->Func = func;
        pthread_create(&data->ID, nullptr, ThreadEntry, data);
        return (Thread*) data;
    }

    void Thread_Free(Thread* thread)
    {
        delete (AndroidThread*)thread;
    }

    void Thread_Wait(Thread* thread)
    {
        pthread_t pthread = ((AndroidThread*) thread)->ID;
        pthread_join(pthread, NULL);
    }

    Semaphore* Semaphore_Create()
    {
        AndroidSemaphore* semaphore = (AndroidSemaphore*) malloc(sizeof(AndroidSemaphore));
        pthread_mutex_init(&semaphore->mutex, NULL);
        pthread_cond_init(&semaphore->cond, NULL);
        semaphore->val = 0;
        return (Semaphore*) semaphore;
    }

    void Semaphore_Free(Semaphore* sema)
    {
        AndroidSemaphore* semaphore = (AndroidSemaphore*) sema;
        pthread_mutex_destroy(&semaphore->mutex);
        pthread_cond_destroy(&semaphore->cond);
        free(semaphore);
    }

    void Semaphore_Reset(Semaphore* sema)
    {
        AndroidSemaphore* semaphore = (AndroidSemaphore*) sema;
        pthread_mutex_lock(&semaphore->mutex);
        semaphore->val = 0;
        pthread_cond_broadcast(&semaphore->cond);
        pthread_mutex_unlock(&semaphore->mutex);
    }

    void Semaphore_Wait(Semaphore* sema)
    {
        AndroidSemaphore* semaphore = (AndroidSemaphore*) sema;
        pthread_mutex_lock(&semaphore->mutex);
        while (semaphore->val == 0)
            pthread_cond_wait(&semaphore->cond, &semaphore->mutex);

        semaphore->val--;
        pthread_mutex_unlock(&semaphore->mutex);
    }

    void Semaphore_Post(Semaphore* sema, int count)
    {
        AndroidSemaphore* semaphore = (AndroidSemaphore*) sema;
        pthread_mutex_lock(&semaphore->mutex);
        semaphore->val += count;
        pthread_cond_broadcast(&semaphore->cond);
        pthread_mutex_unlock(&semaphore->mutex);
    }

    Mutex* Mutex_Create() {
        AndroidMutex* mutex = (AndroidMutex*) malloc(sizeof(AndroidMutex));
        pthread_mutex_init(mutex, nullptr);
        return (Mutex*) mutex;
    }

    void Mutex_Free(Mutex* mutex) {
        pthread_mutex_destroy((AndroidMutex*) mutex);
        free((AndroidMutex*) mutex);
    }

    void Mutex_Lock(Mutex* mutex) {
        pthread_mutex_lock((AndroidMutex*) mutex);
    }

    void Mutex_Unlock(Mutex* mutex) {
        pthread_mutex_unlock((AndroidMutex*) mutex);
    }

    bool Mutex_TryLock(Mutex* mutex) {
        return pthread_mutex_trylock((AndroidMutex*) mutex) == 0;
    }

    void* GL_GetProcAddress(const char* proc)
    {
        return NULL;
        //return eglGetProcAddress(proc);
    }

    void WriteNDSSave(const u8* savedata, u32 savelen, u32 writeoffset, u32 writelen)
    {
        if (ROMManager::NDSSave)
            ROMManager::NDSSave->RequestFlush(savedata, savelen, writeoffset, writelen);
    }

    void WriteGBASave(const u8* savedata, u32 savelen, u32 writeoffset, u32 writelen)
    {
        if (ROMManager::GBASave)
            ROMManager::GBASave->RequestFlush(savedata, savelen, writeoffset, writelen);
    }

    bool MP_Init()
    {
        int opt_true = 1;
        int res;

        MPSocket = socket(AF_INET, SOCK_DGRAM, 0);
        if (MPSocket < 0)
        {
            return false;
        }

        res = setsockopt(MPSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt_true, sizeof(int));
        if (res < 0)
        {
            closesocket(MPSocket);
            MPSocket = INVALID_SOCKET;
            return false;
        }

        sockaddr_t saddr;
        saddr.sa_family = AF_INET;
        *(u32*)&saddr.sa_data[2] = htonl(Config::SocketBindAnyAddr ? INADDR_ANY : INADDR_LOOPBACK);
        *(u16*)&saddr.sa_data[0] = htons(7064);
        res = bind(MPSocket, &saddr, sizeof(sockaddr_t));
        if (res < 0)
        {
            closesocket(MPSocket);
            MPSocket = INVALID_SOCKET;
            return false;
        }

        res = setsockopt(MPSocket, SOL_SOCKET, SO_BROADCAST, (const char*)&opt_true, sizeof(int));
        if (res < 0)
        {
            closesocket(MPSocket);
            MPSocket = INVALID_SOCKET;
            return false;
        }

        MPSendAddr.sa_family = AF_INET;
        *(u32*)&MPSendAddr.sa_data[2] = htonl(INADDR_BROADCAST);
        *(u16*)&MPSendAddr.sa_data[0] = htons(7064);

        return true;
    }

    void MP_DeInit()
    {
        if (MPSocket >= 0)
            closesocket(MPSocket);
    }

    void MP_Begin()
    {
    }

    void MP_End()
    {
    }

    int MP_SendPacket(u8* data, int len, u64 timestamp)
    {
        return 0;
    }

    int MP_RecvPacket(u8* data, u64* timestamp)
    {
        return 0;
    }

    int MP_SendCmd(u8* data, int len, u64 timestamp)
    {
        return 0;
    }

    int MP_SendReply(u8* data, int len, u64 timestamp, u16 aid)
    {
        return 0;
    }

    int MP_SendAck(u8* data, int len, u64 timestamp)
    {
        return 0;
    }

    int MP_RecvHostPacket(u8* data, u64* timestamp)
    {
        return 0;
    }

    u16 MP_RecvReplies(u8* data, u64 timestamp, u16 aidmask)
    {
        return 0;
    }

    int MP_SendPacket(u8* data, int len)
    {
        if (MPSocket < 0)
            return 0;

        if (len > 2048-8)
        {
            printf("MP_SendPacket: error: packet too long (%d)\n", len);
            return 0;
        }

        *(u32*)&PacketBuffer[0] = htonl(0x4946494E); // NIFI
        PacketBuffer[4] = NIFI_VER;
        PacketBuffer[5] = 0;
        *(u16*)&PacketBuffer[6] = htons(len);
        memcpy(&PacketBuffer[8], data, len);

        int slen = sendto(MPSocket, (const char*)PacketBuffer, len+8, 0, &MPSendAddr, sizeof(sockaddr_t));
        if (slen < 8) return 0;
        return slen - 8;
    }

    int MP_RecvPacket(u8* data, bool block)
    {
        if (MPSocket < 0)
            return 0;

        fd_set fd;
        struct timeval tv;

        FD_ZERO(&fd);
        FD_SET(MPSocket, &fd);
        tv.tv_sec = 0;
        tv.tv_usec = block ? 5000 : 0;

        if (!select(MPSocket+1, &fd, 0, 0, &tv))
        {
            return 0;
        }

        sockaddr_t fromAddr;
        socklen_t fromLen = sizeof(sockaddr_t);
        int rlen = recvfrom(MPSocket, (char*)PacketBuffer, 2048, 0, &fromAddr, &fromLen);
        if (rlen < 8+24)
        {
            return 0;
        }
        rlen -= 8;

        if (ntohl(*(u32*)&PacketBuffer[0]) != 0x4946494E)
        {
            return 0;
        }

        if (PacketBuffer[4] != NIFI_VER)
        {
            return 0;
        }

        if (ntohs(*(u16*)&PacketBuffer[6]) != rlen)
        {
            return 0;
        }

        memcpy(data, &PacketBuffer[8], rlen);
        return rlen;
    }



    bool LAN_Init()
    {
        if (Config::DirectLAN)
        {
            if (!LAN_PCap::Init(true))
                return false;
        }
        else
        {
            if (!LAN_Socket::Init())
                return false;
        }

        return true;
    }

    void LAN_DeInit()
    {
        // checkme. blarg
        //if (Config::DirectLAN)
        //    LAN_PCap::DeInit();
        //else
        //    LAN_Socket::DeInit();
        LAN_PCap::DeInit();
        LAN_Socket::DeInit();
    }

    int LAN_SendPacket(u8* data, int len)
    {
        if (Config::DirectLAN)
            return LAN_PCap::SendPacket(data, len);
        else
            return LAN_Socket::SendPacket(data, len);
    }

    int LAN_RecvPacket(u8* data)
    {
        if (Config::DirectLAN)
            return LAN_PCap::RecvPacket(data);
        else
            return LAN_Socket::RecvPacket(data);
    }

    void Sleep(u64 usecs)
    {
        usleep(usecs);
    }

    void StopEmu()
    {
    }

    void Camera_Start(int num)
    {
    }

    void Camera_Stop(int num)
    {
    }

    void Camera_CaptureFrame(int num, u32* frame, int width, int height, bool yuv)
    {
    }
}
