/*
 * VKShaderBindingLayout.cpp
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "VKShaderBindingLayout.h"
#include "../../SPIRV/SpirvModule.h"
#include "../../../Core/Assertion.h"
#include "../../../Core/CoreUtils.h"
#include "../../../Core/MacroUtils.h"
#include <LLGL/Misc/ForRange.h>
#include <algorithm>

#ifdef LLGL_ENABLE_SPIRV_REFLECT
#   include "../../SPIRV/SpirvReflect.h"
#endif


namespace LLGL
{


bool VKShaderBindingLayout::BuildFromSpirvModule(const void* data, std::size_t size)
{
    #ifdef LLGL_ENABLE_SPIRV_REFLECT

    /* Reflect all SPIR-V binding points */
    std::vector<SpirvReflect::SpvBindingPoint> bindingPoints;
    auto result = SpirvReflectBindingPoints(SpirvModuleView{ data, size }, bindingPoints);
    if (result != SpirvResult::Success)
        return false;

    /* Convert binding points into to module bindings */
    bindings_.resize(bindingPoints.size());

    auto ConvertBindingPoint = [](ModuleBinding& dst, const SpirvReflect::SpvBindingPoint& src)
    {
        dst.srcDescriptorSet    = src.set;
        dst.srcBinding          = src.binding;
        dst.dstDescriptorSet    = src.set;
        dst.dstBinding          = src.binding;
        dst.spirvDescriptorSet  = src.setWordOffset;
        dst.spirvBinding        = src.bindingWordOffset;
    };

    for_range(i, bindingPoints.size())
        ConvertBindingPoint(bindings_[i], bindingPoints[i]);

    /* Sort module bindings by descriptor set and binding points */
    std::sort(
        bindings_.begin(), bindings_.end(),
        [](const ModuleBinding& lhs, const ModuleBinding& rhs) -> bool
        {
            if (lhs.srcDescriptorSet < rhs.srcDescriptorSet) { return true;  }
            if (lhs.srcDescriptorSet > rhs.srcDescriptorSet) { return false; }
            return lhs.dstBinding < rhs.dstBinding;
        }
    );

    return true;

    #else

    /* Cannot build binding layout from SPIR-V module without capability of SPIR-V reflection */
    return false;

    #endif
}

//private
bool VKShaderBindingLayout::AssignBindingSlot(
    ModuleBinding&      binding,
    const BindingSlot&  slot,
    std::uint32_t       dstSet,
    std::uint32_t*      dstBinding)
{
    bool modified = false;

    if (binding.dstDescriptorSet != dstSet)
    {
        binding.dstDescriptorSet = dstSet;
        modified = true;
    }

    if (dstBinding != nullptr && binding.dstBinding != *dstBinding)
    {
        binding.dstBinding = *dstBinding++;
        modified = true;
    }

    return modified;
}

std::uint32_t VKShaderBindingLayout::AssignBindingSlots(
    ConstFieldRangeIterator<BindingSlot>    iter,
    std::uint32_t                           dstSet,
    bool                                    dstBindingInAscendingOrder)
{
    std::uint32_t numBindings = 0, dstBinding = 0;

    while (auto slot = iter.Next())
    {
        auto* binding = FindInSortedArray<ModuleBinding>(
            bindings_.data(), bindings_.size(),
            [slot](const ModuleBinding& entry) -> int
            {
                LLGL_COMPARE_SEPARATE_MEMBERS_SWO(slot->set  , entry.srcDescriptorSet);
                LLGL_COMPARE_SEPARATE_MEMBERS_SWO(slot->index, entry.srcBinding      );
                return 0;
            }
        );
        if (binding != nullptr)
        {
            if (AssignBindingSlot(*binding, *slot, dstSet, (dstBindingInAscendingOrder ? &dstBinding : nullptr)))
                ++numBindings;
        }
    }

    return numBindings;
}

void VKShaderBindingLayout::UpdateSpirvModule(void* data, std::size_t size)
{
    auto* words = reinterpret_cast<std::uint32_t*>(data);
    const auto numWords = size/4;

    for (const auto& binding : bindings_)
    {
        if (binding.spirvDescriptorSet < numWords)
            words[binding.spirvDescriptorSet] = binding.dstDescriptorSet;
        if (binding.spirvBinding < numWords)
            words[binding.spirvBinding] = binding.dstBinding;
    }
}


} // /namespace LLGL



// ================================================================================
