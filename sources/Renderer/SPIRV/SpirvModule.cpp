/*
 * SpirvModule.cpp
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "SpirvModule.h"


namespace LLGL
{


static SpirvResult ReadSpirvHeader(const std::uint32_t* words, std::size_t wordCount, SpirvHeader& outHeader)
{
    if (wordCount < sizeof(SpirvHeader)/sizeof(std::uint32_t))
        return SpirvResult::InvalidModule;

    auto* header = reinterpret_cast<const SpirvHeader*>(words);
    if (header->spirvMagic != spv::MagicNumber)
        return SpirvResult::InvalidHeader;

    outHeader = *header;
    return SpirvResult::Success;
}


/*
 * SpirvModule class
 */

SpirvModule::SpirvModule(std::vector<value_type>&& data) :
    words_ { std::forward<std::vector<value_type>&&>(data) }
{
}

SpirvModule::SpirvModule(const void* data, size_type size) :
    words_ { reinterpret_cast<const value_type*>(data), reinterpret_cast<const value_type*>(data) + size/sizeof(value_type) }
{
}

SpirvModule::SpirvModule(const ArrayView<value_type>& words) :
    words_ { words.begin(), words.end() }
{
}

SpirvResult SpirvModule::ReadHeader(SpirvHeader& outHeader) const
{
    return ReadSpirvHeader(Words().data(), Words().size(), outHeader);
}

std::uint32_t SpirvModule::WordOffset(const const_iterator& iter) const
{
    return std::distance(Words().data(), iter.Ptr());
}


/*
 * SpirvModuleView class
 */

SpirvModuleView::SpirvModuleView(const ArrayView<value_type>& words) :
    words_ { words }
{
}

SpirvModuleView::SpirvModuleView(const void* data, std::size_t size) :
    words_ { reinterpret_cast<const value_type*>(data), size/sizeof(value_type) }
{
}

SpirvResult SpirvModuleView::ReadHeader(SpirvHeader& outHeader) const
{
    return ReadSpirvHeader(Words().data(), Words().size(), outHeader);
}

std::uint32_t SpirvModuleView::WordOffset(const const_iterator& iter) const
{
    return std::distance(Words().data(), iter.Ptr());
}


} // /namespace LLGL



// ================================================================================
