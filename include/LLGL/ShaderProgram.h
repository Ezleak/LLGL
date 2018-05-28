/*
 * ShaderProgram.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_SHADER_PROGRAM_H
#define LLGL_SHADER_PROGRAM_H


#include "RenderSystemChild.h"
#include "Shader.h"
#include "VertexFormat.h"
#include "StreamOutputFormat.h"
#include "BufferFlags.h"
#include "ShaderUniform.h"
#include <string>
#include <vector>


namespace LLGL
{


/**
\brief Shader program interface.
\remarks A shader program combines multiple instances of the Shader class to be used in a complete shader pipeline.
\see RenderSystem::CreateShaderProgram
*/
class LLGL_EXPORT ShaderProgram : public RenderSystemChild
{

    public:

        /**
        \brief Attaches the specified shader to this shader program.
        \param[in] shader Specifies the shader which is to be attached to this shader program.
        Each shader type can only be added once for each shader program.
        \remarks This must be called, before "LinkShaders" is called.
        \throws std::invalid_argument If a shader is attached to this shader program, which is not allowed in the current state.
        This will happend if a different shader of the same type has already been attached to this shader program for instance.
        \see Shader::GetType
        */
        virtual void AttachShader(Shader& shader) = 0;

        /**
        \brief Detaches all shaders from this shader program.
        \remarks After this call, the link status will be invalid, and the shader program must be linked again.
        \see LinkShaders
        */
        virtual void DetachAll() = 0;

        /**
        \brief Links all attached shaders to the final shader program.
        \return True on success, otherwise "QueryInfoLog" can be used to query the reason for failure.
        \remarks Each attached shader must be compiled first!
        \see QueryInfoLog
        */
        virtual bool LinkShaders() = 0;

        //! Returns the information log after the shader linkage.
        virtual std::string QueryInfoLog() = 0;

        /**
        \brief Returns a descriptor of the shader pipeline layout with all required shader resources.
        \remarks The list of resource views in the output descriptor (i.e. 'resourceViews' attribute) is always sorted in the following manner:
        First sorting criterion is the resource type (in descending order), second sorting criterion is the binding slot (in descending order).
        Here is an example of such a sorted list (pseudocode):
        \code{.txt}
        resourceViews[0] = { type: ResourceType::ConstantBuffer, slot: 0 }
        resourceViews[1] = { type: ResourceType::ConstantBuffer, slot: 2 }
        resourceViews[2] = { type: ResourceType::Texture, slot: 0 }
        resourceViews[3] = { type: ResourceType::Texture, slot: 1 }
        resourceViews[4] = { type: ResourceType::Texture, slot: 2 }
        resourceViews[5] = { type: ResourceType::Sampler, slot: 2 }
        \endcode
        \see ShaderReflectionDescriptor::resourceViews
        \throws std::runtime_error If shader reflection failed.
        */
        virtual ShaderReflectionDescriptor QueryReflectionDesc() const = 0;

        /**
        \brief Builds the input layout with the specified vertex format for this shader program.
        \param[in] numVertexFormats Specifies the number of entries in the vertex format array. If this is zero, the function call has no effect.
        \param[in] vertexFormats Specifies the input vertex formats. This must be a pointer to an array of vertex formats with at least as much entries as specified by 'numVertexFormats'.
        If this is null, the function call has no effect.
        \remarks Can only be used for a shader program which has a successfully compiled vertex shader attached.
        If this is called after the shader program has been linked, the shader program might be re-linked again.
        \see AttachShader(VertexShader&)
        \see Shader::Compile
        \see LinkShaders
        \throws std::invalid_argument If the name of an vertex attribute is invalid or the maximal number of available vertex attributes is exceeded.
        */
        virtual void BuildInputLayout(std::uint32_t numVertexFormats, const VertexFormat* vertexFormats) = 0;

        //TODO: add FragmentFormat structure to provide CPU side fragment binding for OpenGL
        #if 0
        virtual void BuildOutputLayout(const FragmentFormat& fragmentFormat) = 0;
        #endif

        /**
        \brief Binds the specified constant buffer to this shader.
        \param[in] name Specifies the name of the constant buffer within this shader.
        \param[in] bindingIndex Specifies the binding index. This index must match the index which will be used for "RenderContext::BindConstantBuffer".
        \remarks This function is only necessary if the binding index does not match the default binding index of the constant buffer within the shader.
        \see QueryConstantBuffers
        \see RenderContext::BindConstantBuffer
        */
        virtual void BindConstantBuffer(const std::string& name, std::uint32_t bindingIndex) = 0;

        /**
        \brief Binds the specified storage buffer to this shader.
        \param[in] name Specifies the name of the storage buffer within this shader.
        \param[in] bindingIndex Specifies the binding index. This index must match the index which will be used for "RenderContext::BindStorageBuffer".
        \remarks This function is only necessary if the binding index does not match the default binding index of the storage buffer within the shader.
        \see RenderContext::BindStorageBuffer
        */
        virtual void BindStorageBuffer(const std::string& name, std::uint32_t bindingIndex) = 0;

        /**
        \brief Locks the shader uniform handler.
        \return Pointer to the shader uniform handler or null if the render system does not support individual shader uniforms.
        \remarks This must be called to set individual shader uniforms.
        \code
        if (auto myUniformHandler = myShaderProgram->LockShaderUniform()) {
            myUniformHandler->SetUniform1i("mySampler1", 0);
            myUniformHandler->SetUniform1i("mySampler2", 1);
            myUniformHandler->SetUniform4x4fv("projection", &myProjectionMatrix[0]);
            myShaderProgram->UnlockShaderUniform();
        }
        \endcode
        \note Only supported with: OpenGL.
        \see UnlockShaderUniform
        */
        virtual ShaderUniform* LockShaderUniform() = 0;

        /**
        \brief Unlocks the shader uniform handler.
        \see LockShaderUniform
        */
        virtual void UnlockShaderUniform() = 0;

        #ifdef LLGL_ENABLE_BACKWARDS_COMPATIBILITY

        [[deprecated("use extended version of 'LLGL::ShaderProgram::BuildInputLayout' instead")]]
        void BuildInputLayout(const VertexFormat& vertexFormat);

        #endif // /LLGL_ENABLE_BACKWARDS_COMPATIBILITY

    protected:

        //! Linker error codes for internal error checking.
        enum class LinkError
        {
            NoError,
            InvalidComposition,
            InvalidByteCode,
            TooManyAttachments,
            IncompleteAttachments,
        };

        /**
        \brief Validates the composition of the specified shader attachments.
        \param[in] shaders Array of Shader objects that belong to this shader program. Null pointers within the array are ignored.
        \param[in] numShaders Specifies the number of entries in the array 'shaders'. This must not be larger than the number of entries in the 'shaders' array.
        \return True if the shader composition is valid, otherwise false.
        \remarks For example, a composition of a compute shader and a fragment shader is invalid,
        but a composition of a vertex shader and a fragment shader is valid.
        */
        /*static*/ bool ValidateShaderComposition(Shader* const * shaders, std::size_t numShaders) const;

        /**
        \brief Sorts the resource views of the specified shader reflection descriptor as described in the QueryReflectionDesc function.
        \see QueryReflectionDesc
        */
        static void FinalizeShaderReflection(ShaderReflectionDescriptor& reflectionDesc);

        //! Returns a string representation for the specified shader linker error, or null if the no error is entered (i.e. LinkError::NoError).
        static const char* LinkErrorToString(const LinkError errorCode);

};


} // /namespace LLGL


#endif



// ================================================================================
