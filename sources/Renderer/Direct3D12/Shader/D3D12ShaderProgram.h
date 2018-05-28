/*
 * D3D12ShaderProgram.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_D3D12_SHADER_PROGRAM_H
#define LLGL_D3D12_SHADER_PROGRAM_H


#include <LLGL/ShaderProgram.h>
#include "../../DXCommon/ComPtr.h"
#include <vector>
#include <d3d12.h>


namespace LLGL
{


class D3D12Shader;

class D3D12ShaderProgram : public ShaderProgram
{

    public:

        D3D12ShaderProgram();

        void AttachShader(Shader& shader) override;
        void DetachAll() override;

        bool LinkShaders() override;

        std::string QueryInfoLog() override;

        ShaderReflectionDescriptor QueryReflectionDesc() const override;

        void BuildInputLayout(std::uint32_t numVertexFormats, const VertexFormat* vertexFormats) override;
        void BindConstantBuffer(const std::string& name, std::uint32_t bindingIndex) override;
        void BindStorageBuffer(const std::string& name, std::uint32_t bindingIndex) override;

        ShaderUniform* LockShaderUniform() override;
        void UnlockShaderUniform() override;

        /* ----- Extended internal functions ----- */

        D3D12_INPUT_LAYOUT_DESC GetInputLayoutDesc() const;

        inline D3D12Shader* GetVS() const { return vs_; }
        inline D3D12Shader* GetPS() const { return ps_; }
        inline D3D12Shader* GetHS() const { return hs_; }
        inline D3D12Shader* GetDS() const { return ds_; }
        inline D3D12Shader* GetGS() const { return gs_; }
        inline D3D12Shader* GetCS() const { return cs_; }

        inline UINT GetNumSRV() const
        {
            return numSRV_;
        }

        inline UINT GetNumCBV() const
        {
            return numCBV_;
        }

        inline UINT GetNumUAV() const
        {
            return numUAV_;
        }

    private:

        std::vector<D3D12_INPUT_ELEMENT_DESC>   inputElements_;

        union
        {
            struct
            {
                D3D12Shader*                    vs_;
                D3D12Shader*                    hs_;
                D3D12Shader*                    ds_;
                D3D12Shader*                    gs_;
                D3D12Shader*                    ps_;
                D3D12Shader*                    cs_;
            };
            D3D12Shader*                        shaders_[6] = {};
        };

        LinkError                               linkError_  = LinkError::NoError;

        UINT                                    numSRV_     = 0;//TODO: use "TextureViewDescriptor" or the like
        UINT                                    numCBV_     = 0;
        UINT                                    numUAV_     = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
