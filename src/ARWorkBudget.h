#ifndef ARWORKBUDGET_H
#define ARWORKBUDGET_H

#include "types.h"

namespace melonDS
{
class ARWorkBudget
{
public:
    static constexpr u64 MaxOperations = 1ULL << 20;

    explicit constexpr ARWorkBudget(u64 operations = MaxOperations) : RemainingOperations(operations) {}

    constexpr bool Consume(u64 operations)
    {
        if (operations > RemainingOperations)
            return false;
        RemainingOperations -= operations;
        return true;
    }

    constexpr u64 Remaining() const { return RemainingOperations; }

    static constexpr u64 OperationsForBytes(u32 byteLength)
    {
        return (static_cast<u64>(byteLength) + 3) / 4;
    }

private:
    u64 RemainingOperations;
};

enum class ARBudgetResult
{
    Accepted,
    CheatExhausted,
    FrameExhausted,
};

inline constexpr ARBudgetResult ConsumeARWork(
    ARWorkBudget& cheatBudget, ARWorkBudget& frameBudget, u64 operations)
{
    if (operations > cheatBudget.Remaining())
        return ARBudgetResult::CheatExhausted;
    if (operations > frameBudget.Remaining())
        return ARBudgetResult::FrameExhausted;
    cheatBudget.Consume(operations);
    frameBudget.Consume(operations);
    return ARBudgetResult::Accepted;
}
}

#endif // ARWORKBUDGET_H
