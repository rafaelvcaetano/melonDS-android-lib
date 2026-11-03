#ifndef SAVESTATEINVARIANT_H
#define SAVESTATEINVARIANT_H

#include "types.h"

namespace melonDS
{
struct SavestateInvariant
{
    static constexpr bool CircularQueueValid(
        u32 capacity, u32 occupied, u32 readPosition, u32 writePosition)
    {
        if (capacity == 0 || occupied > capacity ||
            readPosition >= capacity || writePosition >= capacity)
            return false;
        return writePosition == (readPosition + occupied) % capacity;
    }

    static constexpr bool EventMaskValid(u32 mask, u32 eventCount)
    {
        if (eventCount >= 32) return true;
        if (eventCount == 0) return mask == 0;
        return (mask & ~((1U << eventCount) - 1)) == 0;
    }

    static constexpr bool EventFunctionValid(u32 functionId, u32 functionCount, bool registered)
    {
        return functionId < functionCount && registered;
    }

    static constexpr bool SPUChannelValid(
        u32 readPosition, u32 writePosition, u32 level, s32 adpcmIndex, s32 adpcmLoopIndex)
    {
        return readPosition < 32 && writePosition < 8 && level <= 32 &&
            adpcmIndex >= 0 && adpcmIndex <= 88 &&
            adpcmLoopIndex >= 0 && adpcmLoopIndex <= 88;
    }

    static constexpr bool SPUCaptureValid(u32 readPosition, u32 writePosition, u32 level)
    {
        return readPosition < 4 && writePosition < 16 && level <= 16;
    }

    static constexpr bool GPUCountsValid(
        u32 bank, u32 vertices, u32 polygons, u32 opaquePolygons, u32 renderPolygons)
    {
        return bank < 2 && vertices <= 6144 && polygons <= 2048 &&
            opaquePolygons <= polygons && renderPolygons <= 2048;
    }

    static constexpr bool GPUTransientValid(
        u32 commandCount, u32 execParameterCount, s32 projectionStack,
        s32 positionStack, s32 textureStack, u32 vertexNumberInPolygon)
    {
        return commandCount <= 4 && execParameterCount < 32 &&
            projectionStack >= 0 && projectionStack <= 1 &&
            positionStack >= 0 && positionStack <= 63 &&
            textureStack >= 0 && textureStack <= 1 &&
            vertexNumberInPolygon < 4;
    }

    static constexpr bool IndexOrNullValid(u32 index, u32 capacity)
    {
        return index == UINT32_MAX || index < capacity;
    }

    static constexpr bool PolygonValid(u32 vertices, u32 top, u32 bottom)
    {
        return vertices <= 10 && (vertices == 0 || (top < vertices && bottom < vertices));
    }

    static constexpr bool WifiRuntimeValid(
        u32 communicationStatus, u32 txSlot, u32 rxBufferPosition, u32 rxBufferSize)
    {
        if (communicationStatus & ~3U) return false;
        if ((communicationStatus & 2U) && txSlot >= 6) return false;
        if ((communicationStatus & 1U) &&
            (rxBufferPosition > rxBufferSize || rxBufferSize - rxBufferPosition < 2))
            return false;
        return true;
    }

    static constexpr bool CountdownCanTick(s32 value, s32 interval)
    {
        return interval > 0 && value >= 0;
    }

    static constexpr bool WifiClientMaskValid(u16 clientMask)
    {
        return (clientMask & 0x0001U) == 0;
    }

    static constexpr bool AACCommandValid(u32 state, u32 parameterCount)
    {
        return (state == 0 && parameterCount == 0) ||
            (state == 1 && parameterCount < 10) ||
            (state == 2 && parameterCount == 10);
    }

    static constexpr bool RenderablePolygonValid(u32 vertices)
    {
        return vertices >= 2 && vertices <= 10;
    }

    static constexpr bool BPTWLCursorValid(u32 cursor)
    {
        return cursor == UINT32_MAX || cursor < 0x100;
    }

    static constexpr bool SDHostStateValid(u32 fifo, u32 blockLength16, u32 blockLength32)
    {
        return fifo < 2 && blockLength16 <= 0x200 && blockLength32 <= 0x200;
    }

    static constexpr bool CameraStateValid(
        u32 buffer, u32 read0, u32 write0, u32 read1, u32 write1)
    {
        return buffer < 2 && read0 <= 512 && write0 <= 512 &&
            read1 <= 512 && write1 <= 512;
    }

    static constexpr bool CameraFrameDimensionsValid(u32 width, u32 height)
    {
        return width >= 2 && width <= 640 && height >= 2 && height <= 480 &&
            (width & 1U) == 0;
    }

    static constexpr bool GPU2DFIFOValid(u32 readPosition, u32 writePosition)
    {
        return readPosition < 16 && writePosition <= 14 && (writePosition & 1) == 0;
    }

    static constexpr bool TeakraDMAChannelValid(u32 channel)
    {
        return channel < 8;
    }

    static constexpr bool BtdmpQueueValid(u32 size)
    {
        return size <= 16;
    }

    static constexpr bool SPUMixIntervalValid(u32 interval)
    {
        return interval == 704 || interval == 1024;
    }

    static constexpr u32 SPUInterpolationPosition(u32 timer, u32 reload)
    {
        if (reload >= 0x10000 || timer <= reload) return 0;
        const u64 numerator = static_cast<u64>(timer - reload) * 0x100;
        const u32 denominator = 0x10000 - reload;
        const u64 position = numerator / denominator;
        return position > 0xFF ? 0xFF : static_cast<u32>(position);
    }

    static constexpr bool VRAMMappingValid(u32 mask)
    {
        return (mask & ~0x1FFU) == 0;
    }

    static constexpr bool AESModeValid(u32 mode)
    {
        return mode <= 3;
    }

    static constexpr bool GPUVertexStateValid(u32 mode, u32 vertexNumber)
    {
        return mode < 4 && vertexNumber < ((mode & 1U) ? 4U : 3U);
    }

    static constexpr bool RTCCursorsValid(
        u32 inputBit, u32 inputPosition, u32 outputBit, u32 outputPosition)
    {
        return inputBit < 8 && inputPosition < 8 && outputBit < 8 && outputPosition < 8;
    }

    static constexpr bool SolarLevelValid(u32 level)
    {
        return level <= 10;
    }
};
}

#endif // SAVESTATEINVARIANT_H
